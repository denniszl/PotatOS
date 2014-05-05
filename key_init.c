#include "key_init.h"

#define ENTER_DOWN 0x1c
#define BACKSPACE_DOWN 0X0e
#define ALTSIZE 8
#define TRM_COUNT 3

// Our input buffer for our read function.   
// Now has 3 of them for each terminal
static char readChar[TRM_COUNT][MAX_BUF_SIZE+1];
static int rIndex[TRM_COUNT]={0,0,0};

static int altcode[ALTSIZE];
static int altindex = 0;
static int32_t altAscii;

static int32_t digitsToNum(int32_t* buffer);

/*INPUT : NONE
*OUTPUT : NONE
*EFFECTS : Will initialize the keyboard on the pic. This will 
*enable interrupts to occur from the keyboard. The keyboard occupies IR1 
*On the MASTER PIC*/
void keyboard_init(void)
{
    enable_irq(KEYBOARD_IR);
    altSet();
}

void altSet()
{
    int i =0;
    for(; i<ALTSIZE; i++)
    {
        altcode[i] = -1;
    }
}

// Flags for lct 0x9d | lsh 0xaa | rsh 0xb6 | lal 0x38
int lct=0;
int lsh=0;
int rsh=0;
int lal=0;

int capsDown=0;     // Caps lock  0xba is breakcode. 
int caps = 0;

int clear_f=0;    // Ctrl+L flag.

// int stage0=0;       // For multistage codes.

/*  IN:  NONE
    OUT: Last character
    Just gets the last character.
    */
char getKeyLast()
{
	pcb_t * pcb_p = find_pcb();
	int loc = pcb_p->alt_n;
    return readChar[loc][rIndex[loc]];
}

/*  IN:     NO
    OUT:    NO
    Gets the index.
*/
int getKeyIndex()
{
    return rIndex[viewTerm];
}

char * getKBufPointer()
{
    return readChar[viewTerm];
}

/*  IN:     NO
    OUT:    NO
    Just reset the index.
 */
void resetKeyBuf()
{
    rIndex[viewTerm] = 0;
    int i=0;
    for( ; i<MAX_BUF_SIZE; i++)
        readChar[viewTerm][i]='\0';
}

/*INPUT : NONE
*OUTPUT : NONE
*EFFECTS : Will output characters from the keyboard onto the terminal
*This is the keyboard interrupt handler. When a key is pressed on the keyboard, 
*This interrupt handler will be called, and takes the correct keycode 
*and match it up with the appropriate characters*/
void key_c(void)
{
    uint32_t charv;
    charv=inb(0x60);  // Read from this port.
    // Only allow enter or backspace when buffer index=126
    clear_f = 0;
        // Fail non-enter on full:      Fail backspace on empty:      Enter ok:
    if( (rIndex[viewTerm]!=MAX_BUF_SIZE && charv!=BACKSPACE_DOWN) || (rIndex[viewTerm]!=0 && charv==BACKSPACE_DOWN) || charv==ENTER_DOWN )
    {
        switch(charv)
        {
            case 0x02:      // One entry per key.
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'!':'1';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x03:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'@':'2';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x04:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'#':'3';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x05:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'$':'4';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x06:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'%':'5';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x07:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'^':'6';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x08:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'&':'7';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x09:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'*':'8';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x0a:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'(':'9';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x0b:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?')':'0';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x0c:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'_':'-';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x0d:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'+':'=';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case BACKSPACE_DOWN:
                rIndex[viewTerm]--;
                printf("%c", '\b');
            break;
            case 0x0f:      // Tab
                // readChar[viewTerm][rIndex[viewTerm]++] = '\t';
            break;
            case 0x10:      // Top row.
                readChar[viewTerm][rIndex[viewTerm]++] = ( (lsh||rsh)^caps )?'Q':'q';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x11:
                readChar[viewTerm][rIndex[viewTerm]++] = ( (lsh||rsh)^caps )?'W':'w';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x12:
                readChar[viewTerm][rIndex[viewTerm]++] = ( (lsh||rsh)^caps )?'E':'e';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x13:
                readChar[viewTerm][rIndex[viewTerm]++] = ( (lsh||rsh)^caps )?'R':'r';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x14:
                readChar[viewTerm][rIndex[viewTerm]++] = ( (lsh||rsh)^caps )?'T':'t';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x15:
                readChar[viewTerm][rIndex[viewTerm]++] = ( (lsh||rsh)^caps )?'Y':'y';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x16:
                readChar[viewTerm][rIndex[viewTerm]++] = ( (lsh||rsh)^caps )?'U':'u';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x17:
                readChar[viewTerm][rIndex[viewTerm]++] = ( (lsh||rsh)^caps )?'I':'i';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x18:
                readChar[viewTerm][rIndex[viewTerm]++] = ( (lsh||rsh)^caps )?'O':'o';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x19:
                readChar[viewTerm][rIndex[viewTerm]++] = ( (lsh||rsh)^caps )?'P':'p';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x1a:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'{':'[';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x1b:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'}':']';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case ENTER_DOWN:      // Enter pressed
                readChar[viewTerm][rIndex[viewTerm]] = '\n';
                printf("\n");
                
                // rIndex[viewTerm] = 0;
            break;
            case 0x1d:      // Left ctrl pressed
                lct = 1;
            break;
            case 0x1e:      // Home row.
                readChar[viewTerm][rIndex[viewTerm]++] = ((lsh||rsh)^caps)?'A':'a';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x1f:
                readChar[viewTerm][rIndex[viewTerm]++] = ((lsh||rsh)^caps)?'S':'s';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x20:
                readChar[viewTerm][rIndex[viewTerm]++] = ((lsh||rsh)^caps)?'D':'d';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x21:
                readChar[viewTerm][rIndex[viewTerm]++] = ((lsh||rsh)^caps)?'F':'f';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x22:
                readChar[viewTerm][rIndex[viewTerm]++] = ((lsh||rsh)^caps)?'G':'g';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x23:
                readChar[viewTerm][rIndex[viewTerm]++] = ((lsh||rsh)^caps)?'H':'h';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x24:
                readChar[viewTerm][rIndex[viewTerm]++] = ((lsh||rsh)^caps)?'J':'j';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x25:
                readChar[viewTerm][rIndex[viewTerm]++] = ((lsh||rsh)^caps)?'K':'k';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x26:
                if(!lct)    // Check for clear screen.
                {
                    readChar[viewTerm][rIndex[viewTerm]++] = ((lsh||rsh)^caps)?'L':'l';
                    printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
                }
                else
                {
                    clear_f = 1;
                    rIndex[viewTerm] = 0;
                    clear();
                    initStatMode((colors_t)WHITE,(colors_t)BLACK, viewTerm);
                    printf("Screen clear.");
                    exitStatMode(viewTerm);
                    printf("391OS> ");
                }
            break;
            case 0x27:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?':':';';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x28:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'\"':'\'';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x29:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'~':'`';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x2a:      // Left shift.
                lsh = 1;
            break;
            case 0x2b:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'|':'\\';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x2c:
                readChar[viewTerm][rIndex[viewTerm]++] = ((lsh||rsh)^caps)?'Z':'z';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x2d:
                readChar[viewTerm][rIndex[viewTerm]++] = ((lsh||rsh)^caps)?'X':'x';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x2e:
                readChar[viewTerm][rIndex[viewTerm]++] = ((lsh||rsh)^caps)?'C':'c';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x2f:
                readChar[viewTerm][rIndex[viewTerm]++] = ((lsh||rsh)^caps)?'V':'v';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x30:
                readChar[viewTerm][rIndex[viewTerm]++] = ((lsh||rsh)^caps)?'B':'b';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x31:
                readChar[viewTerm][rIndex[viewTerm]++] = ((lsh||rsh)^caps)?'N':'n';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x32:
                readChar[viewTerm][rIndex[viewTerm]++] = ((lsh||rsh)^caps)?'M':'m';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x33:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'<':',';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x34:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'>':'.';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x35:
                readChar[viewTerm][rIndex[viewTerm]++] = (lsh||rsh)?'?':'/';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x36:      // Right shift.
                rsh = 1;
            break;
            case 0x37:
                readChar[viewTerm][rIndex[viewTerm]++] = '*';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x38:      // Left alt.
                lal = 1;
            break;
            case 0x39:
                readChar[viewTerm][rIndex[viewTerm]++] = ' ';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x3a:      // Caps Lock
                if(!capsDown)
                {
                    caps ^= 0x01;   // Toggle its state.
                    capsDown = 1;
                    // outb(0xED,0x60);
                }
            break;
            // Function keys:
            case 0x3b:      // F1
                if(lal)
                {
                    //viewTerm=0; //view term changed in actual page swap
                    if(PCB_trm_search(2)==-1)
                    {
                        clearStat(viewTerm);
                        initStatMode((colors_t)BLACK,(colors_t)WHITE, viewTerm);
                        printf("Trm switch failed.");
                        exitStatMode(viewTerm);
                    }
                }
            break;
            case 0x3c:      // F2
                if(lal)
                {
                    //viewTerm=1;
                    if(PCB_trm_search(1)==-1)
                    {
                        clearStat(viewTerm);
                        initStatMode((colors_t)BLACK,(colors_t)WHITE, viewTerm);
                        printf("Trm switch failed.");
                        exitStatMode(viewTerm);
                    }
                }
            break;
            case 0x3d:      // F3
                if(lal)
                {
                    //viewTerm=2;
                    if(PCB_trm_search(0)==-1)
                    {
                        clearStat(viewTerm);
                        initStatMode((colors_t)BLACK,(colors_t)WHITE, viewTerm);
                        printf("Trm switch failed.");
                        exitStatMode(viewTerm);
                    }
                }
            break;
            // Not used for now:
            case 0x3e:      // F4
            case 0x3f:      // F5
            case 0x40:      // F6
            case 0x41:      // F7
            case 0x42:      // F8
            case 0x43:      // F9
            case 0x44:      // F10
                clearStat(viewTerm);
                initStatMode((colors_t)BLACK,(colors_t)WHITE, viewTerm);
                printf("That's not a button, silly.");
                exitStatMode(viewTerm);
            break;
            case 0x45:      // Num lock
            case 0x46:      // Scroll Lock
            break;
            case 0x47:      // Numpad: 789
                if(lal && altindex<ALTSIZE)
                {
                    altcode[altindex++]=7;
                }
            break;
            case 0x48:      // Also up arrow.
                if(lal && altindex<ALTSIZE)
                {
                    altcode[altindex++]=8;
                }
                // clrLine();
                // readChar = memcpy(readChar,history1,MAX_BUF_SIZE);
            break;
            case 0x49:
                if(lal && altindex<ALTSIZE)
                {
                    altcode[altindex++]=9;
                }
            break;
            case 0x4a:
                readChar[viewTerm][rIndex[viewTerm]++] = '-';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x4b:      // Numpad: 456
                if(lal && altindex<ALTSIZE)
                {
                    altcode[altindex++]=4;
                }
            break;
            case 0x4c:
                if(lal && altindex<ALTSIZE)
                {
                    altcode[altindex++]=5;
                }
            break;
            case 0x4d:
                if(lal && altindex<ALTSIZE)
                {
                    altcode[altindex++]=6;
                }
            break;
            case 0x4e:
                readChar[viewTerm][rIndex[viewTerm]++] = '+';
                printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
            break;
            case 0x4f:      // Numpad: 123
                if(lal && altindex<ALTSIZE)
                {
                    altcode[altindex++]=1;
                }
            break;
            case 0x50:
                if(lal && altindex<ALTSIZE)
                {
                    altcode[altindex++]=2;
                }
            break;
            case 0x51:
                if(lal && altindex<ALTSIZE)
                {
                    altcode[altindex++]=3;
                }
            break;
            case 0x52:      // Numpad: 0
                if(lal && altindex<ALTSIZE)
                {
                    altcode[altindex++]=0;
                }
            break;
            case 0x53:      // Numpad: .
            break;
            case 0x57:      // F11-12
            case 0x58:
                clearStat(viewTerm);
                initStatMode((colors_t)BLACK,(colors_t)WHITE, viewTerm);
                printf("That's not a button, silly.");
                exitStatMode(viewTerm);
            break;
            // Break codes:
            case 0x9d:      // Left ctrl
                lct = 0;
            break;
            case 0xaa:      // Left shift
                lsh = 0;
            break;
            case 0xba:      // Caps Lock
                capsDown = 0;
            break;
            case 0xb6:      // Right shift
                rsh = 0;
            break;
            case 0xb8:      // Left alt
                lal = 0;
                altAscii = digitsToNum(altcode);
                if(altAscii > 0)
                {
                    if(altAscii!='\b' && altAscii!='\n')
                    {
                        readChar[viewTerm][rIndex[viewTerm]++] = (char)altAscii;
                        printf("%c", readChar[viewTerm][rIndex[viewTerm]-1]);
                    }
                    else
                    {
                        if( rIndex[viewTerm]>0 )
                        {
                            rIndex[viewTerm]--;
                            printf("\b");
                        }
                    }
                }
                altSet();
                altindex = 0;
            break;
            default:
            break;
        }
    }
    // printf("%x\n", charv);
    return;
}


/*
    IN:    buffer-the string to convert
    OUT:   the number
    WHAT:  For use in altcodes
*/
int32_t
digitsToNum(int32_t* buffer)
{
    int digits=0;
    int i=0;
    int final=0;
    
    while(digits < ALTSIZE)
    {
        if(buffer[digits]==-1)
            break;
        digits++;
    }
    
    if(digits<=0)
        return -1;
    
    for( ; i<digits; i++ )
    {
        final += buffer[i]*power(10,digits-i-1);
    }
    
    // printf("%d", final);
    
    return final;
}


/*
    IN:    esp
    OUT:   NO
    WHAT:  saves the esp into the pcb.
*/
void save_esp(uint32_t esp)
{
    pcb_t * ptr;
    ptr = find_pcb();

    ptr->iret_esp = esp;
    ptr->eip_val = *(uint32_t*)(esp-40);
}
