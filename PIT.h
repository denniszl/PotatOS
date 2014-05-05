#ifndef _PIT_H
#define _PIT_H

#include "i8259.h"
#include "lib.h"
#include "x86_desc.h"
#include "syscall.h"
#include "schedule.h"

#define IRQ_PIT 0
#define PIT_COM 0x43    // command register
#define PIT_C0 0x40 // Regular data port. The C is for channel.
#define PIT_C2 0x42 // Speaker port

/*
    - Channel 0 is regular ints, Channel 2 is to PC-speaker.  [7:6]
        = 1 is deprecated probably. 4 is the read-back command (Most likely not useful for this project.)
    - Access mode:  which bytes to get from the data-port.  [5:4]
        = masked hi:lo, 00 is not useful for this project.
    - Operating mode:  [3:1]
        = 000, interrupt on terminal count:
            > When counts down to 0, resets counter high, until reload reg is reset.
        = 001, hardware-retriggerable one-shot:
            > Like above, except count condition is rising edge of gate input.
        = x10, rate generator:
            > Frequency divider with short pulses.
        = x11, square-wave generator:
            > Like above, but fed to flip-flop for square wave.
        = 100, software-triggered strobe:
            > Too lazy to read.
        = 101, hardware-triggered strobe:
            > Too lazy to read.
    - BCD/Binary [0]
        = 0 binary (16-bits)
        = 1 BCD (4-digits)
*/
        
#define STD_COM 0x34  // Channel 0, lo-hi, rate-generator, bin, 0b00110100
#define SPK_COM 0xb6  // Channel 2, lo-hi, square wave, bin, 0b10110110

// RLD values:
#define RL_FMIN 0x10000
#define RL_FMAX 1
// Freq limits:
#define FMIN 18
#define FMAX 0x1234dd

// Initialiser:
void PIT_init();
// Changer:
void timer_phase(int hz);
// Interrupt handler:
void PIT_c(void);
void timer_wait(int ticks);
// Operations:
/*int PIT_read(int32_t buf, int32_t count, fd_table_entry_t * fd, int buf_size);
int PIT_write(int32_t r, int32_t count, fd_table_entry_t * fd);
int PIT_open(const int8_t * fd);
int PIT_close(int8_t fd);*/

int getCurrentHz();

#endif
