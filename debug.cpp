
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
