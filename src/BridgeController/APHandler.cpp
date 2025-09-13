#include "APHandler.h"

APHandler::APHandler(IPAddress ipAddr, IPAddress gw, IPAddress sn)
  : ip(ipAddr), gateway(gw), subnet(sn), server(80)
{
    ssid = "Systems12_AP";
    password = "sys12-@#";
}


void APHandler::begin() {
    if (WiFi.softAP(ssid, password)) {
        Serial.println("AP started successfully");
    } else {
        Serial.println("AP start failed!");
    }
    WiFi.softAPConfig(ip, gateway, subnet);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    server.begin();
}

WiFiClient APHandler::getClient() {
    return server.available();
}
