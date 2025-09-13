#ifndef WEBSERVERHANDLER_H
#define WEBSERVERHANDLER_H

#include <Arduino.h>
#include <WiFi.h>

#include "BridgeSystem.h"

class WebServerHandler {
    String header;
public:
    void handleClient(WiFiClient& client, BridgeSystem& system);

private:
    void sendResponse(WiFiClient& client, BridgeSystem& system);
    void sendSensorData(WiFiClient& client, BridgeSystem& system);
    void renderFlipButton(WiFiClient& client, BridgeDevice& device);
    void renderRadioButton(WiFiClient& client, BridgeDevice& device);
};

#endif
