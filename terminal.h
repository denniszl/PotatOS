#ifndef TERMINAL_H
#define TERMINAL_H

#include "key_init.h"
#include "x86_desc.h"
#include "filesys.h"

// OPS:
extern int trm_read(int32_t destString, int length, fd_table_entry_t * fd, int buf_size);
extern int trm_write(int32_t inString, int nbytes, fd_table_entry_t * fd);
extern int trm_open( const int8_t* fd );
extern int trm_close( int8_t fd );

// extern int trm_cmd_clear(char * cmd);
extern int PCB_trm_search(int alt_n);

#endif
