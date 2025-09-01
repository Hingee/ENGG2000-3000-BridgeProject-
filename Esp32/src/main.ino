#include <WiFi.h>
#include <WiFiClient.h>
#include <Arduino.h>

// WiFi credentials
const char* ssid = "ENGG2K3K";
const char* pass = "YOUR_PASSWORD";
const char* serverIp = "YOUR_SERVER_IP";
unsigned int serverPort = 9999;
WiFiClient client;

// Forward declarations
void send(String msg);
String receive();

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
        continue;
      }
    }

    send("Hello");
    String msg = receive();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
    delay(500);
  }
  Serial.println("Connected to WiFi");

  // Create task pinned to Core 0
  xTaskCreatePinnedToCore(core0, "Core0", 4096, NULL, 1, NULL, 0);
}

void loop() {
  delay(5000);
}

void send(String msg) {
  client.println(msg);
  Serial.print("Sent: ");
  Serial.println(msg);
}

String receive() {
  if (client.available()) {
    String line = client.readStringUntil('\n');
    Serial.print("Received: ");
    Serial.println(line);
    return line;
  }
  return ".";
}