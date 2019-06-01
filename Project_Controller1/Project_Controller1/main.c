#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#define F_CPU 1000000
#define sysPeriod 250
#define MAXMSG 30
//Global variables======================================================================
//Global Joystick variables for shifting retrieved message==============================
unsigned char joyUp;
unsigned char joyDown;
unsigned char joyLeft;
unsigned char joyRight;
//Global variables for creating messages================================================
unsigned char keyChar;
unsigned short keyCharLen;
char TOPMSG[17];
char BOTMSG[17];
char MSG[20];
unsigned char MSGSIZE;
unsigned MSGSPOT;
//Global variables to tell if the vertical or horizontal is greater than threshold======
int vert;
int horiz;
//Creating the task structure===========================================================
typedef struct _task {
	signed char state; //Task's current state
	unsigned long int period; //Task period
	unsigned long int elapsedTime; //Time elapsed since last task tick
	int (*TickFct)(int); //Task tick function
} task;











//LCD function for displaying and changing displayed messages===========================
enum LCDState{Display, ScrollUp, ScrollDown, ScrollRight, ScrollLeft};
int LCDTick(int LCDState){
	char OUTPUT[33];
	switch(LCDState){
		case Display :
			//create top line of message
			for(unsigned msgat = 0; msgat < 16; msgat++){
				OUTPUT[msgat] = TOPMSG[msgat];
			}
			for(msgat = 0; msgat < 16; msgat++){
				OUTPUT[msgat + 16] = BOTMSG[msgat];
			}
		break;
		case ScrollUp :
			if(MSGSPOT == 0)
				break;
			else{
				MSGSPOT--;
				unsigned char bot = 0;
				char tempChar[MAXMSG];
				while(BOTMSG[bot] != 0 && bot < MAXMSG){
					BOTMSG[bot] =
				}
			}
	}
}










//Keyboard function for entry into the message to be sent===============================
enum KeyState{WAIT, ONE, ZERO, ENTER, DELETE};
int KeyBoardTick(int KeyState){
	switch(KeyState){
		case WAIT :
			if((PINA & 0xF0) == 0x70){
				KeyState = ONE;
				keyCharLen++;
				keyChar = ((keyChar | 0x01) << 1);
			}else if((PINA & 0xF0) == 0xB0){
				KeyState = ZERO;
				keyCharLen++;
				keyChar = (keyChar << 1);
			}else if((PINA & 0xF0) == 0xD0){
				KeyState = DELETE;
				keyCharLen--;
				keyChar = (keyChar >> 1);
			}else if((PINA & 0xF0) == 0xE0){
				KeyState = ENTER;
				if(MSGSIZE < 20){
					MSG[MSGSIZE] = keyChar;
					MSGSIZE++;
				}
			}
		break;
		case ONE :
			if((PINA & 0xF0) == 0x70)
				KeyState = ONE;
			else
				KeyState = WAIT;
		break;
		case ZERO :
			if((PINA & 0xF0) == 0xB0)
				KeyState = ZERO;
			else
				KeyState = WAIT;
		break;
		case DELETE :
			if((PINA & 0xF0) == 0xD0)
				KeyState = DELETE;
			else
				KeyState = WAIT;
		break;
		case ENTER :
			if((PINA & 0xF0) == 0xE0)
				KeyState = ENTER;
			else
				KeyState = WAIT;
		break;
		default :
			KeyState = WAIT;
		break;
	}
}














//Function to use to pull joystick input================================================
enum JoyState {INPUTJOY};
int JoyStickTick(int JoyState){
	switch (ADMUX){
		case 0x40 :
			ADCSRA |=(1<<ADSC);
			while ( !(ADCSRA & (1<<ADIF)));
			horiz = ADC;
			ADC=0;
			ADMUX=0x41;
		break;
		case 0x41 :
			ADCSRA |=(1<<ADSC);
			while ( !(ADCSRA & (1<<ADIF)));
			vert = ADC;
			ADC=0;
			ADMUX=0x40;
		break;
	}
	if (horiz > 520 + 500){
		joyRight = 0x01;
	}
	if (horiz < 100){
		joyLeft = 0x01;
	}
	if (vert > 520 + 500){
		joyUp = 0x01;
	}
	if (vert < 100){
		joyDown = 0x01;
	}
	if(vert >= 420 && vert <=620){
		joyUp = 0;
		joyDown = 0;
	}
	if(horiz >= 420 && horiz <=620){
		joyLeft = 0;
		joyRight = 0;
	}
	return state;
}










int main(void)
{
	MSGSPOT = 0;
	MSGSIZE = 0;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	_delay_ms(50);//giving delay of 50ms
	DDRA = 0x00; PORTA = 0xFF;//Joystick input.
	ADMUX |=(1<<REFS0);//setting the reference of ADC
	ADCSRA |=(1<<ADEN) |(1<ADPS2)|(1<ADPS1) |(1<<ADPS0);
	static task task1, task2;
	task *tasks[] = { &task1, &task2};
	//task1 init
	task1.state = -1;
	task1.period = 250;
	task1.elapsedTime = 250;
	task1.TickFct = &JoyStickTick;
	task2.state = -1;
	task2.period = 250;
	task2.elapsedTime = 250;
	task2.TickFct = &KeyBoardTick;
	while(1)
	{
		for ( i = 0; i < 2; i++ ) {
		if ( tasks[i]->elapsedTime == tasks[i]->period ) {
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			tasks[i]->elapsedTime = 0;
		}
		tasks[i]->elapsedTime += 1;
	}
	//Remember to add timer to code
	while(!TimerFlag);
	TimerFlag = 0;
	}
}
