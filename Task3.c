//TASK 3

// Intiliase variables
volatile uint32_t totalTime, runTime, flashGreenTime, flashRedTime;                     
volatile uint32_t entryTime[4] = {0}; // Array to allow multiple cars through

// Maximum Car space avaible for light beam sepperation
uint8_t BlinkPeriod = 200, maxLength = 4, flashGreen = 0, flashRed = 0;
float speed, maxSpeed = 100, dutyCycle = 0;


// The CTC interrupt, triggered at OCR1A matching
ISR(TIMER2_COMPA_vect)
{
    runTime++; // increment timer 1 tick count each millisecond count on timer overflow
}
// External interrupt INT0
ISR(INT0_vect)
{
    addTime();
  	if (!flashRed)
    {
        flashRed = 1;
        flashRedTime = runTime;
    }
  	
}
// External interrupt INT1
ISR(INT1_vect)
{
    if (entryTime[0] != 0)
    { // Check there is still a car to exit
      
      	totalTime = runTime - entryTime[0];
     	removeTime();
        speed = 72000 / (float)(totalTime) ; //Speed in km/h * 1000
        dutyCycle = speed / 100.0; // duty cycle is directly proportioal to the speed
      	//OCR1A is set to the duty cycle multiplied by TOP, where duty cycle is capped at 1 (100%)
      	OCR1A = 62499 * (dutyCycle <= 1 ? dutyCycle : 1);  
      	if (!flashGreen)
    	{
       		flashGreen = 1;
        	flashGreenTime = runTime;
    	}
    }
    
}

int main(void)
{

    // Setting LEDs & PWM Signal
    DDRB |= (1 << DDB1) | (1 << DDB2) | (1 << DDB3); // Settings pins 9, 11 & 13 to output
    DDRD &= ~((1 << DDD3) | (1 << DDD2));            // Setting pin 1 & 2 to input
    cli(); // Disable all interrupts during configuration

    // Setting both Buttons to detect rising edge
    EIMSK |= (1 << INT0) | (1 << INT1);
    EICRA |= (1 << ISC11) | (1 << ISC10) | (1 << ISC01) | (1 << ISC00);

  	// Initilise timer 1 to mode 14 - fast PWM mode 
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1A |= (1 << WGM11);
    TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS12); // add 256 prescaler
    TCNT1 = 0; // reset timer to 0
    ICR1 = 62499; // top value 
  	OCR1A = 0;
    TCCR1A |= (1 << COM1A1); // enables the control of OCR1A pin (Pin 9)
  	
  	
  	//Initialise timer 2 to CTC mode on compare match A
  	TCCR2A = 0;
    TCCR2B = 0;
    TCCR2A |= (1 << WGM21);
    TCCR2B |= (1 << CS22); // add prescaler of 64
    OCR2A = 249; // Compare match value 
    TIMSK2 |= (1 << OCIE2A); //enable compare interupt vector 

    sei(); // Enable all interupts

    while (1)
    {
      //If either of the LEDs should be blinking, turn on for 250ms, then disable
      	if (flashGreen){
            runTime < flashGreenTime + BlinkPeriod ? PORTB |= (1 << PB2) : PORTB &= ~(1 << PB2);
            if (runTime >= flashGreenTime + BlinkPeriod){
                flashGreen = 0;
            }
        }
     	if (flashRed){
            runTime < flashRedTime + BlinkPeriod ? PORTB |= (1 << PB3) : PORTB &= ~(1 << PB3);
            if (runTime >= flashRedTime + BlinkPeriod){
                flashRed = 0;
            }
        }
      //Small delay for speeding up TinkerCAD simulation
      _delay_ms(1);
      
    }
}


void addTime(void)
{
  	//Resetting run time to 1 to avoid overflows
  	if(entryTime[0] == 0 && !flashGreen && !flashRed){
  		runTime = 1;
    }
  	//Adding entry time to end of the queue 
    for (int i = 0; i < maxLength; i++)
    {
        if (entryTime[i] == 0)
        {
            entryTime[i] = runTime;
            //Setting i to immediately exit for loop
            i = maxLength;
        }
    }
}

void removeTime(void)
{
  	//Removing the first entry time
    entryTime[0] = 0;
  	//Shifting the reaming entry times forward in the queue
    for (int i = 0; i < maxLength - 1; i++)
    {
        entryTime[i] = entryTime[i + 1];
        entryTime[i + 1] = 0;
    }
}