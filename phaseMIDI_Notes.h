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
#include "Arduino.h"

#define MIDI_NOTE_ON 0x90

struct midiNote {
  int   beat;
  byte  note;
  byte  velocity;
  int   duration;
};

// Sends a MIDI command
// If DEBUG is defined, prints the command using standard print commands
// If not debug, sends raw data out the Serial port
void midiSendCommand (midiCommand c)
{
  
#ifdef DEBUG
  Serial.println (c.command, HEX);
  Serial.println (c.data1, HEX);
  if (!(c.data2 & 0x80)) Serial.println (c.data2, HEX);
  Serial.println("-------------");
#else
  Serial.write (c.command);
  Serial.write (c.data1);
  if (!(c.data2 & 0x80)) Serial.write (c.data2);
#endif

}

