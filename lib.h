/* lib.h - Defines for useful library functions
 * vim:ts=4 noexpandtab
 */

#ifndef _LIB_H
#define _LIB_H

#include "types.h"
#include "extralib.h"

#define MAX_BUF_SIZE 128
#define VIDEO 0xB8000
#define NUM_COLS 80
#define NUM_ROWS 24
#define ATTRIB 0x0a
 
#define VGA_IN 0x3D4
#define VGA_DATA 0x3D5
#define TERMS 3
#define FOURKILOBYTES 4096

char videomem[TERMS][FOURKILOBYTES*2];
int curr_task;

typedef enum colors{
	BLACK,
	DBLUE,
	DGREEN,
	DCYAN,
	DRED,
	DMAGENTA,
	ORANGE,
	LGREY,
	DGREY,
	LBLUE,
	LGREEN,
	LCYAN,
	LRED,
	LMAGENTA,
	YELLOW,
	WHITE
} colors_t;
/* 4-bits: 0=black, 1=blue, 2=green, 3=cyan, 4=red, 5=magenta, 6=orange, 7=grey, 
 8=darkGrey, 9=lightBlue, a=lightGreen, b=lightCyan, c=lightRed, d=lightMagenta, e=yellow, f=white*/

int32_t printf(int8_t *format, ...);
void putc(uint8_t c);
void putc_buf(uint8_t c, uint32_t loc);
int32_t puts(int8_t *s);
int8_t *itoa(uint32_t value, int8_t* buf, int32_t radix);
int8_t *strrev(int8_t* s);
uint32_t strlen(const int8_t* s);
void clear(void);
void flipColor(void);

void test_interrupts(void);

void* memset(void* s, int32_t c, uint32_t n);
void* memset_word(void* s, int32_t c, uint32_t n);
void* memset_dword(void* s, int32_t c, uint32_t n);
void* memcpy(void* dest, const void* src, uint32_t n);
void* memmove(void* dest, const void* src, uint32_t n);
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n);
int8_t* strcpy(int8_t* dest, const int8_t*src);
int8_t* strncpy(int8_t* dest, const int8_t*src, uint32_t n);

/* Userspace address-check functions */
int32_t bad_userspace_addr(const void* addr, int32_t len);
int32_t safe_strncpy(int8_t* dest, const int8_t* src, int32_t n);

// Position transfer functions
void flipCurs(void);
void saveCurs(void);

// Mostly taken from:
//     http://www.osdever.net/bkerndev/Docs/printing.htm
// Move blinky cursor:
void move_csr(void);
// Set attribute:
// Mnemonic:  <fg> on <bg>.
void set_attrib(unsigned char fgc, unsigned char bgc);
// Useful to fast restore attributes.
void set_char_attrib(char newAttr);
// Get current attributes:
char get_attrib(void);
uint8_t get_attrib_bg(void);
uint8_t get_attrib_fg(void);

// Scroll:
void scroll(void);
void scroll_b(uint32_t loc);
// Cursor x,y
int getScreen_x(int loc);
int getScreen_y(int loc);
void setScreen_x(int newX, int loc);
void setScreen_y(int newY, int loc);

//view term
extern int viewTerm;


/* Port read functions */
/* Inb reads a byte and returns its value as a zero-extended 32-bit
 * unsigned int */
static inline uint32_t inb(port)
{
	uint32_t val;
	asm volatile("xorl %0, %0\n \
			inb   (%w1), %b0" 
			: "=a"(val)
			: "d"(port)
			: "memory" );
	return val;
} 

/* Reads two bytes from two consecutive ports, starting at "port",
 * concatenates them little-endian style, and returns them zero-extended
 * */
static inline uint32_t inw(port)
{
	uint32_t val;
	asm volatile("xorl %0, %0\n   \
			inw   (%w1), %w0"
			: "=a"(val)
			: "d"(port)
			: "memory" );
	return val;
}

/* Reads four bytes from four consecutive ports, starting at "port",
 * concatenates them little-endian style, and returns them */
static inline uint32_t inl(port)
{
	uint32_t val;
	asm volatile("inl   (%w1), %0"
			: "=a"(val)
			: "d"(port)
			: "memory" );
	return val;
}

/* Writes a byte to a port */
#define outb(data, port)                \
do {                                    \
	asm volatile("outb  %b1, (%w0)"     \
			:                           \
			: "d" (port), "a" (data)    \
			: "memory", "cc" );         \
} while(0)

/* Writes two bytes to two consecutive ports */
#define outw(data, port)                \
do {                                    \
	asm volatile("outw  %w1, (%w0)"     \
			:                           \
			: "d" (port), "a" (data)    \
			: "memory", "cc" );         \
} while(0)

/* Writes four bytes to four consecutive ports */
#define outl(data, port)                \
do {                                    \
	asm volatile("outl  %l1, (%w0)"     \
			:                           \
			: "d" (port), "a" (data)    \
			: "memory", "cc" );         \
} while(0)

/* Clear interrupt flag - disables interrupts on this processor */
#define cli(void)                           \
do {                                    \
	asm volatile("cli"                  \
			:                       \
			:                       \
			: "memory", "cc"        \
			);                      \
} while(0)

/* Save flags and then clear interrupt flag
 * Saves the EFLAGS register into the variable "flags", and then
 * disables interrupts on this processor */
#define cli_and_save(flags)             \
do {                                    \
	asm volatile("pushfl        \n      \
			popl %0         \n      \
			cli"                    \
			: "=r"(flags)           \
			:                       \
			: "memory", "cc"        \
			);                      \
} while(0)

/* Set interrupt flag - enable interrupts on this processor */
#define sti(void)                           \
do {                                    \
	asm volatile("sti"                  \
			:                       \
			:                       \
			: "memory", "cc"        \
			);                      \
} while(0)

/* Restore flags
 * Puts the value in "flags" into the EFLAGS register.  Most often used
 * after a cli_and_save_flags(flags) */
#define restore_flags(flags)            \
do {                                    \
	asm volatile("pushl %0      \n      \
			popfl"                  \
			:                       \
			: "r"(flags)            \
			: "memory", "cc"        \
			);                      \
} while(0)

#endif /* _LIB_H */
