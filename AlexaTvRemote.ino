
#include "debug.h"              // Serial debugger printing
#include "WifiConnection.h"     // Wifi connection 
#include "Wemulator.h"
#include "WemoCallbackHandler.h"
#include <IRsend.h>    // IR library 


WifiConnection* wifi;           // wifi connection
IRsend* irSend;                 // infrared sender
Wemulator* wemulator;           // wemo emulator
bool setupDone = false;

//This is used as a crude workaround for a threading issue
bool commandReceived = false;   // command flag

//SET YOUR WIFI CREDS 
const char* myWifiSsid      = "mina"; 
const char* myWifiPassword  = "HappyTime";

//SET TO MATCH YOUR HARDWARE 
#define SERIAL_BAUD_RATE    9600

//PIN 0 is D3 ON THE CHIP 
#define IR_PIN              0
#define LED_PIN             2

//turn on/off the tv by sending IR command
void toggleTv();
void doSetup();
void blinkLed(int, int);


// ************************************************************************************
//Runs once, when device is powered on or code has just been flashed 
//
void setup()
{
  //if set wrong, your serial debugger will not be readable 
  Serial.begin(SERIAL_BAUD_RATE);

  //initialize the IR 
  irSend = new IRsend(IR_PIN, true);
  irSend->begin();
  digitalWrite(IR_PIN, HIGH);

  doSetup();
}

// ************************************************************************************
// do wifi and wemo emulator setup
//
void doSetup()
{
  //initialize wifi connection 
  wifi = new WifiConnection(myWifiSsid, myWifiPassword); 
  wifi->begin(); 

  //initialize wemo emulator 
  wemulator = new Wemulator(); 

  //connect to wifi 
  if (wifi->connect())
  {
    wemulator->begin();
    
    wemulator->addDevice("tv", 80, new WemoCallbackHandler(&commandReceived)); 
    wemulator->addDevice("television", 80, new WemoCallbackHandler(&commandReceived)); 

    setupDone = true;
  }
}


// ************************************************************************************
// Runs constantly 
//
void loop() 
{
  if (!setupDone)
    doSetup(); 
    
  //let the wemulator listen for voice commands 
  if (wifi->isConnected)
  {
    //blinkLed(1, 100);
    wemulator->listen();
  }

  //if we've received a command, do the action 
  if (commandReceived)
  {
    debugPrintln("COMMAND OUTGOING:");
    commandReceived = false; 
    toggleTv(); 
    delay(100); 
  }
}


// ************************************************************************************
// turn on/off the tv by sending IR command. This one is set for LG tv (NEC protocol). 
//
void toggleTv()
{
  debugPrintln("Sending IR command"); 
  //irSend->sendLG(0x00FFE01FUL, 32, 13); 
  irSend->sendNEC(0x20DF10EF, 32, 3); 
}

// ************************************************************************************
// blink the LED a given number of times 
//
void blinkLed(int count, int delayMs)
{
  for(int n=0; n<count; n++)
  {
    delay(delayMs);
    debugPrintln("blink");
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_PIN, HIGH);
    delay(delayMs);
  }
}

