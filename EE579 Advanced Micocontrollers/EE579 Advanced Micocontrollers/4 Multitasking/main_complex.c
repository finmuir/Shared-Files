#include "io430.h"

#define IsTime(X) ((CurrentTime.sec == X.sec) && (CurrentTime.ms == X.ms)) 

#define M1_LEDBits BIT0+BIT6                    // Bits for separate LEDs (Mode 1, Port 1)
#define M2_LEDBits BIT1+BIT3                    // Bits for bicolour LED (Mode 2, Port 2)
#define SwitchBit BIT3
#define ClockPeriod 16                          //Clock tick internal in 16384's of a second

struct Time {
   int sec;
   int ms;
}; 

struct Time CurrentTime = {0,0};
struct Time ADCSchedule = {0,512};
struct Time LEDSchedule = {0,512};
struct Time SwitchSchedule = {0,-1}; 

int LEDPeriod = 512;
int ADCPeriod = 512;
int LEDMode = 0;

//Function to return the current time plus the duration in ms

struct Time Schedule (int duration)
{
  struct Time newtime;
  newtime.sec = CurrentTime.sec;
  newtime.ms = CurrentTime.ms+duration;         //add in the duration in ms
  while ((newtime.ms-=1024) >= 0)               //subtract whole seconds until negative
    if (newtime.sec++ == 60) newtime.sec = 0;   //adding to seconds each time
                                                //roll seconds over at a minute
  newtime.ms+=1024;                             //add back the extra 1024 that made us negative
  return newtime;                               //and return that time
} 

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
                                            // Set up the ADC
  ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
  P1DIR |= M1_LEDBits;                      // Make Mode 1 LED bit(s) output
  P2DIR |= M2_LEDBits;                      // Make Mode 2 LED bit(s) output 
  P1OUT &= ~(M1_LEDBits);                   // Start with separate LEDs off
  P2OUT |= BIT1;                            // And one colour on the bicolour, so XORing swaps it
  P1OUT |= SwitchBit;                       // Select pull up resistor on switch
  P1REN |= SwitchBit;                       // and enable 
  P1IES |= SwitchBit;                       // Switch high to low edge
  P1IFG &= ~SwitchBit;                      // Switch IFG cleared
  P1IE |=  SwitchBit;                       // Switch interrupt enabled
  ADC10CTL1 = INCH_1;                       // input A1
  ADC10AE0 |= 0x02;                         // PA.1 ADC option select
  TA0CCTL0 = CCIE;                          // CCR0 interrupt enabled
  TA0CCR0 = ClockPeriod;                    // Have time tick every ~ms 
  TA0CTL = TASSEL_1 + MC_1;                 // ACLK, upmode

  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  if (CurrentTime.ms++ == 1024)
  {
    CurrentTime.ms = 0;
    if (CurrentTime.sec++ == 60) CurrentTime.sec = 0;
  }
  if (IsTime(LEDSchedule)) 
  {
    if (LEDMode)                                        // Depending on the mode
      P1OUT ^= M1_LEDBits;                              // Toggle the separate LEDs
    else
      P2OUT ^= M2_LEDBits;                              // Or the bicolour
    if (LEDPeriod>0) LEDSchedule = Schedule(LEDPeriod); // If I'm not stopped, reschedule
    else LEDSchedule.ms = -1;                           
  }
  if (IsTime(SwitchSchedule))                           // End of debounce?
  {
    if (!(P1IN & SwitchBit))                            // Valid press?
    {
      if (LEDMode)                                      // If so, swap the LED mode
      {
        LEDMode = 0;                                    // And intialise the other LEDs
        P1OUT &= ~(M1_LEDBits);                         // Turn off the old set
        P2OUT |= BIT1;                                  // And turn on one of the new
      }
      else
      {
        LEDMode = 1;                                    // Same for teh other port if other mode
        P1OUT |= BIT0;
        P2OUT &= ~(M2_LEDBits);
      }
    }
    SwitchSchedule.ms = -1;
  }
  if (IsTime(ADCSchedule))                              // Time for an ADC conversion?
  {
    ADCSchedule = Schedule(ADCPeriod);                  // If so, schedule the next
    ADC10CTL0 |= ENC + ADC10SC;                         // And start the conversion
  }
}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port1 (void)
{
    P1IFG &= ~SwitchBit;                       // Clear switch interrupt flag
    SwitchSchedule = Schedule(10);
}

// ADC interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
  int LEDtime;
  LEDtime = ADC10MEM + 100;
  if (LEDtime > 1000) LEDPeriod = -1;                           //if top of range, switch off
  else 
  {
    if (LEDSchedule.ms == -1) LEDSchedule = Schedule(LEDtime);  //if LED toggling stopped, restart
    LEDPeriod = LEDtime;                                        //set LED period to ADC value +100
  }
}