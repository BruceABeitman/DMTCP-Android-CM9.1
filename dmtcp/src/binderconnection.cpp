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

// THESE INCLUDES ARE IN RANDOM ORDER.  LET'S CLEAN IT UP AFTER RELEASE. - Gene
#include "constants.h"
#include "syscallwrappers.h"
#include "connection.h"
#include  "../jalib/jassert.h"
#include  "../jalib/jfilesystem.h"
#include  "../jalib/jconvert.h"
#include "kernelbufferdrainer.h"
#include "syscallwrappers.h"
#include "connectionrewirer.h"
#include "connectionmanager.h"
#include "dmtcpmessagetypes.h"
#include "util.h"
#include "resource_manager.h"
#include  "../jalib/jsocket.h"
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <termios.h>
#include <iostream>
#include <ios>
#include <fstream>
#include <linux/limits.h>
#include <arpa/inet.h>
#ifdef ANDROID
#include <linux/ashmem.h>
#include <linux/binder.h>
#endif
#include "threadsync.h"


////////////
///// BINDER CHECKPOINTING

void dumpmap(){
  char buf[1024];
  int map_fd = _real_open("/proc/self/maps", O_RDONLY);
  ssize_t len;
  while ((len = _real_read(map_fd, buf, 1024))!=0) {
    _real_write(STDOUT_FILENO, buf, len);
  }
  _real_close(map_fd);
}

void dmtcp::BinderConnection::preCheckpoint ( const dmtcp::vector<int>& fds,
                                              KernelBufferDrainer& drain ){
  JTRACE ("Checkpointing binder") (fds[0]);
  if (_map_addr) {
    dumpmap();
    _real_munmap(_map_addr, _map_size);
  }
  _real_close (fds[0]);
  restoreOptions ( fds );
}

void dmtcp::BinderConnection::postCheckpoint ( const dmtcp::vector<int>& fds,
                                               bool isRestart ) {
  if (isRestart)
    restoreOptions ( fds );
  // always restart binder
  JTRACE ("Restoring binder content") (fds[0]);
  if (_map_addr) {
    KernelDeviceToConnection::instance().dbgSpamFds();
    void * _new_addr = _real_mmap(_map_addr, _map_size, _map_prot,
                                  _map_flags, fds[0], 0);
    JASSERT (_new_addr == _map_addr) (_new_addr) (_map_addr)
            (_map_size) (_map_prot) (_map_flags) (JASSERT_ERRNO);
  }
/*
  if (!_is_context_mgr) {
    reconnect();
  }
*/
}

void dmtcp::BinderConnection::restore ( const dmtcp::vector<int>& fds,
                                        ConnectionRewirer* ){
}

void dmtcp::BinderConnection::restoreOptions ( const dmtcp::vector<int>& fds ) {
  JTRACE ("Restoring binder fd") (fds[0]);
  for(size_t i=0; i<fds.size(); ++i){
    int fd = fds[i];
    int tmpFd = -1;
    tmpFd= _real_open("/dev/binder", O_RDWR);

    if (_max_threads) {
      _real_ioctl(tmpFd, BINDER_SET_MAX_THREADS, &_max_threads);
    }
    if (_timeout) {
      _real_ioctl(tmpFd, BINDER_SET_IDLE_TIMEOUT, &_timeout);
    }
    if (_idle_priority) {
      _real_ioctl(tmpFd, BINDER_SET_IDLE_PRIORITY, &_idle_priority);
    }
    if (_is_context_mgr) {
      _real_ioctl(tmpFd, BINDER_SET_CONTEXT_MGR, 0);
    }

    JWARNING ( _real_dup2 ( tmpFd, fd ) == fd ) ( tmpFd ) ( fd ) ( JASSERT_ERRNO );
    if (tmpFd != fd)
      _real_close (tmpFd);
  }
  KernelDeviceToConnection::instance().dbgSpamFds();
}

void dmtcp::BinderConnection::serializeSubClass ( jalib::JBinarySerializer& o ) {
  JSERIALIZE_ASSERT_POINT ( "dmtcp::BinderConnection" );
  o & _max_threads & _timeout & _idle_priority;
  o & _map_size & _map_addr & _map_prot & _map_flags;
  o & _is_context_mgr;
}

void dmtcp::BinderConnection::mergeWith ( const Connection& that ) {
}

void dmtcp::BinderConnection::restartDup2(int oldFd, int newFd) {
  restore(dmtcp::vector<int>(1,newFd), NULL);
}

static void blockBinder() {
  while (dmtcp::ThreadSync::isBlockBinder()) {
    sleep(1);
  }
}

int dmtcp::BinderConnection::ioctl(int fd, int request, va_list args) {
  va_list local_ap;
  va_copy(local_ap, args);
  if (request == BINDER_SET_MAX_THREADS) {
    size_t *max = va_arg(local_ap, size_t*);
    _max_threads = *max;
    JTRACE ("set binder max threads") ( id() ) ( _max_threads );
  } else if (request == BINDER_SET_IDLE_TIMEOUT){
    int64_t *timeout = va_arg(local_ap, int64_t*);
    _timeout = *timeout;
    JTRACE ("set binder time out") ( id() ) ( _timeout );
  } else if (request == BINDER_SET_IDLE_PRIORITY){
    int64_t *priority = va_arg(local_ap, int64_t*);
    _idle_priority = *priority;
    JTRACE ("set binder idle priority") ( id() ) ( _idle_priority );
  } else if (request == BINDER_SET_CONTEXT_MGR) {
    _is_context_mgr = true;
    JTRACE ("set binder context mgr") ( id() ) ( _is_context_mgr );
  } else if (request == BINDER_THREAD_EXIT) {
    JTRACE ("binder thread exit") ( id() );
  } else if ((unsigned)request == BINDER_VERSION) {
    JTRACE ("get binder version") ( id() );
  } else if ((unsigned)request == BINDER_WRITE_READ) {
    binder_write_read *bwr = va_arg(local_ap, binder_write_read*);
    JTRACE ("BAB binder read/write request: ") ( request );
    JTRACE ("binder read/write") ( id() )
           (bwr->write_size)
           (bwr->write_consumed)
           (bwr->write_buffer)
           (bwr->read_size)
           (bwr->read_consumed)
           (bwr->read_buffer);
    #if 0
    if (bwr->write_size > 0) {
      writeHandler(bwr);
    }
    if (bwr->read_size > 0) {
      /* Binder read in w/r sholde be done ioctl first */
      int ret = readHandler(fd, bwr);
      va_end(local_ap);
      return ret;
    }
    #endif
  } else {
    JTRACE ("Unhandle ioctl for binder!") ( id() ) ( request );
  }
  va_end(local_ap);
  return Connection::ioctl(fd, request, args);
}

void dmtcp::BinderConnection::mmap(void *addr, size_t len, int prot,
                                   int flags, int fd, off_t off) {
  _map_addr = addr;
  _map_size = len;
  _map_prot = prot;
  _map_flags = flags;
  JTRACE("mmap for binder") ( _map_addr ) ( _map_size )
                            ( _map_prot ) ( _map_flags );
}

void dmtcp::BinderConnection::mmap64(void *addr, size_t len, int prot,
                                     int flags, int fd, off64_t off) {
  JASSERT(false).Text("Unhandl mmap64 for binder");
}

void dmtcp::BinderConnection::munmap(void *addr, size_t len) {
  _map_addr = NULL;
  _map_size = 0;
  _map_prot = 0;
  _map_flags = 0;
}

typedef void *VoidPtr;

template<class T>
static inline
T getAndAdvance(VoidPtr &ptr){
  T ret = *(T*)ptr;
  ptr = (VoidPtr)((char*)ptr + sizeof(T));
  return ret;
}

void dmtcp::BinderConnection::writeHandler(struct binder_write_read *bwr) {
  uint32_t cmd;
  void *ptr = (void*)((char*)bwr->write_buffer + bwr->write_consumed);
  void *end = (void*)((char*)bwr->write_buffer + bwr->write_size);
  while (ptr < end) {
    cmd = getAndAdvance<uint32_t>(ptr);
    switch (cmd) {
      case BC_INCREFS:
      case BC_ACQUIRE:
      case BC_RELEASE:
      case BC_DECREFS:
      {
        uint32_t target = getAndAdvance<uint32_t>(ptr);
        JTRACE("Handle for BC_INCREFS | BC_ACQUIRE | BC_RELEASE | BC_DECREFS");
        break;
      }
      case BC_INCREFS_DONE:
      case BC_ACQUIRE_DONE:
      {
        void *node_ptr = getAndAdvance<void *>(ptr);
        void *cookie = getAndAdvance<void *>(ptr);
        JTRACE("Handle for BC_INCREFS_DONE | BC_ACQUIRE_DONE");
        break;
      }
      case BC_ATTEMPT_ACQUIRE:
      case BC_ACQUIRE_RESULT:
      {
        /* Not yet implement in driver! */
        JTRACE("Handle for BC_ATTEMPT_ACQUIRE | BC_ACQUIRE_RESULT");
        break;
      }
      case BC_FREE_BUFFER:
      {
        void *data_ptr = getAndAdvance<void *>(ptr);
        JTRACE("Handle for BC_FREE_BUFFER");
        break;
      }
      case BC_TRANSACTION:
      case BC_REPLY:
      {
        struct binder_transaction_data tr =
          getAndAdvance<binder_transaction_data>(ptr);
        transactionHandler(&tr, cmd == BC_REPLY);
        JTRACE("Handle for BC_TRANSACTION | BC_REPLY");
        break;
      }
      case BC_REGISTER_LOOPER:
      {
        uint32_t target = getAndAdvance<uint32_t>(ptr);
        void *cookie = getAndAdvance<void*>(ptr);

        JTRACE("Handle for BC_REGISTER_LOOPER");
        break;
      }
      case BC_ENTER_LOOPER:
      {
        JTRACE("Handle for BC_ENTER_LOOPER");
        break;
      }
      case BC_EXIT_LOOPER:
      {
        JTRACE("Handle for BC_EXIT_LOOPER");
        break;
      }
      case BC_REQUEST_DEATH_NOTIFICATION:
      case BC_CLEAR_DEATH_NOTIFICATION:
      {
        uint32_t target = getAndAdvance<uint32_t>(ptr);
        void *cookie = getAndAdvance<void*>(ptr);

        JTRACE("Handle for BC_REQUEST_DEATH_NOTIFICATION"
               " | BC_CLEAR_DEATH_NOTIFICATION");
        break;
      }
      case BC_DEAD_BINDER_DONE:
      {
        void *cookie = getAndAdvance<void*>(ptr);
        JTRACE("Handle for BC_DEAD_BINDER_DONE");
        break;
      }
      default:
        JASSERT(false).Text("Unhandled Binder cmd!!");
    }
  }
}

int dmtcp::BinderConnection::readHandler(int fd,
                                         struct binder_write_read *bwr) {
  struct binder_write_read old_bwr;
  memcpy(&old_bwr, bwr, sizeof(struct binder_write_read));
  int ret = _real_ioctl(fd, BINDER_WRITE_READ, bwr);
  if (ret < 0) return ret;
  JTRACE("readHandler");
  uint32_t cmd;
  void *ptr = (void*)((char*)old_bwr.read_buffer + old_bwr.read_consumed);
  void *end = (void*)((char*)old_bwr.read_buffer + old_bwr.read_size);
  struct binder_transaction_data tr;
  int32_t err;
  while (ptr < end) {
    struct binder_work *w;
    struct binder_transaction *t = NULL;
    cmd = getAndAdvance<uint32_t>(ptr);
    switch(cmd){
      case BR_TRANSACTION_COMPLETE:
        {
          JTRACE("Handle for BR_TRANSACTION_COMPLETE");
          return ret;
        }
        break;
      case BR_DEAD_REPLY:
        {
          JTRACE("Handle for BR_DEAD_REPLY");
          return ret;
        }
        break;
      case BR_FAILED_REPLY:
        {
          JTRACE("Handle for BR_FAILED_REPLY");
          return ret;
        }
        break;
      case BR_ACQUIRE_RESULT:
        {
          JTRACE("Handle for BR_ACQUIRE_RESULT");
        }
        break;
      case BR_REPLY:
        {
          JTRACE("Handle for BR_REPLY") (ptr);
          tr = getAndAdvance<struct binder_transaction_data>(ptr);
          JTRACE("post Handle for BR_REPLY") (ptr);
          JTRACE ("Transaction") (tr.target.handle)
                                 (tr.cookie)
                                 (tr.code)
                                 (tr.flags)
                                 (tr.sender_pid)
                                 (tr.sender_euid)
                                 (tr.data_size)
                                 (tr.offsets_size)
                                 (tr.data.ptr.buffer);
          return ret;
        }
        break;
      case BR_ERROR:
        {
          err = getAndAdvance<uint32_t>(ptr);
          JTRACE("Handle for BR_ERROR");
        }
        break;
      case BR_OK:
        {
          JTRACE("Handle for BR_OK");
        }
        break;
      case BR_ACQUIRE:
      case BR_RELEASE:
      case BR_INCREFS:
      case BR_DECREFS:
      case BR_ATTEMPT_ACQUIRE:
        {
          int32_t ref = getAndAdvance<uint32_t>(ptr);
          int32_t obj = getAndAdvance<uint32_t>(ptr);
          JTRACE("Handle for BR_ACQUIRE | BR_RELEASE | BR_INCREFS | "
                            "BR_DECREFS | BR_ATTEMPT_ACQUIRE");
        }
        break;
      case BR_TRANSACTION:
        {
          JTRACE("Handle for BR_TRANSACTION");
          tr = getAndAdvance<struct binder_transaction_data>(ptr);
        }
        break;
      case BR_DEAD_BINDER:
      case BR_CLEAR_DEATH_NOTIFICATION_DONE:
        {
          int32_t proxy= getAndAdvance<uint32_t>(ptr);
          JTRACE("Handle for BR_DEAD_BINDER | BR_CLEAR_DEATH_NOTIFICATION_DONE");
        }
        break;
      case BR_FINISHED:
      case BR_NOOP:
      case BR_SPAWN_LOOPER:
        {
          JTRACE("Handle for BR_FINISHED | BR_NOOP | BR_SPAWN_LOOPER");
        }
        break;
      default:
        JTRACE("Unhandled cmd in read Handler") (cmd);
    }
  }
  return ret;
}

void dmtcp::BinderConnection::transactionHandler(
                                struct binder_transaction_data *tr,
                                bool reply) {
  size_t *offp, *off_end;
  void *buffer_data = (void*)tr->data.ptr.buffer;
  offp = (size_t*)((char*)buffer_data + tr->data_size);
  off_end = (size_t*)((char*)offp + tr->offsets_size);
  JTRACE ("Transaction") (tr->target.handle)
                         (tr->cookie)
                         (tr->code)
                         (tr->flags)
                         (tr->sender_pid)
                         (tr->sender_euid)
                         (tr->data_size)
                         (tr->offsets_size)
                         (tr->data.ptr.buffer)
                         (offp) (off_end);
  for (; offp < off_end; offp++) {
    struct flat_binder_object *fp;
    fp = (struct flat_binder_object *)((char*)buffer_data + *offp);
    switch (fp->type) {
      case BINDER_TYPE_BINDER:
      case BINDER_TYPE_WEAK_BINDER:
        {
          JTRACE("Handle for BINDER_TYPE_BINDER | BINDER_TYPE_WEAK_BINDER");
          break;
        }
      case BINDER_TYPE_HANDLE:
      case BINDER_TYPE_WEAK_HANDLE:
        {
          JTRACE("Handle for BINDER_TYPE_HANDLE | BINDER_TYPE_WEAK_HANDLE");
          break;
        }
      case BINDER_TYPE_FD:
        {
          JTRACE("Handle for BINDER_TYPE_FD");
          break;
        }
      default:
        JASSERT(false).Text("Unhandled transaction for binder!");
    }
  }
}
