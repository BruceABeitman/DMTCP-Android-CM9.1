#include <sysdep-i386.h>

#define __NR_sigprocmask 126
#define SIG_SETMASK 2 
#define oLINK 4 
#define oSS_SP 8 
#define oSS_SIZE 16 
#define oGS 20 
#define oFS 24 
#define oEDI 36 
#define oESI 40 
#define oEBP 44 
#define oESP 48 
#define oEBX 52 
#define oEDX 56 
#define oECX 60 
#define oEAX 64 
#define oEIP 76 
#define oFPREGS 96 
#define oSIGMASK 108 
    .text
    .type setcontext, @function
    .globl setcontext
    .align 4

setcontext:
	/* Load address of the context data structure.  */
	movl	4(%esp), %eax

	/* Get the current signal mask.  Note that we preserve EBX in case
	   the system call fails and we return from the function with an
	   error.  */
	pushl	%ebx
	xorl	%edx, %edx
	leal	oSIGMASK(%eax), %ecx
	movl	$SIG_SETMASK, %ebx
	movl	$__NR_sigprocmask, %eax
	ENTER_KERNEL
	popl	%ebx
	cmpl	$-4095, %eax		/* Check %eax for error.  */

	/* EAX was modified, reload it.  */
	movl	4(%esp), %eax

	/* Restore the floating-point context.  Not the registers, only the
	   rest.  */
	movl	oFPREGS(%eax), %ecx
	fldenv	(%ecx)

	/* Restore the FS segment register.  We don't touch the GS register
	   since it is used for threads.  */
	movl	oFS(%eax), %ecx
	movw	%cx, %fs

	/* Fetch the address to return to.  */
	movl	oEIP(%eax), %ecx

	/* Load the new stack pointer.  */
	movl	oESP(%eax), %esp

	/* Push the return address on the new stack so we can return there.  */
	pushl	%ecx

	/* Load the values of all the 32-bit registers (except ESP).
	   Since we are loading from EAX, it must be last.  */
	movl	oEDI(%eax), %edi
	movl	oESI(%eax), %esi
	movl	oEBP(%eax), %ebp
	movl	oEBX(%eax), %ebx
	movl	oEDX(%eax), %edx
	movl	oECX(%eax), %ecx
	movl	oEAX(%eax), %eax

	ret
