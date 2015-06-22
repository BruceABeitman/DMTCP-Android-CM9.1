/****************************************************************************
 *   Copyright (C) 2006-2010 by Jason Ansel, Kapil Arya, and Gene Cooperman *
 *   jansel@csail.mit.edu, kapil@ccs.neu.edu, gene@ccs.neu.edu              *
 *                                                                          *
 *   This file is part of the dmtcp/src module of DMTCP (DMTCP:dmtcp/src).  *
 *                                                                          *
 *  DMTCP:dmtcp/src is free software: you can redistribute it and/or        *
 *  modify it under the terms of the GNU Lesser General Public License as   *
 *  published by the Free Software Foundation, either version 3 of the      *
 *  License, or (at your option) any later version.                         *
 *                                                                          *
 *  DMTCP:dmtcp/src is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU Lesser General Public License for more details.                     *
 *                                                                          *
 *  You should have received a copy of the GNU Lesser General Public        *
 *  License along with DMTCP:dmtcp/src.  If not, see                        *
 *  <http://www.gnu.org/licenses/>.                                         *
 ****************************************************************************/

#include "dmtcpaware.h"
#include "dmtcpcoordinatorapi.h"
#include "dmtcpworker.h"
#include "threadsync.h"
#include "dmtcpmessagetypes.h"
#include "dmtcp_coordinator.h"
#include "syscallwrappers.h"
#include "mtcpinterface.h"
#include "dmtcpalloc.h"
#include <string>
#include <unistd.h>
#include <time.h>

#ifndef EXTERNC
# define EXTERNC extern "C"
#endif

//global counters
static int numCheckpoints = 0;
static int numRestarts    = 0;

//user hook functions
typedef dmtcp::vector<DmtcpFunctionPointer> DmtcpFunctionPointers;
static DmtcpFunctionPointers userHookEarlyCheckpoints;
static DmtcpFunctionPointers userHookPreCheckpoint;
static DmtcpFunctionPointers userHookPostCheckpoint;
static DmtcpFunctionPointers userHookPostRestart;

static pthread_mutex_t checkpointBarrier = PTHREAD_MUTEX_INITIALIZER;

//I wish we could use pthreads for the trickery in this file, but much of our
//code is executed before the thread we want to wake is restored.  Thus we do
//it the bad way.
#if defined(__i386__) || defined(__x86_64__)
static inline void memfence(){  asm volatile ("mfence" ::: "memory"); }
#elif defined(__arm__)
static inline void memfence(){  asm volatile ("dmb" ::: "memory"); }
#endif

//needed for sizeof()
static const dmtcp::DmtcpMessage * const exampleMessage = NULL;

static inline void _runCoordinatorCmd(char c, int* result){
  _dmtcp_lock();
  {
    dmtcp::DmtcpCoordinatorAPI coordinatorAPI;
    coordinatorAPI.useAlternateCoordinatorFd();

    dmtcp::ThreadSync::delayCheckpointsLock();
    coordinatorAPI.connectAndSendUserCommand(c, result);
    dmtcp::ThreadSync::delayCheckpointsUnlock();
  }
  _dmtcp_unlock();
}

/*
 * ___real_dmtcpXXX: Does the real work expected by dmtcpXXX
 * __dyn_dmtcpXXX:   Dummy trampolines to support static linking of user code
 *                   to libdmtcpaware.a
 * dmtcpXXX:         Dummy trampolines to support dynamic linking of user code
 *                   to libdmtcpaware.so
 *
 * NOTE: We do not want __dyn_dmtcpXXX for call dmtcpXXX functions directly
 * because if the user binary is compiled with -rdynamic and libdmtcpaware.a,
 * this would result in a call to libdmtcpaware.a:dmtcpXXX, thus creating an
 * infinite loop.
 */

int __real_dmtcpIsEnabled() { return 1; }

int __real_dmtcpCheckpoint(){
  int rv = 0;
  int oldNumRestarts    = numRestarts;
  int oldNumCheckpoints = numCheckpoints;
  memfence(); //make sure the reads above don't get reordered

  if(dmtcpRunCommand('c')){ //request checkpoint
    //and wait for the checkpoint
    while(oldNumRestarts==numRestarts && oldNumCheckpoints==numCheckpoints){
      //nanosleep should get interrupted by checkpointing with an EINTR error
      //though there is a race to get to nanosleep() before the checkpoint
      struct timespec t = {1,0};
      nanosleep(&t, NULL);
      memfence();  //make sure the loop condition doesn't get optimized
    }
    rv = (oldNumRestarts==numRestarts ? DMTCP_AFTER_CHECKPOINT : DMTCP_AFTER_RESTART);
  }else{
  	/// TODO: Maybe we need to process it in some way????
    /// EXIT????
    /// -- Artem
    //	printf("\n\n\nError requesting checkpoint\n\n\n");
  }

  return rv;
}

int __real_dmtcpRunCommand(char command){
  int result[DMTCPMESSAGE_NUM_PARAMS];
  int i = 0;
  while (i < 100) {
    _runCoordinatorCmd(command, result);
  // if we got error result - check it
	// There is possibility that checkpoint thread
	// did not send state=RUNNING yet or Coordinator did not receive it
	// -- Artem
    if (result[0] == dmtcp::DmtcpCoordinatorAPI::ERROR_NOT_RUNNING_STATE) {
      struct timespec t;
      t.tv_sec = 0;
      t.tv_nsec = 1000000;
      nanosleep(&t, NULL);
      //printf("\nWAIT FOR CHECKPOINT ABLE\n\n");
    } else {
//      printf("\nEverything is OK - return\n");
      break;
    }
    i++;
  }
  return result[0]>=0;
}

const DmtcpCoordinatorStatus* __real_dmtcpGetCoordinatorStatus(){
  int result[DMTCPMESSAGE_NUM_PARAMS];
  _runCoordinatorCmd('s',result);

  //must be static so memory is not deleted.
  static DmtcpCoordinatorStatus status;

  status.numProcesses = result[0];
  status.isRunning = result[1];
  return &status;
}

const DmtcpLocalStatus* __real_dmtcpGetLocalStatus(){
  //these must be static so their memory is not deleted.
#ifndef ANDROID
  static dmtcp::string ckpt;
  static dmtcp::string pid;
#endif
  static DmtcpLocalStatus status;
#ifndef ANDROID
  ckpt.reserve(1024);
#endif

  //get filenames
#ifndef ANDROID
  pid=dmtcp::UniquePid::ThisProcess().toString();
  ckpt=dmtcp::UniquePid::getCkptFilename();
#endif

  status.numCheckpoints          = numCheckpoints;
  status.numRestarts             = numRestarts;
#ifndef ANDROID
  status.checkpointFilename      = ckpt.c_str();
  status.uniquePidStr            = pid.c_str();
#else
  status.checkpointFilename      = NULL;
  status.uniquePidStr            = NULL;
#endif
  return &status;
}

int __real_dmtcpInstallHooks( DmtcpFunctionPointer earlyCheckpoint
                              , DmtcpFunctionPointer preCheckpoint
                              , DmtcpFunctionPointer postCheckpoint
                              , DmtcpFunctionPointer postRestart){
  if (earlyCheckpoint)userHookEarlyCheckpoints.push_back(earlyCheckpoint);
  if (preCheckpoint)  userHookPreCheckpoint.push_back(preCheckpoint);
  if (postCheckpoint) userHookPostCheckpoint.push_back(postCheckpoint);
  if (postRestart)    userHookPostRestart.push_back(postRestart);
  return 1;
}

int __real_dmtcpDelayCheckpointsLock(){
  dmtcp::ThreadSync::delayCheckpointsLock();
  return 1;
}

int __real_dmtcpDelayCheckpointsUnlock(){
  dmtcp::ThreadSync::delayCheckpointsUnlock();
  return 1;
}

void dmtcp::userHookEarlyCheckpoint() {
  for (DmtcpFunctionPointers::iterator itr = userHookEarlyCheckpoints.begin();
       itr != userHookEarlyCheckpoints.end();
       ++itr) {
    DmtcpFunctionPointer userHooks = *itr;
    (*userHooks)();
  }
}

void dmtcp::userHookTrampoline_preCkpt() {
  for (DmtcpFunctionPointers::iterator itr = userHookPreCheckpoint.begin();
       itr != userHookPreCheckpoint.end();
       ++itr) {
    DmtcpFunctionPointer userHooks = *itr;
    (*userHooks)();
  }
}

void dmtcp::userHookTrampoline_postCkpt(bool isRestart) {
  //this function runs before other threads are resumed
  if(isRestart){
    numRestarts++;
    for (DmtcpFunctionPointers::iterator itr = userHookPostRestart.begin();
         itr != userHookPostRestart.end();
         ++itr) {
      DmtcpFunctionPointer userHooks = *itr;
      (*userHooks)();
    }
  }else{
    numCheckpoints++;
    for (DmtcpFunctionPointers::iterator itr = userHookPostCheckpoint.begin();
         itr != userHookPostCheckpoint.end();
         ++itr) {
      DmtcpFunctionPointer userHooks = *itr;
      (*userHooks)();
    }
  }

  // Unlock the checkpointBarrier !
  int pmul = pthread_mutex_unlock(&checkpointBarrier);

  JTRACE("BAB User called userHookTrampoline, with tryUNlock returning: ") (pmul);

  int pmul_dup = pthread_mutex_unlock(&checkpointBarrier);
  JTRACE("BAB User called userHookTrampoline, with DUPLICATE tryUNlock returning: ") (pmul_dup);
}

int __real_dmtcpSynchronize(){
  int result[DMTCPMESSAGE_NUM_PARAMS];
  JTRACE("send SYNCHRONIZE msg");
  dmtcp::DmtcpWorker::startSynchronize();
  _runCoordinatorCmd('x',result);

  JTRACE("start SYNCHRONIZE");
  dmtcp::DmtcpWorker::waitSynchronize();
  JTRACE("SYNCHRONIZE done");
  return 1;
}

int __real_dmtcpRaiseCheckpointBarrier(){
  /*userHookTrampoline_postCkpt
   * Just try lock, to make checkpointBarrier block.
   */
  JTRACE("User called dmtcpRaiseCheckpointBarrier");
  //int pmtl = pthread_mutex_trylock(&checkpointBarrier);
  //JTRACE("BAB User called dmtcpRaiseCheckpointBarrier, with trylock returning: ") (pmtl);
  JTRACE("BAB Commented out trylock");
  return 1;
}

int __real_dmtcpCheckpointBarrier(){
  /*
   * Block until checkpoint finish, checkpointBarrier will release in
   * dmtcp::userHookTrampoline_postCkpt.
   */
  JTRACE("User called dmtcpCheckpointBarrier");
  //JTRACE("BAB User called dmtcpCheckpointBarrier, MUTEX - checkpointBarrier: ") (checkpointBarrier);
  int pml = pthread_mutex_lock(&checkpointBarrier);
  JTRACE("BAB User called dmtcpCheckpointBarrier, with trylock returning: ") (pml);
  //JTRACE("BAB User called dmtcpCheckpointBarrier, MUTEX - checkpointBarrier: ") (checkpointBarrier);
  JTRACE("dmtcpCheckpointBarrier release");
  return 1;
}

int __real_dmtcpInstallPerThreadHooks(
      DmtcpPreSuspendUserThreadFunctionPointer preSuspend,
      DmtcpPreResumeUserThreadFunctionPointer preResume){
  if (preSuspend) dmtcp::preSuspendUserThreadFuncs.push_back(preSuspend);
  if (preResume) dmtcp::preResumeUserThreadFuncs.push_back(preResume);
  return 1;
}

int __real_dmtcpBlockBinder() {
  dmtcp::ThreadSync::setBlockBinder(true);
  return 1;
}

int __real_dmtcpUnblockBinder() {
  dmtcp::ThreadSync::setBlockBinder(false);
  return 1;
}

int __real_dmtcpIsBlockBinder() {
  return dmtcp::ThreadSync::isBlockBinder();
}

extern "C" int __dynamic_dmtcpIsEnabled(){
  return 3;
}

//These dummy trampolines support static linking of user code to libdmtcpaware.a
//See dmtcpaware.c .
EXTERNC int __dyn_dmtcpIsEnabled(){
  return __real_dmtcpIsEnabled();
}
EXTERNC int __dyn_dmtcpCheckpoint(){
  return __real_dmtcpCheckpoint();
}
EXTERNC int __dyn_dmtcpRunCommand(char command){
  return __real_dmtcpRunCommand(command);
}
EXTERNC int __dyn_dmtcpDelayCheckpointsLock(){
  return __real_dmtcpDelayCheckpointsLock();
}
EXTERNC int __dyn_dmtcpDelayCheckpointsUnlock(){
  return __real_dmtcpDelayCheckpointsUnlock();
}
EXTERNC int __dyn_dmtcpInstallHooks( DmtcpFunctionPointer earlyCheckpoint
                                    , DmtcpFunctionPointer preCheckpoint
                                    ,  DmtcpFunctionPointer postCheckpoint
                                    ,  DmtcpFunctionPointer postRestart){
  return __real_dmtcpInstallHooks(earlyCheckpoint, preCheckpoint,
                                  postCheckpoint, postRestart);
}
EXTERNC const DmtcpCoordinatorStatus* __dyn_dmtcpGetCoordinatorStatus(){
  return __real_dmtcpGetCoordinatorStatus();
}
EXTERNC const DmtcpLocalStatus* __dyn_dmtcpGetLocalStatus(){
  return __real_dmtcpGetLocalStatus();
}

EXTERNC int __dyn_dmtcpSynchronize(){
  return __real_dmtcpSynchronize();
}
EXTERNC int __dyn_dmtcpRaiseCheckpointBarrier(){
  return __real_dmtcpRaiseCheckpointBarrier();
}
EXTERNC int __dyn_dmtcpCheckpointBarrier(){
  return __real_dmtcpCheckpointBarrier();
}

EXTERNC int __dyn_dmtcpInstallPerThreadHooks(
              DmtcpPreSuspendUserThreadFunctionPointer preSuspend
            , DmtcpPreResumeUserThreadFunctionPointer preResume){
  return __real_dmtcpInstallPerThreadHooks(preSuspend, preResume);
}

EXTERNC int __dyn_dmtcpBlockBinder() {
  return __real_dmtcpBlockBinder();
}

EXTERNC int __dyn_dmtcpUnblockBinder() {
  return __real_dmtcpUnblockBinder();
}

EXTERNC int __dyn_dmtcpIsBlockBinder() {
  return __real_dmtcpIsBlockBinder();
}

//These dummy trampolines support dynamic linking of user code to libdmtcpaware.so
EXTERNC int dmtcpIsEnabled(){
  return __real_dmtcpIsEnabled();
}
EXTERNC int dmtcpCheckpoint(){
  return __real_dmtcpCheckpoint();
}
EXTERNC int dmtcpRunCommand(char command){
  return __real_dmtcpRunCommand(command);
}
EXTERNC int dmtcpDelayCheckpointsLock(){
  return __real_dmtcpDelayCheckpointsLock();
}
EXTERNC int dmtcpDelayCheckpointsUnlock(){
  return __real_dmtcpDelayCheckpointsUnlock();
}
EXTERNC int dmtcpInstallHooks( DmtcpFunctionPointer earlyCheckpoint,
                               DmtcpFunctionPointer preCheckpoint,
                               DmtcpFunctionPointer postCheckpoint,
                               DmtcpFunctionPointer postRestart){
  return __real_dmtcpInstallHooks(earlyCheckpoint, preCheckpoint,
                                  postCheckpoint, postRestart);
}
EXTERNC const DmtcpCoordinatorStatus* dmtcpGetCoordinatorStatus(){
  return __real_dmtcpGetCoordinatorStatus();
}
EXTERNC const DmtcpLocalStatus* dmtcpGetLocalStatus(){
  return __real_dmtcpGetLocalStatus();
}
EXTERNC int dmtcpSynchronize(){
  return __real_dmtcpSynchronize();
}
EXTERNC int dmtcpRaiseCheckpointBarrier(){
  return __real_dmtcpRaiseCheckpointBarrier();
}
EXTERNC int dmtcpCheckpointBarrier(){
  return __real_dmtcpCheckpointBarrier();
}

EXTERNC int dmtcpInstallPerThreadHooks(
              DmtcpPreSuspendUserThreadFunctionPointer preSuspend
            , DmtcpPreResumeUserThreadFunctionPointer preResume){
  return __real_dmtcpInstallPerThreadHooks(preSuspend, preResume);
}
