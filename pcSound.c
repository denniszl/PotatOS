#include "pcSound.h"

//Play sound using built in speaker
static void play_sound(uint32_t nFrequence) 
{
    uint32_t Div;
    uint8_t tmp;

    //Set the PIT to the desired frequency
    Div = FMAX / nFrequence;
    outb(SPK_COM, PIT_COM);
    outb((uint8_t) (Div), PIT_C2);
    outb((uint8_t) (Div >> 8), PIT_C2 );

    //And play the sound using the PC speaker
    tmp = inb(IOPORT);
    
    if (tmp != (tmp | 3)) 
    {
        outb(tmp | 3, IOPORT);
    }
}

//make it shutup
static void nosound() 
{
    uint8_t tmp = (inb(IOPORT) & 0xFC);

    outb(tmp, IOPORT);
}

//Make a beep
void
beep(int freq, int duration) 
{
    int saveHz = getCurrentHz();
    
    // printf("\n%d>", saveHz);
       
    play_sound(freq);
    timer_wait(duration);
    nosound();
    
    timer_phase(saveHz);
}

void
trogdor(void)
{
    int saveHz = getCurrentHz();
    
    int rpt;
    int stoccato = 10;
    int sixteenth = 1;
    int eighth = ((sixteenth + stoccato)<<1) - stoccato;
    int quarter = ((sixteenth + stoccato)<<2) - stoccato;
    int triplet = ((sixteenth + stoccato)<<2)/3 - stoccato;
    
    timer_phase(1);
    // 1
    for(rpt=0; rpt<3; rpt++)
    {
        beep(NOTED MOD6, triplet);
        timer_wait(stoccato);
    }
    beep(NOTEAS MOD6, quarter);
    timer_wait(stoccato);
    for(rpt=0; rpt<3; rpt++)
    {
        beep(NOTED MOD6, triplet);
        timer_wait(stoccato);
    }
    beep(NOTEG MOD6, quarter);
    timer_wait(stoccato);
    // 2
    for(rpt=0; rpt<3; rpt++)
    {
        beep(NOTED MOD6, triplet);
        timer_wait(stoccato);
    }
    beep(NOTEAS MOD6, eighth);
    timer_wait(stoccato);
    beep(NOTED MOD6, eighth);
    timer_wait(stoccato);
    beep(NOTEG MOD6, eighth);
    timer_wait(stoccato);
    beep(NOTED MOD6, eighth);
    timer_wait(stoccato);
    beep(NOTEF MOD6, eighth);
    timer_wait(stoccato);
    beep(NOTEG MOD6, eighth);
    timer_wait(stoccato);
    
    timer_phase(saveHz);
}

void
scale(void)
{
    int saveHz = getCurrentHz();
    
    // int rpt;
    int stoccato = 5;
    int sixteenth = 2;
    // int eighth = ((sixteenth + stoccato)<<1) - stoccato;
    int quarter = ((sixteenth + stoccato)<<2) - stoccato;
    int half = ((sixteenth + stoccato)<<3) - stoccato;
    // int whole = ((sixteenth + stoccato)<<4) - stoccato;
    
    timer_phase(1);
    
    beep(NOTEC MOD2, quarter);
    timer_wait(stoccato);
    beep(NOTEB MOD1, quarter);
    timer_wait(stoccato);
    beep(NOTEC MOD2, quarter);
    timer_wait(stoccato);
    beep(NOTED MOD2, quarter);
    timer_wait(stoccato);
    beep(NOTEEF MOD2, quarter);
    timer_wait(stoccato);
    beep(NOTEF MOD2, quarter);
    timer_wait(stoccato);
    beep(NOTEG MOD2, quarter);
    timer_wait(stoccato);
    beep(NOTEAF MOD2, quarter);
    timer_wait(stoccato);
    beep(NOTEB MOD2, quarter);
    timer_wait(stoccato);
    beep(NOTEC MOD3, quarter);
    timer_wait(stoccato);
    beep(NOTED MOD3, quarter);
    timer_wait(stoccato);
    beep(NOTEEF MOD3, quarter);
    timer_wait(stoccato);
    beep(NOTEF MOD3, quarter);
    timer_wait(stoccato);
    beep(NOTEG MOD3, quarter);
    timer_wait(stoccato);
    beep(NOTEAF MOD3, quarter);
    timer_wait(stoccato);
    beep(NOTEB MOD3, quarter);
    timer_wait(stoccato);
    beep(NOTEC MOD4, half);
    timer_wait(stoccato);
    
    timer_phase(saveHz);
}


void
ghostbusters(void)
{
    int saveHz = getCurrentHz();
    
    int rpt;
    int stoccato = 22;
    int sixteenth = 4;
    int eighth = ((sixteenth + stoccato)<<1) - stoccato;
    int quarter = ((sixteenth + stoccato)<<2) - stoccato;
    
    timer_phase(2);
    
    // m10
    beep(NOTEB MOD4, sixteenth);
    timer_wait(stoccato);
    beep(NOTEB MOD4, sixteenth);
    timer_wait(stoccato);
    beep(NOTEDS MOD5, eighth);
    timer_wait(stoccato);
    beep(NOTEB MOD4, eighth);
    timer_wait(stoccato);
    beep(NOTECS MOD5, eighth);
    timer_wait(stoccato);
    beep(NOTEA MOD4, eighth);
    timer_wait(stoccato);
    beep(NOTEE MOD1, sixteenth);
    timer_wait(stoccato*3+eighth+sixteenth);
    beep(NOTEB MOD1, sixteenth);
    timer_wait(stoccato*3+eighth+sixteenth);
    // m11
    for(rpt=0; rpt<4;rpt++)
    {
        beep(NOTEB MOD4, sixteenth);
        timer_wait(stoccato);
    }
    beep(NOTEA MOD4, eighth);
    timer_wait(stoccato);
    beep(NOTEB MOD4, eighth);
    timer_wait(stoccato);
    // m12
    beep(NOTEE MOD1, sixteenth);
    timer_wait(stoccato*3+eighth+sixteenth);
    beep(NOTEB MOD1, sixteenth);
    timer_wait(stoccato*2+sixteenth);
    beep(NOTEB MOD4, sixteenth);
    timer_wait(stoccato);
    beep(NOTEB MOD4, sixteenth);
    timer_wait(stoccato);
    beep(NOTEDS MOD5, eighth);
    timer_wait(stoccato);
    beep(NOTEB MOD4, eighth);
    timer_wait(stoccato);
    beep(NOTECS MOD5, eighth);
    timer_wait(stoccato);
    beep(NOTEA MOD4, eighth);
    timer_wait(stoccato);
    timer_wait(quarter+stoccato);
    //m13
    timer_wait(quarter+stoccato);
    
    // Got separated here:
    beep(NOTEA MOD4, eighth);
    timer_wait(stoccato);
    beep(NOTEA MOD4, eighth);
    timer_wait(stoccato);
    beep(NOTEGS MOD4, eighth);
    timer_wait(stoccato);
    printf("There's ");
    beep(NOTEB MOD4, eighth);
    timer_wait(stoccato);
    // m18
    printf("some");
    beep(NOTED MOD5, eighth);
    timer_wait(stoccato);
    printf("thing ");
    beep(NOTEB MOD4, eighth);
    timer_wait(stoccato);
    printf("weird, ");
    beep(NOTED MOD5, quarter);
    timer_wait(stoccato);
    beep(NOTEA MOD4, eighth);
    timer_wait(stoccato*2+eighth);
    beep(NOTEGS MOD4, eighth);
    timer_wait(stoccato);
    printf("and ");
    beep(NOTEB MOD4, sixteenth);
    timer_wait(stoccato);
    printf("it ");
    beep(NOTEA MOD4, sixteenth);
    timer_wait(stoccato);
    // m19
    printf("don't ");
    beep(NOTEB MOD4, eighth);
    timer_wait(stoccato);
    printf("look ");
    beep(NOTEB MOD4, eighth);
    timer_wait(stoccato);
    printf("good.\n");
    beep(NOTEB MOD4, quarter);
    timer_wait(stoccato);
    beep(NOTEA MOD4, eighth);
    timer_wait(stoccato*2+eighth);
    beep(NOTEGS MOD4, eighth);
    timer_wait(stoccato*2+eighth);
    // m20
    printf("Who ");
    beep(NOTEB MOD3, sixteenth+stoccato);
    printf("you ");
    beep(NOTEB MOD3, sixteenth);
    timer_wait(stoccato);
    printf("gon");
    beep(NOTEB MOD3, sixteenth+stoccato);
    printf("na ");
    beep(NOTEB MOD3, sixteenth);
    timer_wait(stoccato);
    printf("call?  ");
    timer_wait(stoccato+quarter);
    beep(NOTEA MOD4, eighth);
    timer_wait(stoccato*2+eighth);
    printf("Ghost");
    beep(NOTEGS MOD4, eighth);
    timer_wait(stoccato*2+eighth);
    // m21
    printf("bust");
    beep(NOTEB MOD3, eighth);
    timer_wait(stoccato);
    printf("ers!\n");
    beep(NOTEB MOD3, eighth);
    timer_wait(stoccato);
    timer_wait(quarter+stoccato);
    beep(NOTEC MOD5, eighth);
    timer_wait(stoccato);
    beep(NOTEC MOD5, eighth);
    timer_wait(stoccato);
    beep(NOTEB MOD4, eighth);
    timer_wait(stoccato);
    beep(NOTEF MOD4, eighth);
    timer_wait(stoccato);
    // mm 22-23
    for(rpt = 0; rpt<4; rpt++)
    {
        beep(NOTED MOD5, eighth+stoccato+sixteenth);
        timer_wait(stoccato);
        beep(NOTEB MOD4, sixteenth);
        timer_wait(stoccato);
        timer_wait(eighth+stoccato);
    }
    beep(NOTED MOD5, eighth);
    timer_wait(stoccato);
    beep(NOTEB MOD4, eighth);
    timer_wait(stoccato);
    beep(NOTEA MOD4, sixteenth);
    timer_wait(stoccato);
    beep(NOTEAS MOD4, sixteenth);
    timer_wait(stoccato);
    beep(NOTEB MOD4, eighth);
    timer_wait(stoccato);
    // m24
    beep(NOTED MOD5, eighth+stoccato+sixteenth);
    timer_wait(stoccato);
    beep(NOTEB MOD4, sixteenth);
    timer_wait(stoccato);
    timer_wait(eighth+stoccato);
    beep(NOTED MOD5, eighth+stoccato+sixteenth);
    timer_wait(stoccato);
    beep(NOTEB MOD4, sixteenth);
    timer_wait(stoccato);
    timer_wait(eighth+stoccato);
    beep(NOTED MOD5, eighth+stoccato+sixteenth);
    timer_wait(stoccato);
    beep(NOTEB MOD4, sixteenth);
    timer_wait(stoccato);
    // m25
    printf("I ");
    timer_wait(sixteenth+stoccato);
    printf("ain't ");
    timer_wait(sixteenth+stoccato);
    beep(NOTED MOD5, sixteenth+stoccato);
    printf("'fraid ");
    beep(NOTED MOD5, eighth);
    timer_wait(stoccato);
    printf("of ");
    beep(NOTEB MOD4, sixteenth);
    timer_wait(stoccato);
    printf("no ");
    timer_wait(stoccato+eighth);
    printf("ghost!\n");
    beep(NOTED MOD5, eighth);
    timer_wait(stoccato);
    beep(NOTEB MOD4, eighth);
    timer_wait(stoccato);
    beep(NOTEA MOD4, sixteenth);
    timer_wait(stoccato);
    beep(NOTEAS MOD4, sixteenth);
    timer_wait(stoccato);
    beep(NOTEB MOD4, eighth);
    timer_wait(stoccato);
    // mm 26-27
    for (rpt = 0; rpt < 4; rpt++)
    {
        beep(NOTEFS MOD5, eighth+stoccato+sixteenth);
        timer_wait(stoccato);
        beep(NOTED MOD5, sixteenth);
        timer_wait(stoccato);
        timer_wait(eighth+stoccato);
    }
    beep(NOTEFS MOD5, eighth);
    timer_wait(stoccato);
    beep(NOTED MOD5, eighth);
    timer_wait(stoccato);
    beep(NOTEA MOD4, sixteenth);
    timer_wait(stoccato);
    beep(NOTEAS MOD4, sixteenth);
    timer_wait(stoccato);
    beep(NOTEB MOD4, eighth);
    timer_wait(stoccato);
    // m28
    beep(NOTEFS MOD5, eighth+stoccato+sixteenth);
    timer_wait(stoccato);
    beep(NOTED MOD5, sixteenth);
    timer_wait(stoccato);
    timer_wait(eighth+stoccato);
    beep(NOTEFS MOD5, eighth+stoccato+sixteenth);
    timer_wait(stoccato);
    beep(NOTED MOD5, sixteenth);
    timer_wait(stoccato);
    timer_wait(eighth+stoccato);
    beep(NOTEFS MOD5, eighth+stoccato+sixteenth);
    timer_wait(stoccato);
    beep(NOTED MOD5, sixteenth);
    timer_wait(stoccato);
    // m29
    printf("I ");
    timer_wait(sixteenth+stoccato);
    printf("ain't ");
    timer_wait(sixteenth+stoccato);
    beep(NOTEFS MOD5, sixteenth+stoccato);
    printf("'fraid ");
    beep(NOTEFS MOD5, eighth);
    timer_wait(stoccato);
    printf("of ");
    beep(NOTED MOD5, sixteenth);
    timer_wait(stoccato);
    printf("no ");
    timer_wait(stoccato+eighth);
    printf("ghost!\n");
    beep(NOTEFS MOD5, eighth);
    timer_wait(stoccato);
    beep(NOTED MOD5, eighth);
    timer_wait(stoccato);
    beep(NOTEA MOD4, sixteenth);
    timer_wait(stoccato);
    beep(NOTEAS MOD4, sixteenth);
    timer_wait(stoccato);
    beep(NOTEB MOD4, eighth);
    timer_wait(stoccato);
    // m30
    beep(NOTEB MOD4, eighth);
    timer_wait(stoccato);
    beep(NOTEB MOD4, eighth);
    timer_wait(stoccato);
    
    timer_phase(saveHz);
}
