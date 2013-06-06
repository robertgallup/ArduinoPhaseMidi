///////////////////////////////////////////////////////////////
//
//  MIDIQ.cpp
//
//  Implements a simple midi queue
//  MIDI events are stored in an array of queueEvent structs 
//  Active events are nodes organized in a binary tree
//  Inactive events are organized in a linked list (LIFO)
//  Events can be enqueued, dequeued. And, the queue can  be cleared.
//
//  Note: Arduino arrays indexes begin at zero, but we're ignoring that one
//  so we can use zero as an end marker.
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

#include "MIDIQ.h"

// Constructor : clear queues
MIDIQ::MIDIQ ()
{
  queueClear();
}

// Queue MIDI event
int MIDIQ::eventEnQ(queueEvent *e)
{

  // Allocate a new event from the heap
  int newP = allocEvent();
  
  // If none left, return error
  if (0 != newP) {
    
    // Else use the allocated event to enqueue this midi event  
    queue[newP] = *e;
    
    int prev = 0;
    int ptr = queueHead;
    while (0 != ptr) {
      if (((queue[ptr].beat > (*e).beat)) || ((queue[ptr].beat == (*e).beat) && (queue[ptr].tick >= (*e).tick))) break;
      prev = ptr;
      ptr = queue[ptr].next;
    }
    if (ptr == queueHead) {
      queue[newP].next = queueHead;
      queueHead = newP;
    } else {
      queue[newP].next = ptr;
      queue[prev].next = newP;
    }
  }
  return newP;
}

// Dequeue MIDI event
boolean MIDIQ::eventDeQ(uint16_t b, uint8_t t)
{
  // Quick Check. If nothing to return, return nothing
  int qB = queue[queueHead].beat;
  int qT = queue[queueHead].tick;
  if ((0 == queueHead) || (b < qB) || ((b == qB) && (t < qT))) return false;
  
  // OK, there's something to return. Return the head of the queue
  int eventNum = queueHead;
  event = queue[eventNum];
  queueHead = event.next;
  freeEvent (eventNum);
  return true;
  
}

// Clear MIDI queue
void MIDIQ::queueClear()
{

  // Initialize heap (use the nextEvent links to make a linked list)
  for (int i=1; i<QLENGTH-1; i++) queue[i].next = i+1;
  queue[QLENGTH].next = 0; // 0 = end of queue
  heapHead = 1;
  
  // Set beginning of queue to null
  queueHead = QNULL;
  
}

///////////////////////////////////
// HELPER ROUTINES
///////////////////////////////////
int MIDIQ::allocEvent()
{

 if (0 == heapHead) return 0;
 
 int h = heapHead;
 heapHead = queue[heapHead].next;
 return h;
 
}

void MIDIQ::freeEvent(int e)
{
 queue[e].next = heapHead;
 heapHead = e;
}

///////////////////////////////////
// DEBUG
///////////////////////////////////

#ifdef DEBUG

void MIDIQ::printQueue()
{
  Serial.println ("> QUEUE ||||||||||||||||||||||||||||||||||||||");
  printList (queueHead);
}

void MIDIQ::printHeap()
{
  Serial.println ("> HEAP |||||||||||||||||||||||||||||||||||||||");
  printList (heapHead);
}

void MIDIQ::printList(int head)
{   
  int ptr = head;
  while (ptr != 0) {
    printNode(ptr);
    ptr = queue[ptr].next;
  }  
}

void MIDIQ::printNode(int ptr)
{
    Serial.println ("------------------------");
    Serial.print ("\nEvent : "); Serial.println(ptr);
    Serial.print ("Beat  : "); Serial.println(queue[ptr].beat);
    Serial.print ("Tick  : "); Serial.println(queue[ptr].tick);
    Serial.print ("Cmd   : "); Serial.println(queue[ptr].midiCommand);
    Serial.print ("Data1 : "); Serial.println(queue[ptr].midiData1);
    Serial.print ("Data2 : "); Serial.println(queue[ptr].midiData2);
    Serial.print ("Next  : "); Serial.println(queue[ptr].next); Serial.println();
}


#endif
