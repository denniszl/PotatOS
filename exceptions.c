#include "exceptions.h"


/* Applies to like, all of these.
    IN:    NO
    OUT:   NO
    WHAT:  Exception trap
*/
void do_divide_error(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("You tried to divide by zero. I bet you think you're pretty clever.");
    exitStatMode(find_curprocess_num());
    // printf("Please reauthenticate with Chuck Norris privileges.");
    asm volatile(".2: hlt; jmp .2;");
}

void do_debug(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("Debug. I don't know what this is.");
    exitStatMode(find_curprocess_num());
    asm volatile(".3: hlt; jmp .3;");
}

void do_nmi(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("NMI never removes his mask.");
    exitStatMode(find_curprocess_num());
    asm volatile(".4: hlt; jmp .4;");
}

void do_int3(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("What even is int3?");
    exitStatMode(find_curprocess_num());
    asm volatile(".5: hlt; jmp .5;");
}

void do_overflow(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("My cup runneth over.");
    exitStatMode(find_curprocess_num());
    asm volatile(".6: hlt; jmp .6;");
}

void do_bounds(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("His glory knows no bounds. Your OS does.");
    exitStatMode(find_curprocess_num());
    asm volatile(".7: hlt; jmp .7;");
}

void do_invalid_op(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("Invalid op. Kinda sucks to be op right now. Poor oppie.");
    exitStatMode(find_curprocess_num());
    asm volatile(".8: hlt; jmp .8;");
}

void do_device_not_available(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("Device not available. Rain check?");
    exitStatMode(find_curprocess_num());
    asm volatile(".9: hlt; jmp .9;");
}

void do_double_fault(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("Double fault. Both are yours.");
    exitStatMode(find_curprocess_num());
    asm volatile(".10: hlt; jmp .10;");
}

void do_coprocessor_segment_overrun(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("Coprocessor segment overrun (with zombies). Please evacuate immediately.");
    exitStatMode(find_curprocess_num());
    asm volatile(".11: hlt; jmp .11;");
}

void do_invalid_TSS(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("Invalid TSS. Please show your support.");
    exitStatMode(find_curprocess_num());
    asm volatile(".12: hlt; jmp .12;");
}

void do_segment_not_present(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("Mystery of the missing sponsored segment.");
    exitStatMode(find_curprocess_num());
    asm volatile(".13: hlt; jmp .13;");
}

void do_stack_segment(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("Stack segment is a noun.");
    exitStatMode(find_curprocess_num());
    asm volatile(".14: hlt; jmp .14;");
}

void do_general_protection(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("Allgemeine Schutzverletzung! General protection fault!");
    exitStatMode(find_curprocess_num());
    asm volatile(".15: hlt; jmp .15;");
}

void do_page_fault(void)
{
	cli();
    uint32_t addr;
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    asm volatile
    (
        "movl %%cr2, %0;"
        : "=r" (addr)
    );
    printf("Seitenfehler (page fault) @ ");
    printf("addr:%x ", addr);
    exitStatMode(find_curprocess_num());
    asm volatile(".16: hlt; jmp .16;");
}

void do_coprocessor_error(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("Coprocessor is become error.");
    exitStatMode(find_curprocess_num());
    asm volatile(".17: hlt; jmp .17;");
}

void do_alignment_check(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("Your zodiac says: bad alignment. Come back tomorrow.");
    exitStatMode(find_curprocess_num());
    asm volatile(".18: hlt; jmp .18;");
}

void do_machine_check(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("Machine check. I'm pretty sure that's a foul.");
    exitStatMode(find_curprocess_num());
    asm volatile(".19: hlt; jmp .19;");
}

void do_simd_coprocessor_error(void)
{
	cli();
    initStatMode((colors_t)BLACK, (colors_t)YELLOW, find_curprocess_num());
    printf("SIMD coprocessor is error.");
    exitStatMode(find_curprocess_num());
    asm volatile(".20: hlt; jmp .20;");
}

// That's all folks.
