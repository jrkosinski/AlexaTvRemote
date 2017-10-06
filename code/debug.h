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


#endif
