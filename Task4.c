// TASK 4

#include <avr/interrupt.h>

// Initialse variables
volatile uint8_t colourState = 2, count = 0, flashLED = 0;
volatile uint16_t runTime = 0;

ISR(TIMER1_OVF_vect) // Timer1 overflow interupt
{
  // Cycle the colour state (2->1->0->2->...) to allow the super loop to change the LEDs accordingly
  colourState = colourState > 0 ? colourState - 1 : 2;
}

// The CTC interrupt, triggered at OCR1A matching
ISR(TIMER2_COMPA_vect)
{
  // If the White LED can blink increment the run time
  if (flashLED)
  {
    runTime++;
  }
}

ISR(INT1_vect) // external interupt 1
{
  // If the white LED is not currently blinking, and the red LED is on, enable
  // The blinking of the white LED and reset the run time
  if (!flashLED && colourState == 2)
  {
    flashLED = 1;
    runTime = 0;
  }

  count++;

  // Set OCR1A depending on the duty cycle, limited from 0-1
  OCR1A = (62499 * ((float)(count > 100 ? 100 : count)) / 100.0);
}

int main(void)
{

  // configure pins 8-12 as outputs
  DDRB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB2) | (1 << DDB3) | (1 << DDB4); // set scope signal as output
  DDRD &= ~(1 << DDD3);                                                        // Setting pin 3 to input

  cli(); // disbable all interupts for configuration

  // Initilise timer 1 to mode 14 - fast PWM mode
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS12); // 256 prescaler
  TCNT1 = 0;                                           // reset timer to 0
  ICR1 = 62499;                                        // top value
  TIMSK1 |= (1 << TOIE1);
  OCR1A = 0;
  TCCR1A |= (1 << COM1A1);

  // Initialise timer 2 to CTC mode on compare match A
  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2A |= (1 << WGM21);
  TCCR2B |= (1 << CS22); // 64 prescaler
  TCNT2 = 0;             // reset timer to 0
  OCR2A = 250;           // top value

  TIMSK2 |= (1 << OCIE2A);

  EIMSK |= (1 << INT1);
  EICRA |= (1 << ISC11) | (1 << ISC10);

  sei(); // enable all interupts

  // ****SUPERLOOP****
  while (1)
  {
    // Setting traffic light LEDs
    SetLEDS();

    if (flashLED)
    {
      // Turning on LED between 0 to 125ms and 250 to 375ms after INT1 interuupt
      (runTime < 125 || (runTime >= 250 && runTime < 375)) ? PORTB |= (1 << PB0) : PORTB &= ~(1 << PB0);

      // Disabling white LED after 500ms
      if (runTime >= 500)
      {
        flashLED = 0;
      }
    }

    _delay_ms(1);
  }
}

void SetLEDS()
{
  // This fucntion writes a specific LED high depending on the colourState
  // Only one LED can be HIGH at any one time, subsequently, the remaining two LEDS are set low
  if (colourState == 0)
  {
    PORTB &= ~((1 << PB4) | (1 << PB3));
    PORTB |= (1 << PB2);
  }
  else if (colourState == 1)
  {
    PORTB &= ~((1 << PB2) | (1 << PB3));
    PORTB |= (1 << PB4);
  }
  else if (colourState == 2)
  {
    PORTB &= ~((1 << PB2) | (1 << PB4));
    PORTB |= (1 << PB3);
  }
}