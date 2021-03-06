// TASK 2

#include <avr/interrupt.h>
//intialise global variables 
volatile uint8_t colourState = 2, configMode = 0, canSwitch = 0, configState = 1, prevConfigState = 0, overflows = 0;
volatile uint16_t runTime = 0;


ISR(TIMER1_OVF_vect){  //Timer1 overflow interupt
   // overflow set to occur every 1 second 
  if(!configMode){
    //Cycle the colour state (2->1->0->2->...) to allow the super loop to change the LEDs accordingly
  	colourState = colourState > 0 ? colourState-1 : 2;
  }
}

// The CTC interrupt, triggered at OCR1A matching
ISR(TIMER2_COMPA_vect)
{
  if(configMode){
  	runTime++; // increments runTime every 1ms in configMode
    if(runTime >= 1000/configState){
    	runTime = 0; // reset runtime every 125, 167, 250 or 500 ms depending on
      				 // the current configuration state
      	overflows++; // increments overFlows 
    }
    // Turning on white LED when less than 125, 167, 250 or 500 ms and for the right amount of times
    if((runTime < 1000/(configState*2)) && (overflows < configState)){
    	PORTB |= (1<<PB0);
    }else{
    	PORTB &= ~(1<<PB0);
    }
    
    //Resetting overflows after 3 seconds
    if(overflows > configState * 3){
    	overflows = 0;
    }
  }
  	
}


ISR(INT1_vect){
  	// Setting a bool which allows the mode to switch under the right conditions in the super loop
	canSwitch = 1;
}

ISR(ADC_vect){
  // Only update the config state when the ADC value changes between the 4 possible ranges
  if(prevConfigState != ((int)(ADCH/64))+1){
    // Assigning config state to either 1,2,3 or 4
  	configState = ((int)(ADCH/64))+1;
    // Setting the TOP values for TIMER1
    ICR1 = (15625.0 * configState) - 1;
    
  }
  prevConfigState = configState;
}

int main(void){
  
  	Serial.begin(9600);
  
 	// Setup
    DDRB |= (1<<DDB1) | (1<<DDB2) | (1<<DDB3) | (1<<DDB0); // set output pins
  	DDRD &= ~(1<<DDD3); //Setting pin 3 to input

    cli(); // Disable all interupts for configuration
  
  	//Intialise timer 1 to Fast PWM mode 
  	TCCR1A = 0;
  	TCCR1B = 0;
  	TCCR1A |= (1<<WGM11);
	TCCR1B |= (1<<WGM12) | (1<<WGM13) | (1<<CS12) | (1<<CS10); //1024 prescaler 
    TCNT1 = 0;
  	ICR1 = 15624;
  	OCR1A = 15624;
  	TIMSK1 |= (1<<TOIE1);

  
  	// Intialise timer 2 to CTC mode
    TCCR2A = 0;
    TCCR2B = 0;
    TCCR2A |= (1 << WGM21); 
    TCCR2B |= (1 << CS22); //64 prescaler

    OCR2A = 249; //Compare match value
  	TIMSK2 |= (1 << OCIE2A); //enable compare interupt vector 
  
  	// potentiometer Set up 
  	ADMUX  |= (1<<REFS0) | (1<<ADLAR);	
  	ADCSRA |= (1<<ADEN) | (1<<ADIE);  
 	
  
  	//Setting Button to detect a rising edge
    EIMSK |= (1<<INT1);
    EICRA |= (1<<ISC11) |(1<<ISC10); 
    
  	sei(); // enable all interupts
    
  	// ****SUPERLOOP****
    while(1){
      //If the button has been pressed and the red LED is on, switch between modes
      if(canSwitch && colourState == 2){
      	canSwitch = 0;
       	configMode = !configMode;
        //Resetting timer
        TCNT1 = 0;
      }      
       
      //Only enables the ADC conversion for the potentiometer in configuration mode
       if(configMode){
         ADCSRA |= (1<<ADSC);
  			while(ADSC == 1){
		}
            
       }
       
       SetLEDS();
      //Small delay for speeding up TinkerCAD simulation
       _delay_ms(1); 
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

