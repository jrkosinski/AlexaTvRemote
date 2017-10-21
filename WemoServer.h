#ifndef __WEMO_SERVER_H__
#define __WEMO_SERVER_H__

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266WebServer.h>
#include <functional> 

#define SERVER_COUNT_LIMIT 20


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
 * WemoServer
 * ----------
 * Web server interface for one single wemo device or command (e.g. "turn on X")
 */
class WemoServer
{
  private:
    char _deviceName[70];
    int _localPort;
    IWemoCallbackHandler* _callbackHandler;
    String _uuid;
    WiFiUDP _udp; 
    String _serial;
    ESP8266WebServer* _server = NULL;
    
  public:
    WemoServer(const char* deviceName, int localPort, IWemoCallbackHandler* callbackHandler);
    ~WemoServer();

    const char* getDeviceName() { return this->_deviceName;}
    int getLocalPort() { return this->_localPort;}

    void respondToSearch(IPAddress& senderIP, unsigned int senderPort);
    void listen();
    void stop();
    
  private:
    void startWebServer();
    void handleEventservice(); 
    void handleUpnpControl(); 
    void handleRoot(); 
    void handleSetupXml();
};
/****************************************/


/*---------------------------------------*/
WemoServer::WemoServer(const char* deviceName, int localPort, IWemoCallbackHandler* callbackHandler)
{
  strcpy(this->_deviceName, deviceName);
  this->_localPort = localPort;
  this->_callbackHandler = callbackHandler;
  
  uint32_t chipId = ESP.getChipId();
  char uuid[64];
  sprintf_P(uuid, PSTR("38323636-4558-4dda-9188-cda0e6%02x%02x%02x"),
          (uint16_t) ((chipId >> 16) & 0xff),
          (uint16_t) ((chipId >>  8) & 0xff),
          (uint16_t)   chipId        & 0xff);

  this->_serial = String(uuid);
  this->_uuid = "Socket-1_0-" + this->_serial+"-"+ String(localPort);

  this->startWebServer(); 
}

/*---------------------------------------*/
WemoServer::~WemoServer()
{
  delete this->_callbackHandler;
  delete this->_server;
}

/*---------------------------------------*/
void WemoServer::respondToSearch(IPAddress& senderIP, unsigned int senderPort) 
{
  DEBUG_PRINTLN("");
  DEBUG_PRINT("Sending response to ");
  DEBUG_PRINTLN(senderIP);
  DEBUG_PRINT("Port : ");
  DEBUG_PRINTLN(senderPort);

  IPAddress localIP = WiFi.localIP();
  char s[16];
  sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);

  String response =
    "HTTP/1.1 200 OK\r\n"
    "CACHE-CONTROL: max-age=86400\r\n"
    "DATE: Sat, 26 Nov 2016 04:56:29 GMT\r\n"
    "EXT:\r\n"
    "LOCATION: http://" + String(s) + ":" + String(this->_localPort) + "/setup.xml\r\n"
    "OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
    "01-NLS: b9200ebb-736d-4b93-bf03-835149d13983\r\n"
    "SERVER: Unspecified, UPnP/1.0, Unspecified\r\n"
    "ST: urn:Belkin:device:**\r\n"
    "USN: uuid:" + this->_uuid + "::urn:Belkin:device:**\r\n"
    "X-User-Agent: redsonic\r\n\r\n";

  DEBUG_PRINTLN(String("Sending response to ") + senderIP.toString() + " at port " + senderPort + " from " + localIP.toString()); 
  DEBUG_PRINTLN(response); 
  this->_udp.beginPacket(senderIP, senderPort);
  this->_udp.write(response.c_str());
  this->_udp.endPacket();
  this->listen();
}

/*---------------------------------------*/
void WemoServer::listen()
{
  if (this->_server != NULL) {
    this->_server->handleClient();
    delay(1);
  }  
}

/*---------------------------------------*/
void WemoServer::stop()
{
  if (this->_server != NULL) {
    this->_server->close();
    delay(1);
  }  
}

/*---------------------------------------*/
void WemoServer::startWebServer()
{
  DEBUG_SHOWHEAP;
  this->_server = new ESP8266WebServer(this->_localPort);
  DEBUG_SHOWHEAP;

  this->_server->on("/index.html", [&]() {
    DEBUG_PRINTLN("index.html");
    this->_server->send(200, "text/plain", "Hello World!");
  });
    
  this->_server->on("/", [&]() {
    this->handleRoot();
  });

  this->_server->on("/setup.xml", [&]() {
    this->handleSetupXml();
  });

  this->_server->on("/upnp/control/basicevent1", [&]() {
    this->handleUpnpControl();
  });

  this->_server->on("/eventservice.xml", [&]() {
    this->handleEventservice();
  });

  //this->_server->onNotFound(handleNotFound);
  this->_server->begin();
  DEBUG_SHOWHEAP;

  DEBUG_PRINTLN("WebServer started on port: ");
  DEBUG_PRINTLN(_localPort);
}

/*---------------------------------------*/
void WemoServer::handleEventservice()
{
  DEBUG_PRINTLN(" ########## Responding to eventservice.xml ... ########\n");

  String eventServiceXml = "<?scpd xmlns=\"urn:Belkin:service-1-0\"?>"
        "<actionList>"
          "<action>"
            "<name>SetBinaryState</name>"
            "<argumentList>"
              "<argument>"
                "<retval/>"
                "<name>BinaryState</name>"
                "<relatedStateVariable>BinaryState</relatedStateVariable>"
                "<direction>in</direction>"
              "</argument>"
            "</argumentList>"
             "<serviceStateTable>"
              "<stateVariable sendEvents=\"yes\">"
                "<name>BinaryState</name>"
                "<dataType>Boolean</dataType>"
                "<defaultValue>0</defaultValue>"
              "</stateVariable>"
              "<stateVariable sendEvents=\"yes\">"
                "<name>level</name>"
                "<dataType>string</dataType>"
                "<defaultValue>0</defaultValue>"
              "</stateVariable>"
            "</serviceStateTable>"
          "</action>"
        "</scpd>\r\n"
        "\r\n";

  this->_server->send(200, "text/plain", eventServiceXml.c_str());
}

/*---------------------------------------*/
void WemoServer::handleUpnpControl()
{
  DEBUG_PRINTLN("########## Responding to  /upnp/control/basicevent1 ... ##########");

  //for (int x=0; x <= HTTP.args(); x++) {
  //  DEBUG_PRINTLN(HTTP.arg(x));
  //}

  String request = this->_server->arg(0);
  DEBUG_PRINT("request:");
  DEBUG_PRINTLN(request);

  if(request.indexOf("<BinaryState>1</BinaryState>") > 0) 
  {
    DEBUG_PRINTLN("Got Turn on request");
    this->_callbackHandler->handleCallback(1);
  }

  if(request.indexOf("<BinaryState>0</BinaryState>") > 0) 
  {
    DEBUG_PRINTLN("Got Turn off request");
    this->_callbackHandler->handleCallback(0);
  }

  DEBUG_PRINTLN("Responding to Control request");
  this->_server->send(200, "text/plain", "");
}

/*---------------------------------------*/
void WemoServer::handleRoot()
{
  this->_server->send(200, "text/plain", "You should tell Alexa to discover devices");
}

/*---------------------------------------*/
void WemoServer::handleSetupXml()
{
  DEBUG_PRINTLN(" ########## Responding to setup.xml ... ########\n");

  IPAddress localIP = WiFi.localIP();
  char s[16];
  sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);

  String setupXml = "<?xml version=\"1.0\"?>"
        "<root>"
         "<device>"
            "<deviceType>urn:Belkin:device:controllee:1</deviceType>"
            "<friendlyName>"+ String(this->_deviceName) +"</friendlyName>"
            "<manufacturer>Belkin International Inc.</manufacturer>"
            "<modelName>Emulated Socket</modelName>"
            "<modelNumber>3.1415</modelNumber>"
            "<UDN>uuid:"+ this->_uuid +"</UDN>"
            "<serialNumber>221517K0101769</serialNumber>"
            "<binaryState>0</binaryState>"
            "<serviceList>"
              "<service>"
                  "<serviceType>urn:Belkin:service:basicevent:1</serviceType>"
                  "<serviceId>urn:Belkin:serviceId:basicevent1</serviceId>"
                  "<controlURL>/upnp/control/basicevent1</controlURL>"
                  "<eventSubURL>/upnp/event/basicevent1</eventSubURL>"
                  "<SCPDURL>/eventservice.xml</SCPDURL>"
              "</service>"
          "</serviceList>"
          "</device>"
        "</root>\r\n"
        "\r\n";

  DEBUG_PRINT("Sending:");
  DEBUG_PRINTLN(setupXml);
  this->_server->send(200, "text/xml", setupXml.c_str());
}

#endif
