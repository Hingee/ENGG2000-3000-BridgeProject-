#include "APHandler.h"

APHandler::APHandler(const IPAddress ipAddr, const IPAddress gw, const IPAddress sn)
  : ip(ipAddr), gateway(gw), subnet(sn), server(80) {
  ssid = "Systems12_AP";
  password = "sys12-@#";
}


void APHandler::begin() {
  Serial.println("[APHandler] Initializing Access Point...");

  if (!WiFi.softAP(ssid, password)) {
    Serial.println("[APHandler] ERROR: Failed to start Access Point!");
    return;  //Defensive return in case of error
  }

  if (!WiFi.softAPConfig(ip, gateway, subnet)) {
    Serial.println("[APHandler] WARNING: Failed to configure network parameters!");
  }

  Serial.print("[APHandler] Access Point started. IP address: ");
  Serial.println(WiFi.softAPIP());
  server.begin();
  Serial.println("[APHandler] Web server started on port 80");
}

WiFiClient APHandler::getClient() {
  return server.available();
}
