/* x86_desc.h - Defines for various x86 descriptors, descriptor tables, 
 * and selectors
 * vim:ts=4 noexpandtab
 */

#ifndef _X86_DESC_H
#define _X86_DESC_H

#include "types.h"

/* Segment selector values */
#define KERNEL_CS 0x0010
#define KERNEL_DS 0x0018
#define USER_CS 0x0023
#define USER_DS 0x002B
#define KERNEL_TSS 0x0030
#define KERNEL_LDT 0x0038
#define FOURKB 4096
#define TWOKB 2048
#define MAXPARAMSIZE 128
#define PCBDATASIZE 1264
#define PD_SIZE 1024
#define FD_ARRAY_SIZE 8
#define N_TABLES 6
#define N_TERM 3
/* Size of the task state segment (TSS) */
#define TSS_SIZE 104

/* Number of vectors in the interrupt descriptor table (IDT) */
#define NUM_VEC 256
 
// Paging:
#define TOP_TEN 0xFFC00000
#define MIDDLE_TEN 0x003FF000

#ifndef ASM
/* This structure is used to load descriptor base registers
 * like the GDTR and IDTR */
typedef struct x86_desc {
    uint16_t padding;
    uint16_t size;
    uint32_t addr;
} x86_desc_t;

/*file descriptor struct*/
typedef struct fd_table_entry
{
	uint32_t file_op_p; //pointer to file operations table
	uint32_t inode_p;
	uint32_t file_position; //keeps track of where we last read should update every read
	uint32_t flags;
} fd_table_entry_t;

/* This is a segment descriptor.  It goes in the GDT. */
typedef struct seg_desc {
    union {
        uint32_t val;
        struct {
            uint16_t seg_lim_15_00;
            uint16_t base_15_00;
            uint8_t base_23_16;
            uint32_t type : 4;
            uint32_t sys : 1;
            uint32_t dpl : 2;
            uint32_t present : 1;
            uint32_t seg_lim_19_16 : 4;
            uint32_t avail : 1;
            uint32_t reserved : 1;
            uint32_t opsize : 1;
            uint32_t granularity : 1;
            uint8_t base_31_24;
        } __attribute__((packed));
    };
} seg_desc_t;

/* TSS structure */
typedef struct __attribute__((packed)) tss_t {
    uint16_t prev_task_link;
    uint16_t prev_task_link_pad;

    uint32_t esp0;
    uint16_t ss0;
    uint16_t ss0_pad;

    uint32_t esp1;
    uint16_t ss1;
    uint16_t ss1_pad;

    uint32_t esp2;
    uint16_t ss2;
    uint16_t ss2_pad;

    uint32_t cr3;

    uint32_t eip;
    uint32_t eflags;

    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;

    uint16_t es;
    uint16_t es_pad;

    uint16_t cs;
    uint16_t cs_pad;

    uint16_t ss;
    uint16_t ss_pad;

    uint16_t ds;
    uint16_t ds_pad;

    uint16_t fs;
    uint16_t fs_pad;

    uint16_t gs;
    uint16_t gs_pad;

    uint16_t ldt_segment_selector;
    uint16_t ldt_pad;

    uint16_t debug_trap : 1;
    uint16_t io_pad : 15;
    uint16_t io_base_addr;
} tss_t;

/* Page struct implementation */
typedef struct page_desc {
    union {
        uint32_t val;
        struct {
        uint32_t present : 1;
        uint32_t rw : 1;
        uint32_t us : 1;
        uint32_t write_thru : 1;
        uint32_t cache_disabled : 1;
        uint32_t accessed : 1;
        uint32_t zero_bit : 1;
        uint32_t page_size : 1;
        uint32_t ignored : 1; /*Global bit!*/
        uint32_t avail : 3;
        uint32_t page_tbl_index : 10;
        uint32_t page_dir_index : 10;
    } __attribute__((packed));
    };
} page_desc_t;

/*Process control block. Should be 8KB.*/
typedef struct PCB
{
    uint32_t pcb_n;
    uint32_t prevpcb; /*Pointer to previous PCB*/
    uint32_t prevesp;
    uint32_t alt_n;
    uint32_t isOrphan;
    struct PCB * parent;
    uint32_t iret_esp;
    uint32_t eip_val;
    uint32_t signal_n;
    uint32_t signal_addr;
    fd_table_entry_t fd_array[FD_ARRAY_SIZE];
    uint8_t progargs[MAXPARAMSIZE];
    uint8_t data[PCBDATASIZE]; /*Random data. For filler right now.*/
    uint8_t kernel_stack[TWOKB]; /*Don't touch this, we're using it for kernel stack.*/
}pcb_t;

/* Some external descriptors declared in .S files */
extern x86_desc_t gdt_desc;

extern uint16_t ldt_desc;
extern uint32_t ldt_size;
extern seg_desc_t ldt_desc_ptr;
extern seg_desc_t gdt_ptr;
extern uint32_t ldt;
extern uint32_t gdt_init_ptr;
extern page_desc_t page_dir[PD_SIZE];
extern page_desc_t page_table[PD_SIZE];
extern page_desc_t process_0[PD_SIZE];
extern page_desc_t process_1[PD_SIZE];
extern page_desc_t process_2[PD_SIZE];
extern page_desc_t process_3[PD_SIZE];
extern page_desc_t process_4[PD_SIZE];
extern page_desc_t process_5[PD_SIZE];
extern page_desc_t page_table_0[PD_SIZE];
extern page_desc_t page_table_1[PD_SIZE];

extern page_desc_t page_table_2[PD_SIZE];
extern page_desc_t page_table_3[PD_SIZE];
extern page_desc_t page_table_4[PD_SIZE];
extern page_desc_t page_table_5[PD_SIZE];

extern uint32_t tss_size;
extern seg_desc_t tss_desc_ptr;
extern tss_t tss;

/* Sets runtime-settable parameters in the GDT entry for the LDT */
#define SET_LDT_PARAMS(str, addr, lim) \
do { \
    str.base_31_24 = ((uint32_t)(addr) & 0xFF000000) >> 24; \
        str.base_23_16 = ((uint32_t)(addr) & 0x00FF0000) >> 16; \
        str.base_15_00 = (uint32_t)(addr) & 0x0000FFFF; \
        str.seg_lim_19_16 = ((lim) & 0x000F0000) >> 16; \
        str.seg_lim_15_00 = (lim) & 0x0000FFFF; \
} while(0)

/* Sets runtime parameters for the TSS */
#define SET_TSS_PARAMS(str, addr, lim) \
do { \
    str.base_31_24 = ((uint32_t)(addr) & 0xFF000000) >> 24; \
        str.base_23_16 = ((uint32_t)(addr) & 0x00FF0000) >> 16; \
        str.base_15_00 = (uint32_t)(addr) & 0x0000FFFF; \
        str.seg_lim_19_16 = ((lim) & 0x000F0000) >> 16; \
        str.seg_lim_15_00 = (lim) & 0x0000FFFF; \
} while(0)

/* An interrupt descriptor entry (goes into the IDT) */
typedef union idt_desc_t {
    uint32_t val;
    struct {
        uint16_t offset_15_00;
        uint16_t seg_selector;
        uint8_t reserved4;
        uint32_t reserved3 : 1;
        uint32_t reserved2 : 1;
        uint32_t reserved1 : 1;
        uint32_t size : 1;
        uint32_t reserved0 : 1;
        uint32_t dpl : 2;
        uint32_t present : 1;
        uint16_t offset_31_16;
    } __attribute__((packed));
} idt_desc_t;

/* The IDT itself (declared in x86_desc.S */
extern idt_desc_t idt[NUM_VEC];
/* The descriptor used to load the IDTR */
extern x86_desc_t idt_desc_ptr;

// All the exceptions in the IDT from 0x00 - 0x0f
extern uint32_t divide_error, debug, nmi, int3, overflow, bounds, invalid_op, device_not_available, double_fault, coprocessor_segment_overrun, invalid_TSS, segment_not_present, stack_segment, general_protection, page_fault, coprocessor_error, alignment_check, machine_check, simd_coprocessor_error;

/*Define macros for the handlers*/
extern uint32_t keyboard_handler;
extern uint32_t rtc_handler;
extern uint32_t pit_handler;

/* Sets runtime parameters for an IDT entry */
#define SET_IDT_ENTRY(str, handler) \
do { \
    str.offset_31_16 = ((uint32_t)(handler) & 0xFFFF0000) >> 16; \
        str.offset_15_00 = ((uint32_t)(handler) & 0xFFFF); \
        str.seg_selector = KERNEL_CS; \
        str.size = 1; \
        str.present = 1; \
        str.dpl = 3; \
} while(0)

// An interrupt gate.
#define set_intr_gate(str, handler)\
do { \
    str.offset_31_16 = ((uint32_t)(handler) & 0xFFFF0000) >> 16; \
        str.offset_15_00 = ((uint32_t)(handler) & 0xFFFF); \
        str.seg_selector = KERNEL_CS; \
        str.reserved4 = 0; \
        str.reserved3 = 0;\
        str.reserved2 = 1;\
        str.reserved1 = 1;\
        str.size = 1; \
        str.reserved0 = 0;\
        str.present = 1; \
        str.dpl = 0; \
} while(0)

//  A trap gate.
#define set_trap_gate(str, handler)\
do { \
    str.offset_31_16 = ((uint32_t)(handler) & 0xFFFF0000) >> 16; \
        str.offset_15_00 = ((uint32_t)(handler) & 0xFFFF); \
        str.seg_selector = KERNEL_CS; \
        str.reserved4 = 0; \
        str.reserved3 = 1;\
        str.reserved2 = 1;\
        str.reserved1 = 1;\
        str.size = 1; \
        str.reserved0 = 0;\
        str.present = 1; \
        str.dpl = 0; \
} while(0)

// A system gate.
#define set_system_gate(str, handler) \
do { \
    str.offset_31_16 = ((uint32_t)(handler) & 0xFFFF0000) >> 16; \
        str.offset_15_00 = ((uint32_t)(handler) & 0xFFFF); \
        str.seg_selector = KERNEL_CS; \
        str.reserved4 = 0; \
        str.reserved3 = 1;\
        str.reserved2 = 1;\
        str.reserved1 = 1;\
        str.size = 1; \
        str.reserved0 = 0;\
        str.present = 1; \
        str.dpl = 3; \
} while(0)

// A system interrupt gate.
#define set_system_intr_gate(str, handler) \
do { \
    str.offset_31_16 = ((uint32_t)(handler) & 0xFFFF0000) >> 16; \
        str.offset_15_00 = ((uint32_t)(handler) & 0xFFFF); \
        str.seg_selector = KERNEL_CS; \
        str.reserved4 = 0; \
        str.reserved3 = 0;\
        str.reserved2 = 1;\
        str.reserved1 = 1;\
        str.size = 1; \
        str.reserved0 = 0;\
        str.present = 1; \
        str.dpl = 3; \
} while(0)

/* 
    Paging macros:
 */
// Megabyte-sized page set to present.
#define set_page_MB_on(str, index)\
do \
{\
    str.present = 1;\
    str.rw = 1;\
    str.us = 0;\
    str.write_thru = 1;\
    str.cache_disabled = 1;\
    str.accessed = 0;\
    str.zero_bit = 0;\
    str.page_size = 1;\
    str.ignored = 1; \
    str.avail = 0;\
    str.page_dir_index = (((uint32_t)(index) & TOP_TEN)>>22);\
}while(0)

//supervisor bit off
#define set_page_MB_on_sv_off(str, index)\
do \
{\
    str.present = 1;\
    str.rw = 1;\
    str.us = 1;\
    str.write_thru = 1;\
    str.cache_disabled = 1;\
    str.accessed = 0;\
    str.zero_bit = 0;\
    str.page_size = 1;\
    str.ignored = 1; \
    str.avail = 0;\
    str.page_dir_index = (((uint32_t)(index) & TOP_TEN)>>22);\
}while(0)

// Kilobyte-sized page set to present.
#define set_page_KB_on(str, index)\
do \
{\
    str.present = 1;\
    str.rw = 1;\
    str.us = 0;\
    str.write_thru = 1;\
    str.cache_disabled = 1;\
    str.accessed = 0;\
    str.zero_bit = 0;\
    str.page_size = 0;\
    str.ignored = 1;\
    str.avail = 0;\
    str.page_tbl_index = (((uint32_t)(index) & MIDDLE_TEN) >> 12);\
    str.page_dir_index = (((uint32_t)(index) & TOP_TEN) >> 22);\
}while(0)

// Megabyte sized page clear present.
#define set_page_MB_off(str, index)\
do \
{\
    str.present = 0;\
    str.rw = 1;\
    str.us = 0;\
    str.write_thru = 1;\
    str.cache_disabled = 1;\
    str.accessed = 0;\
    str.zero_bit = 0;\
    str.page_size = 1;\
    str.ignored = 1;\
    str.avail = 0;\
    str.page_dir_index = (((uint32_t)(index) & TOP_TEN) >> 22);\
}while(0)

// Kilobyte sized page clear present.
#define set_page_KB_off(str, index)\
do \
{\
    str.present = 0;\
    str.rw = 1;\
    str.us = 0;\
    str.write_thru = 1;\
    str.cache_disabled = 1;\
    str.accessed = 0;\
    str.zero_bit = 0;\
    str.page_size = 0;\
    str.ignored = 0;\
    str.avail = 0;\
    str.page_tbl_index = (((uint32_t)(index) & MIDDLE_TEN) >> 12);\
    str.page_dir_index = (((uint32_t)(index) & TOP_TEN) >> 22);\
}while(0)

/* Load task register.  This macro takes a 16-bit index into the GDT,
 * which points to the TSS entry.  x86 then reads the GDT's TSS
 * descriptor and loads the base address specified in that descriptor
 * into the task register */
#define ltr(desc)                       \
do {                                    \
    asm volatile("ltr %w0"              \
            :                           \
            : "r" (desc)                \
            : "memory", "cc" );         \
} while(0)

/* Load the interrupt descriptor table (IDT).  This macro takes a 32-bit
 * address which points to a 6-byte structure.  The 6-byte structure
 * (defined as "struct x86_desc" above) contains a 2-byte size field
 * specifying the size of the IDT, and a 4-byte address field specifying
 * the base address of the IDT. */
#define lidt(desc)                      \
do {                                    \
    asm volatile("lidt (%0)"            \
            :                           \
            : "g" (desc)                \
            : "memory");                \
} while(0)

/* Load the local descriptor table (LDT) register.  This macro takes a
 * 16-bit index into the GDT, which points to the LDT entry.  x86 then
 * reads the GDT's LDT descriptor and loads the base address specified
 * in that descriptor into the LDT register */
#define lldt(desc)                      \
do {                                    \
    asm volatile("lldt %%ax"            \
            :                           \
            : "a" (desc)                \
            : "memory" );               \
} while(0)

#endif /* ASM */

#endif /* _x86_DESC_H */
