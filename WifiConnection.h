
#ifndef __WIFI_CONNECTION_H__
#define __WIFI_CONNECTION_H__

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include "debug.h" 


/****************************************
 * WifiConnection
 * --------------
 * Connects to the Wifi. 
 */
class WifiConnection
{
  private: 
    ESP8266WiFiMulti _wiFiMulti;
    const char* _wifiPasswd; 
    const char* _wifiSsid; 
    bool _enabled = false; 

  public:
    bool isConnected; 
    
  public:
    WifiConnection(const char* ssid, const char* passwd); 

    void begin(); 
    bool connect(); 
};
/****************************************/


// ************************************************************************************
// constructor 
//  
// args:
//  ssid: wifi username
//  passwd: wifi password 
// 
WifiConnection::WifiConnection(const char* ssid, const char* passwd)
{
  this->isConnected = false;
  this->_wifiSsid = ssid; 
  this->_wifiPasswd = passwd; 
}

// ************************************************************************************
// initializes the instance and gets it ready to use; call once (in setup) 
// 
void WifiConnection::begin()
{
  DEBUG_PRINTLN("Wifi:begin"); 
  this->_enabled = true; 
}

// ************************************************************************************
// attempts to connect to the wifi 
//
// returns: true on successful connection 
// 
bool WifiConnection::connect()
{
  DEBUG_PRINTLN(String("Wifi:connect ") + this->_wifiSsid + " / " + this->_wifiPasswd); 
  
  boolean state = true;
  int i = 0;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(this->_wifiSsid, this->_wifiPasswd);

  // Wait for connection
  DEBUG_PRINTLN("Wifi:Connecting ...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG_PRINT(".");
    if (i > 10){
      state = false;
      break;
    }
    i++;
  }

  DEBUG_PRINTLN(""); 
  if (state){
    DEBUG_PRINT(this->_wifiSsid);
    DEBUG_PRINT("  IP address: ");
    DEBUG_PRINTLN(WiFi.localIP());
  }
  else {
    DEBUG_PRINTLN("Connection failed.");
  }
  
  this->isConnected = state;
  return this->isConnected;
}

#endif
