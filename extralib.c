#include "extralib.h"

// Status bar mode:  set the cursor position to the last line.
int coord[2];
char saveAttr;
int init=0;

int
getStatInit()
{
    return init;
}

// Fire up status mode.
void
initStatMode(int8_t fgc, int8_t bgc, int loc)
{
    coord[0] = getScreen_x(loc);
    coord[1] = getScreen_y(loc);
    
    saveAttr = get_attrib();
    
    setScreen_x(0, loc);
    setScreen_y(NUM_ROWS, loc);
    
    set_attrib( (uint8_t) fgc, (uint8_t) bgc );
    
    init = 1;
}

// Just ready the statbar w/o alter colors.
void
quickStat(int loc)
{
    coord[0] = getScreen_x(loc);
    coord[1] = getScreen_x(loc);
    
    saveAttr = get_attrib();
    
    setScreen_x(0, loc);
    setScreen_y(NUM_ROWS, loc);
    
    init = 1;
}

// Restore from status mode.
void
exitStatMode(int loc)
{
    if(init)
    {
        set_char_attrib( saveAttr );
        
        setScreen_x(coord[0], loc);
        setScreen_y(coord[1], loc);
        init = 0;
    }
}

// erase the statusbar.
void
clearStat(int loc)
{
    initStatMode(saveAttr>>4, saveAttr&0xf, loc);
    uint32_t blank = 0x20 | (saveAttr << 8);
    memset_word((char*) VIDEO+(NUM_ROWS)*NUM_COLS*2, blank, NUM_COLS);
    exitStatMode(loc);
}
