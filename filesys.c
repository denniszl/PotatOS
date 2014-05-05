#include "filesys.h"

/*Global variables*/
uint32_t cur_directory_read=1; //read pointer that points to the last read location
uint32_t dir_entries_fs=0; //the directory entry we are reading
uint32_t mod_start; //Start of the file system. Grabbed when os was initialized

/*Define the file operations for all types*/
file_ops_t file_operations =
{
    .read = file_read,
    .write = file_write, 
    .open = file_open,  //  Called from system call already.
    .close = file_close,
};

file_ops_t stdin = 
{
    .read = trm_read,
    // .write = trm_write,
    .open = trm_open,
    .close = trm_close,
};

file_ops_t stdout = 
{
    // .read = trm_read,
    .write = trm_write,
    .open = trm_open,
    .close = trm_close,
};

file_ops_t rtc_ops = 
{
    .read = rtc_read,
    .write = rtc_write,
    .open = rtc_open,
    .close = rtc_close,
};

file_ops_t file_directory =
{
    .read = directory_read,
    .write = directory_write,
    .open = directory_open,
    .close = directory_close,
};

/*INPUT : None
  OUTPUT : The fd table, based on the current process that is running
  DESCRIPTION : This function will find the current fd table based on the process
  that is currently running. i.e. the current value inside of esp.*/
fd_table_entry_t * find_fd_table(void)
{
	/*find the fd_table*/
	pcb_t * pcb_pt;
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
	fd_table_entry_t * fd_table_pcb = pcb_pt->fd_array;
	return fd_table_pcb;
}
/*INPUT : fd index to fill in
* OUTPUT : file descriptor index on success, -1 otherwise
* DESCRIPTION : Put in relevant information into the array. See 7.2
*Note : doesn't need to do anything for 3.2. For now, we can actually just not
*pass in anything*/
int
file_open(const int8_t * fd)
{
	fd_table_entry_t * fd_table_pcb= find_fd_table();
	int index = (*fd);
	fd_table_pcb[index].file_op_p = (uint32_t)&file_operations; 
	fd_table_pcb[index].file_position = 0;
	fd_table_pcb[index].flags = 1;
	return index;

}

/*INPUT : 32bit buffer, number of bytes to read, the fd table, size of the buffer
* OUTPUT : number of successful bytes read
*Effects : writes to the buffer*/
int
file_read(int32_t buf, int32_t count, fd_table_entry_t * fd, int buf_size)
{
    if(fd->flags != 1) /*Simple error checking*/
        return -1;
    int i;
    int j;

    int index = (fd->inode_p);
    int valid_blocks;
    int curr_data_index;
    char * curr_data;
    int n_inodes = *(uint32_t*)(mod_start+DIR_ENTRIES_OFFSET);
    int n_read = 0;

    int n_blocks = *(uint32_t*)(mod_start+DIRENT_INODE_OFFSET);
    
    uint32_t start_p = mod_start+FS_MEM_SIZE; /*This will be the first inode.*/
    start_p += FS_MEM_SIZE*(index);
    valid_blocks = (*((int32_t*)start_p)/FS_MEM_SIZE)+1;
    int data_len = (*(int32_t*)start_p);

    if(fd->file_position == data_len)
    {
        return 0;
    }
    start_p += START_P_OFFSET;
    curr_data_index = *((int32_t*)(start_p));
    if(curr_data_index > n_blocks-1)
    {
        return -1; /*Error checking*/
    }
    /*Pointer to current block*/
    curr_data = (char*) (mod_start+(FS_MEM_SIZE*(n_inodes+1))+(curr_data_index*FS_MEM_SIZE));
	for(j = 0; j < valid_blocks; j++)
	{
		for(i = 0; i < FS_MEM_SIZE; i++)
		{
			if(fd->file_position == data_len)
			{
				((char*)buf)[n_read] = '\0';
				return n_read;
			}
			
			if(n_read == count-1  )
			{
				((char*)buf)[n_read] = (curr_data[fd->file_position - j*FS_MEM_SIZE]);
				fd->file_position++;
				n_read++;
				return n_read;
			}
			
			if(fd->file_position >= ((j+1)*FS_MEM_SIZE)-1)
				break;


				((char*)buf)[n_read] = (curr_data[fd->file_position - j*FS_MEM_SIZE]);
			
			fd->file_position++;
			n_read++;
		}
		/*Get the next data block.*/
		start_p += START_P_OFFSET;
		curr_data_index = *((uint32_t*)(start_p));
		if(curr_data_index > n_blocks-1)
		{
			return -1; /*Error checking*/
		}
		curr_data = (char*) (mod_start+(FS_MEM_SIZE*(n_inodes+1))+(curr_data_index*FS_MEM_SIZE));
	}
    return 0;
    
}

/*INPUT : buffer and count. Dummies. Don't actually do anything
*OUTPUT : NONE
*EFFECT : NONE
*We can't write to the file system, so just print out an error message and return -1*/
int
file_write(int32_t buffer, int32_t count, fd_table_entry_t * fd)
{
    quickStat(viewTerm);
    printf("Filesystem is read-only.");
    exitStatMode(viewTerm);
	return -1;
}

/*INPUT : fd, file descriptor, index to the array and also the fd information
*OUTPUT : 0 on success, -1 otherwise
*EFFECTS : Removes an entry from the fd array*/
int
file_close(int8_t fd)
{
	fd_table_entry_t *fd_table_pcb= find_fd_table();
    fd_table_pcb[(int)fd].file_op_p = 0;
    fd_table_pcb[(int)fd].inode_p = 0;
    fd_table_pcb[(int)fd].file_position = 0;
    fd_table_pcb[(int)fd].flags = 0;
    return 0;
}


/*INPUT : inode number, offset, buffer to write to, and length of bytes to read
*OUTPUT : Populates the buffer passed in, passes back number of bytes read
*Effect : Moves the read pointer, reads data into the buffer passed, amount read is based on length
*Returns the number of bytes read (basically the number of characters since
*each character is a byte)*/
int32_t
read_data(uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length, int buf_size)
{
    int curr_data_index;
    char * curr_data;
    int i;
    int j;
    int file_position = offset;
    int n_read = 0;
    int n_inodes = *(uint32_t*)(mod_start+DIR_ENTRIES_OFFSET);
    int n_blocks = *(uint32_t*)(mod_start+DIRENT_INODE_OFFSET);
    uint32_t start_p = mod_start+(FS_MEM_SIZE*(inode+1)); /*The inode in question.*/
    if(inode > n_inodes)
    {
        return -1;
    }
    int data_len = *(uint32_t*)(start_p);
    int valid_blocks = (data_len/FS_MEM_SIZE)+1;
    start_p += START_P_OFFSET; /*The first index of the data blocks.*/
    curr_data_index = *((int32_t*)(start_p));
    curr_data = (char*) (mod_start+(FS_MEM_SIZE*(n_inodes+1))+(curr_data_index*FS_MEM_SIZE));

    if(buf_size < length)
    {
        for(j = 0; j < valid_blocks; j++)
        {
            for(i = 0; i < FS_MEM_SIZE; i++)
            {
                if(file_position == data_len-1)
                    return 0;
                if(n_read == buf_size)
                    return n_read;
                if((j*FS_MEM_SIZE)+i+offset >= (j+1)*FS_MEM_SIZE)
                    break;

                
                if(((int)(offset - j*FS_MEM_SIZE)) >= 0)
                    buf[i+(j*FS_MEM_SIZE)] = (curr_data[i+(offset-j*FS_MEM_SIZE)]);
                else
                    buf[i+(j*FS_MEM_SIZE)] = (curr_data[i]);

                file_position++;
                n_read++;
            }
            /*Get the next data block.*/
            start_p += START_P_OFFSET;
            curr_data_index = *((int32_t*)(start_p));
            if(curr_data_index > n_blocks-1)
            {
                return -1; /*Error checking*/
            }
            printf("curr_data_index:%d\n", curr_data_index);
            curr_data = (char*) (mod_start+(FS_MEM_SIZE*(n_inodes+1))+(curr_data_index*FS_MEM_SIZE));
        }
        return buf_size;
    }
    else
    {
        for(j = 0; j < valid_blocks; j++)
        {
            for(i = 0; i < FS_MEM_SIZE; i++)
            {
                if(file_position == data_len-1)
                    return 0;
                
                if(n_read == length)
                    return n_read;
                
                if((j*FS_MEM_SIZE)+i+offset >= (j+1)*FS_MEM_SIZE)
                    break;

                
                if(((int)(offset - j*FS_MEM_SIZE)) >= 0)
                    buf[i+(j*FS_MEM_SIZE)] = (curr_data[i+(offset-j*FS_MEM_SIZE)]);
                else
                    buf[i+(j*FS_MEM_SIZE)] = (curr_data[i]);
                
                file_position++;
                n_read++;
            }
            /*Get the next data block.*/
            start_p += START_P_OFFSET;
            curr_data_index = *((uint32_t*)(start_p));
            if(curr_data_index > n_blocks-1)
            {
                return -1; /*Error checking*/
            }
            curr_data = (char*) (mod_start+(FS_MEM_SIZE*(n_inodes+1))+(curr_data_index*FS_MEM_SIZE));
        }
        return length;
    }
}

/*INPUT : The fd index that we're supposed to fill in the entry for
* OUTPUT : returns the fd index
* Effect : Opens the file directory*/
int
directory_open(const int8_t* fd)
{
	fd_table_entry_t * fd_table_pcb= find_fd_table();
	int fd_int = *fd;
	fd_table_pcb[fd_int].file_op_p = (uint32_t)&file_directory;
	fd_table_pcb[fd_int].file_position = 0;
	fd_table_pcb[fd_int].flags = 1;
	return fd_int;
}

/*INPUT : buffer, and count but count doesn't really do anything?
*OUTPUT : Returns 0 if read last entry in directory, -1 otherwise
*EFFECT : Writes the file name into the buffer*/
int
directory_read(int32_t buffer, int count, fd_table_entry_t * fd, int buf_size)
{
    if(cur_directory_read>=dir_entries_fs){ //if we read all the directories already
		//cannot read anymore, but reset the directory pointer
		cur_directory_read=1;
        return 0; //cannot read anymore
    }
    dentry_t dentry;
    int retval=read_dentry_by_index(cur_directory_read,&dentry);
    if(retval==-1){
		return -1; //fail read
	}
	cur_directory_read+=1; //increment
	return dname_copy((int8_t *)buffer, (int8_t *)(dentry.filename),count);
	
}

/*INPUT : buffer, count
*OUTPUT : None
*EFFECT : None. We can't write to directory. Just return -1.*/
int
directory_write(int32_t buffer, int32_t count, fd_table_entry_t * fd)
{
	return -1;
}

/*INPUT : FD index
*OUTPUT : None
*Effect : Closes the directory; removing the entry from the fd table.*/
int
directory_close(int8_t fd)
{
	if(fd>=FDT_SIZE || fd<0){
		return -1;
	}
	fd_table_entry_t *fd_table_pcb= find_fd_table();
	if(fd_table_pcb[(int)fd].flags == 0){
		return -1; //already closed
	}
    fd_table_pcb[(int)fd].file_op_p = 0;
    fd_table_pcb[(int)fd].inode_p = 0;
    fd_table_pcb[(int)fd].file_position = 0;
    fd_table_pcb[(int)fd].flags = 0;
	return 0;
}

/*INPUT : name of file, pointer to entry
*OUTPUT : returns -1 on failure
*EFFECTS : Fills up the dentry
*Abstracts away the system memory*/
int32_t
read_dentry_by_name(const uint8_t * fname, dentry_t * dentry)
{
	if(dentry == NULL){
		return -1; //can't read into non existence dentry
	}
	//calculate starting address
	uint32_t start_p = mod_start + INITIAL_DIRECTORY_OFFSET + DENTRY_SIZE; //skip the first directory (Self)
	int i;
	for(i=1;i<dir_entries_fs;i++){ //i is reading the entry name
		uint32_t length = strlen((int8_t *) fname); 
		if(strncmp((int8_t *)fname, (int8_t *) start_p, length)==0){
			strcpy(((int8_t *)dentry->filename), ((int8_t *)start_p));
			start_p+=FILE_NAME_SIZE;
			dentry->file_type = (*(uint32_t *)start_p);
			start_p+=FILE_TYPE_SIZE;
			dentry->inode = (*(uint32_t *)start_p);
			return 0;
		}
		start_p+=DENTRY_SIZE; //move to next entry
	}
	return -1; //not found case
}

/*INPUT : index to read from, pointer to dentry
*OUTPUT : returns -1 on failure
*EFFECTS : Fills up the dentry
*Abstracts away the memory level*/
int32_t
read_dentry_by_index(uint32_t index, dentry_t * dentry)
{
	if((index)>=dir_entries_fs || dentry ==NULL){ //if we read all the directories already, or if dentry is not valid
		return -1; //cannot read anymore
	}
	//find where we are starting to read
	uint32_t start_p = mod_start + INITIAL_DIRECTORY_OFFSET+(index*DENTRY_SIZE);
	strcpy(((int8_t *)dentry->filename), ((int8_t *)start_p));
	start_p+=FILE_NAME_SIZE;
	dentry->file_type = (*(uint32_t *) start_p);
	start_p+=FILE_TYPE_SIZE;
	dentry->inode = (*(uint32_t *)start_p);
	//rest 24 bytes are reserved

	return 0;
}

/*INPUT : value to load into the globals
*OUTPUT : none
*Effects : fills in the global variables in this file. this function is called at the very beginning of booting up the OS in kernel.c*/
void
fill_in_globals_fs(uint32_t mod_start_g)
{
	mod_start = mod_start_g;
	dir_entries_fs = *((uint32_t*)mod_start_g);
}

/*INPUT : directory name to copy from and to
*OUTPUT : number of bytes copied
*EFFECTS : Copies over the string from one buffer to another*. This is used instead of string copy because we don't want to treat our filenames as null terminaled.*/
int32_t
dname_copy(int8_t* dest, const int8_t* src, uint32_t n)
{
	//clean the buffer first
	int j=0;
	while(j<n){
		dest[j] = '\0';
		j++;
	}
	int32_t i=0;
	while(src[i] != '\0' && i <= n) {
		dest[i] = src[i];
		i++;
	}
	return i;
}

/*INPUT : passes in the fd table to initialize
*OUTPUT : none
*EFFECTS : Makes stdin stdout in use. The 0th and 1st entry of the fd_table.
*Initializes file descriptors*/
 void init_fd(fd_table_entry_t * fd_table)
 {   
     fd_table[STDIN].file_op_p = (uint32_t)&stdin;
     fd_table[STDIN].inode_p = 0;
     fd_table[STDIN].file_position = 0;
     fd_table[STDIN].flags = 1;

     fd_table[STDOUT].file_op_p  = (uint32_t)&stdout;
     fd_table[STDOUT].inode_p = 0;
     fd_table[STDOUT].file_position = 0;
     fd_table[STDOUT].flags = 1;
	 //clean out the fd table
	 int i=2;
	 for(i=2;i<FDT_SIZE;i++){
	 fd_table[i].file_op_p  = 0;
     fd_table[i].inode_p = 0;
     fd_table[i].file_position = 0;
     fd_table[i].flags = 0;
	 }
 }
/*INPUT : Filename we're trying to find the inode of
* OUTPUT : -1 on failure, otherwise the actual inode number.
* EFFECTS : None. This is just a function to find the inode number given a filename.*/
int
find_inode(const char * filename)
{
    int i;
    int j;
    int strcomp = 1;
    uint32_t start_p = mod_start + INITIAL_DIRECTORY_OFFSET;
    for(i = 2; i < FDT_SIZE; i++)
    {
		for(j = 0; j < dir_entries_fs; j++)
		{
			strcomp = strncmp(filename, (char*)start_p, 32);
			if(!strcomp)
			{
				return *((char*)start_p+36); 
			}
			start_p += NEXT_DIR_ENTRY;
		}
		return -1; // Could not find filename.
    }
    return -1; // Could not find empty spot in FDT.
}

/*INPUT : NONE
* OUTPUT : 0 if there is still space in the fd_table, -1 if there is no more space left.
* DESCRIPTION : Checks if there's space in the fd table. It walks through the fd_table until it finds an available space or until it reaches the end of the table. Looks at the flags bit to see if available.*/
int
check_fd_table(void)
{
	fd_table_entry_t * fd_table_pcb =  find_fd_table();
	int i;
	for(i = 2; i < FDT_SIZE; i++)
    {
        if(fd_table_pcb[i].flags == 0)
        {
			return 0; //healthy value
		}
	}
	return -1; //no more space
}
/*Input character pointer to a string
* output: is the file associated with the filename an executable
* DESCRIPTION : none
* Checks if file is an executable*/
int
checkexec(char * filename)
{
	int space = check_fd_table();
	//no more space
	if(space==-1){
		return -1;
	}
    int n_inodes = *(uint32_t*)(mod_start+DIR_ENTRIES_OFFSET);
    int n_blocks = *(uint32_t*)(mod_start+DIRENT_INODE_OFFSET);
	/*Examine the first four characters in the file to see if it's an executable*/
    int8_t elf_mn[4];
    elf_mn[0] = 0x7F;   // Magic
    elf_mn[1] = 0x45;   // 'E'
    elf_mn[2] = 0x4c;   // 'L'
    elf_mn[3] = 0x46;   // 'F'
                        // Roll 1d20 for initiative.

    int index = find_inode((const char *) filename);
    if(index > n_inodes)
    {
        return -1;
    }
    uint32_t start_p = mod_start+(FS_MEM_SIZE*(index+1)); /*The index(inode) in question.*/

    start_p += START_P_OFFSET;

    int curr_data_index = *((int32_t*)(start_p));
    if(curr_data_index > n_blocks)
    {
        return -1;
    }
    
    char * curr_data = (char*) (mod_start+(FS_MEM_SIZE*(n_inodes+1))+(curr_data_index*FS_MEM_SIZE));

    if(!strncmp(elf_mn, curr_data, 4))
    {
        return 1;
    }
    return 0;
}

/*Input character pointer to a string
* output: the address of where you need to iret
* EFFECTS : Loads program image into memory
* Loads program image into memory*/
uint32_t
load_file(char* filename)
{
    int retval;
    int inode = find_inode((const char *) filename);

    retval = read_data(inode, 0,(unsigned char*)(PROGRAMADDR), MB_F, MB_F);

    // printf("retval:%d\n", retval);

    return *(uint32_t*)(PROGRAMADDR+RESERVE_LEN);
}
