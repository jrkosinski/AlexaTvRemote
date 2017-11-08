  /*

FAUXMO ESP 2.2.0

Copyright (C) 2016 by Xose Pérez <xose dot perez at gmail dot com>

The MIT License (MIT)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#ifndef __FAUXMO_H__
#define __FAUXMO_H__

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
        "<modelName>FauxmoESP</modelName>"
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

#ifdef DEBUG_FAUXMO
    #define DEBUG_MSG_FAUXMO(...) DEBUG_FAUXMO.printf( __VA_ARGS__ )
#else
    #define DEBUG_MSG_FAUXMO(...)
#endif

#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <WiFiUdp.h>
#include <functional>
#include <vector>
#include <ESP8266WiFi.h>

typedef std::function<void(unsigned char, const char *, bool)> TStateFunction;

typedef struct {
    char * name;
    char * uuid;    
    bool hit;
    AsyncServer * server;
} fauxmoesp_device_t;

class Fauxmo 
{
  private:
    bool _enabled = true;
    unsigned int _basePort = DEFAULT_TCP_BASE_PORT;
    std::vector<fauxmoesp_device_t> _devices;
    WiFiUDP _udp;
    AsyncClient* _clients[TCP_MAX_CLIENTS];
    TStateFunction _callback = NULL;
    unsigned int _roundsLeft = 0;
    unsigned int _current = 0;
    unsigned long _lastTick;
    IPAddress _remoteIP;
    unsigned int _remotePort;
        
  public:
    Fauxmo(unsigned int port = DEFAULT_TCP_BASE_PORT);
    void addDevice(const char * device_name);
    void onMessage(TStateFunction fn) { _callback = fn; }
    void enable(bool enable) { _enabled = enable; }
    void handle();
    std::vector<fauxmoesp_device_t>* getDevices() { return &this->_devices;}

  private:
    void sendUDPResponse(unsigned int deviceId);
    void nextUDPResponse();
    void handleUDPPacket(const IPAddress remoteIP, unsigned int remotePort, uint8_t *data, size_t len);
    void sendTCPPacket(AsyncClient *client, const char * response);
    AcConnectHandler getTCPClientHandler(unsigned int deviceId);
    void handleTCPPacket(unsigned int deviceId, AsyncClient *client, void *data, size_t len);
};




Fauxmo::Fauxmo(unsigned int port) 
{
  this->_basePort = port;

  // UDP Server
  this->_udp.beginMulticast(WiFi.localIP(), UDP_MULTICAST_IP, UDP_MULTICAST_PORT);
}

void Fauxmo::sendUDPResponse(unsigned int deviceId) 
{
  fauxmoesp_device_t device = this->_devices[deviceId];
  DEBUG_MSG_FAUXMO("[FAUXMO] UDP response for device #%d (%s)\n", _current, device.name);

  char response[strlen(UDP_TEMPLATE) + 40];
  sprintf_P(response, UDP_TEMPLATE,
    WiFi.localIP().toString().c_str(),
    this->_basePort + _current, device.uuid, device.uuid
  );

  WiFiUDP udpClient;
  udpClient.beginPacket(this->_remoteIP, this->_remotePort);
  udpClient.write(response);
  udpClient.endPacket();
}

void Fauxmo::nextUDPResponse() 
{
  while (this->_roundsLeft) 
  {
    if (this->_devices[this->_current].hit == false) 
      break;
    if (++this->_current == this->_devices.size()) {
      --this->_roundsLeft;
      this->_current = 0;
    }
  }

  if (this->_roundsLeft > 0) 
  {
    this->sendUDPResponse(this->_current);
    if (++this->_current == this->_devices.size())
    {
      --this->_roundsLeft;
      this->_current = 0;
    }
  }
}

void Fauxmo::handleUDPPacket(IPAddress remoteIP, unsigned int remotePort, uint8_t *data, size_t len) 
{
  if (!this->_enabled) 
    return;

  DEBUG_MSG_FAUXMO("[FAUXMO] Got UDP packet from %s\n", remoteIP.toString().c_str());

  data[len] = 0;
  String content = String((char *) data);

  if (content.indexOf(UDP_SEARCH_PATTERN) == 0) 
  {
    if (content.indexOf(UDP_DEVICE_PATTERN) > 0) 
    {
      //DEBUG_PRINTF("[FAUXMO] Search request from %s\n", remoteIP.toString().c_str());
      debugPrintln("[FAUXMO] Search request received");

      // Set hits to false
      for (unsigned int i = 0; i < this->_devices.size(); i++) {
        this->_devices[i].hit = false;
      }

      // Send responses
      this->_remoteIP = remoteIP;
      this->_remotePort = remotePort;
      this->_current = random(0, this->_devices.size());
      this->_roundsLeft = UDP_RESPONSES_TRIES;
    }
  }
}

void Fauxmo::sendTCPPacket(AsyncClient *client, const char * response) 
{
  char buffer[strlen(HEADERS) + strlen(response) + 10];
  sprintf_P(buffer, HEADERS, strlen(response), response);
  client->write(buffer);
}

void Fauxmo::handleTCPPacket(unsigned int deviceId, AsyncClient *client, void *data, size_t len) 
{
  ((char * )data)[len] = 0;
  String content = String((char *) data);

  fauxmoesp_device_t device = _devices[deviceId];

  if (content.indexOf("GET /setup.xml") == 0) 
  {
    DEBUG_MSG_FAUXMO("[FAUXMO] /setup.xml response for device #%d (%s)\n", deviceId, device.name);
    debugPrintln("[FAUXMO] /setup.xml response for device");

    this->_devices[deviceId].hit = true;
    char response[strlen(SETUP_TEMPLATE) + 50];
    sprintf_P(response, SETUP_TEMPLATE, device.name, device.uuid);
    this->sendTCPPacket(client, response);
    client->close();
  }

  if (content.indexOf("POST /upnp/control/basicevent1") == 0) 
  {
    if (content.indexOf("<BinaryState>0</BinaryState>") > 0) 
    {
      if (_callback) _callback(deviceId, device.name, false);
    }

    if (content.indexOf("<BinaryState>1</BinaryState>") > 0) 
    {
      if (_callback) _callback(deviceId, device.name, true);
    }

    this->sendTCPPacket(client, "");
    client->close();
  }
}

AcConnectHandler Fauxmo::getTCPClientHandler(unsigned int deviceId) 
{
  return [this, deviceId](void *s, AsyncClient * client) 
  {
    if (!_enabled) return;

    for (int i = 0; i < TCP_MAX_CLIENTS; i++) 
    {
      if (!_clients[i] || !_clients[i]->connected()) 
      {
        _clients[i] = client;

        client->onAck([this, i](void *s, AsyncClient *c, size_t len, uint32_t time) {
                    //DEBUG_MSG_FAUXMO("[FAUXMO] Got ack for client %i len=%u time=%u\n", i, len, time);
        }, 0);

        client->onData([this, i, deviceId](void *s, AsyncClient *c, void *data, size_t len) {
                    //DEBUG_MSG_FAUXMO("[FAUXMO] Got data from client %i len=%i\n", i, len);
                    this->handleTCPPacket(deviceId, c, data, len);
        }, 0);

        client->onDisconnect([this, i](void *s, AsyncClient *c) {
                    //DEBUG_MSG_FAUXMO("[FAUXMO] Disconnect for client %i\n", i);
                    this->_clients[i]->free();
        }, 0);
        client->onError([this, i](void *s, AsyncClient *c, int8_t error) {
                    //DEBUG_MSG_FAUXMO("[FAUXMO] Error %s (%i) on client %i\n", c->errorToString(error), error, i);
        }, 0);
        client->onTimeout([this, i](void *s, AsyncClient *c, uint32_t time) {
                    //DEBUG_MSG_FAUXMO("[FAUXMO] Timeout on client %i at %i\n", i, time);
                    c->close();
        }, 0);

        return;
      }
    }

    DEBUG_MSG_FAUXMO("[FAUXMO] Rejecting client - Too many connections already.\n");

    // We cannot accept this connection at the moment
    client->onDisconnect([](void *s, AsyncClient *c) {
      delete(c);
    });
        
    client->stop();
  };
}

void Fauxmo::addDevice(const char * deviceName) 
{
  fauxmoesp_device_t newDevice;
  unsigned int deviceId = _devices.size();

  // Copy name
  newDevice.name = strdup(deviceName);

  // Create UUID
  char uuid[15];
  sprintf(uuid, "444556%06X%02X\0", ESP.getChipId(), deviceId); // "DEV" + CHIPID + DEV_ID
  newDevice.uuid = strdup(uuid);

  // TCP Server
  newDevice.server = new AsyncServer(_basePort + deviceId);
  newDevice.server->onClient(this->getTCPClientHandler(deviceId), 0);
  newDevice.server->begin();

  // Attach
  this->_devices.push_back(newDevice);
}

void Fauxmo::handle() 
{
  int len = _udp.parsePacket();
  if (len > 0) 
  {
    IPAddress remoteIP = _udp.remoteIP();
    unsigned int remotePort = _udp.remotePort();
    uint8_t data[len];
    _udp.read(data, len);
    this->handleUDPPacket(remoteIP, remotePort, data, len);
  }

  if (_roundsLeft > 0) 
  {
    if (millis() - _lastTick > UDP_RESPONSES_INTERVAL) 
    {
      _lastTick = millis();
      this->nextUDPResponse();
    }
  }
}

#endif
