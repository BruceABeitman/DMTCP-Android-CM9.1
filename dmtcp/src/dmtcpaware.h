/*************************************************************************
 * TODO: Replace this header with appropriate header showing MIT OR BSD  *
 *       License                                                         *
 * This file, dmtcpaware.h, is placed in the public domain.              *
 * The motivation for this is to allow anybody to freely use this file   *
 * without restriction to statically link this file with any software.   *
 * This allows that software to communicate with the DMTCP libraries.    *
 * -  Jason Ansel, Kapil Arya, and Gene Cooperman                        *
 *      jansel@csail.mit.edu, kapil@ccs.neu.edu, gene@ccs.neu.edu        *
 *************************************************************************/

#ifndef DMTCPAWARE_H
#define DMTCPAWARE_H

#ifdef __cplusplus
extern "C" {
#endif

/// Return value of dmtcpCheckpoint
#define DMTCP_AFTER_CHECKPOINT 1
/// Return value of dmtcpCheckpoint
#define DMTCP_AFTER_RESTART    2

/// Returned when DMTCP is disabled, unless stated otherwise
#define DMTCP_ERROR_DISABLED -128

/// Pointer to a "void foo();" function
typedef void (*DmtcpFunctionPointer)(void);

typedef void (*DmtcpPreSuspendUserThreadFunctionPointer)(void);
typedef void (*DmtcpPreResumeUserThreadFunctionPointer)(int is_ckpt,
                                                        int is_restart);


/// Returned by dmtcpGetCoordinatorStatus()
typedef struct _DmtcpCoordinatorStatus {

  /// Number of processes connected to dmtcp_coordinator
  int numProcesses;

  /// 1 if all processes connected to dmtcp_coordinator are in a running state
  int isRunning;

} DmtcpCoordinatorStatus;

/// Returned by dmtcpGetLocalStatus()
typedef struct _DmtcpLocalStatus {

  /// The number of times this process has been checkpointed (excludes restarts)
  int numCheckpoints;

  /// The number of times this process has been restarted
  int numRestarts;

  /// Filename of .dmtcp checkpoint file for this process
  const char* checkpointFilename;

  /// The DMTCP cluster-wide unique process identifier for this process.
  /// Format is "HostHash-PID-Timestamp"
  const char* uniquePidStr;

} DmtcpLocalStatus;

  /**
   * If synchronization log and replay is enabled, adds a generic "user" event
   * to the log. Returns 1 on success, 0 on error.
   */
int dmtcp_userSynchronizedEvent();
int dmtcp_userSynchronizedEventBegin();
int dmtcp_userSynchronizedEventEnd();
/**
 * Returns 1 if executing under dmtcp_checkpoint, 0 otherwise
 */
int dmtcpIsEnabled();

/**
 * Checkpoint the entire distributed computation, block until checkpoint is
 * complete.
 * - returns DMTCP_AFTER_CHECKPOINT if the checkpoint succeeded.
 * - returns DMTCP_AFTER_RESTART    after a restart.
 * - returns <=0 on error.
 */
int dmtcpCheckpoint();

/**
 * Prevent a checkpoint from starting until dmtcpDelayCheckpointsUnlock() is
 * called.
 * - Has (recursive) lock semantics, only one thread may acquire it at time.
 * - Only prevents checkpoints locally, remote processes may be suspended.
 *   Thus, send or recv to another checkpointed process may create deadlock.
 * - Returns 1 on success, <=0 on error
 */
int dmtcpDelayCheckpointsLock();

/**
 * Re-allow checkpoints, opposite of dmtcpDelayCheckpointsLock().
 * - Returns 1 on success, <=0 on error
 */
int dmtcpDelayCheckpointsUnlock();

/**
 * Sets the hook functions that DMTCP calls when it checkpoints/restarts.
 * - These functions are called from the DMTCP thread while all user threads
 *   are suspended.
 * - First preCheckpoint() is called, then either postCheckpoint() or
 *   postRestart() is called.
 * - Set to NULL to disable.
 * - Returns 1 on success, <=0 on error
 */
int dmtcpInstallHooks( DmtcpFunctionPointer earlyCheckpoint
                      , DmtcpFunctionPointer preCheckpoint
                      , DmtcpFunctionPointer postCheckpoint
                      , DmtcpFunctionPointer postRestart);

int dmtcpInstallPerThreadHooks( DmtcpPreSuspendUserThreadFunctionPointer preSuspend
                              , DmtcpPreResumeUserThreadFunctionPointer preResume);

/**
 * Gets the coordinator-specific status of DMTCP.
 * - Calling this function invalidates older DmtcpCoordinatorStatus structures.
 * - Returns NULL on error.
 */
const DmtcpCoordinatorStatus* dmtcpGetCoordinatorStatus();

/**
 * Gets the local-node-specific status of DMTCP.
 * - Calling this function invalidates older DmtcpLocalStatus structures.
 * - Returns NULL on error.
 */
const DmtcpLocalStatus* dmtcpGetLocalStatus();

/**
 * Send a command to the dmtcp_coordinator as if it were typed on the console.
 * - Returns 1 if command was sent and well-formed, <= 0 otherwise.
 */
int dmtcpRunCommand(char command);

/*
 *  Synchronize all user program.
 *  - Returns 1 on success, <=0 on error
 */
int dmtcpSynchronize();

/*
 *  Raise the barrier for dmtcpCheckpointBarrier.
 *  - Returns 1 on success, <=0 on error
 */
int dmtcpRaiseCheckpointBarrier();

/*
 *  This barrier will block the user thread until checkpoint is finish to make
 *  precise `checkpoint point`.
 *
 *  The user should guarantee the checkpoint will occur between
 *  dmtcpRaiseCheckpointBarrier and dmtcpCheckpointBarrier.
 *  Otherwise it's will block the user program forever.
 *  - Returns 1 on success, <=0 on error
 */
int dmtcpCheckpointBarrier();

int dmtcpBlockBinder();
int dmtcpUnblockBinder();
int dmtcpIsBlockBinder();

/*
 * Stop dmtcp and disconnect with dmtcp_coordinator!
 */
int dmtcpShutdown();

#ifdef __cplusplus
} //extern "C"
#endif

#endif //DMTCPAWARE_H
