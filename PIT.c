#include "PIT.h"

#define DIVIDE_IT 0x369E99

#define SOMETIME 0xDBB3A062

int timer_ticks = 0;
static int current_hz=0;

// Getter
int getCurrentHz()
{
    return current_hz;
}

// Initialise
/*
    IN:     NO
    OUT:    NO
    WHAT:   Initialize to 50 hz.
*/
void
PIT_init(void)
{
    enable_irq(IRQ_PIT);
    timer_phase(50);
}

// Change Hz
/*
    IN:     hz-cycles per second. Appears to allow minimum of 18.
    OUT:    NO
    WHAT:   
*/
void
timer_phase(int hz)
{
    if(hz==0)
        return;
    int divisor = FMAX / hz;       /* Calculate our divisor */
    outb(STD_COM, PIT_COM);             /* Set our command byte 0x36 */
    outb(divisor & 0xFF, PIT_C0);   /* Set low byte of divisor */
    outb(divisor >> 8, PIT_C0);     /* Set high byte of divisor */
    current_hz = hz;
}

// Just the handler.
/*
    IN:     NO
    OUT:    NO
    WHAT:   Increment the ticks for interval control, switch to next task.
*/
void
PIT_c(void)
{
    timer_ticks++;
    // if(timer_ticks%2==0)
        next_task();
    // printf("%d ", timer_ticks);
}

/*
    IN:     ticks-interval unit
    OUT:    NO
    WHAT:   blocks and waits
*/
void
timer_wait(int ticks)
{
    unsigned long eticks=0;
    // int tmp=0;
    eticks = timer_ticks + ticks;
    while(timer_ticks < eticks)
    {
        // if(tmp<timer_ticks)
        // {
        //     tmp = timer_ticks;
        //     printf("%d,%d ", timer_ticks, eticks);
        // }
    }
}
