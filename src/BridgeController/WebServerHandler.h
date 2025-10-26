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
  void sendDeviceData(WiFiClient& client, BridgeSystem& system);
  void renderFlipButton(WiFiClient& client, BridgeDevice& device);
  void renderTransFlipButton(WiFiClient& client, BridgeDevice& device, String jsonKey);
  void renderRadioButton(WiFiClient& client, BridgeDevice& device);

  void sendHTMLHeader(WiFiClient& client);
  void renderSensorSection(WiFiClient& client);
  void sendJSONHeader(WiFiClient& client);
};

#endif
