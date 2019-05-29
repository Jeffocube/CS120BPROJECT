/*
 * CS120B_LAB11_PART1.c
 *
 * Created: 5/29/2019 10:15:05 AM
 * Author : ucrcse
 */ 
/*
 * CS120B_LAB11_PART3.c
 *
 * Created: 5/27/2019 11:58:49 PM
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

unsigned char GetKeypadKey() {
	PORTA = 0x0F;
	LCD_Cursor(1);

	PORTA = 0xEF; // Enable col 4 with 0, disable others with 1Åfs
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINA,0)==0) { return('1'); }
	if (GetBit(PINA,1)==0) { return('4'); }
	if (GetBit(PINA,2)==0) { return('7'); }
	if (GetBit(PINA,3)==0) { return('*'); }

// Check keys in col 3
PORTA = 0xBF; // Enable col 6 with 0, disable others with 1Åfs
asm("nop"); // add a delay to allow PORTC to stabilize before checking
if (GetBit(PINA,0)==0) { return('3'); }
if (GetBit(PINA,1)==0) { return('6'); }
if (GetBit(PINA,2)==0) { return('9'); }
if (GetBit(PINA,3)==0) { return('#'); }
	// Check keys in col 2
	PORTA = 0xDF; // Enable col 5 with 0, disable others with 1Åfs
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINA,0)==0) { return('2'); }
	if (GetBit(PINA,1)==0) { return('5'); }
	if (GetBit(PINA,2)==0) { return('8'); }
	if (GetBit(PINA,3)==0) { return('0'); }



	// Check keys in col 4
	PORTA = 0x7F; // Enable col 7 with 0, disable others with 1Åfs
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINA,0)==0) { return('A'); }
	if (GetBit(PINA,1)==0) { return('B'); }
	if (GetBit(PINA,2)==0) { return('C'); }
	if (GetBit(PINA,3)==0) { return('D'); }
	return('\0'); // default value

}
int TickFct_Keypad(int state) {
	keypadKey = GetKeypadKey();
	return state;
};

int main(void)

{
	/*
	task task1;
	task *tasks[] = { &task1};
	task1.state = -1;//Task initial state.
	task1.period = 500;//Task Period.
	task1.elapsedTime = 0;//Task current elapsed time.
	task1.TickFct = &TickFct_Keypad;//Function pointer for the tick.
	*/
	DDRA = 0xF0; PORTA = 0x0F; // Configure port A's 8 pins as inputs
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	TimerSet(100);
	TimerOn();
	LCD_init();
	while(1){
		/*unsigned char i;
		for (i = 0; i < 1; ++i) {
			if(tasks[i]->elapsedTime >= tasks[i]->period) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		*/
		unsigned char x = GetKeypadKey();
		switch (x) {
			case '\0': PORTB = 0x1F; break; // All 5 LEDs on
			case '1': PORTB = 0x01; break; // hex equivalent
			case '2': PORTB = 0x02; break;
			case '3': PORTB = 3; break;
			case '4': PORTB = 4; break;
			case '5': PORTB = 5; break;
			case '6': PORTB = 6; break;
			case '7': PORTB = 7; break;
			case '8': PORTB = 8; break;
			case '9': PORTB = 9; break;
			case 'A': PORTB = 10; break;
			case 'B': PORTB = 11; break;
			case 'C': PORTB = 12; break;
			// . . . ***** FINISH *****

			case 'D': PORTB = 0x0D; break;
			case '*': PORTB = 0x0E; break;
			case '0': PORTB = 0x00; break;
			case '#': PORTB = 0x0F; break;
			default: PORTB = 0x1B; break; // Should never occur. Middle LED off.
		}
		while(!TimerFlag){}
			TimerFlag = 0;
		//Sleep();
	}
}