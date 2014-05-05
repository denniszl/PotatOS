#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "lib.h"
#include "x86_desc.h"
#include "filesys.h"
#include "paging.h"
#define PROGRAMADDR ((128*MB_SIZE)+0x00048000)  // Program start address
#define CLRFL "0xFFFFE700"
#define SETINT "0x00000200"
#define DEFESP "0x083FFFFC"
#define USER_CS_IASM "0x0023"
#define USER_DS_IASM "0x002B"
#define PCB_SIZE 8192
#define KERNEL_END 8388603
#define KERNEL_START 0x400000


int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void); /*We'll write sethandler and sigreturn later.*/
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
/*find the pcb*/
pcb_t * find_pcb(void);
uint32_t find_curprocess_num(void);

//global variables
extern uint32_t mod_start;
extern uint32_t dir_entries_fs;
#endif
