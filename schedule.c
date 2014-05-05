#include "schedule.h"

/*
next_task:
INPUT: NONE
OUTPUT: NONE
EFFECTS: Helper function called by the pit for context switching for scheduler.
Will flush TLBs due to changing page tables.
*/
void
next_task()
{
	curr_task = (curr_task+1)%N_TERM;
	if(-1 == get_new_task(curr_task))
	{
		printf("task switch failed");
	}
}

/*
get_new_task:
INPUT: num_t, the terminal number for the new task.
OUTPUT: 0 on success, -1 on failure.
EFFECTS: Helper function called by the pit for context switching for scheduler.
Will flush TLBs due to changing page tables.
*/
int
get_new_task(int num_t)
{
	pcb_t * tmp;
	int count;
	uint32_t iret_esp;
	uint32_t eip_val;
	int i;
	pcb_t* temp;
	tmp = (pcb_t *)(KERNEL_END - PCB_SIZE);
	count = 0;

	
	for(i = 0; i < MAX_PCB; i++)
    {
        if(tmp->alt_n == num_t && tmp->pcb_n != 0) 
        {
            temp = tmp;
        } 
        count++;
        tmp = (pcb_t*)((uint32_t)tmp - PCB_SIZE);
    }
	iret_esp = temp->iret_esp;
	eip_val = temp->eip_val;
	
	

	swap_pages_sched((temp->pcb_n)-1);
	send_eoi(0);

	
	asm volatile
	(
		"movl %0, %%esp;"
		"popal;"
		"iret;"
		:
		: "r" (iret_esp)
	);
	
	return 0;
		// "#pushl %%edi;"
		// "#movl %1, %%edi;"
		// "#movl %%edi, 4(%%esp);"
		// "#popl %%edi;" "g" (eip_val)
}
