#ifndef KEY_INIT_H
#define KEY_INIT_H

#include "i8259.h"
#include "lib.h"
#include "mathlib.h"
#include "syscall.h"
#include "terminal.h"
#define KEYBOARD_IR 1
#define MOUSE_IR 12

extern int viewTerm;

/*Initializes the keyboard. Enables interrupts for keyboard*/
void keyboard_init(void);
// Altcode buffer to -1.
void altSet();
// Return the last character pushed by keyboard.
extern char getKeyLast(void);
// Return the location of the readChar buffer.
extern char * getKBufPointer(void);
// Get rIndex.
extern int getKeyIndex(void);
// Reset rIndex
extern void resetKeyBuf(void);

void save_esp(uint32_t esp);


/*Keyboard interrupt handler*/
void key_c(void);

// void clrBuf(void);

// The buffer.
// extern char readChar[128];
// extern int rIndex;        // Current character position.

// Flags for lct 0x9d | lsh 0xaa | rsh 0xb6 
extern int lct;
extern int lsh;
extern int rsh;

extern int capsDown;     // Caps lock  0xba is breakcode. 
extern int caps;

extern int clear_f;    // Ctrl+L flag.

#endif
