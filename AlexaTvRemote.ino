
//Wifi connection 
#include "WifiConnection.h" 

//Our Wemo emulator 
#include "Wemulator.h" 

//IR library 
#include <IRremoteESP8266.h>


WifiConnection* wifi; 
Wemulator* wemulator;
IRsend* irSend; 
bool commandReceived = false;


//SET YOUR WIFI CREDS 
const char* myWifiSsid      = "****"; 
const char* myWifiPassword  = "********";

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
    wemulator->onMessage([](unsigned char device_id, const char * device_name, bool state) {
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
  irSend->sendLG(0x00FFE01FUL, 0); 
}

