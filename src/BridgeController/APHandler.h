#ifndef APHANDLER_H
#define APHANDLER_H

#include <WiFi.h>

class APHandler {
public:
  APHandler(const IPAddress ipAddr, const IPAddress gw, const IPAddress sn);
  void begin();
  WiFiClient getClient();

private:
  const char* ssid;
  const char* password;
  IPAddress ip, gateway, subnet;
  WiFiServer server;
};

#endif
