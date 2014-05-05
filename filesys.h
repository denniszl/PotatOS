#ifndef FILESYS_H
#define FILESYS_H
#include "lib.h"
#include "multiboot.h"
//Drivers for fops table.
#include "terminal.h"
#include "rtc.h"
#include "x86_desc.h"
#include "syscall.h"
#include "paging.h"

/*Define necessary constants*/
#define INITIAL_DIRECTORY_OFFSET 64
#define FILE_NAME_SIZE 32
#define FILE_TYPE_SIZE 4
#define DIRENT_INODE_OFFSET 8
#define DIR_ENTRIES_OFFSET 4
#define NAME_LEN 32
#define START_P_OFFSET 4
#define NEXT_DIR_ENTRY 64
#define STDIN 0
#define STDOUT 1
#define FS_MEM_SIZE 4096
#define DENTRY_SIZE 64
#define DIR_ENTRIES_SIZE 4
#define RESERVE_LEN 24
#define FDT_SIZE 8
#define FILE_TYPE_RTC 0
#define FILE_TYPE_DIRECTORY 1
#define FILE_TYPE_REGFILE 2
//see appendex A 7.1
/*directory entry. Each has a specific amount of occupying space*/
typedef struct dentry
{
	unsigned char filename[NAME_LEN]; //has to occupy 32 bytes
	uint32_t file_type; //4 bytes
	uint32_t inode; //inode number, 4 bytes
	unsigned char reserved[RESERVE_LEN];
} dentry_t;


/*Holds function pointers to read, write, open, and close*/
/*Different read/write has different parameters*/
typedef struct file_ops
{
	int (*read)(int32_t buffer, int32_t count, fd_table_entry_t * fd, int buf_size);
	int (*write)(int32_t buffer, int32_t count, fd_table_entry_t * fd);
	int (*open)(const int8_t* fd);
	int (*close)(int8_t fd);
}file_ops_t;

extern file_ops_t file_operations;
extern file_ops_t stdin;
extern file_ops_t stdout;
/*See 7.2 for diagram*/
fd_table_entry_t fd_table[FDT_SIZE];

/*Read the entry by name*/
int32_t read_dentry_by_name(const uint8_t * fname, dentry_t * dentry);
/*Read entry by index*/
int32_t read_dentry_by_index(uint32_t index, dentry_t * dentry);
/*Read the data from the entry*/
int32_t read_data (uint32_t inode, uint32_t offeset, uint8_t * buf, uint32_t length, int buf_size);
/*Loads appropriate information into array*/
int file_open(const char * filename);
/*Reads specified file*/
int file_read(int32_t buf, int32_t count, fd_table_entry_t * fd, int buf_size);
/*Can't actually write, but here it is*/
int file_write(int32_t buffer, int32_t count, fd_table_entry_t * fd);
/* Close fs */
int file_close(int8_t fd);
/*will open the directory*/
int directory_open(const int8_t* fd);
/*Will read from the directory*/
int directory_read(int32_t buffer, int count, fd_table_entry_t * fd, int buf_size);
/*Will write to the directory, which we can't do.*/
int directory_write(int32_t buffer, int32_t count, fd_table_entry_t * fd);
/*Will not do anything. But supposedly closes the directory*/
int directory_close(int8_t fd);
/*Fills in the global variables*/
void fill_in_globals_fs(uint32_t mod_start_g);
/*copies over directory name without null termination*/
int32_t dname_copy(int8_t* dest, const int8_t* src, uint32_t n);
/*Helper function for checkexec*/
int find_inode(const char * filename);
/*Find the fdtable*/
fd_table_entry_t * find_fd_table(void);


void init_fd(fd_table_entry_t * fd_table);

int checkexec(char * filename);

uint32_t load_file(char * filename);


#endif
