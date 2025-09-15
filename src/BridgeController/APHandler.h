#ifndef APHANDLER_H
#define APHANDLER_H

#include <WiFi.h>

class APHandler {
    const char* ssid;
    const char* password;
    IPAddress ip, gateway, subnet;
    WiFiServer server;

public:
    APHandler(IPAddress ipAddr, IPAddress gw, IPAddress sn);

    void begin();
    
    WiFiClient getClient();
};

#endif
