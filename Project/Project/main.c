#include <avr/io.h>
#include <avr/eeprom.h>
//#include <util/delay.h>
#include <timer.h>
#include "io.c"
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
char TOPMSG[33];
char BOTMSG[33];
char MSG[17] = "               ";
char MSG2[] = "HELLO WORLD";
unsigned char up;
unsigned char MSGSIZE;
unsigned MSGSPOT;
static unsigned char t;
unsigned char shiftRightUp;
unsigned char shiftRightDown;
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

enum LCDSTATE{DISPLAY};
int LCDTICK(int LCDSTATE){
	LCD_DisplayString(1, MSG);
	LCD_DisplayString(17, MSG2);
	
	if(joyUp == 0x01 && up == 0){
		t = 0;
		up = 1;
	}
	if(joyDown == 0x01 && up == 1){
		t = 16;
		up = 0;
	}
	
	if(joyRight == 0x01 && t < 15 && up == 1){
		t++;
		joyRight = 0;
	}
	if(joyLeft == 0x01 && t > 0 && up == 1){
		t--;
		joyLeft = 0;
	}
	if(joyRight == 0x01 && t < 32 && up == 0){
		t++;
		joyRight = 0;
	}
	if(joyLeft == 0x01 && t > 16 && up == 0){
		t--;
		joyLeft = 0;
	}
	LCD_Cursor(MSGSIZE + t + 1);
	return LCDSTATE;
}







//Keyboard function for entry into the message to be sent===============================
enum KeyState{WAIT, ONE, ZERO, ENTER, DELETE, SEND};
int KeyBoardTick(int KeyState){
	if(t < 16){
		switch(KeyState){
			case WAIT :
				if((PINA & 0xF0) == 0x70){
					keyChar = keyChar << 1;
					keyChar |= 0x01;
					KeyState = ONE;
				}else if((PINA & 0xF0) == 0xB0){
					keyChar = keyChar << 1;
					KeyState = ZERO;
				}else if((PINA & 0xF0) == 0xD0){
					if(keyChar == 0x00)
						MSG[t] = 0x20;
					t++;
					KeyState = ENTER;
				}else{
					KeyState = WAIT;
				}
			break;
			case ONE :
				if((PINA & 0xF0) == 0x70){
					KeyState = ONE;
					//LCD_DisplayString(1, "HELLO");
				}else
					KeyState = WAIT;
			break;
			case ZERO :
				if((PINA & 0xF0) == 0xB0){
					KeyState = ZERO;
				}else
					KeyState = WAIT;
			break;
			case ENTER :
				if((PINA & 0xF0) == 0xD0){
					KeyState = ENTER;
				}else{
					KeyState = WAIT;
					keyChar = 0x00;
				}
			break;
			default:
			KeyState = WAIT;
			break;
		}
		MSG[t] = keyChar;
	}
	return KeyState;
}














//Function to use to pull joystick input================================================
enum JoyState {INPUTJOY};
int JoyStickTick(int JoyState){
	switch (ADMUX){
		case 0x40 :
			ADCSRA |= (1 << ADSC);
			while (!(ADCSRA & (1 << ADIF)));
			horiz = ADC;
			ADC = 0;
			ADMUX = 0x41;
		break;
		case 0x41 :
			ADCSRA |= (1 << ADSC);
			while (!(ADCSRA & (1 << ADIF)));
			vert = ADC;
			ADC = 0;
			ADMUX = 0x40;
		break;
	}
	if (horiz > 520 + 300){
		joyRight = 0x01;
	}
	if (horiz < 300){
		joyLeft = 0x01;
	}
	if (vert > 520 + 300){
		joyUp = 0x01;
	}
	if (vert < 300){
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
	return JoyState;
}










int main(void)
{
	up = 1;
	t = 0;
	keyChar = 0x00;
	MSGSPOT = 0;
	MSGSIZE = 0;
	DDRD = 0xFF; PORTD = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRA = 0x00; PORTA = 0xFF;//Joystick input.
	ADMUX |= (1 << REFS0);//setting the reference of ADC
	ADCSRA |= (1<<ADPS0) | (1<ADPS1)| (1<ADPS2) | (1<<ADEN) ;
	static task task1, task2, task3;
	task *tasks[] = { &task1, &task2, &task3};
	//task1 init
	task1.state = -1;
	task1.period = 20;
	task1.elapsedTime = 20;
	task1.TickFct = &JoyStickTick;
	task2.state = -1;
	task2.period = 20;
	task2.elapsedTime = 20;
	task2.TickFct = &KeyBoardTick;
	task3.state = -1;
	task3.period = 100;
	task3.elapsedTime = 100;
	task3.TickFct = &LCDTICK;
	TimerSet(20);
	TimerOn();
	LCD_init();
	LCD_Cursor(0);
	while(1)
	{
		for ( unsigned i = 0; i < 3; i++ ) {
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 20;
		}
		//Remember to add timer to code
		while(!TimerFlag);
		TimerFlag = 0;
		//LCD_ClearScreen();
	}
}