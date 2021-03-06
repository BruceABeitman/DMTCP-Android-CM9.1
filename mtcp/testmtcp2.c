/*****************************************************************************
 *   Copyright (C) 2006-2008 by Michael Rieker, Jason Ansel, Kapil Arya, and *
 *                                                            Gene Cooperman *
 *   mrieker@nii.net, jansel@csail.mit.edu, kapil@ccs.neu.edu, and           *
 *                                                          gene@ccs.neu.edu *
 *                                                                           *
 *   This file is part of the MTCP module of DMTCP (DMTCP:mtcp).             *
 *                                                                           *
 *  DMTCP:mtcp is free software: you can redistribute it and/or              *
 *  modify it under the terms of the GNU Lesser General Public License as    *
 *  published by the Free Software Foundation, either version 3 of the       *
 *  License, or (at your option) any later version.                          *
 *                                                                           *
 *  DMTCP:dmtcp/src is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU Lesser General Public License for more details.                      *
 *                                                                           *
 *  You should have received a copy of the GNU Lesser General Public         *
 *  License along with DMTCP:dmtcp/src.  If not, see                         *
 *  <http://www.gnu.org/licenses/>.                                          *
 *****************************************************************************/

/********************************************************************************************************************************/
/*																*/
/*  Simple multi-threaded test program												*/
/*  Checkpoint is written to testmtcp2.mtcp every 10 seconds									*/
/*  It uses the mtcp_wrapper_clone routine to create the threads								*/
/*																*/
/*  Four threads are created													*/
/*  First thread prints 1 11 21 31 41 51 61 ...											*/
/*  Second thread prints 2 12 22 32 42 52 62 ...										*/
/*  Third thread prints 3 13 23 33 43 53 63 ...											*/
/*  Fourth thread prints 4 14 24 34 44 54 64 ...										*/
/*  When checkpoint is restored, the counts should resume where they left off							*/
/*																*/
/********************************************************************************************************************************/

#define u32 unsigned int

#define _GNU_SOURCE
#include <asm/unistd.h>
#include <errno.h>
#include <sched.h>

#ifndef __user
// this is needed to compile futex.h on LXR/Suse10.2
#  define __user
#endif

#include <linux/futex.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mtcp_internal.h" // for atomic_setif_int(), and STACKSIZE
#include "mtcp.h"
#include "mtcp_futex.h"

/*atomic_setif_int():                          */
/* Set *loc to newval iff *loc equal to oldval */
/* Return 0 if failed, 1 if succeeded          */


#define THREADFLAGS (CLONE_FS | CLONE_FILES | CLONE_VM)

static int printfutex = 0;

static void lockstdout (void)

{
  int rc;

  while (!atomic_setif_int (&printfutex, 1, 0)) {
    rc = mtcp_futex (&printfutex, FUTEX_WAIT, 0, NULL);
    if ((rc < 0) && (rc != -EAGAIN) && (rc != -EWOULDBLOCK) && (rc != -EINTR)) {
      fprintf (stderr, "testmtcp2: FUTEX_WAIT error %d\n", rc);
      abort ();
    }
  }
}

static void unlkstdout (void)

{
  int rc;

  printfutex = 0;
  rc = mtcp_futex (&printfutex, FUTEX_WAKE, 1, NULL);
  if (rc < 0) {
    fprintf (stderr, "testmtcp2: FUTEX_WAKE error %d\n", rc);
    abort ();
  }
}

static int thread1_func (void *dummy);

int main ()

{
  char *thread1_stack;
  int i, thread1_tid;

  mtcp_init ("testmtcp2.mtcp", 10, 0);

  for (i = 0; i < 3; i ++) {
    thread1_stack = malloc (STACKSIZE);
    thread1_tid = clone (thread1_func, thread1_stack + STACKSIZE, THREADFLAGS, (void *)(long)(i + 2));
    if (thread1_tid < 0) {
      fprintf (stderr, "error creating thread1: %s\n", strerror (errno));
      return (-1);
    }
  }

  thread1_func ((void *)1);
  return (0);
}

static int thread1_func (void *dummy)
{
  int count, delay;

  mtcp_ok ();

  count = (long)dummy;

  while (1) {
    for (delay = 100; -- delay >= 0;) usleep (10);
    lockstdout ();
    printf (" %d", count);
    fflush (stdout);
    unlkstdout ();
    count += 10;
  }
  return -1;
}
