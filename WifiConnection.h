
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




#endif
