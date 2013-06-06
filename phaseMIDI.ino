//#define DEBUG

/////////////////////////////////////////////////////////////////////////////
//
// MIDI PHASE MUSIC 1.0
// 
// Uses the MIDI Thing controller to play a pattern against itself with phasing control (delay)
// There are three controls: a button, a left knob, and a right knob
// 
// The button chooses one of four patterns (switches between them with each press). Pattern changes come only at pattern boundaries.
// The left knob varies overall pace.
// The right knob changes the phase difference.
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

#include <avr/interrupt.h>
#include <avr/io.h>

#include "CS_Pot.h"
#include "CS_LEDBar.h"
#include "CS_Button.h"

#include "MIDIQ.h"

#include "phaseMIDI_Notes.h"

// event structure {beat, tick, cmd, data1, data2, next}
MIDIQ      queue;
queueEvent event;
queueEvent phaseEvent;

// Controls
CS_Pot     knob_L(0);
CS_Pot     knob_R(1);
CS_Button  button(6, true);
CS_LEDBar  LEDx4(2, 4);

// Phasing variables
boolean knobRHooked;
int phaseDelay;

// For counting beats/ticks
#define MIN_TICKS_PER_BEAT 120
#define MAX_TICKS_PER_BEAT 250
volatile int ticksPerBeat = MAX_TICKS_PER_BEAT;
volatile unsigned int beat = 0;
volatile unsigned int tick = 0;

unsigned int oldBeat = 1;
unsigned int newTicksPerBeat;

// Blue LED Flashing
int flashTime = 20;
long int starttime;

// Button
int oldButtonState;
int newButtonState;

byte scale [] = {
  24,26,28,31,33,
  36,38,40,43,45,
  48,50,52,55,57,
  60,62,64,67,69,
  72,74,76,79,81,
  84,86,88,91,93,
  96  
};

// Pattern definition
int patternLengthBeats = 16;
int patternLengthNotes = 16;
int patternLengthFrames = 4;

int patternNote = 0;
byte patternFrame = 0;
byte newPatternFrame = patternLengthFrames;
int  patternBeat = 0;

midiNote pattern[][16] = \
{
  {
  {0, 10, 127, 2},
  {1, 6, 127, 2},
  {2, 30, 127, 1},
  {3, 17, 127, 2},
  {4, 127, 0, 2},
  {5, 27, 127, 2},
  {6, 28, 127, 2},
  {7, 16, 127, 2},
  {8, 22, 0, 2},
  {9, 19, 127, 2},
  {10, 28, 127, 2},
  {11, 16, 127, 2},
  {12, 10, 0, 2},
  {13, 9, 127, 2},
  {14, 27, 127, 2},
  {15, 16, 127, 128}
  },
  
  {
  {0, 4, 127, 24},
  {1, 6, 127, 2},
  {2, 20, 127, 1},
  {3, 17, 127, 2},
  {4, 15, 80, 2},
  {5, 27, 127, 2},
  {6, 20, 127, 2},
  {7, 16, 127, 2},
  {8, 12, 120, 2},
  {9, 19, 127, 2},
  {10, 28, 127, 2},
  {11, 16, 127, 2},
  {12, 127, 0, 2},
  {13, 24, 127, 8},
  {14, 12, 127, 2},
  {15, 22, 127, 128}
  },
  
  {
  {0, 10, 100, 1},
  {1, 10, 127, 2},
  {2, 16, 127, 1},
  {3, 6, 80, 2},
  {4, 127, 0, 2},
  {5, 8, 127, 2},
  {6, 20, 127, 2},
  {7, 6, 127, 2},
  {8, 22, 0, 2},
  {9, 19, 127, 2},
  {10, 28, 127, 2},
  {11, 9, 127, 2},
  {12, 10, 100, 2},
  {13, 6, 127, 2},
  {14, 27, 127, 2},
  {15, 30, 127, 128}
  },
  
  {
  {0, 30, 127, 1},
  {1, 25, 127, 2},
  {2, 29, 127, 1},
  {3, 26, 127, 2},
  {4, 28, 100, 2},
  {5, 27, 127, 2},
  {6, 20, 127, 2},
  {7, 16, 127, 2},
  {8, 15, 127, 62},
  {9, 19, 127, 2},
  {10, 17, 127, 2},
  {11, 16, 127, 2},
  {12, 12, 127, 2},
  {13, 24, 127, 32},
  {14, 27, 127, 2},
  {15, 30, 127, 2}
  }
  
};

// Intermediate variables for MIDI note and command
midiNote note;
midiCommand command;

void setup() {
  
  // MIDI serial speed
#ifdef DEBUG
  Serial.begin (9600);
#else
  Serial.begin (31250);      // MIDI rate
#endif

  // Wait for a few seconds for a button press
  starttime = millis();
  while (((millis() - starttime) < 3000) && (button.state() != LOW));

  // By default, compose a new piece
  // If the button is pressed during startup, use the default piece
  if (button.state() == HIGH) {
    compose();
  } else {
    LEDx4.displayNum(15);
    while (button.state() == LOW) delay(10);
  }
  attract();
  LEDx4.displayNum(0);

  // Wait for it
  while (button.state() == LOW) delay (10);
  
  // Go! Setup timer#2
  setupTimer();
    
  // Start out phase control as unhooked
  knobRHooked = false;

  // Setup Display, etc.
  oldButtonState = LOW;
  
}

void loop() {
  
  // Check button to change to new frame
  newButtonState = button.state() && button.state() && button.state();   // Quick debounce
  if (newButtonState != oldButtonState) {
    if (newButtonState == HIGH) {
      newPatternFrame = (newPatternFrame + 1) % (patternLengthFrames+1);
      if (newPatternFrame >= patternLengthFrames) {
        LEDx4.displayNum (0);
      } else {
        LEDx4.displayNum(0x01 << newPatternFrame);
      }
    }
    oldButtonState = newButtonState;
  };

  // Require phase knob to be returned to zero to "hook" it
  if (knobRHooked) {
    phaseDelay = map(knob_R.value(), 0, 1023, 0, (ticksPerBeat << 1) + ticksPerBeat);
  } else {
    knobRHooked = (0 == knob_R.value());
  }
  
  // Generate notes once a beat
  if (beat != oldBeat) {
    oldBeat = beat;
  
    // Check pattern frame on beat 0
    patternBeat = beat % patternLengthBeats;
    if (0 == patternBeat) {
      patternNote = 0;
      if (patternFrame != newPatternFrame) {
        patternFrame = newPatternFrame;
        knobRHooked = false;
        phaseDelay = 0;
      }
    }
    
    // Check note
    if (patternLengthFrames > patternFrame) {
      note = pattern[patternFrame][patternNote];
      if (note.beat == patternBeat) {
        command.command = MIDI_NOTE_ON;
        command.data1 = scale[note.note];
        command.data2 = note.velocity;
        midiSendCommand (command);
        
        event.beat = beat;
        event.tick = tick;
        event.cmd  = command;
        phaseEvent = event;
        phaseEvent.cmd.command |= 1;
        
        // Original note off
        int tickDuration = note.duration * (ticksPerBeat >> 4);
        delayEvent(&event, 0, tickDuration);
        event.cmd.data2 = 0;
        queue.eventEnQ (&event);
    
        // Phase note and off
        delayEvent(&phaseEvent, 0, phaseDelay);
        queue.eventEnQ (&phaseEvent);    
        delayEvent(&phaseEvent, 0, tickDuration);
        phaseEvent.cmd.data2 = 0;
        queue.eventEnQ (&phaseEvent);
         
        patternNote++; // = (patternNote + 1) % patternLengthNotes;
      }
    }
  }
  
  while (queue.eventDeQ(beat, tick)) midiSendCommand (queue.event.cmd);
  
  newTicksPerBeat = map(knob_L.value(), 0, 1023, MAX_TICKS_PER_BEAT, MIN_TICKS_PER_BEAT);
  if (newTicksPerBeat != ticksPerBeat) ticksPerBeat = newTicksPerBeat;
}

// Delay the input queueEvent by b beats and t ticks
void delayEvent (queueEvent *e, int b, int t)
{
  int newBeat = (*e).beat + b;
  int newTick = (*e).tick + t;
  if (newTick > ticksPerBeat) {
    newBeat += newTick / ticksPerBeat;
    newTick %= ticksPerBeat;
  }
  (*e).beat = newBeat;
  (*e).tick = newTick; 
}

// Vertical cylon-like display while waiting
// for a button press to begin
void attract () {
  oldBeat = 1;
  phaseDelay = 1;
  while (button.state() != LOW) {
    LEDx4.displayNum(oldBeat);
    if (1 == phaseDelay) {
      oldBeat = oldBeat << 1;
    } else {
      oldBeat = oldBeat >> 1;
    }
    if ((8 == oldBeat) || (1 == oldBeat)) {
      phaseDelay = 1 - phaseDelay;
    }
    delay (60);
  }
}

// Simple composition function that generates
// pattern notes for each of the frames
void compose () {

  // Try to create a bit of a random seed
  int seed = 1;
  seed *= analogRead(4); delay(10);
  seed *= analogRead(4); delay(10);
  seed *= analogRead(4); delay(10);
  randomSeed(seed);

  // Generate 4 frames of notes
  for (patternFrame=0; patternFrame < patternLengthFrames; patternFrame++) {
    
    // Starting Note
    int nextNote = random(12, 30);
    
    // patternRange determines how "variable" the notes are (how much the note
    // can jump from note to note
    int patternRange = random (2, 4);
    
    // Generate a note for each note in the pattern
    for (patternBeat=0; patternBeat < patternLengthBeats; patternBeat++) {

      // Display the pattern note number being generated
      LEDx4.displayNum(patternBeat);

      // Save the note in the pattern
      pattern[patternFrame][patternBeat].note = nextNote;

      // Generate the next note (if it goes beyond range on either end
      // move it in by two octaves)
      nextNote += random(-patternRange, patternRange);
      if (oldBeat <  0) nextNote += 10;
      if (oldBeat > 30) nextNote -= 10;

      delay (100);
    }
  }

}
