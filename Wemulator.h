#ifndef __WEMULATOR_H__
#define __WEMULATOR_H__

/* 
 *  Note that there are many Wemo emulators. This one is taken from https://bitbucket.org/xoseperez/fauxmoesp
 *  
 *  I prefer this one because it uses the async TCP library (https://github.com/me-no-dev/ESPAsyncTCP and https://github.com/me-no-dev/ESPAsyncWebServer) 
 *  Using this method, we can run more concurrent listener sockets, something that the ESP8266 struggles with because of its single-core, low-memory environment. 
 *  If you experiment user "simpler" methods, you will likely find that the responsiveness starts to drop off at around 8-10 virtual devices, and eventually the 
 *  Alexa will start ignoring some of your virtual devices. Using this one, however, I can get that number up to 14-16 virtual devices. 
 *  
 *  That's the main advantage of this one. The main drawback is that it's more of a black box; it's less easy to see what it's actually doing under the hood. 
 *  I'll explain it briefly: 
 *  - the Alexa broadcasts a UDP request for Belkin devices on the network 
 *  - the socket listener running here, receives that UDP request, and sends back a response (see: SETUP_TEMPLATE) saying "hi, I am a Wemo Belkin device"
 *  - that sets up the handshake; further communication is done via TCP requests/responses. The Alexa device sends "turn on" and "turn off" commands, which 
 *    are handled by the running web server. 
 *    
 *  For simpler 
 */

#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <WiFiUdp.h>
#include <functional>
#include <vector>
#include <ESP8266WiFi.h>
#include "debug.h" 

#define DEFAULT_TCP_BASE_PORT   52000
#define UDP_MULTICAST_IP        IPAddress(239,255,255,250)
#define UDP_MULTICAST_PORT      1900
#define TCP_MAX_CLIENTS         10

#define UDP_SEARCH_PATTERN      "M-SEARCH"
#define UDP_DEVICE_PATTERN      "urn:Belkin:device:**"

#define UDP_RESPONSES_INTERVAL  100
#define UDP_RESPONSES_TRIES     5

const char UDP_TEMPLATE[] PROGMEM =
    "HTTP/1.1 200 OK\r\n"
    "CACHE-CONTROL: max-age=86400\r\n"
    "DATE: Sun, 20 Nov 2016 00:00:00 GMT\r\n"
    "EXT:\r\n"
    "LOCATION: http://%s:%d/setup.xml\r\n"
    "OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
    "01-NLS: %s\r\n"
    "SERVER: Unspecified, UPnP/1.0, Unspecified\r\n"
    "ST: urn:Belkin:device:**\r\n"
    "USN: uuid:Socket-1_0-%s::urn:Belkin:device:**\r\n\r\n";

const char SETUP_TEMPLATE[] PROGMEM =
    "<?xml version=\"1.0\"?>"
    "<root><device>"
        "<deviceType>urn:Belkin:device:controllee:1</deviceType>"
        "<friendlyName>%s</friendlyName>"
        "<manufacturer>Belkin International Inc.</manufacturer>"
        "<modelName>WemulatorESP</modelName>"
        "<modelNumber>2.0.0</modelNumber>"
        "<UDN>uuid:Socket-1_0-%s</UDN>"
    "</device></root>";

const char HEADERS[] PROGMEM =
    "HTTP/1.1 200 OK\r\n"
    "CONTENT-LENGTH: %d\r\n"
    "CONTENT-TYPE: text/xml\r\n"
    "DATE: Sun, 01 Jan 2017 00:00:00 GMT\r\n"
    "LAST-MODIFIED: Sat, 01 Jan 2017 00:00:00 GMT\r\n"
    "SERVER: Unspecified, UPnP/1.0, Unspecified\r\n"
    "X-USER-AGENT: redsonic\r\n"
    "CONNECTION: close\r\n\r\n"
    "%s\r\n";

#ifdef DEBUG_Wemulator
    #define DEBUG_MSG_Wemulator(...) DEBUG_Wemulator.printf( __VA_ARGS__ )
#else
    #define DEBUG_MSG_Wemulator(...)
#endif

//function callback signature 
typedef std::function<void(unsigned char, const char *, bool)> TStateFunction;

typedef struct {
    char * name;
    char * uuid;    
    bool hit;
    AsyncServer * server;
} Wemulatoresp_device_t;

/****************************************
 * Wemulator
 * --------------
 * Wemo Emulator; runs UDP and TCP listeners to communicate with Alexa, spoofing a Wemo Belkin device. 
 * Uses async TCP for better performance. 
 */
class Wemulator 
{
  private:
    bool _enabled = true;
    unsigned int _basePort = DEFAULT_TCP_BASE_PORT;
    std::vector<Wemulatoresp_device_t> _devices;
    WiFiUDP _udp;
    AsyncClient* _clients[TCP_MAX_CLIENTS];
    TStateFunction _callback = NULL;
    unsigned int _roundsLeft = 0;
    unsigned int _current = 0;
    unsigned long _lastTick;
    IPAddress _remoteIP;
    unsigned int _remotePort;
        
  public:
    Wemulator(unsigned int port = DEFAULT_TCP_BASE_PORT);
    void addCommand(const char * device_name);
    void onMessage(TStateFunction fn) { _callback = fn; }
    void enable(bool enable) { _enabled = enable; }
    void handle();
    std::vector<Wemulatoresp_device_t>* getDevices() { return &this->_devices;}

  private:
    void sendUDPResponse(unsigned int deviceId);
    void nextUDPResponse();
    void handleUDPPacket(const IPAddress remoteIP, unsigned int remotePort, uint8_t *data, size_t len);
    void sendTCPPacket(AsyncClient *client, const char * response);
    AcConnectHandler getTCPClientHandler(unsigned int deviceId);
    void handleTCPPacket(unsigned int deviceId, AsyncClient *client, void *data, size_t len);
};

#endif



