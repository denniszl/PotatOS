#define ASM 1
#include "syscall_s.h"

.text

.globl system_call

system_call:
	pushal #save registers
	# pushl %ds
 #    pushl %es
 #    pushl %fs
 #    pushl %gs
	movl 36(%esp), %edi
	# movl $0xffffe000, %ebx #8 kB stack #movl $0xfffff000 for the other kind
	# andl %esp, %ebx
	#we'll ignore thread_info for now.
	cmpl $0, %EAX
	je bad_sys
	cmpl $NR_SYSCALLS, %EAX #NR_syscalls is the number of system calls in the table.
	jb nobadsys
	bad_sys:
	movl $ENOSYS, %eax
	negl %eax
	movl %eax, 24(%esp) #"function not implemented"In errno.h, 38. (We don't have it.)
	jmp resume_userspace #the system call isn't valid.

	nobadsys:
	#do the same for all functions
	pushl %edx
	pushl %ecx
	pushl %ebx

	movl $KERNEL_DS, %esi
	movl %esi, %ds
	
	call *(sys_call_table-4)(,%eax,4) #valid system call
	
	addl $12, %esp #get rid of the stack values
	movl %eax, 28(%esp)
	# cli
	# movl 8(%ebp), %ecx
	# testw $0xffff, %cx #checks flags. We may want to ignore this for now...
	# je restore_all
	# tss.ss0 = KERNEL_DS;
 	# tss.esp0 = ESP;
 	# movl %esp, tss+32
 	# movl $KERNEL_DS, tss+64
    # popl %gs
    # popl %fs
    # popl %es
    # popl %ds
	popal
	iret

resume_userspace:
	popal
	orl $0xffffffff, %eax  	# return -1 for syserr
	iret

sys_call_table:
.long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
