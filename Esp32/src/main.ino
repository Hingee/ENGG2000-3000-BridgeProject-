#include <WiFi.h>
#include <WiFiClient.h>
#include <Arduino.h>

// WiFi credentials
#define SSID "ENGG2K3K";
#define PASS = "YOUR_PASSWORD";
#define SERVER_IP = "YOUR_SERVER_IP";
#define SERVER_PORT = 9090;

WiFiClient client;

// Forward declarations
void send(String msg);
String receive();

// Task function
void core0(void *parameter) {
  Serial.println("Task 0 running on core " + String(xPortGetCoreID()));
  client.connect(SERVER_IP, SERVER_PORT)

  receive(); //HELO
  send("OK");
  receive(); //AUTH
  send("OK");

  while (true) {
    receive();
    send("REDY");
    String msg = receive();

    if (msg == "STATRQ") {
      send("STAT . . . ."); //TODO Data
    }else if (msg == "EXEC") {
      send("OK");
      //TODO Execute
    }else if (msg == "QUIT") {
      send("QUIT");
      client.stop();
      Serial.println("Disconnected from server");
      vTaskDelete(NULL);
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(SSID, PASS);
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

int send(String msg) {
  if (client.connected()) {
    client.println(msg);
    Serial.print("Sent: ");
    Serial.println(msg);
    return 1;
  }
  return 0; // Not connected
}

String receive() {
  int timeout = 0;
  while (!client.available()) {
    if (timeout > 500) {
      Serial.println("Receive timeout");
      return ".";
    }
    delay(1);
    timeout++;
  }
  
  String line = client.readStringUntil('\n');
  Serial.print("Received: ");
  Serial.println(line);
  return line;
}