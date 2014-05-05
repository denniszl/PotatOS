#include "syscall.h"
#define USER_DS_ASM "0x002B"
#define USER_CS_ASM "0x0023"
#define FIRSTFREE 33
#define PAGE_MULTIPLIER 4

/*the 8 bit argument from BL.*/
uint8_t halt_ret_val;

/*INPUT : Status should be value of ebx by calling convention. To extract BL from
* OUTPUT : Value in range from 0 to 255
* DESCRIPTION : Halt stops a program from executing. It actually doesn't return inside of this function
* but rather, returns in execute because we will have to return to parent processes before ending
* a program. Halt takes care of modifying the pcb_n (status of the pcb) and also the TSS.
* it also flushes the TLB.*/
int32_t
halt(uint8_t status)
{
    cli();
    int i;
    asm volatile
    ("movb %%bl, %0"
        :"=r" (halt_ret_val)
    );

    uint32_t EBP;
    uint32_t prevesp;
    int curr_pcb_n;
    pcb_t * curr_pcb = find_pcb();
  
    /*while(curr_pcb->pcb_n != 0)
    {
        curr_pcb = (pcb_t*)((uint32_t)curr_pcb - PCB_SIZE);
    }

    curr_pcb = (pcb_t*)((uint32_t)curr_pcb + PCB_SIZE);*/
    
    /*Need to subtract one to fit convention.*/
    curr_pcb_n = (curr_pcb->pcb_n)-1;

    /*Clear PCB for use since we're done with this process*/
    for(i = 0; i < FD_ARRAY_SIZE; i++)
    {
        close(i);
    }
    /*Clear the page. TLB flush is taken care of here.*/
    EBP = curr_pcb->prevpcb;
    prevesp = curr_pcb->prevesp;
    curr_pcb->pcb_n = 0;
    curr_pcb->alt_n = 0x9;
    
    if(curr_pcb->isOrphan == 1)
    {
        clearStat(viewTerm);
        initStatMode((colors_t)LGREEN,(colors_t)DRED, viewTerm);
        printf("Closing last shell process forbidden. Launching new shell.");
        exitStatMode(viewTerm);
        curr_pcb->pcb_n = 0;
        execute((unsigned char*)("shell"));
    }

    page_clear(curr_pcb_n, (curr_pcb->parent->pcb_n)-1);
    

    /*Changing TSS values*/
    /*You have to do this to ensure that when you switch back from privilage level 3, your
    * TSS is correct.
    */
    asm("xorl %%eax, %%eax;"
        "movl $" USER_DS_IASM ", %%eax;"
        "movl %%eax, %%ds"
        :
        :
        :"%eax"
        );
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_END - ((curr_pcb->parent->pcb_n-1)*PCB_SIZE);
    
    asm volatile(
        "movl %0, %%ebp;"

        :
        :"r" (EBP)
        );
    asm volatile("jmp stupid_goto_hack");
    /*        "movl %1, %%esp;"
, "r" (prevesp)*/
    
    return 0; /*It should never reach here.*/
}

/*INPUT : The command number
* OUTPUT : -1 on failure. 0 on success
* DESCRIPTION : This function isn't even 25% done.*/
int32_t
execute(const uint8_t* command)
{   
    /*Critical since we have to set up stacks properly.*/
    cli();
    if(command == NULL)
    {
        return -1;
    }
    int i = 0;
    unsigned char* command_p = (uint8_t*)command;
    char filename[MAX_BUF_SIZE];
    int isexec; /* 1 = executable, 0 = not an executable, -1 = error.*/
    pcb_t * tmp;
    int shell_count = 0; //number of shells open
    int str_length;
    // int fileindex;
    char args[MAXPARAMSIZE];
    uint32_t iret_addr;
    
    uint32_t ESP;
    asm volatile
    (
        "movl %%ebp, %0;"
        : "=r" (ESP)
    );
    uint32_t prevesp = tss.esp0;
    /*We'll try using the previous ESP0 as our value.
    asm volatile
        (
        "movl 24(%%ebp), %0;"
        : "=r" (ESP)
        );*/

    str_length = strlen((char*)command_p);
    
    if(str_length == 0)
    {
        return -1;
    }

	//check if the file exists
	/*if(check_file_existence(command)==-1){
		return -1;
	}*/
    while(*command_p == ' ')
    {
        command_p++; /*This gets rid of all leading spaces.*/
    }
    
    /*Get the filename.*/
    /*The file must be an executable!*/
    while((*command_p) != ' ') /*Ascii code 32, I'll change it if this doesn't work.*/
    {
        if(*command_p=='\0')
        {
            filename[i] = *command_p;
            break;
        }
		else{
			filename[i] = *command_p;
			i++;
		}
        command_p++; /*Next character*/
    }
    
    if(*command_p == ' '){
        filename[i] = '\0';
	}
    
    while(*command_p == ' ')
    {
        command_p++; /*This gets rid of all trailing spaces.*/
    }

    i = 0;
    while(*command_p != '\0')
    {
        if(i >= MAXPARAMSIZE)
        {
            initStatMode((colors_t)WHITE,(colors_t)BLACK, viewTerm);
            printf("Too many arguments.");
            exitStatMode(viewTerm);
            return -1; /*Too many arguments over cap.*/
        }
        args[i] = *(command_p++);
        i++;
    }
    
    args[i] = '\0';

    isexec = checkexec(filename);

    switch(isexec)
    {
        case 0:
            //printf("Not an executable!\n");
            return -1;
        
        case 1:
            break; /*It's an executable. Keep going.*/
        
        default:
            //printf("There was a problem with the file. (e.g. it doesn't exist)\n");
            return -1;
    }
    
    /*Our current implementation to dynamically allocate pages before execution is simply to check
    * if the page is "present" or not. This is a hack. Later we'll try to integrate the PCB, but
    * we haven't made that yet.
    */
    tmp = (pcb_t*)(KERNEL_END - PCB_SIZE);
   
    shell_count = 0;
    while(tmp->pcb_n != 0)
    {
        shell_count++;
        if(shell_count > 5)
        {
            initStatMode((colors_t)LGREEN, (colors_t)DBLUE, viewTerm);
            printf("No more, please!");
            exitStatMode(viewTerm);
            return -1;
        }
        tmp = (pcb_t*)((uint32_t)tmp - PCB_SIZE);
    }

    page_process(shell_count); /*Page process is a void function, unless something goes horribly wrong I'll
								* leave it as such.*/
    //uint32_t paging_test = *(uint32_t*)0x083FFFFC;
    /*Need a file loader here.*/
    iret_addr = load_file(filename);
    /*Commented out for testing*/
    //iret_addr += PROGRAMADDR;
    
    // int usercs = USER_CS;
    
    /*Setting up the proper stack for iret*/

    /*Error, EIP, CS, EFLAGS, ESP, SS*/
    /*      Top                    Bottom*/
    /*Not sure if error is needed. It causes a GPF*/

    /*Changing TSS values*/
    /*You have to do this to ensure that when you switch back from privilage level 3, your
    * TSS is correct.
    */
    asm volatile(
        "xorl %%eax, %%eax;"
        "movl $" USER_DS_IASM ", %%eax;"
        "movl %%eax, %%ds"
        :
        :
        :"%eax"
        );
    tss.ss0 = KERNEL_DS;
    
    /*esp0 is the kernel stack of the new process, which is the bottom of an 8kB block.*/
    tss.esp0 = (uint32_t)tmp+PCB_SIZE;
    
    /*Fill in the new PCB.*/
    /*Our PCB doesn't have any human readable name right now. We'll worry about that later.*/
    tmp->pcb_n = shell_count+1;
    
    /*This is setup for the initial three terminals. The way it's set up is that we start in terminal 1.*/
    if(shell_count < 3)
    {
        tmp->alt_n = (tmp->pcb_n)-1;//N_TERM - shell_count;
        tmp->isOrphan = 1;
    }
    else
    {  
		pcb_t * prevtemp = find_pcb();
        tmp->alt_n = prevtemp->alt_n;
        tmp->isOrphan = 0;
        tmp->parent = prevtemp;
    }
    
    tmp->prevpcb = ESP;
    tmp->prevesp = prevesp;
    init_fd(tmp->fd_array);
    /*In theory, our prevpcb may be the end of the kernel memory. If that's the case, when
    * halt is called, we'll simply return back to the kernel.*/
    
    i = 0;
    while(args[i] != '\0')
    {
        if(args[i] == 0)
        {
            break;
        }
        tmp->progargs[i] = args[i];
        i++;
    }
    
    tmp->progargs[i] = '\0';
    uint32_t iret_esp;
    if(shell_count < 2)
    {
    /*This is setup for the initial three terminals.*/
        asm volatile
        (
            "pushl $" USER_DS_IASM ";"
            "pushl $" DEFESP ";"
            "pushfl;"
            "popl %%eax;"
            "orl $" SETINT ", %%eax;"
            "pushl %%eax;"
            "pushl $" USER_CS_IASM ";"
            "popl %%eax;"
            "orl $3, %%eax;"
            "pushl %%eax;"
            "pushl %1;"
            "pushal;"
            "movl %%esp, %0;"
            : "=g" (iret_esp)
            : "g" (iret_addr)
            : "%eax"
        );
        tmp->iret_esp = iret_esp;
        //tmp->eip_val = *((uint32_t*)iret_esp);
        execute((uint8_t*)"shell");
    }

    asm volatile
    (
        "pushl $" USER_DS_IASM ";"
        "pushl $" DEFESP ";"
        "pushfl;"
        "popl %%eax;"
        "orl $" SETINT ", %%eax;"
        "pushl %%eax;"
        "pushl $" USER_CS_IASM ";"
        "popl %%eax;"
        "orl $3, %%eax;"
        "pushl %%eax;"
        "pushl %0;"
        :
        : "g" (iret_addr)
        : "%eax"
    );
    /*iret. Interrupts are enabled due to the stack setup.*/
    asm volatile(
		"iret");

    
    /*We'll use a goto and come here after halt is called.*/
 
    asm volatile("stupid_goto_hack:");
    /*Halt is critical.*/
    //sti();

    uint32_t retval = halt_ret_val;
    return retval;
}

/*
    IN: fd - file descriptor - e.g.:  0 - stdin, 1 - stdout.
        buf - the buffer to read into
        nbytes - number of bytes to read.
    OUT:    Amount read on success, -1 on failure.
    DESCRIPTION:   Takes fd, go to appropriate operation based on the fd_array.
*/
int32_t
read(int32_t fd, void* buf, int32_t nbytes)
{
    // int ret_eip;
    // asm volatile
    // (
    //     "movl %%edi, %0;"
    //     : "=g" (ret_eip)
    // );
	if(fd==1 || buf==NULL)
    { //invalid fd
		return -1;
	}
	int bytes=0;
    int WTFFILESIZE = PD_SIZE;
	if((uint32_t)fd<FDT_SIZE)
    {
		pcb_t * pcb_pt = find_pcb();
  //       pcb_pt->eip_val = ret_eip;
		if(pcb_pt->fd_array[fd].flags==0)
        {
			return -1;
		}
		else
        {
			bytes = (((file_ops_t*) pcb_pt->fd_array[fd].file_op_p)->read)( (int32_t) buf, nbytes, &(pcb_pt->fd_array[fd]), WTFFILESIZE );
		}
	}
	else
    {
		return -1;
	}

    return bytes;
}

/*
    IN: filename - name of the file to open
    OUT:    fd on success, -1 on failure.
    OPEN: Find appropriate open function based on the filename. 
*/
int32_t
open(const uint8_t* filename)
{
    int i, j; //counter to be used for the for loops
    int nameLength = strlen((int8_t *)filename);
	fd_table_entry_t * fd_table_pcb= find_fd_table();
    if( filename==NULL || nameLength==0 ){
        return -1;
    }
    int strcomp = 1;
    uint32_t start_p = mod_start + INITIAL_DIRECTORY_OFFSET;
    for(i = 2; i < FD_ARRAY_SIZE; i++){
        if(fd_table_pcb[i].flags == 0){
            for(j = 0; j < dir_entries_fs; j++){
                strcomp = strncmp((const int8_t *) filename, (const int8_t *)start_p, 32);
                if(!strcomp){
                    int filetype = *(int32_t*)(start_p+32);
                    //reading a directory
                    if( filetype == FILE_TYPE_DIRECTORY){
						return directory_open((const int8_t  *) &i); //i is the fd
                    }
                    else if(filetype ==FILE_TYPE_RTC){
                        return rtc_open ((const int8_t *) &i);
                    }
                    else if(filetype == FILE_TYPE_REGFILE){

						fd_table_pcb[i].inode_p = *((char*)start_p +36);
                        return file_open((const int8_t *) &i);
                    }
                }
                start_p += NEXT_DIR_ENTRY;
            }
            return -1; // Could not find filename.
        }
    }

    return -1; // Could not find empty spot in FDT.
}

/*
    IN: fd - which to close
    OUT:    0 on success, -1 on failure.
    OPEN:   Finds the appropriate close to call based on the fops table.
*/
int32_t
close(int32_t fd)
{
    // int ret_eip;
    // asm volatile
    // (
    //     "movl %%edi, %0;"
    //     : "=g" (ret_eip)
    // );
	int32_t retval=0;
    if((uint32_t)fd<FD_ARRAY_SIZE && fd>1)
    {   
         pcb_t * pcb_pt = find_pcb();
        // pcb_pt->eip_val = ret_eip;
		if(pcb_pt->fd_array[fd].flags==0){
			return -1;
		}
		else{
            retval = (((file_ops_t*) pcb_pt->fd_array[fd].file_op_p)->close)(fd);
		}
    }
    else{
        return -1;
    }
    return retval;
}

/*INPUT : Pointer to a pre-allocated buffer, and the number of bytes we want to read.
* OUTPUT : 0 on success, -1 on failure.
* DESCRIPTION : Gets the argument that follows the command in the terminal.*/
int32_t getargs(uint8_t* buf, int32_t nbytes)
{
    if(buf==NULL)
    {
        return -1;
    }
    // Searching for the top pcb.
    int i=0;
    pcb_t * curr_pcb = find_pcb();
    /*(pcb_t*)(KERNEL_END - PCB_SIZE);

    while(curr_pcb->pcb_n != 0)
    {
        curr_pcb = (pcb_t*)((uint32_t)curr_pcb - PCB_SIZE);
    }

    curr_pcb = (pcb_t*)((uint32_t)curr_pcb + PCB_SIZE);*/
    // Done search.
    
    while( curr_pcb->progargs[i] != '\0' )
    {
        buf[i] = curr_pcb->progargs[i];
        i++;
    }
    buf[i] = '\0';
    
    return 0;
}

/*INPUT : Double pointer where we have to put in the page address into
* OUTPUT : Returns the page address that we have mapped
* DESCRIPTION : Grants the user access to the video memory indirectly. Basically, since
* the users don't have enough privilege to access the video memory, we make a new
* page directory entry, and a corresponding page that points to video memory. 
* We pick the page directory entry based on one that is first free when we traverse
* the table.*/
int32_t
vidmap(uint8_t** screen_start)
{
    //clear();
    //if invalid argument, to NULL or kernel memory
	flipColor();
    if(screen_start == 0x0 || screen_start == (uint8_t **)KERNEL_START){
        return -1;
    }
    
    int i;
    page_desc_t * new_pg_dir;
    page_desc_t * new_pg_table;

	//cr3 should be correct if this part of the code is executing

    asm volatile
    (
        "movl %%cr3, %0;"
        :"=r" (new_pg_dir)
    );

    new_pg_table = new_pg_dir + N_TABLES*PD_SIZE;

    // Set the first entry:
    new_pg_dir[FIRSTFREE].present = 1;
    new_pg_dir[FIRSTFREE].rw = 1;
    new_pg_dir[FIRSTFREE].us = 1;
    new_pg_dir[FIRSTFREE].write_thru = 1;
    new_pg_dir[FIRSTFREE].cache_disabled = 1;
    new_pg_dir[FIRSTFREE].accessed = 0;
    new_pg_dir[FIRSTFREE].zero_bit = 0;
    new_pg_dir[FIRSTFREE].page_size = 0;
    new_pg_dir[FIRSTFREE].ignored = 1;
    new_pg_dir[FIRSTFREE].avail = 0;
    new_pg_dir[FIRSTFREE].page_tbl_index = (((uint32_t)new_pg_table & MIDDLE_TEN) >> 12);
    new_pg_dir[FIRSTFREE].page_dir_index = (((uint32_t)new_pg_table & TOP_TEN) >> 22);
    // Initialize many 4-kb sized pages.  ignored at video memory locations.
	for(i=0;i<KB_SIZE;i++)
	{	
		 if((i*(KB_SIZE*PAGE_MULTIPLIER) < VIDEO + NUM_COLS*(NUM_ROWS+1)) && ((i*(KB_SIZE*PAGE_MULTIPLIER)) >= VIDEO)) 
		 {
			set_page_KB_on(new_pg_table[i], i*KB_SIZE*PAGE_MULTIPLIER); /*Video memory*/
			new_pg_table[i].us = 1; //set supervisor bit
			break;
		}
		else
		{
		 set_page_KB_off(page_table[i], i*KB_SIZE*PAGE_MULTIPLIER);
		}
	}

    /*asm volatile
    (
    "movl %%cr3, %%eax;"
    "movl %%eax, %%cr3;"
    :
    :
    : "%eax"
    );*/

    (*screen_start) = (uint8_t*) (PAGE_MULTIPLIER*FIRSTFREE*MB_SIZE)+i*KB_SIZE*PAGE_MULTIPLIER;
    return  (PAGE_MULTIPLIER*FIRSTFREE*MB_SIZE)+i*KB_SIZE*PAGE_MULTIPLIER;

}

int32_t set_handler(int32_t signum, void* handler_address)
{
    // pcb_t * tmp;
    // if(handler_address == NULL)
    // {
    //     tmp = find_pcb();
    //     tmp->signal_n = ALARM;
    //     /*We'll declare function pointers and stuff later for signals.*/
    //     //tmp->signal_hand = (uint32_t)sig_hand_p;
    // }

    // if(signum >= 0 && signum <= 4)
    // {
    //     /*We now have an address for our handler, and a signal number*/
    //     tmp = find_pcb();
    //     tmp->signal_n = signum;
    //     tmp->signal_addr = (uint32_t)handler_address;
    //     return 0;
    // }
    return -1;
}

int32_t sigreturn(void)
{
    return -1;
}

/*
*   IN: fd - fd of file to open.
*       buf - buffer to write.
*       nbytes - bytes to write.
*   OUT:    fd on success, -1 on failure.
*   OPEN:   write with appropriate function. Designated with the file ops structure.
*/
int32_t
write(int32_t fd, const void* buf, int32_t nbytes)
{
    // int ret_eip;
    // asm volatile
    // (
    //     "movl %%edi, %0;"
    //     : "=g" (ret_eip)
    // );
	if(fd==0)
    { //if invalid fd
		return -1;
	}
	if((uint32_t)fd<FDT_SIZE){
		pcb_t * pcb_pt =find_pcb();
  //       pcb_pt->eip_val = ret_eip;
		if(pcb_pt->fd_array[fd].flags==0)
        {
			return -1;
		}
		else
        {
			((((file_ops_t*) pcb_pt->fd_array[fd].file_op_p)->write)( (int32_t) buf, nbytes, &(pcb_pt->fd_array[fd])));
		}
	}
	else
    {
		return -1;
	}
    return 0;
}



/*INPUT : none
* OUTPUT : returns the pointer to the appropriate pcb structure
* DESCRIPTION : Finds the appropriate pcb structure based on the current process
* that is running. This is done by looking at the esp.*/
pcb_t *
find_pcb(void)
{
    pcb_t * pcb_pt;
	//finds the pcb based on the current esp
	asm volatile
	(
		"movl %%esp, %%esi;"
		"andl $0xFFFFE000, %%esi;"
		"subl $5, %%esi;"
		"movl %%esi, %0"
		: "=r" (pcb_pt)
		: /*"g" (pcb_pt) */
		: "%esi"
	);
	
	return pcb_pt;

}

/*INPUT : none
* OUTPUT : returns the current running terminal number 0, 1, or 2.
* DESCRIPTION : Finds the currently running program, and fetch the corresponding terminal for display.*/
uint32_t find_curprocess_num (void)
{
	pcb_t * pcb_pt = find_pcb();
	return pcb_pt->alt_n;
}
