/* src/config.h.  Generated from config.h.in by configure.  */
/* src/config.h.in.  Generated from configure.ac by autoheader.  */

/* Compiling in 32 bit mode on 64 bit linux. */
#define CONFIG_M32 /**/

/* Verbose debug output and log files in $DMTCP_TMPDIR */
/* #undef DEBUG */
#define DEBUG

/* Always enable this. */
#define DMTCP /**/

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/epoll.h> header file. */
#define HAVE_SYS_EPOLL_H 1

/* Define to 1 if you have the <sys/eventfd.h> header file. */
#define HAVE_SYS_EVENTFD_H 1

/* Define to 1 if you have the <sys/signalfd.h> header file. */
#define HAVE_SYS_SIGNALFD_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1


/* Socket with no peer is external; wait until communication finished */
/* #undef EXTERNAL_SOCKET_HANDLING */

/* Child process does checkpointing */
/* #undef FORKED_CHECKPOINTING */

/* Use delta compression. */
/* #undef HBICT_DELTACOMP */

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* Use a custom allocator based on mmap */
/* #undef OVERRIDE_GLOBAL_ALLOCATOR */

/* Name of package */
#define PACKAGE "dmtcp"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "DMTCP"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "DMTCP 1.2.4"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "dmtcp"

/* Define to the home page for this package. */
#define PACKAGE_URL "http://dmtcp.sourceforge.net"

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.2.4"

/* Use virtual pids for pid-related system calls. */
/* #define PID_VIRTUALIZATION */

/* Use support for ptrace system call. */
/* #undef PTRACE */

/* No output, not even NOTE and WARNING */
/* #undef QUIET */

/* Record to log and then replay. */
/* #undef RECORD_REPLAY */

/* Socket with no peer is stale (no peer exists); ignore socket */
#define STALE_SOCKET_HANDLING /**/

/* Record timing information to stderr and jtimings.csv */
#define TIMING

/* Use unique filenames for checkpoint images */
/* #undef UNIQUE_CHECKPOINT_FILENAMES */

/* Version number of package */
#define VERSION "1.2.4"
