#include "paging.h"
#define VIDMEM_ENTRY 33
#define PAGE_MULT 4
#define VIDMEM_ENTRY 33
#define PAGE_MULT 4
#define BASE_USER_PROGRAM 3
#define TERM_MEM_OFFSET 2
//#include "nawos.h"

char zero_array[KB_F];
// void clear_vid_mem(void);

/*
*Function: init_paging
*INPUT : None
*OUTPUT : None
*Effects: Called when we have to initialize paging. Enables virtual memory and identity pages
*0~8 MB.
*/
void init_paging(void)
{
    int i; //used as a counter t find the video memory region
        
    // Set the first entry:
    page_dir[0].present = 1;
    page_dir[0].rw = 1;
    page_dir[0].us = 0;
    page_dir[0].write_thru = 1;
    page_dir[0].cache_disabled = 1;
    page_dir[0].accessed = 0;
    page_dir[0].zero_bit = 0;
    page_dir[0].page_size = 0;
    page_dir[0].ignored = 1;
    page_dir[0].avail = 0;
    page_dir[0].page_tbl_index = (((uint32_t)page_table & MIDDLE_TEN) >> MID);
    page_dir[0].page_dir_index = (((uint32_t)page_table & TOP_TEN) >> TOP);
    // Initialize many 4-kb sized pages.  ignored at video memory locations.
    for(i = 0; i < KB_SIZE; i++)
    {
        if((i*(KB_F) < VIDEO + NUM_COLS*(NUM_ROWS+1)) && ((i*(KB_F)) >= VIDEO)) 
            set_page_KB_on(page_table[i], i*KB_F); /*Video memory*/
         else
             set_page_KB_off(page_table[i], i*KB_F);
    }
    
    // initialize 4-MB_page.
    set_page_MB_on(page_dir[1], MB_F); /*4 MB block 4MB~8MB*/

    for(i = 2; i < KB_SIZE; i++)
    {
        set_page_MB_off(page_dir[i], i*MB_F); /*All other memory addresses 
        are inaccessible, they should generate a page fault*/
    }
        
    asm ("movl $page_dir, %%eax;"     /*Initialize PDBR*/
        "movl %%eax, %%cr3;"          

        "movl %%cr4, %%eax;"      /*This enables 4 MB pages */
        "orl $0x00000010, %%eax;"
        "movl %%eax, %%cr4;"
        
        "movl %%cr0, %%eax;"
        "orl $0x80000000, %%eax;" /*Sets the paging flag*/
        "movl %%eax, %%cr0;"
        : 
        : 
        : "%eax"
        );
}

/*
    Function: page_process
    IN:     proc_n
    OUT:    NO
    WHAT:   "Dynamically" allocates a page for a new process.
*/
void page_process(int proc_n)
{
    int i;
    page_desc_t * new_pg_dir;
    
    /*2 gives you the first page after kernel.*/
    i = 2+proc_n;
    new_pg_dir = process_0 + proc_n*PD_SIZE;

    // Set the first entry:
    new_pg_dir[0].present = 1;
    new_pg_dir[0].rw = 1;
    new_pg_dir[0].us = 0;
    new_pg_dir[0].write_thru = 1;
    new_pg_dir[0].cache_disabled = 1;
    new_pg_dir[0].accessed = 0;
    new_pg_dir[0].zero_bit = 0;
    new_pg_dir[0].page_size = 0;
    new_pg_dir[0].ignored = 1;
    new_pg_dir[0].avail = 0;
    new_pg_dir[0].page_tbl_index = (((uint32_t)page_table & MIDDLE_TEN) >> MID);
    new_pg_dir[0].page_dir_index = (((uint32_t)page_table & TOP_TEN) >> TOP);
    
    // initialize 4-MB_page for kernel.
    set_page_MB_on(new_pg_dir[1], MB_F); /*4 MB block 4MB~8MB*/

    set_page_MB_on_sv_off(new_pg_dir[OFF], MB_E+(proc_n*MB_F)); 
    /*This maps some physical memory to 128 MB.*/
    asm("movl %0, %%eax;"
        "movl %%eax, %%cr3;"
        :
        : "r" (new_pg_dir)
        :"%eax"
        );
}

/*
    Function: page_clear
    IN:     proc_n, parent_n
    OUT:    NO
    WHAT:   Changes pages to return from a parent process to a child process. Not called
    if an "orphan" tries exiting.
*/
void page_clear(int proc_n, int parent_n)
{
    page_desc_t * new_pg_dir;
    page_desc_t * new_pg_dir2;
    
    new_pg_dir = process_0 + proc_n*PD_SIZE;

    /*This switch is used mainly because I want to go back to page_dir rather than page_table when
    * I reach the top of the process page directories. It's just the memory layout.
    */
    switch(proc_n)
    {
        case 0:
        case 1:
        case 2:
            new_pg_dir2 = new_pg_dir;
            break;
        default:
            new_pg_dir2 = process_0 + (parent_n)*PD_SIZE;
            break;
    }

    asm("movl %0, %%eax;"
    "movl %%eax, %%cr3;"
    :
    : "r" (new_pg_dir2)
    :"%eax"
    );


    set_page_MB_off(new_pg_dir[0], 0);
    set_page_MB_off(new_pg_dir[1], MB_F);
    set_page_MB_off(new_pg_dir[OFF], 0);
    set_page_MB_off(new_pg_dir[33], 0);

    /*Flush TLBs*/

    asm volatile(
    "movl %cr3, %eax;"
    "movl %eax, %cr3;"
    );

}

/*
change_page_table

INPUT : num is the processID, alt_n is terminal number
OUTPUT : writes to video memory and buffers
EFFECTS: This changes the "active" (viewable) terminal.

*/
void
change_active_term(int num, int alt_n)
{
    // pcb_t * tmp;
    pcb_t * temp;
    pcb_t * tmp2;
    int i;
    uint32_t new_offset;
    /*Changing video memory. Critical section.*/
    page_desc_t * new_page_dir; 
    /*since we launch the third terminal first, the numbering is slightly off. Terminal one's page table resides in 0x407000, Terminal two's in 0x408000, Terminal three's in 0x409000*/
    // tmp = find_pcb();
    tmp2 = (pcb_t*)(KERNEL_END - PCB_SIZE);
    for(i = 0; i < 6; i++)
    {
        if(tmp2->alt_n == viewTerm && tmp2->pcb_n != 0) 
        {
            temp = tmp2;
        } 
        tmp2 = (pcb_t*)((uint32_t)tmp2 - PCB_SIZE);
    } 
    new_page_dir = process_0 + num*PD_SIZE;
    //calculate the same for the old page dir
    page_desc_t * old_page_dir;
    new_offset = (temp->pcb_n)-1;
    uint32_t offset = (new_offset) * PD_SIZE;
    old_page_dir = process_0 + offset;

    memcpy(videomem[viewTerm],(char *)VIDEO, FOURKB);
    //clear_vid_mem();`
    saveCurs();
    viewTerm = alt_n; //update visible terminal
    remap_vidmem_buffer(new_page_dir, old_page_dir);
    memcpy((char *)VIDEO, videomem[alt_n], FOURKB);
    flipCurs();
    flipColor();
    move_csr();
}

/*
swap_pages_sched

INPUT : num is the processID
OUTPUT : none
EFFECTS: Called by the PID, this is the context switching part of scheduling.

*/

void swap_pages_sched(int num)
{

    page_desc_t * new_page_dir; 
    new_page_dir = process_0 + num*PD_SIZE;
    // pcb_t * tmp;
    // tmp = (pcb_t*)(KERNEL_END - (num+1)*PCB_SIZE);

        asm("movl %0, %%eax;"
        "movl %%eax, %%cr3;"
        :
        : "r" (new_page_dir)
        :"%eax"
        );

    tss.esp0 = KERNEL_END - num*PCB_SIZE;
    tss.ss0 = KERNEL_DS; 
}


/*INPUT : NONE
* OUTPUT : NONE
*DESCRIPTION : Changes the mapping of the video memory of the current running process to be
* in the background when there's a terminal switch. Needed for scheduling to work properly*/
void remap_vidmem_buffer(page_desc_t * new_pg_dir, page_desc_t * old_pg_dir)
{
	page_desc_t * new_pg_table = new_pg_dir+N_TABLES*PD_SIZE;
	page_desc_t * old_pg_table = old_pg_dir+N_TABLES*PD_SIZE;
	int i=0; //counter for the for loops to find the page we mapped to video memory
	if(old_pg_dir[VIDMEM_ENTRY].present == 1){	
	//switch the old terminal to write to buffer
		for(i=0;i<KB_SIZE;i++){	
			 if((i*(KB_SIZE*PAGE_MULT) < VIDEO + NUM_COLS*(NUM_ROWS+1)) && ((i*(KB_SIZE*PAGE_MULT)) >= VIDEO)){
				pcb_t * pcb_p = find_pcb();
				int loc = pcb_p->alt_n;
				
				set_page_KB_on(old_pg_table[i], videomem[loc]);//i*KB_SIZE*PAGE_MULTIPLIER); /*Video memory*/
				old_pg_table[i].us = 1; //set supervisor bit to user
				break;
			}
		}
	}
	//switch the new terminal to map to video memory, if needed.
	if(new_pg_dir[VIDMEM_ENTRY].present ==1){
		for(i=0;i<KB_SIZE;i++)
		{	
			 if((i*(KB_SIZE*PAGE_MULT) < VIDEO + NUM_COLS*(NUM_ROWS+1)) && ((i*(KB_SIZE*PAGE_MULT)) >= VIDEO)){
				set_page_KB_on(new_pg_table[i], i*KB_SIZE*PAGE_MULT); /*Video memory*/
				new_pg_table[i].us = 1; //set supervisor bit to user
				break;
			}
		}
	}
}


