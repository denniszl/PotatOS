#ifndef RTC_H
#define RTC_H

#include "i8259.h"
#include "lib.h"
#include "x86_desc.h"
#include "filesys.h"
#define IRQ_IR 8
#define REGISTERIN 0x70
#define REGISTEROUT 0x71
#define REGA 0x8A
#define REGB 0x8B
#define REGC 0x0C
#define rtcopen 0x01
#define default_rate 0x0000000F


/*Initializes the RTC*/
void rtc_init(void);
/*Interrupt handler for the RTC*/
void rtc_c(void);
// Operations:
int rtc_read(int32_t buf, int32_t count, fd_table_entry_t * fd, int buf_size);
int rtc_write(int32_t r, int32_t count, fd_table_entry_t * fd);
int rtc_open(const int8_t * fd);
int rtc_close(int8_t fd);

#endif
