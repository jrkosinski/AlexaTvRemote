
#include "debug.h"              // Serial debugger printing
#include "WifiConnection.h"     // Wifi connection 
#include "Wemulator.h"          // Our Wemo emulator 
#include <IRremoteESP8266.h>    // IR library 


WifiConnection* wifi;           // wifi connection
Wemulator* wemulator;           // wemo emulator
IRsend* irSend;                 // infrared sender
bool commandReceived = false;   // command flag


//SET YOUR WIFI CREDS 
const char* myWifiSsid      = "***"; 
const char* myWifiPassword  = "*******";

//SET TO MATCH YOUR HARDWARE 
#define SERIAL_BAUD_RATE    9600

//PIN 0 is D3 ON THE CHIP 
#define IR_PIN              0


//turn on/off the tv by sending IR command
void toggleTv();



/*---------------------------------------*/
//Runs once, when device is powered on or code has just been flashed 
void setup()
{
  //if set wrong, your serial debugger will not be readable 
  Serial.begin(SERIAL_BAUD_RATE);
  
  //initialize wifi connection 
  wifi = new WifiConnection(myWifiSsid, myWifiPassword); 
  wifi->begin(); 

  //initialize the IR 
  irSend = new IRsend(IR_PIN);
  irSend->begin();

  //initialize wemo emulator 
  wemulator = new Wemulator(); 

  //connect to wifi 
  if (wifi->connect())
  {
    //start the wemo emulator (it runs as a series of webservers) 
    wemulator->addCommand("tv"); 
    wemulator->addCommand("television"); 
    wemulator->addCommand("my tv"); 
    wemulator->addCommand("my television"); 

    //set the event handler for when a voice command is received 
    wemulator->onMessage([](unsigned char device_id, const char * device_name, bool state) 
    {
      //the following is just to show that we don't care about state (i.e "on" or "off")
      //since the command works as a toggle, it's the same command regardless of whether we say "turn on" or "turn off" 
      if (state)
        commandReceived = true;
      else
        commandReceived = true;
    });
  }
}


/*---------------------------------------*/
//Runs constantly 
void loop() 
{
  //let the wemulator listen for voice commands 
  if (wifi->isConnected)
  {
    wemulator->handle();
  }

  //if we've received a command, do the action 
  if (commandReceived)
  {
    commandReceived = false; 
    toggleTv(); 
    delay(100); 
  }
}


/*---------------------------------------*/
//turn on/off the tv by sending IR command. This one is set for LG tv. 
void toggleTv()
{
  DEBUG_PRINTLN("Sending IR command"); 
  irSend->sendLG(0x00FFE01FUL, 0); 
}

