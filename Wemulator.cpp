#include "Wemulator.h" 


/*---------------------------------------*/
Wemulator::Wemulator(unsigned int port) 
{
  this->_basePort = port;

  // UDP Server
  this->_udp.beginMulticast(WiFi.localIP(), UDP_MULTICAST_IP, UDP_MULTICAST_PORT);
}

/*---------------------------------------*/
void Wemulator::sendUDPResponse(unsigned int deviceId) 
{
  Wemulatoresp_device_t device = this->_devices[deviceId];
  DEBUG_PRINTF("[Wemulator] UDP response for device #%d (%s)\n", _current, device.name);

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

/*---------------------------------------*/
void Wemulator::nextUDPResponse() 
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

/*---------------------------------------*/
void Wemulator::handleUDPPacket(IPAddress remoteIP, unsigned int remotePort, uint8_t *data, size_t len) 
{
  if (!this->_enabled) 
    return;

  DEBUG_PRINTF("[Wemulator] Got UDP packet from %s\n", remoteIP.toString().c_str());

  data[len] = 0;
  String content = String((char *) data);

  if (content.indexOf(UDP_SEARCH_PATTERN) == 0) 
  {
    if (content.indexOf(UDP_DEVICE_PATTERN) > 0) 
    {
      DEBUG_MSG_Wemulator("[Wemulator] Search request from %s\n", remoteIP.toString().c_str());

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

/*---------------------------------------*/
void Wemulator::sendTCPPacket(AsyncClient *client, const char * response) 
{
  char buffer[strlen(HEADERS) + strlen(response) + 10];
  sprintf_P(buffer, HEADERS, strlen(response), response);
  client->write(buffer);
}

/*---------------------------------------*/
void Wemulator::handleTCPPacket(unsigned int deviceId, AsyncClient *client, void *data, size_t len) 
{
  ((char * )data)[len] = 0;
  String content = String((char *) data);

  Wemulatoresp_device_t device = _devices[deviceId];

  if (content.indexOf("GET /setup.xml") == 0) 
  {
    DEBUG_PRINTF("[Wemulator] /setup.xml response for device #%d (%s)\n", deviceId, device.name);

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

/*---------------------------------------*/
AcConnectHandler Wemulator::getTCPClientHandler(unsigned int deviceId) 
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
                    //DEBUG_MSG_Wemulator("[Wemulator] Got ack for client %i len=%u time=%u\n", i, len, time);
        }, 0);

        client->onData([this, i, deviceId](void *s, AsyncClient *c, void *data, size_t len) {
                    //DEBUG_MSG_Wemulator("[Wemulator] Got data from client %i len=%i\n", i, len);
                    this->handleTCPPacket(deviceId, c, data, len);
        }, 0);

        client->onDisconnect([this, i](void *s, AsyncClient *c) {
                    //DEBUG_MSG_Wemulator("[Wemulator] Disconnect for client %i\n", i);
                    this->_clients[i]->free();
        }, 0);
        client->onError([this, i](void *s, AsyncClient *c, int8_t error) {
                    //DEBUG_MSG_Wemulator("[Wemulator] Error %s (%i) on client %i\n", c->errorToString(error), error, i);
        }, 0);
        client->onTimeout([this, i](void *s, AsyncClient *c, uint32_t time) {
                    //DEBUG_MSG_Wemulator("[Wemulator] Timeout on client %i at %i\n", i, time);
                    c->close();
        }, 0);

        return;
      }
    }

    DEBUG_PRINTLN("[Wemulator] Rejecting client - Too many connections already.\n");

    // We cannot accept this connection at the moment
    client->onDisconnect([](void *s, AsyncClient *c) {
      delete(c);
    });
        
    client->stop();
  };
}

/*---------------------------------------*/
void Wemulator::addCommand(const char * deviceName) 
{
  Wemulatoresp_device_t newDevice;
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

/*---------------------------------------*/
void Wemulator::handle() 
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


