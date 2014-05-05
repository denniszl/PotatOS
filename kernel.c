/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "extralib.h"
#include "i8259.h"
#include "debug.h"
#include "exceptions.h"
#include "paging.h"
#include "rtc.h"
#include "PIT.h"
#include "pcSound.h"
#include "key_init.h"
#include "filesys.h"
#include "terminal.h"
#include "syscall_s.h"
#include "syscall.h"
 
/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))
 
#define SYS_CALL_VEC
/*put some globals here to get relevant information*/
uint32_t mod_start_g;
#define BUFFER_SIZE 33 //for directory read
/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void
entry (unsigned long magic, unsigned long addr)
{
    multiboot_info_t *mbi;

    /* Clear the screen. */
    clear();
	
    colors_t newColor[2] = {LCYAN, BLACK};
	set_attrib(newColor[0], newColor[1]);
    /* Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        printf ("Invalid magic number: 0x%#x\n", (unsigned) magic);
        return;
    }

    /* Set MBI to the address of the Multiboot information structure. */
    mbi = (multiboot_info_t *) addr;

    /* Print out the flags. */
    printf ("flags = 0x%#x\n", (unsigned) mbi->flags);

    /* Are mem_* valid? */
    if (CHECK_FLAG (mbi->flags, 0))
        printf ("mem_lower = %uKB, mem_upper = %uKB\n",
                (unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

    /* Is boot_device valid? */
    if (CHECK_FLAG (mbi->flags, 1))
        printf ("boot_device = 0x%#x\n", (unsigned) mbi->boot_device);

    /* Is the command line passed? */
    if (CHECK_FLAG (mbi->flags, 2))
        printf ("cmdline = %s\n", (char *) mbi->cmdline);

    if (CHECK_FLAG (mbi->flags, 3)) {
        int mod_count = 0;
        int i;
        module_t* mod = (module_t*)mbi->mods_addr;
        while(mod_count < mbi->mods_count) {
            printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
            printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
            printf("First few bytes of module:\n");
            for(i = 0; i<16; i++) {
                printf("0x%x ", *((char*)(mod->mod_start+i)));
            }
            printf("\n");
            mod_count++;
        }
		mod_start_g = mod->mod_start;
		fill_in_globals_fs(mod_start_g);
    }
    /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
    {
        printf ("Both bits 4 and 5 are set.\n");
        return;
    }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG (mbi->flags, 5))
    {
        elf_section_header_table_t *elf_sec = &(mbi->elf_sec);

        printf ("elf_sec: num = %u, size = 0x%#x,"
                " addr = 0x%#x, shndx = 0x%#x\n",
                (unsigned) elf_sec->num, (unsigned) elf_sec->size,
                (unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
    }

    /* Are mmap_* valid? */
    if (CHECK_FLAG (mbi->flags, 6))
    {
        memory_map_t *mmap;

        printf ("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
                (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
        for (mmap = (memory_map_t *) mbi->mmap_addr;
                (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
                mmap = (memory_map_t *) ((unsigned long) mmap
                    + mmap->size + sizeof (mmap->size)))
            printf (" size = 0x%x,     base_addr = 0x%#x%#x\n"
                    "     type = 0x%x,  length    = 0x%#x%#x\n",
                    (unsigned) mmap->size,
                    (unsigned) mmap->base_addr_high,
                    (unsigned) mmap->base_addr_low,
                    (unsigned) mmap->type,
                    (unsigned) mmap->length_high,
                    (unsigned) mmap->length_low);
    }

    /* Construct an LDT entry in the GDT */
    {
        seg_desc_t the_ldt_desc;
        the_ldt_desc.granularity    = 0;
        the_ldt_desc.opsize         = 1;
        the_ldt_desc.reserved       = 0;
        the_ldt_desc.avail          = 0;
        the_ldt_desc.present        = 1;
        the_ldt_desc.dpl            = 0x0;
        the_ldt_desc.sys            = 0;
        the_ldt_desc.type           = 0x2;

        SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
        ldt_desc_ptr = the_ldt_desc;
        lldt(KERNEL_LDT);
    }

    /* Construct a TSS entry in the GDT */
    {
        seg_desc_t the_tss_desc;
        the_tss_desc.granularity    = 0;
        the_tss_desc.opsize         = 0;
        the_tss_desc.reserved       = 0;
        the_tss_desc.avail          = 0;
        the_tss_desc.seg_lim_19_16  = TSS_SIZE & 0x000F0000;
        the_tss_desc.present        = 1;
        the_tss_desc.dpl            = 0x0;
        the_tss_desc.sys            = 0;
        the_tss_desc.type           = 0x9;
        the_tss_desc.seg_lim_15_00  = TSS_SIZE & 0x0000FFFF;

        SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

        tss_desc_ptr = the_tss_desc;

        tss.ldt_segment_selector = KERNEL_LDT;
        tss.ss0 = KERNEL_DS;
        tss.esp0 = 0x800000;
        ltr(KERNEL_TSS);
    }

    /* Populate IDT. */
    {
        set_trap_gate(idt[0x00], &divide_error);  // Divide by zero.
        set_trap_gate(idt[0x01], &debug);
        set_intr_gate(idt[0x02], &nmi);
        set_system_intr_gate(idt[0x03], &int3);
        set_system_gate(idt[0x04], &overflow);
        set_system_gate(idt[0x05], &bounds);
        set_trap_gate(idt[0x06], &invalid_op);
        set_trap_gate(idt[0x07], &device_not_available);
        set_trap_gate(idt[0x08], &double_fault);
        set_trap_gate(idt[0x09], &coprocessor_segment_overrun);
        set_trap_gate(idt[0x0a], &invalid_TSS);
        set_trap_gate(idt[0x0b], &segment_not_present);
        set_trap_gate(idt[0x0c], &stack_segment);
        set_trap_gate(idt[0x0d], &general_protection);
        set_intr_gate(idt[0x0e], &page_fault);
        set_trap_gate(idt[0x10], &coprocessor_error);
        set_trap_gate(idt[0x11], &alignment_check);
        set_trap_gate(idt[0x12], &machine_check);
        set_trap_gate(idt[0x13], &simd_coprocessor_error);
        set_intr_gate(idt[0x20], &pit_handler);
        set_intr_gate(idt[0x21], &keyboard_handler);
		set_intr_gate(idt[0x28], &rtc_handler); //see class notes
        set_system_gate(idt[128], &system_call);
        // set_intr_gate(idt[0x28], &rtc_handler);
    }    

    
    /* Init the PIC */
    i8259_init();
    
    /* Initialize devices, memory, filesystem, enable device interrupts on the
     * PIC, any other initialization stuff... */
    keyboard_init();
    rtc_init();
    PIT_init();

    // Initialize paging (duh)
    init_paging();
    
	/*testing filesystem directory*/
	clear();
    
    // unsigned char buffer[BUFFER_SIZE];  // Directory test

    /* Enable interrupts */
     // sti();
    /* Do not enable the following until after you have set up your
     * IDT correctly otherwise QEMU will triple fault and simple close
     * without showing you any output */
    // Boot screen:

    // trogdor();
    // scale();
    // char backup = get_attrib(); 
    // set_attrib((colors_t) WHITE, (colors_t) DRED);
    // ghostbusters();
    // set_char_attrib(backup);
 
    //Arguments onto stack here:
    uint8_t * cmd = (uint8_t*)("shell");
    //cli();
    if(execute(cmd) != -1);
        printf("error\n");

    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile(".1: hlt; jmp .1;");
    
   
}


