#include "terminal.h"
#include "lib.h"
#include "key_init.h"

//static char readChar[TRM_COUNT][MAX_BUF_SIZE+1]; //keyboard buffer
// Read from keyboard buffer.
// IN:  destString-read from this array
//      length - how many to read
// OUT: number bytes read, -1 on failure.
int viewTerm=2;
int lock_vt=0;
int trm_read(int32_t destString, int length, fd_table_entry_t * fd, int buf_size)
{
    int i=0;
    
	char * cString = (char *) destString;
	
    for( ; i<MAX_BUF_SIZE; i++)
    {
        cString[i]='\0';
    }
    
    i=0;
    if(length>MAX_BUF_SIZE)
        length = MAX_BUF_SIZE;
    else if(length<0)
        return -1;
    
    while( i<length && getKeyLast()!='\n' )
    {
        i = getKeyIndex();
    }

    memcpy(cString, getKBufPointer(), i);
    resetKeyBuf();
    
    if(i!=length)
        cString[i] = '\n';
    
    destString = (int32_t) cString;
    //printf("val:%d\n", i);
    clearStat(viewTerm);
    
    return i+1;
}

/* INPUT:  inString - write this array
* OUTPUT: Returns 0.
* DESCRIPTION : Writes to the appropriate location, depending on which process called the function
* this function will write to different places. If caller process is currently visible, this 
* function will call putc and write to video memory. Otherwise, it will write to the buffer 
* allocated for * the other terminals that are currently not visible*/
int
trm_write(int32_t inString, int32_t nbytes, fd_table_entry_t * fd)
{
	cli();
    int i = 0;
    //viewTerm is visible terminal. loc is the terminal the running process should be displaying in.
    pcb_t * pcb_p = find_pcb();
    int loc;
    loc = pcb_p->alt_n;
    for(i = 0; i<nbytes; i++)
    {
        if(loc==viewTerm)
        {
			putc(((char*)inString)[i]);
		}
		else /*if(loc >= 0 && loc < 3)*/
        {
			putc_buf(((char*)inString)[i], loc);
		}
    }
	
    return 0;
}

/*INPUT : the fd number to open the terminal in
* OUTPUT : 0 on success, -1 on failure.
* DESCRIPTION: Fills in the appropriate slot in the fd_array with terminal functions. 
* The available fd number is found via another function*/
int trm_open( const int8_t * fd )
{

    return 0;
}


/*
    IN:    fd-file descriptor
    OUT:   NO
    WHAT:  close it
*/
int trm_close( int8_t fd )
{
	if(fd==1 || fd==0){
		return -1;
	}
	fd_table_entry_t *fd_table_pcb= find_fd_table();
    fd_table_pcb[(int)fd].file_op_p = 0;
    fd_table_pcb[(int)fd].inode_p = 0;
    fd_table_pcb[(int)fd].file_position = 0;
    fd_table_pcb[(int)fd].flags = 0;
    return 0;
}

/*
    IN:    alt_n-which terminal
    OUT:   NO
    WHAT:  search for appropriate terminal process.
*/
int
PCB_trm_search(int alt_n)
{
	if(alt_n == viewTerm){ //if we're already in the same terminal
		return 0;
    }
    cli();
    pcb_t * tmp;
    
    int count;
    int i;
    // uint32_t iret_esp;
    // uint32_t eip_val;
    pcb_t* temp;
    tmp = (pcb_t *)(KERNEL_END - PCB_SIZE);
    count = 0;
    for(i = 0; i < 6; i++)
    {
        if(tmp->alt_n == alt_n && tmp->pcb_n != 0) 
        {
            temp = tmp;
        } 
        count++;
        tmp = (pcb_t*)((uint32_t)tmp - PCB_SIZE);
    } 
    // iret_esp = temp->iret_esp;
    // eip_val = temp->eip_val;
    // printf("eip_val:%x\n", eip_val);
    //int pcb_n = tmp->pcb_n;
    /*temp is the last PCB that had the alt_n we're interested in.*/ 
    /*This will do some kind of cr3 change and memcpy to video memory.*/
    /*Changing video memory should be critical.*/
    change_active_term((temp->pcb_n)-1, alt_n);
    return 0;
}
