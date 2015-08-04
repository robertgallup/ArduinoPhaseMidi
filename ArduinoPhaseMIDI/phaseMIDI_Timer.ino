//  Macros, interrupt routine, and setup function to
//  set up Timer2
// 
//  The MIT License (MIT)
//  
//  Copyright (c) 2013 Robert W. Gallup (www.robertgallup.com)
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
// 

#define SET_SCALER2(a,b,c) \
  TCCR2B &= ~((1<<CS22) | (1<<CS21) | (1<<CS20));     /* Mask */ \
  TCCR2B |=  ((a<<CS22) | (b<<CS21) | (c<<CS20))      /* Set  */

#define SET_TIMER_TYPE2(a,b) \
  TIMSK2 &= ~((1<<TOIE2) | (1<<OCIE2A));              /* Mask */ \
  TIMSK2 |=  ((a<<TOIE2) | (b<<OCIE2A))               /* Set  */
  
#define SET_TIMER_MODE2(a,b,c)                        /* Sets WGM22:WGM21:WGM20 */ \
  TCCR2B &= ~(1<<WGM22);                              /* Mask */ \
  TCCR2B |=  (a<<WGM22);                              /* Set  */ \
  TCCR2A &= ~((1<<WGM21) | (1<<WGM20));               /* Mask */ \
  TCCR2A |=  ((b<<WGM21) | (c<<WGM20))                /* Set  */

// Interrupt vector -- Tracks ticks and beats
ISR(TIMER2_COMPA_vect) {
  tick++;
  if (tick > ticksPerBeat) {
    tick = 0;
    beat++;
  }
};

void setupTimer()
{
  cli();                           // Clear interrupts (just for good measure)
  ASSR &= ~(1<<AS2);               // Clear asynchronous mode (use internal clock)
  SET_SCALER2 (1, 0, 0);           // Prescaler /64
  SET_TIMER_TYPE2 (0, 1);          // Compare mode (vs. overflow mode)  
  SET_TIMER_MODE2 (0, 1, 0);       // CTC mode (clear timer on compare match)
  OCR2A = 250;                     // Compare value
  sei();                           // Turn on interrupts
}
