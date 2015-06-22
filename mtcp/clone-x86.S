#include <sys/linux-syscalls.h>

#define FN    12
#define TLS   16
#define FLAGS 20
#define ARG   24

.text

/*
 * int  __pthread_clone(int (*fn)(void*), void *tls, int flags,
 *                      void *arg);
 */
.globl __pthread_clone_r
.type __pthread_clone_r, @function
.align 4
__pthread_clone_r:
        pushl   %ebx
        pushl   %ecx
        movl    TLS(%esp), %ecx
        movl    FLAGS(%esp), %ebx

        # 16-byte alignment for the child stack
        andl    $0xfffffff0, %ecx
        subl    $16, %ecx

        # insert arguments onto the child stack
        movl    FN(%esp), %eax
        movl    %eax, (%ecx)
        movl    ARG(%esp), %eax
        movl    %eax, 4(%ecx)
        movl    TLS(%esp), %eax
        movl    %eax, 8(%ecx)

        movl    $__NR_clone, %eax
        int     $0x80
        test    %eax, %eax
        jns     1f

        # an error occured, set errno and return -1
        negl    %eax
        call    __set_errno
        orl     $-1, %eax
        jmp     2f

1:
        jnz     2f

        # we're in the child thread now, terminate the stack frame and call
        # __thread_entry
        movl    $0, %ebp
        call    __thread_entry_r
        hlt

2:
        popl    %ecx
        popl    %ebx
        ret

/* XXX: TODO: Add __bionic_clone here
 *            See bionic/bionic_clone.c and arch-arm/bionic/clone.S
 *            for more details...
 */
