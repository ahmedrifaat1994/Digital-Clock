/*
 * Mini_Project.c
 *
 *  Created on: 2 May 2021
 *      Author: A.Rifaat
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


#define delay 5

unsigned char seconds = 0;
unsigned char minutes = 0;
unsigned char hours = 0;


void int0(void){
	MCUCR |= (1<<ISC01); //Enable falling edge
	SREG |= (1<<7); //Enable Global Interrupt in status Register I-bit
	GICR |= (1<<INT0); //Enable Interrupt 0
}

ISR(INT0_vect){
	//INT0 reset the stop watch
	SREG |=(1<<7); //Enable Global Interrupt in status Register I-bit
	seconds = 0;
	minutes = 0;
	hours = 0;
	PORTC = 0xff; //Set the output to 1 == ON
}

void int1(void){
	MCUCR |= (1<<ISC11) | (1<<ISC10); //Enable rising edge
	SREG |=(1<<7); //Enable Global Interrupt in status Register I-bit
	GICR |= (1<<INT1); //Enable Interrupt 1
}

ISR(INT1_vect){
	//INT1 pause the stop watch
	SREG |=(1<<7); //Enable Global Interrupt in status Register I-bit
	TCCR1B &= (~(1<<CS11)); //Stop clock
	TCCR1B &= (~(1<<CS10));
}

void int2(void){
	MCUCSR |= (1<<ISC2); //Enable falling edge
	SREG |=(1<<7); //Enable Global Interrupt in status Register I-bit
	GICR |= (1<<INT2); //Enable Interrupt 2
}

ISR(INT2_vect){
	//INT2 resume the stop watch
	SREG |=(1<<7); //Enable Global Interrupt in status Register I-bit
	TCCR1B |= (1<<CS11) | (1<<CS10); //Start clock
}


void timer1(void){
	SREG |=(1<<7); //Enable Global Interrupt in status Register I-bit
	TCCR1A |= (1<<FOC1A) | (1<<FOC1B); //Enable compare mode in timer 1
	TIMSK |= (1<<OCIE1A); //Choose compare mode
	TCCR1B |= (1<<WGM12) | (0<<CS12) | (1<<CS11) | (1<<CS10); // Choose pre-scaler 64
	/* Why 64?
	 * 1s == 1000ms, so 1000 / 64 = 15625, the is no no fractions as you can see, an accurate second*/
	TCNT1 = 0; //Initial value
	OCR1A = 15625; //Compare value
}

ISR(TIMER1_COMPA_vect){
	SREG |=(1<<7); //Enable Global Interrupt in status Register I-bit

	seconds++;

	if(seconds == 60)
	{
		seconds = 0; //Reset seconds
		minutes++;
	}

	if(minutes == 60)
	{
		minutes = 0; //Reset minutes
		hours++;
	}

	if((hours == 24) && (minutes == 60) && (seconds == 60))
	{
		TCCR1B &= (~(1<<CS11)); //Stop clock
		TCCR1B &= (~(1<<CS10));
	}
}

int main(){
	//OUTPUT pins of PORTA to 7447
	DDRA = 0x0f; //Set the first 4 pins of PORTA to OUTPUT signals to decoder 7447
	PORTA = 0x00; //Set the output to zero/off

	//OUTPUT pins of PORTC to NPN transistors
	DDRC = 0x3f; //Set the first 6 pins of PORTC to OUTPUT signals to NPN transistor
	PORTC = 0xff; //Set the output to 1 == ON

	//Enable INT2
	DDRB |= ~(1<<2); //Set pin 2 of PORTB to INPUT
	PORTB |= (1<<2); //Enable internal pull up resistor of pin 2 of PORTB

	//Enable INT0 and INT1
	DDRD |= (~(1<<2)) | (~(1<<3)); //Set pin 2 and 3 of PORTD to be OUTPUT
	PORTD |= (1<<2); //Enable internal pull up resistor of pin 2 of PORTD

	/* PORTB pin 2 reads 1 as input / pin 2 or INT2 resume the stop watch / Falling edge
	 * PORTD pin 2 reads 1 as input / pin 2 or INT0 reset the stop watch / Falling edge
	 * PORTD pin 3 reads 0 as input / pin 3 or INT1 pause the stop watch / Rising edge
	 */


	timer1();

	int0();
	int1();
	int2();

	while(1){
		//count segments from right to left
			//Seconds Segments
			//First Segment of the seconds segments
			PORTC = 0x01; //opens its NPN
			PORTA = seconds % 10; //OUTPUT on the first segment of the seconds segments
			_delay_ms(delay);

			//Second Segment of the seconds segments
			PORTC = 0x02; //opens its NPN
			PORTA = seconds / 10; //OUTPUT on the second of the seconds segments
			_delay_ms(delay);

			//Minutes Segments
			//Third Segment of the minutes segment
			PORTC = 0x04; //opens its NPN
			PORTA = minutes % 10;//OUTPUT on the first segment of the minutes segments
			_delay_ms(delay);

			//Fourth Segment of the minutes segments
			PORTC = 0x08; //opens its NPN
			PORTA = minutes / 10; //OUTPUT on the second segment of the minutes segments
			_delay_ms(delay);

			//Hours Segments
			//Fifth Segment of the hours segments
			PORTC = 0x10; //opens its NPN
			PORTA = hours % 10; //OUTPUT on the first segment of the hours segments
			_delay_ms(delay);

			//Sixth Segment of the hours segments
			PORTC = 0x20; //opens its NPN
			PORTA = hours / 10; //OUTPUT on the second segment of the hours segments
			_delay_ms(delay);

	}
}
