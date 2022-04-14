// TASK 1

#include <avr/interrupt.h>
uint8_t colourState = 2; // Intialise global variable

ISR(TIMER1_OVF_vect){ // Timer1 overflow interupt 
  //overflow set to occur every second
  	colourState = colourState > 0 ? colourState-1 : 2; // Loop through colours 
}

int main(void){
  
    DDRB |= (1<<DDB1) | (1<<DDB2) | (1<<DDB3); //Initialise Arduino pins 9-11 as outputs

    cli(); // Disable all interupts for configuration
  
  	//Intialise Timer/Counter1 to Fast PWM mode 
  	TCCR1A = 0;
  	TCCR1B = 0;
  	TCCR1A |= (1<<WGM11); // Set timer to mode 14
	TCCR1B |= (1<<WGM12) | (1<<WGM13) | (1<<CS12); //add 256 prescaler 
    TCNT1 = 0; // set timer to initail value 0
  	ICR1 = 62499; // define the timers top value 
  	TIMSK1 |= (1<<TOIE1); //enable timer/counter1 overflow 
    
  	sei(); // enable all interupts
   
  //****SUPERLOOP****  
    while(1){
        SetLEDS(); 
      _delay_ms(1); // small delay for simulation purposes 
    }  
}

void SetLEDS(){
  //This fucntion writes a specific LED high depending on the colourState
  //Only one LED can be HIGH at any one time, subsequently, the remaining two LEDS are set low
    if (colourState == 0){
        PORTB &= ~((1<<PB2) | (1<<PB3));
        PORTB |= (1<<PB1);
    }else if (colourState == 1){
        PORTB &= ~((1<<PB1) | (1<<PB3));
        PORTB |= (1<<PB2);
    }else if (colourState == 2){
        PORTB &= ~((1<<PB1) | (1<<PB2));
        PORTB |= (1<<PB3);
    }
}