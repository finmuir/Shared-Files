#include "io430.h"

#define IsTime(X) ((CurrentTime.sec == X.sec) && (CurrentTime.ms == X.ms)) 

#define LEDBit BIT0
#define SwitchBit BIT3
#define ClockPeriod 16                          //Clock tick internal in 16384's of a second

struct Time {
   int sec;
   int ms;
}; 

struct Time CurrentTime = {0,0};
struct Time ADCSchedule = {0,-1};
struct Time LEDSchedule = {0,512};
struct Time SwitchSchedule = {0,-1}; 

int LEDPeriod = 512;

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
  P1DIR |= LEDBit;                          // Make the LED bit(s) output
  P1OUT |= LEDBit;                          // Start them high
  P1OUT |= SwitchBit;                       // Select pull up resistor on switch
  P1REN |= SwitchBit;                       // and enable 
  P1IES |= SwitchBit;                       // Switch high to low edge
  P1IFG &= ~SwitchBit;                      // Switch IFG cleared
  P1IE |=  SwitchBit;                       // Switch interrupt enabled
  TA0CCTL0 = CCIE;                          // CCR0 interrupt enabled
  TA0CCR0 = ClockPeriod;                             // Have time tick every ~ms 
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
    P1OUT ^= LEDBit;                            // Toggle LEDs
    LEDSchedule = Schedule(LEDPeriod);
  }
  if (IsTime(SwitchSchedule)) 
  {
    if (!(P1IN & SwitchBit))
    {
      LEDPeriod ^= 0x0180;                            // Toggle between 512 and 256
      LEDSchedule = Schedule(LEDPeriod);
    }
    SwitchSchedule.ms = -1;
  }
}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port1 (void)
{
    P1IFG &= ~SwitchBit;                       // Clear switch interrupt flag
    SwitchSchedule = Schedule(10);
}
