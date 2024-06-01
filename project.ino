#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define Mot1f PB0 //8
#define Mot1b PB4 //12
#define Mot1Spd PD6 //6

#define Mot2f PB2 //10
#define Mot2b PB3 //11
#define Mot2Spd PD5 //5

#define Bazar PB1 //9

volatile unsigned char state = '0';

void setSpeed(unsigned char spd1 , unsigned char spd2 ){
	OCR0A=spd1;
	OCR0B=spd2;
}

void setmot(){
	DDRB|= 1<<Mot1f | 1 << Mot1b | 1 << Mot2f | 1 <<Mot2b;
  DDRD |= (1<<Mot1Spd |1<<Mot2Spd); // make port 5 and 6 output to control the speed of mot2 and 1 respectivly 
	// Fast PWM set WGM00 and WGM01 to 1 and WGM02 to 0
	// SET COM0A1 to  1 and COM0A0 to 0 and the same for B to make
	//Non-inverted Clear OC0 on compare match  set 0C0 at TOP
	TCCR0A=1<<WGM00 | 1 <<WGM01  | 1<< COM0A1 | 1<< COM0B1;
	TCCR0B=0x03;  //clk /64 
}

void backward() {
	PORTB |= (1 << Mot1f);
	PORTB &= ~(1 << Mot1b);
	PORTB |= (1 << Mot2f);
	PORTB &= ~(1 << Mot2b);
	setSpeed(180 , 180);
}

void forward() {
	PORTB &= ~(1 << Mot1f);
	PORTB |= (1 << Mot1b);
	PORTB &= ~(1 << Mot2f);
	PORTB |= (1 << Mot2b);
	setSpeed(180 , 180);
}

void right() {
	PORTB |= (1 << Mot1f);
	PORTB &= ~(1 << Mot1b);
	PORTB &= ~(1 << Mot2f);
	PORTB |= (1 << Mot2b);
	setSpeed(180 , 180);
}

void left() {
	PORTB &= ~(1 << Mot1f);
	PORTB |= (1 << Mot1b);
	PORTB |= (1 << Mot2f);
	PORTB &= ~(1 << Mot2b);
	setSpeed(180 , 180);
}

void stop() {
	PORTB &= ~(1 << Mot1f);
	PORTB &= ~(1 << Mot1b);
	PORTB &= ~(1 << Mot2f);
	PORTB &= ~(1 << Mot2b);
  setSpeed(0,0);
}

// UART Initialization
void initBluetooth() {
  // Initialize UART for Bluetooth communication
  UBRR0H = 0;
  UBRR0L = 103; // 9600 baud rate for 16 MHz clock
  UCSR0B = (1 << RXEN0) | (1 << TXEN0); // Enable RX and TX
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 data bits, 1 stop bit
}

char readBluetoothCommand() {
  // Check if data is available from Bluetooth
  if (UCSR0A & (1 << RXC0)) {
    char command = UDR0;
    while (!(UCSR0A & (1 << UDRE0))); // Wait until buffer is empty
    return command;
  }
  return '\0'; // No command received
}



void enableSig(){
	// Set pin 9 (OC1A) as output
	DDRB |= (1 << Bazar);

	// Configure Timer 1 for Fast PWM mode with ICR1 as top value non inverted
	TCCR1A = (1 << COM1A1) | (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS12) | (1 << CS10); // Prescaler 1024

	// Set the top value for the timer to achieve the desired frequency
	ICR1 = 2500; // Top value for around 6 Hz PWM frequency

	// Set the duty cycle to 50%
	OCR1A = 1250; // Half of ICR1 for 50% duty cycle
}

void disableSig(){
	TCCR1A = 0; // Set TCCR1A to 0 to turn off PWM
  TCCR1B = 0;
  OCR1A = 0;
}



void setObsDet(){
    DDRD &= ~ ( 1<<2 | 1<<3);
}
int main(void)
{
  setObsDet();
	setmot();
	initBluetooth();
	while (1)
	{
    if (( PIND & (1 << 3) ) && ( !(( PIND & (1 << 2) )))){
      	enableSig(); 
        forward();
        _delay_ms(200);
        disableSig(); 
        stop();
    } else if( (( PIND & (1 << 2) )) && (!( PIND & (1 << 3) ) )){
        	enableSig(); 
          backward();
          _delay_ms(200);
          disableSig(); 
          stop();
    } else{
            state = readBluetoothCommand();
            switch(state){
              case 'F':
              forward();
              break;
              
              case 'B':
              backward();
              break;
              
              case 'R':
              right();
              break;
              
              case 'L':
              left();
              break;
              
              case 'S':
              stop();
              break;
            }
    }
		
			
	}
}