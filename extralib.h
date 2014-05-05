#ifndef EXTRALIB_H
#define EXTRALIB_H

#include "lib.h"

int getStatInit();

void initStatMode(int8_t fgc, int8_t bgc, int loc);

void quickStat(int loc);

void exitStatMode(int loc);

void clearStat(int loc);

#endif
