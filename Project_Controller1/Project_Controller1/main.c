#include <avr/io.h>
#define F_CPU 1000000
#include <util/delay.h>
unsigned char joyUp;
unsigned char joyDown;
unsigned char joyLeft;
unsigned char joyRight;
unsigned char keyChar;
unsigned short keyCharLen;
char MSG[20];
unsigned char MSGSIZE;
int vert;
int horiz;

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
		joyUp = 0x01;
	}
	if (horiz < 100){
		joyDown = 0x01;
	}
	if (vert > 520 + 500){
		joyLeft = 0x01;
	}
	if (vert < 100){
		joyRight = 0x01;
	}
	return state;
}
int main(void)
{
	DDRB = 0xFF; PORTB = 0x00;
	//putting portB and portD as output pins
	DDRD = 0xFF; PORTD = 0x00;
	_delay_ms(50);//giving delay of 50ms
	DDRA = 0x00; PORTA = 0xFF;//Joystick input.
	ADMUX |=(1<<REFS0);//setting the reference of ADC
	ADCSRA |=(1<<ADEN) |(1<ADPS2)|(1<ADPS1) |(1<<ADPS0);
	//enabling the ADC,, setting prescalar 128
	MSGSIZE = 0;
	while(1)
	{
	}
}