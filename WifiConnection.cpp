#include "WifiConnection.h" 

/*---------------------------------------*/
WifiConnection::WifiConnection(const char* ssid, const char* passwd)
{
  this->isConnected = false;
  this->_wifiSsid = ssid; 
  this->_wifiPasswd = passwd; 
}

/*---------------------------------------*/
void WifiConnection::begin()
{
  DEBUG_PRINTLN("Wifi:begin"); 
  this->_enabled = true; 
}

/*---------------------------------------*/
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
