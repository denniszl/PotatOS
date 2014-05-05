#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include "lib.h"
#include "extralib.h"
#include "syscall.h"
extern int viewTerm;// visible terminal

/*exception handlers*/
void do_divide_error(void);

void do_debug(void);

void do_nmi(void);

void do_int3(void);

void do_overflow(void);

void do_bounds(void);

void do_invalid_op(void);

void do_device_not_available(void);

void do_thirty_one(void);

void do_coprocessor_segment_overrun(void);

void do_invalid_TSS(void);

void do_segment_not_present(void);

void do_stack_segment(void);

void do_general_protection(void);

void do_page_fault(void);

void do_coprocessor_error(void);

void do_alignment_check(void);

void do_machine_check(void);

void do_simd_coprocessor_error(void);

#endif
