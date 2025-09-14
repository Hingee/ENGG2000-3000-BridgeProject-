#include <Arduino.h>

#include "APHandler.h"
#include "WebServerHandler.h"
#include "BridgeSystem.h"

APHandler ap(IPAddress(192,168,1,1), IPAddress(192,168,1,1), IPAddress(255,255,255,0));
WebServerHandler webHandler;
BridgeSystem bridgeSystem;

void setup() {
    Serial.begin(115200);
    ap.begin();
}

//TODO Add updating functionality so it updates page if a state change occurs
//TODO Add transitioning button states
void loop(){
    WiFiClient client = ap.getClient();
    webHandler.handleClient(client, bridgeSystem);
    delay(1);
}
