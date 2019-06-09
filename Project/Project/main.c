#include <avr/io.h>
#include <avr/eeprom.h>
//#include <util/delay.h>
#include <timer.h>
#include "io.c"
//#include <usart.h>
#define sysPeriod 250
#define MAXMSG 30
//Global variables======================================================================
//Global Joystick variables for shifting retrieved message==============================
unsigned char joyUp;
unsigned char joyDown;
unsigned char joyLeft;
unsigned char joyRight;
unsigned char joyUp2;
unsigned char joyDown2;
unsigned char joyLeft2;
unsigned char joyRight2;
//Global variables for creating messages================================================
unsigned char keyChar;
unsigned short keyCharLen;
char MSG[17]  = "               ";
char MSG2[17] = "               ";
char WAITMESSAGE[17] = "WAIT           ";
char DONEMESSAGE[17] = "DONE           ";
unsigned char up;
unsigned char MSGSIZE;
unsigned MSGSPOT;
static unsigned char t;
unsigned char shiftRightUp;
unsigned char shiftRightDown;
unsigned char sent;
unsigned char received;
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
//Global variables for sendign out strings via USART====================================
#define F_CPU 8000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALE (((F_CPU / (BAUD_RATE * 16UL))) - 1)
unsigned char READYTOSEND;
unsigned char READYTORECEIVE;
//Sends and receives strings via USART==================================================
void receive_string(char* stringswapped){
	static unsigned char flush;

	for(unsigned j = 0; j < 16; j++){
		MSG2[j] = ' ';
	}
	MSG2[16] = 0;
	for(unsigned j = 0; j < 16; j++){
		while ( !(UCSR0A & (1 << RXC0)) );
		MSG2[j] = UDR0;
	}
	while ( UCSR0A & (1 << RXC0) ) { flush = UDR0; }
	LCD_Cursor(0);
}
void transmit_string(char* stringswapped){
	for(unsigned j = 0; j < 16; j++){
		while( !(UCSR0A & (1 << UDRE0)) );
			UDR0 = MSG[j];
	}
	for(unsigned j = 0; j < 16; j++){
		MSG[j] = ' ';
	}
	MSG[16] = 0;
	LCD_Cursor(0);
}

enum USARTSTATE{WAITSHAKE, WAITTRANSMIT, WAITRECEIVE, TRANSMIT, RECEIVE};
int USARTTICK(int USARTSTATE){
	switch(USARTSTATE){
		case WAITSHAKE :
			if(READYTOSEND == 1){
				PORTD = PORTD | 0x20;
				USARTSTATE = WAITTRANSMIT;
			}
			if(READYTORECEIVE == 1){
				PORTD = PORTD | 0x08;
				USARTSTATE = WAITRECEIVE;
			}
		break;
		case WAITTRANSMIT :
			if(READYTOSEND == 0){
				PORTD = PORTD & 0xDF;
				USARTSTATE = WAITSHAKE;
				break;
			}
			if((PORTD & 0x04) == 0x04){
				USARTSTATE = TRANSMIT;
			}
		break;
		case WAITRECEIVE :
			if(READYTORECEIVE == 0){
				PORTD = PORTD & 0xF7;
				USARTSTATE = WAITSHAKE;
				break;
			}
			if((PIND & 0x10) == 0x10){
				USARTSTATE = RECEIVE;
			}
		break;
		case TRANSMIT :
			transmit_string(MSG);
			PORTD = PORTD & 0xDF;
			READYTOSEND = 0;
			USARTSTATE = WAITSHAKE;
		break;
		case RECEIVE :
			receive_string(MSG2);
			PORTD = PORTD & 0xF7;
			eeprom_update_block(MSG2, (void *)1, 17);
			READYTORECEIVE = 0;
			USARTSTATE = WAITSHAKE;
		break;
		default :
			USARTSTATE = WAITSHAKE;
		break;
	}
	
	return USARTSTATE;
}

//Updates the strings to be displayed===================================================
enum LCDSTATE{DISPLAY, SENDING, RECE, OKAY1, OKAY2};
int LCDTICK(int LCDSTATE){
	static unsigned char WAIT = 0;
	switch(LCDSTATE){
		case DISPLAY :
			LCD_DisplayString(1, MSG);
			LCD_DisplayString(17, MSG2);
			if(joyUp == 0x01 && up == 0){
				keyChar = 0;
				t = 0;
				up = 1;
			}
			if(joyDown == 0x01 && up == 1){
				keyChar = 0;
				t = 16;
				up = 0;
			}
	
			if(joyRight == 0x01 && t < 11 && up == 1){
				keyChar = 0;
				t++;
				joyRight2 = 1;
			}
			if(joyLeft == 0x01 && t > 0 && up == 1){
				keyChar = 0;
				t--;
				joyLeft2 = 1;
			}
			if(joyRight == 0x01 && t < 32 && up == 0){
				t++;
				joyRight2 = 1;
			}
			if(joyLeft == 0x01 && t > 16 && up == 0){
				t--;
				joyLeft2 = 1;
			}
			LCD_Cursor(MSGSIZE + t + 1);
			if(READYTOSEND == 1){
				LCDSTATE = SENDING;
				LCD_DisplayString(1, WAITMESSAGE);
				break;
			}
			if(READYTORECEIVE == 1){
				LCDSTATE = RECE;
				LCD_DisplayString(1, WAITMESSAGE);
				break;
			}
			LCDSTATE = DISPLAY;
		break;
		case SENDING :
			if(READYTOSEND == 1)
				LCDSTATE = SENDING;
			else{
				LCD_DisplayString(1, DONEMESSAGE);
				LCDSTATE = OKAY1;
			}
		break;
		case RECE :
			if(READYTORECEIVE == 1)
				LCDSTATE = RECE;
			else{
				LCD_DisplayString(1, DONEMESSAGE);
				LCDSTATE = OKAY2;
			}
		break;
		case OKAY1 :
			LCD_Cursor(5);
			LCD_WriteData(3);
			LCD_Cursor(6);
			LCD_WriteData(2);
			LCD_Cursor(7);
			LCD_WriteData(1);
			WAIT++;
			if(WAIT < 40){
				LCDSTATE = OKAY1;
			}else{
				LCDSTATE = DISPLAY;
			}
			LCD_Cursor(0);
		break;
		case OKAY2 :
			LCD_Cursor(5);
			LCD_WriteData(3);
			LCD_Cursor(6);
			LCD_WriteData(2);
			LCD_Cursor(7);
			LCD_WriteData(1);
			WAIT++;
			if(WAIT < 20){
				LCDSTATE = OKAY2;
			}else{
				LCDSTATE = DISPLAY;
			}
			LCD_Cursor(0);
		break;
		default :
			LCDSTATE = DISPLAY;
		break;
	}
	return LCDSTATE;
}




//Keyboard function for entry into the message to be sent===============================
enum KeyState{WAIT, ONE, ZERO, ENTER, SEND, RECEIVING};
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
					keyChar = 0;
					if(t < 11)
						t++;
					KeyState = ENTER;
				}else if((PINA & 0xF0) == 0xE0){
					KeyState = SEND;
					READYTOSEND = !READYTOSEND & 0x01;
				}else if((PINA & 0xF8) == 0xF0){
					KeyState = RECEIVING;
					READYTORECEIVE = !READYTORECEIVE & 0x01;
				}else{
					KeyState = WAIT;
				}
			break;
			case ONE :
				if((PINA & 0xF0) == 0x70){
					KeyState = ONE;
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
			case SEND :
				if((PINA & 0xF0) == 0xE0){
					KeyState = SEND;
				}else{
					KeyState = WAIT;
				}
			break;
			case RECEIVING :
				if((PINA & 0xF8) == 0xF0){
					KeyState = RECEIVING;
				}else{
					KeyState = WAIT;
				}
			break;
			
			default:
			KeyState = WAIT;
			break;
		}
		if(keyChar != 0)
			MSG[t] = keyChar;
	}
	return KeyState;
}



void LCD_build(){
	LCD_WriteCommand(0x48);
	LCD_WriteData(0x00);
	LCD_WriteData(0x10);
	LCD_WriteData(0x30);
	LCD_WriteData(0x70);
	LCD_WriteData(0xFF);
	LCD_WriteData(0xFF);
	LCD_WriteData(0xFF);
	LCD_WriteData(0xFF);
	LCD_WriteCommand(0x50);
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x24);
	LCD_WriteData(0x00);
	LCD_WriteData(0x42);
	LCD_WriteData(0x24);
	LCD_WriteData(0x18);
	LCD_WriteCommand(0x58);
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x02);
	LCD_WriteData(0x00);
	LCD_WriteData(0x04);
	LCD_WriteData(0x02);
	LCD_WriteData(0x01);
}










//Function to use to pull joystick input================================================
enum JoyState {INPUTJOY};
int JoyStickTick(int JoyState){
	if(joyRight2 == 1){
		joyRight = 0;
	}
	if(joyLeft2 == 1){
		joyLeft = 0;
	}
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
	if (horiz > 820){
		joyRight = 1;
	}
	if (horiz < 320){
		joyLeft = 1;
	}
	if (vert > 820){
		joyUp = 1;
	}
	if (vert < 320){
		joyDown = 1;
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
	
	joyUp = 0;
	joyDown = 0;
	joyLeft = 0;
	joyRight = 0;
	joyUp2 = 0;
	joyDown2 = 0;
	joyLeft2 = 0;
	joyRight2 = 0;
	sent = 0;
	received = 0;
	
	READYTORECEIVE = 0;
	READYTOSEND = 0;
	
	UCSR0B |= (1 << RXEN0)  | (1 << TXEN0);
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);
	UBRR0L = BAUD_PRESCALE;
	UBRR0H = (BAUD_PRESCALE >> 8);
	
	eeprom_read_block(MSG2, 0x01, 12);
	
	up = 1;
	t = 0;
	keyChar = 0x00;
	MSGSPOT = 0;
	MSGSIZE = 0;
	
	DDRD = 0xAA; PORTD = 0x55;
	DDRC = 0xFF; PORTC = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRA = 0x00; PORTA = 0xFF;//Joystick input.
	
	ADMUX |= (1 << REFS0);//setting the reference of ADC
	ADCSRA |= (1<<ADPS0) | (1<ADPS1)| (1<ADPS2) | (1<<ADEN) ;
	static task task1, task2, task3, task4;
	task *tasks[] = { &task1, &task2, &task3, &task4};
	//task1 init
	task1.state = -1;
	task1.period = 40;
	task1.elapsedTime = 40;
	task1.TickFct = &KeyBoardTick;
	
	task2.state = -1;
	task2.period = 20;
	task2.elapsedTime = 20;
	task2.TickFct = &JoyStickTick;
	
	task3.state = -1;
	task3.period = 200;
	task3.elapsedTime = 200;
	task3.TickFct = &LCDTICK;
	
	task4.state = -1;
	task4.period = 20;
	task4.elapsedTime = 20;
	task4.TickFct = &USARTTICK;
	
	TimerSet(20);
	TimerOn();
	LCD_init();
	LCD_build();
	LCD_Cursor(0);
	while(1)
	{
		
		for ( unsigned i = 0; i < 4; i++ ) {
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 20;
		}
	
		//Remember to add timer to code
		while(!TimerFlag);
		TimerFlag = 0;
	}
}