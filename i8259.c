/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */

/* Initialize the 8259 PIC */
/*
    IN:    NO
    OUT:   NO
    WHAT:  Initialize the 8259 PIC
*/
void
i8259_init(void)
{
    // Master init:
    outb( ICW1, MASTER_8259_PORT );
    outb( ICW2_MASTER, MASTER_8259_PORT+1 );
    outb( ICW3_MASTER, MASTER_8259_PORT+1 );
    outb( ICW4, MASTER_8259_PORT+1 );
    
    // Slave init:
    outb( ICW1, SLAVE_8259_PORT );
    outb( ICW2_SLAVE, SLAVE_8259_PORT+1 );
    outb( ICW3_SLAVE, SLAVE_8259_PORT+1 );
    outb( ICW4, SLAVE_8259_PORT+1 );
    
    //mask all interrupts
    master_mask = maskf;
    slave_mask = maskf;
    outb( master_mask, MASTER_8259_PORT+1 ); //Master
    outb( slave_mask, SLAVE_8259_PORT+1 );  //Slave
    // outb();
    
    // restore_flags(flags);
    // sti();
}

/* INPUT : IR number to be enabled
* OUTPUT : NONE
*Effects : Enable (unmask) the specified IRQ 
*Function to unmask the specified IRQ. Mater owns IR num 0 through 7 and Slave
*own IR num 8 through 15*/
void
enable_irq(uint32_t irq_num)
{  
    uint16_t port;
    uint8_t value;
    if(irq_num<irq_n)
    {
        port = MASTER_8259_PORT+1;
    }
    else
    {
		//first enable ir2
		enable_irq(2);
        port = SLAVE_8259_PORT+1;
        irq_num -= irq_n;
    }
    
    value = inb(port) & ~(1 << irq_num);
    outb(value, port);
}

/*INPUT : IR num to be disabled
*OUTPUT : None
*EFFECTS : Disable (mask) the specified IRQ 
*Will disable the specified IR num. If ir2 is masked, then 
*the slave won't be able to send interrupts no matter what*/
void
disable_irq(uint32_t irq_num)
{
    uint16_t port;
    uint8_t value;
    
    if(irq_num < irq_n) 
    {
        port = MASTER_8259_PORT+1;
    } 
    else 
    {
        port = SLAVE_8259_PORT+1;
        irq_num -= irq_n;
    }
    
    value = inb(port) | (1 << irq_num);
    outb(value, port);
}

/* INPUT : NONE
* OUTPUT : NONE
*Effects : Send end-of-interrupt signal for the specified IRQ
*Will tell the processor that an interrupt has ended.*/
void
send_eoi(uint32_t irq_num)
{
    if(irq_num >= irq_n)
    {
		irq_num-=irq_n;
        outb(EOI|irq_num, SLAVE_8259_PORT);
        irq_num = 2;
    }
    
    outb(EOI|irq_num, MASTER_8259_PORT);
}

