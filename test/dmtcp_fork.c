#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define BT_SIZE 1024
// #define ENABLE_BACKTRACE
#ifdef ENABLE_BACKTRACE
# include <execinfo.h>
#endif

int isChild = 0;

// See comment in autotest.py; This can trigger bug in 32-bit Linux
// because original app places vdso and libs in low memory when kernel uses
// legacy_va_layout (which is forced when stacksize is small), and the new
// vdso collides with the old libs in mtcp_restart.  So, it gets unmapped.

void myHandler(int i){
#ifdef ENABLE_BACKTRACE
  int nptrs;
  void * buffer[BT_SIZE];
#endif

  printf("(%d) signal %d received.\n", getpid(), i);
#ifdef ENABLE_BACKTRACE
  nptrs = backtrace (buffer, BT_SIZE);
  backtrace_symbols_fd ( buffer, nptrs, STDOUT_FILENO );
  printf("\n");
#endif
}

int main(int argc, char* argv[]){
  int pid;
  pid = fork();
  if (pid != 0) {
    printf("parent fork ! %d\n", getpid());
  } else {
    isChild = 1;
    printf("child fork ! %d\n", getpid());
  }

  while (1){
    sleep(2);
    if (isChild) {
      printf("child fork ! %d\n", getpid());
    } else {
      printf("parent fork ! %d\n", getpid());
    }
  }
}
