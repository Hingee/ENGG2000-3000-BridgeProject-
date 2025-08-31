#include <WiFi.h>
#include <WiFiClient.h>
#include <iostream>
#include <string.h>
#include <Arduino.h>
using namespace std;

// WiFi credentials/Net
const char* ssid = "ENGG2K3K";  //Todo change needed
const char* pass = "YOUR_PASSWORD";  //Todo change needed
const char* serverIp = "YOUR_SERVER_IP";  //Todo change needed
unsigned int serverPort = 9999;  //Todo change needed
WiFiClient client;

// Task function
void core0(void *parameter) {
  while (true) {
    Serial.println("Task 0 running on core " + String(xPortGetCoreID()));
    if (!client.connected()) {
      Serial.print("Connecting to server...");
      if (client.connect(serverIp, serverPort)) {
        Serial.println("Connected!");
      } else {
        Serial.println("Connection failed.");
        delay(1000);
        continue; // Retry connection
      }
    }

    send("Hello");
    String msg = recieve();
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Sleep 1s
  }
}


void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Create tasks pinned to specific cores
  xTaskCreatePinnedToCore(core0, "Core0", 2048, NULL, 1, NULL, 0);
}

void loop() {

  delay(5000); // Wait before sending again
}

void send(String msg) {
  client.println(msg);
  Serial.print("Sent: ");
  Serial.println(msg);
}

String recieve() {
  if (client.available()) {
    String line = client.readStringUntil('\n');
    Serial.print("Received: ");
    Serial.println(line);
  }
  return line;
}
