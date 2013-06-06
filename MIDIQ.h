//#define DEBUG
///////////////////////////////////////////////////////////////
//
//  MIDIQ.h
//
//  MIDI Queue - manages random in / ordered out MIDI queue
//  MIDI events are added to a B-Tree. They can then be easily retreived
//  in order to be sent out at the appropriate time.
//
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

#ifndef MIDIQ_h
#define MIDIQ_h

#include "Arduino.h"

#define QNULL 0
#define QLENGTH 25

struct midiCommand {
  uint8_t   command;
  uint8_t   data1;
  uint8_t   data2;
};

struct queueEvent {
  uint16_t      beat;
  uint16_t      tick;
  midiCommand   cmd;
  uint8_t       next;
};

// CLASS
class MIDIQ
{
  public:
  
    MIDIQ();                                       // Constructor
    int eventEnQ(queueEvent*);                     // Queue MIDI event
    boolean eventDeQ(uint16_t, uint8_t);           // Dequeue MIDI event
    void queueClear();

    queueEvent event;
    
#ifdef DEBUG
    void printHeap();
    void printQueue();
    void printNode(int);
    void printList(int);
#endif

  private:
  
    queueEvent queue [QLENGTH];    
    int queueHead;
    int heapHead;

    int allocEvent();
    void freeEvent(int);
  
};

#endif
