#ifndef PCSOUND
#define PCSOUND

#include "i8259.h"
#include "lib.h"
#include "x86_desc.h"
#include "syscall.h"
#include "PIT.h"

#define IOPORT 0x61
#define NOTEC 32.7
#define NOTECS 34.6
#define NOTEDF NOTECS
#define NOTED 36.7
#define NOTEDS 38.9
#define NOTEEF NOTEDS
#define NOTEE 41.2
#define NOTEF 43.7
#define NOTEFS 46.2
#define NOTEGF NOTEFS
#define NOTEG 49.0
#define NOTEGS 51.9
#define NOTEAF NOTEGS
#define NOTEA 55.0
#define NOTEAS 58.3
#define NOTEBF NOTEAS
#define NOTEB 61.7
// Modifiers:
#define MOD1 *1
#define MOD2 *2
#define MOD3 *4
#define MOD4 *8
#define MOD5 *16
#define MOD6 *32

//Make a beep
void beep(int freq, int duration);

void trogdor(void);

void scale(void);

void ghostbusters(void);

#endif
