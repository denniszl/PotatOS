#ifndef _SCHEDULE_H
#define _SCHEDULE_H

#define MAX_PCB 6

#include "syscall.h"
#include "filesys.h"
#include "x86_desc.h"

void next_task();
int get_new_task(int num_t);

#endif
