#include <ESP32_NOW.h>
#include <ESP32_NOW_Serial.h>

#include <Arduino.h>
#include <ESP32Servo.h>

#include "APHandler.h"
#include "WebServerHandler.h"
#include "BridgeSystem.h"

// Servo setup
Servo myServo;
int servoPin = 13;  // GPIO13 for servo

// Ultrasonic sensor A (front)
#define trigPinA 17
#define echoPinA 16

// Ultrasonic sensor B (back)
#define trigPinB 5
#define echoPinB 18

// Variables
int distanceA, distanceB;
int gatePos = 90;  // Start open (upright)

//Network Variables
APHandler ap(IPAddress(192,168,1,1), IPAddress(192,168,1,1), IPAddress(255,255,255,0));
WebServerHandler webHandler;
BridgeSystem bridgeSystem;

// ---------- STATE MACHINE ----------
enum BridgeState {
  MANUAL,
  AUTO,
};

BridgeState state = AUTO;

void setup() {
    Serial.begin(115200);
    ap.begin();

    // Servo setup
    myServo.attach(servoPin, 900, 2000);  
    myServo.write(gatePos);

    // Ultrasonic setup
    pinMode(trigPinA, OUTPUT);
    pinMode(echoPinA, INPUT);

    pinMode(trigPinB, OUTPUT);
    pinMode(echoPinB, INPUT);

    Serial.println("Boom gate system ready.");

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
  if(bridgeSystem.override.getButton() == 1) {
    state = MANUAL;
  }else {
    state = AUTO;
  }
  
  switch (state) {
    case AUTO:
      bridgeAuto();
      break;
    case MANUAL:
      bridgeManual();
      break;
  }
  delay(500); // update every 0.5s
}

void bridgeAuto() {
//    distanceA = readUltrasonic(trigPinA, echoPinA);
//    distanceB = readUltrasonic(trigPinB, echoPinB);
    distanceA = random(100); 
    distanceB = random(100);

    Serial.print("Sensor A: ");
    if (distanceA == -1) Serial.print("No echo");
    else {
        bridgeSystem.ultra0.updateDist(distanceA); //Critical
        Serial.print(String(distanceA) + " cm  --> Object detected! ");
    }

    Serial.print(" | Sensor B: ");
    if (distanceB == -1) Serial.print("No echo");
    else {
        bridgeSystem.ultra1.updateDist(distanceB); //Critical
        Serial.print(String(distanceB) + " cm  --> Object detected! ");
    }

    // Detection condition (within 20cm)
    if ((distanceA > 0 && distanceA <= 20) || (distanceB > 0 && distanceB <= 20)) {
        moveServoSmooth(0);  // close gate slowly
        bridgeSystem.gate.close();
    } else {
        moveServoSmooth(90); // open gate slowly
        bridgeSystem.gate.open();
    }
}

void bridgeManual() {
  if(bridgeSystem.gate.getButton() == 1){
      moveServoSmooth(90);  // open gate slowly
  }else {
      moveServoSmooth(0);  // close gate slowly
  }
}

// Function to read ultrasonic distance
int readUltrasonic(int trigPin, int echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
    int distance = duration * 0.034 / 2;           // convert to cm
    if (duration == 0) return -1;  // no echo
    return distance;
}

// Smooth servo movement
void moveServoSmooth(int targetPos) {
    if (gatePos == targetPos) return;

    if (targetPos > gatePos) {
        for (int pos = gatePos; pos <= targetPos; pos++) {
        myServo.write(pos);
        delay(15);  // speed of motion
        }
    } else {
        for (int pos = gatePos; pos >= targetPos; pos--) {
        myServo.write(pos);
        delay(15);  // speed of motion
        }
    }
    gatePos = targetPos;
}
