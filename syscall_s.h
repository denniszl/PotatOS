#ifndef _SYSCALL_S_H
#define _SYSCALL_S_H
// #include "syscall.h"
#include "x86_desc.h"
#include "types.h"
#define ENOSYS 38
#define NR_SYSCALLS 10
#define USER_DS 0x002B

#ifndef ASM
extern uint32_t system_call;

#endif  //ASM
#endif  // SYSCALL
