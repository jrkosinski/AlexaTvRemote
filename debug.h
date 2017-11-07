#ifndef __WDEBUG_H__
#define __WDEBUG_H__

#include <Arduino.h> 

#define DEBUG 0
#define DEBUG_PRINT     debugPrint
#define DEBUG_PRINTLN   debugPrintln
#define DEBUG_SHOWHEAP  debugShowHeap()
#define DEBUG_PRINTF    debugPrintf

/*
#ifdef DEBUG
  #define DEBUG_PRINT     debugPrint
  #define DEBUG_PRINTLN   debugPrintln
#else
  #define DEBUG_PRINT     ;
  #define DEBUG_PRINTLN   ;
#endif
*/

void debugPrint(const char*); 
void debugPrint(String&); 
void debugPrint(int); 
void debugPrintf(const char* s, ...); 

void debugPrintln(const char*); 
void debugPrintln(String&); 
void debugPrintln(int); 

void debugShowHeap();



#include "debug.h" 

/*---------------------------------------*/
void debugPrint(const char* s)
{
#ifdef DEBUG
  Serial.print(s); 
#endif
}

/*---------------------------------------*/
void debugPrint(String& s)
{
#ifdef DEBUG
  debugPrint(s.c_str()); 
#endif
}

/*---------------------------------------*/
void debugPrint(int n)
{
#ifdef DEBUG
  Serial.print(n);
#endif
}

/*---------------------------------------*/
void debugPrintf(const char* s, ...)
{
#ifdef DEBUG
  va_list args;                     
  Serial.printf(s, args);
#endif
}

/*---------------------------------------*/
void debugPrintln(const char* s)
{
#ifdef DEBUG
  Serial.println(s); 
#endif
}

/*---------------------------------------*/
void debugPrintln(String& s)
{
#ifdef DEBUG
  debugPrintln(s.c_str()); 
#endif
}

/*---------------------------------------*/
void debugPrintln(int n)
{
#ifdef DEBUG
  Serial.println(n);
#endif
}

/*---------------------------------------*/
void debugShowHeap()
{
#ifdef DEBUG
  DEBUG_PRINTLN(String("HEAP: ") + ESP.getFreeHeap()); 
#endif
}

#endif
