#ifndef __WDEBUG_H__
#define __WDEBUG_H__

#include <Arduino.h> 

#define DEBUG 0
#define DEBUG_PRINT     debugPrint
#define DEBUG_PRINTLN   debugPrintln
#define DEBUG_SHOWHEAP  debugShowHeap()
#define DEBUG_PRINTF    debugPrintf

void debugPrint(const char*); 
void debugPrint(String&); 
void debugPrint(int); 
void debugPrintf(const char* s, ...); 

void debugPrintln(const char*); 
void debugPrintln(String&); 
void debugPrintln(int); 

void debugShowHeap();


// ************************************************************************************
// prints a msg 
//
void debugPrint(const char* s)
{
#ifdef DEBUG
  Serial.print(s); 
#endif
}

// ************************************************************************************
// prints a msg 
//
void debugPrint(String& s)
{
#ifdef DEBUG
  debugPrint(s.c_str()); 
#endif
}

// ************************************************************************************
// prints a numeric value 
//
void debugPrint(int n)
{
#ifdef DEBUG
  Serial.print(n);
#endif
}

// ************************************************************************************
// prints a msg with variable args
//
void debugPrintf(const char* s, ...)
{
#ifdef DEBUG
  va_list args;                     
  Serial.printf(s, args);
#endif
}

// ************************************************************************************
// prints a msg followed by a linebreak
//
void debugPrintln(const char* s)
{
#ifdef DEBUG
  Serial.println(s); 
#endif
}

// ************************************************************************************
// prints a msg followed by a linebreak
//
void debugPrintln(String& s)
{
#ifdef DEBUG
  debugPrintln(s.c_str()); 
#endif
}

// ************************************************************************************
// prints a numeric value followed by a linebreak
//
void debugPrintln(int n)
{
#ifdef DEBUG
  Serial.println(n);
#endif
}

// ************************************************************************************
// prints the current heapsize
//
void debugShowHeap()
{
#ifdef DEBUG
  DEBUG_PRINTLN(String("HEAP: ") + ESP.getFreeHeap()); 
#endif
}

#endif
