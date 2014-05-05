#ifndef _PAGING_H
#define _PAGING_H

#define KB_SIZE 1024
#define MB_SIZE (KB_SIZE*KB_SIZE)
#define KB_F KB_SIZE*4
#define MB_F MB_SIZE*4

#define MB_E MB_SIZE*8 
#define TOP 22
#define MID 12
#define OFF 32


#include "x86_desc.h"
#include "lib.h"
#include "syscall.h"
extern int viewTerm;

void init_paging(void);
void page_process(int proc_n);
void page_clear(int proc_n, int parent_n);
void change_active_term(int num, int alt_n);
void swap_pages_sched(int num);
void remap_vidmem_buffer(page_desc_t * new_pg_dir, page_desc_t * old_pg_dir);

#endif
