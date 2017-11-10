#ifndef __WEMULATOR_H__
#define __WEMULATOR_H__

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <functional> 

#include "Fauxmo.h"

/****************************************
 * IWemoCallbackHandler
 * --------------------
 */
class IWemoCallbackHandler
{
  public:
    virtual void handleCallback(int param);
};

/****************************************
 * Wemulator
 * ---------
 * Handles the wemo-emulating servers, and also the initial UDP request 
 * from the Alexa, scanning for devices on the network. 
 */
class Wemulator
{
  private:
    int _serverCount;
    bool _enabled = false; 
    IWemoCallbackHandler* _callbackHandlers[40]; 
    Fauxmo _fauxmo; 
    int _deviceCount =0; 
    
  public:
    bool isRunning; 
    
  public:
    Wemulator();
    ~Wemulator();

    void begin();
    void listen();
    void stop();
    bool addDevice(const char* deviceName, int localPort, IWemoCallbackHandler* callbackHandler);
    bool replaceDevice(const char* deviceName, const char* newDeviceName, IWemoCallbackHandler* callbackHandler);
    int getDeviceIndexByName(const char* deviceName);
};
/****************************************/

// ************************************************************************************
// constructor 
// 
Wemulator::Wemulator()
{
  this->_serverCount = 0;
  this->isRunning = false;
}

// ************************************************************************************
// destructor 
Wemulator::~Wemulator()
{
}

// ************************************************************************************
// initializes the instance and gets it ready to use; call once (in setup) 
// 
void Wemulator::begin()
{
  this->isRunning = false;
  this->_enabled = true; 
  
  DEBUG_PRINTLN("Wemulator:begin");
 
  std::vector<fauxmoesp_device_t>* devices = (this->_fauxmo.getDevices()); 
  IWemoCallbackHandler** callbackHandlers = this->_callbackHandlers;
  
  this->_fauxmo.onMessage([devices, callbackHandlers](unsigned char deviceId, const char* deviceName, bool state) 
  {
    DEBUG_PRINTLN(String("Wemulator: callback device ") + deviceId + " name: " + deviceName + (state ? " ON" : " OFF")); 

    int index = 0; 
    for (auto & dev : *devices) 
    {
      if (strcmp(deviceName, dev.name) == 0)
      {
        callbackHandlers[index]->handleCallback(1); 
        break;
      }
      
      index++; 
    }
  });
}

// ************************************************************************************
// listen for requests (call in every loop) 
// 
void Wemulator::listen()
{
  if (!this->_enabled)
    return; 

  this->_fauxmo.handle();
}

// ************************************************************************************
// stop listening 
// 
void Wemulator::stop()
{
}

// ************************************************************************************
// adds a new listener for a new command 
// 
// args: 
//  deviceName: the command for which to listen 
//  localPort: deprecated, ignored 
//  callbackHandler: object that will handle a request for the given device 
//
// returns: true on success 
//
// TODO: localPort is ignored, so remove it 
bool Wemulator::addDevice(const char* deviceName, int localPort, IWemoCallbackHandler* callbackHandler)
{
  if (!this->_enabled)
    return false; 
    
  DEBUG_PRINTLN(String("Wemulator: add device ") + deviceName ); 
  this->_fauxmo.addDevice(deviceName); 
  this->_callbackHandlers[this->_deviceCount] = callbackHandler;
  this->_deviceCount++; 
}

#endif 





