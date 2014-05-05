# x86_desc.S - Set up x86 segment descriptors, descriptor tables 
# vim:ts=4 noexpandtab

#define ASM     1
#include "x86_desc.h"
#include "syscall_s.h"

.text

.globl  ldt_size, tss_size
.globl  gdt_desc, ldt_desc, tss_desc
.globl  tss, tss_desc_ptr, ldt, ldt_desc_ptr
.globl  gdt_ptr
.globl  idt_desc_ptr, idt
.globl gdt_init_ptr

# All the exceptions in the IDT from 0x00 - 0x0f
.globl divide_error
.globl debug
.globl nmi
.globl int3
.globl overflow
.globl bounds
.globl invalid_op
.globl device_not_available
.globl double_fault
.globl coprocessor_segment_overrun
.globl invalid_TSS
.globl segment_not_present
.globl stack_segment
.globl general_protection
.globl page_fault
.globl coprocessor_error
.globl alignment_check
.globl machine_check
.globl simd_coprocessor_error

.globl keyboard_handler
.globl rtc_handler
.globl pit_handler

# .globl resume_userspace

.globl page_dir, page_table, process_0, process_1, process_2, process_3, process_4, process_5, page_table_0, page_table_1, page_table_2, page_table_3, page_table_4, page_table_5

.align 4


tss_size:
    .long tss_bottom - tss - 1

ldt_size:
    .long ldt_bottom - ldt - 1

    .word 0 # Padding
ldt_desc:
    .word KERNEL_LDT
    .long ldt

    .align 4
tss:
_tss:
    .rept 104
    .byte 0
    .endr
tss_bottom:
    .align  16
gdt_init_ptr:
    .word gdt_bottom-gdt-1  
    .long gdt

gdt:
_gdt:

    # First GDT entry cannot be used
    .quad 0
    
    # NULL entry
    .quad 0
    
    # Segmentation will not be used
    # CS and DS both are 0-4GB r/w segments
    #
    # The layout is (from Intel IA-32 reference manual):
    #  31        24 23  22  21  20  19   16 15  14 13 12  11   8 7          0
    # |----------------------------------------------------------------------|
    # |            |   | D |   | A |  Seg  |   |  D  |   |      |            |
    # | Base 31:24 | G | / | 0 | V | Limit | P |  P  | S | Type | Base 23:16 |
    # |            |   | B |   | L | 19:16 |   |  L  |   |      |            |
    # |----------------------------------------------------------------------|
    #
    # |----------------------------------------------------------------------|
    # |                                    |                                 |
    # | Base 15:0                          | Segment Limit 15:0              |
    # |                                    |                                 |
    # |----------------------------------------------------------------------|

gdt_ptr:
    # Set up an entry for kernel CS
    .quad 0x00CF9A000000FFFF

    # Set up an entry for kernel DS
    .quad 0x00CF92000000FFFF

    # Set up an entry for user CS
    .quad 0x00CFFA000000FFFF

    # Set up an entry for user DS
    .quad 0x00CFF2000000FFFF

    # Set up an entry for TSS
tss_desc_ptr:
    .quad 0

    # Set up one LDT
ldt_desc_ptr:
    .quad 0

gdt_bottom:

    .align 16
ldt:
    .rept 4
    .quad 0
    .endr
ldt_bottom:

.align 4
    .word 0 # Padding
idt_desc_ptr:
    .word idt_bottom - idt - 1
    .long idt
    
    .align  16
idt:
_idt:
    .rept NUM_VEC
    .quad 0
    .endr

idt_bottom:
#These define the existence of hte processes, page tables and page #directories inside of memory. They are aligned and are each 1024KB
# Paging.
.align 4096
page_dir:
.rept 1024
.long 0
.endr

.align 4096
page_table:
.rept 1024
.long 0
.endr

.align 4096
process_0:
.rept 1024
.long 0
.endr

.align 4096
process_1:
.rept 1024
.long 0
.endr

.align 4096
process_2:
.rept 1024
.long 0
.endr

.align 4096
process_3:
.rept 1024
.long 0
.endr

.align 4096
process_4:
.rept 1024
.long 0
.endr

.align 4096
process_5:
.rept 1024
.long 0
.endr

.align 4096
page_table_0:
.rept 1024
.long 0
.endr

.align 4096
page_table_1:
.rept 1024
.long 0
.endr

.align 4096
page_table_2:
.rept 1024
.long 0
.endr

.align 4096
page_table_3:
.rept 1024
.long 0
.endr

.align 4096
page_table_4:
.rept 1024
.long 0
.endr

.align 4096
page_table_5:
.rept 1024
.long 0
.endr

#Macros defined to save/restore all the registers in the order we want
.macro SAVE_ALL
    # cld
    # pushl %es
    # pushl %ds
    # pushl %eax
    # pushl %ebp
    # pushl %edi
    # pushl %esi
    # pushl %edx
    # pushl %ecx
    # pushl %ebx
    pushal
    movl 28(%esp), %ebx
    movl $USER_DS,%edx
    movl %edx,%ds
    movl %edx,%es
    # pushl %ds
    # pushl %es
    # pushl %fs
    # pushl %gs
.endm

.macro RESTORE_ALL  
    # popl %ebx   
    # popl %ecx   
    # popl %edx   
    # popl %esi   
    # popl %edi   
    # popl %ebp   
    # popl %eax   
    # popl %ds    
    # popl %es
    # popl %gs
    # popl %fs
    # popl %es
    # popl %ds    
    popal
    # addl $4,%esp
    iret 
.endm

# Assembly wrappers around the exceptions
divide_error:
    pushl $0
    pushl $do_divide_error
    jmp error_code

debug:
    pushl $0
    pushl $do_debug
    jmp error_code
    
nmi:
    pushl $0
    pushl $do_nmi
    jmp error_code
    
int3:
    pushl $0
    pushl $do_int3
    jmp error_code

overflow:
    pushl $0
    pushl $do_overflow
    jmp error_code

bounds:
    pushl $0
    pushl $do_bounds
    jmp error_code

invalid_op:
    pushl $0
    pushl $do_invalid_op
    jmp error_code

device_not_available:
    pushl $0
    pushl $do_device_not_available
    jmp error_code
    
double_fault: # this is a double fault
    # pushl $0
    pushl $do_double_fault
    jmp error_code
    
coprocessor_segment_overrun:
    pushl $0
    pushl $do_coprocessor_segment_overrun
    jmp error_code
    
invalid_TSS:
    # pushl $0
    pushl $do_invalid_TSS
    jmp error_code
    
segment_not_present:
    # pushl $0
    pushl $do_segment_not_present
    jmp error_code
    
stack_segment:
    # pushl $0
    pushl $do_stack_segment
    jmp error_code

general_protection:
    # pushl $0
    pushl $do_general_protection
    jmp error_code
    
page_fault:
    # pushl $0
    pushl $do_page_fault
    jmp error_code
    
coprocessor_error:
    pushl $0
    pushl $do_coprocessor_error
    jmp error_code
    
alignment_check:
    pushl $0
    pushl $do_alignment_check
    jmp error_code

machine_check:
    pushl $0
    pushl $do_machine_check
    jmp error_code
    
simd_coprocessor_error:
    pushl $0
    pushl $do_simd_coprocessor_error
    jmp error_code

error_code:         # see page 150 of Understanding Linux Kernel
    pushal
    
    #cld         # clear direction flags
    
    movl 36(%esp), %edx     # Step 3. To the place where we pushl $0'd
    
    no_save_err:
    movl 32(%esp), %edi     # Step 4.  Get function address.
    movl %es, %edx
    movl %edx, 32(%esp)
    
    movl %esp, %eax          # Step 5.
    
    movl $USER_DS, %edx      #step 6
    movl %edx, %ds
    movl %edx, %es      
    
    call *%edi               #step 7, page fault
    
    popal
    addl $4, %esp           # function
    addl $4, %esp           # pop error code, too.
    iret        

rtc_handler:
    SAVE_ALL
    call rtc_c
	pushl $8 #ir8
	call send_eoi
	addl $4, %esp
    RESTORE_ALL
    
keyboard_handler:
    SAVE_ALL
    # pushl %esp
    # call save_esp
    # addl $4, %esp
    call key_c
    pushl $1 #ir1
    call send_eoi
    addl $4, %esp
    RESTORE_ALL

pit_handler:
    SAVE_ALL
    pushl %esp
    call save_esp
    addl $4, %esp
    call PIT_c
    pushl $0
    call send_eoi
    addl $4, %esp
    RESTORE_ALL
