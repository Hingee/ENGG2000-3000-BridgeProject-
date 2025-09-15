#include <Arduino.h>

#include "APHandler.h"
#include "WebServerHandler.h"
#include "BridgeSystem.h"

#define trigPin 17   // D17 = GPIO17
#define echoPin 16   // D16 = GPIO16

long duration;
int distance;
APHandler ap(IPAddress(192,168,1,1), IPAddress(192,168,1,1), IPAddress(255,255,255,0));
WebServerHandler webHandler;
BridgeSystem bridgeSystem;

void setup() {
    Serial.begin(115200);
    ap.begin();

    // Ultrasonic sensor setup
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    xTaskCreatePinnedToCore(networkTask,
        "NetworkTask",
        8192,
        NULL,
        1,
        NULL,
        0 // run on core 0
    );
}

// ============================
// Task for Networking
// ============================
void networkTask(void *parameter) {
  for (;;) {
    WiFiClient client = ap.getClient();
    webHandler.handleClient(client, bridgeSystem);
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

//TODO Add transitioning button states
void loop(){
    servoHandling();
    ultrasonicHandling();

}

void ultrasonicHandling() {
    // Trigger pulse
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Measure echo
    duration = pulseIn(echoPin, HIGH, 30000); // max wait 30ms
    distance = duration * 0.034 / 2;          // convert to cm

    if (duration == 0) {
        Serial.println("No echo received");
    } else {
        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.println(" cm");
        if (distance <= 20) {
            Serial.println("⚠️ Object detected within 20 cm!");
        }
    }

    delay(500);
}

void servoHandling() {
    // Trigger pulse
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Measure echo
    duration = pulseIn(echoPin, HIGH, 30000); // max wait 30ms
    distance = duration * 0.034 / 2;          // convert to cm

    if (duration == 0) {
        Serial.println("No echo received");
    } else {
        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.println(" cm");
        if (distance <= 20) {
            Serial.println("⚠️ Object detected within 20 cm!");
        }
    }

    delay(500);
}