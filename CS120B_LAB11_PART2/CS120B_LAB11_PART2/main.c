/*
 * CS120B_LAB11_PART2.c
 *
 * Created: 5/27/2019 10:32:07 PM
 * Author : Jeff
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.c"
#include <bit.h>
#include <timer.h>

typedef struct task {
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int(*TickFct)(int);
} task;

const unsigned short tasksNum = 1;
const unsigned long taskPeriod = 100;


// Returns '\0' if no key pressed, else returns char '1', '2', ... '9', 'A', ...
// If multiple keys pressed, returns leftmost-topmost one
// Keypad must be connected to port C
/* Keypad arrangement
        PC4 PC5 PC6 PC7
   col  1   2   3   4
row
PC0 1   1 | 2 | 3 | A
PC1 2   4 | 5 | 6 | B
PC2 3   7 | 8 | 9 | C
PC3 4   * | 0 | # | D
*/

unsigned char keypadKey = 0;
enum Keypad_States { idle, key_press, key_hold };
int TickFct_Keypad(int state) {
	switch(state) {
		case(idle):
			if ((PINB & 0x0F) != 0x0F) {
				state = key_press;
			} else {
				state = idle;
			}
		
			break;
		case(key_press):
			if ((PINB & 0x0F) != 0x0F) {
				state = key_hold;
			} else {
				state = idle;
			}
		
			break;
		case(key_hold):
			if ((PINB & 0x0F) != 0x0F) {
				state = key_hold;
			} else {
				state = idle;
			}
		
			break;
		default:
			state = idle;
			break;
	}
	
	switch(state) {
		case(idle):
			keypadKey = '\0';
			break;
		case(key_press):
		    PORTB = 0xEF; // Enable col 4 with 0, disable others with 1Åfs
		    asm("nop"); // add a delay to allow PORTC to stabilize before checking
		    if (GetBit(PINB,0)==0) { keypadKey = ('1'); }
		    if (GetBit(PINB,1)==0) { keypadKey = ('4'); }
		    if (GetBit(PINB,2)==0) { keypadKey = ('7'); }
		    if (GetBit(PINB,3)==0) { keypadKey = ('*'); }

		    // Check keys in col 2
		    PORTB = 0xDF; // Enable col 5 with 0, disable others with 1Åfs
		    asm("nop"); // add a delay to allow PORTC to stabilize before checking
		    if (GetBit(PINB,0)==0) { keypadKey = ('2'); }
		    if (GetBit(PINB,1)==0) { keypadKey = ('5'); }
		    if (GetBit(PINB,2)==0) { keypadKey = ('8'); }
		    if (GetBit(PINB,3)==0) { keypadKey = ('0'); }

		    // Check keys in col 3
		    PORTB = 0xBF; // Enable col 6 with 0, disable others with 1Åfs
		    asm("nop"); // add a delay to allow PORTC to stabilize before checking
		    if (GetBit(PINB,0)==0) { keypadKey = ('3'); }
		    if (GetBit(PINB,1)==0) { keypadKey = ('6'); }
		    if (GetBit(PINB,2)==0) { keypadKey = ('9'); }
		    if (GetBit(PINB,3)==0) { keypadKey = ('#'); }


		    // Check keys in col 4
		    PORTB = 0x7F; // Enable col 7 with 0, disable others with 1Åfs
		    asm("nop"); // add a delay to allow PORTC to stabilize before checking
		    if (GetBit(PINB,0)==0) { keypadKey = ('A'); }
		    if (GetBit(PINB,1)==0) { keypadKey = ('B'); }
		    if (GetBit(PINB,2)==0) { keypadKey = ('C'); }
		    if (GetBit(PINB,3)==0) { keypadKey = ('D'); }
				
			break;
		case(key_hold):
			keypadKey = '\0';
			break;
		default:
			break;
	}

	return state;
};

char string[70] = "                CS120B is Legend... wait for it DARY!";
unsigned short j;
char tempStr[17];
enum LCD_states { msg1, msg2, msg3 };
int TickFct_LCD(int state) {
	for(unsigned i = 0; i < 16; i++){
		tempStr[i] = string[i + j];
	}
	j++;
	LCD_DisplayString(0, tempStr);
	if(j == 54){
		j = 0;
	}
	return state;
};

int main(void)
{
	task task1;
	task *tasks[] = { &task1};
	task1.state = -1;//Task initial state.
	task1.period = 500;//Task Period.
	task1.elapsedTime = 0;//Task current elapsed time.
	task1.TickFct = &TickFct_LCD;//Function pointer for the tick.
	tempStr[16] = 0;
	DDRA = 0x00; PORTA = 0xFF; // Configure port A's 8 pins as inputs
	DDRB = 0xFF; PORTB = 0x00; // Configure port B's 8 pins as outputs, initialize to 0s
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	j = 0;
	TimerSet(1);
	TimerOn();
	LCD_init();
	while(1)
	{
		unsigned char i;
		for (i = 0; i < tasksNum; ++i) {
			if(tasks[i]->elapsedTime >= tasks[i]->period) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag){}	//Sleep();
			TimerFlag = 0;
	}
}