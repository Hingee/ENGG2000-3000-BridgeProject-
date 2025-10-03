#include <ESP32_NOW.h>
#include <ESP32_NOW_Serial.h>

#include <Arduino.h>

#include "APHandler.h"
#include "WebServerHandler.h"
#include "BridgeSystem.h"

//Pins
//Servo
#define servoPin 13  // GPIO13 for servo

// Ultrasonic sensor A (front)
#define trigPinA 17
#define echoPinA 16

// Ultrasonic sensor B (back)
#define trigPinB 5
#define echoPinB 18

//Motor specific
#define motorDriverPin1 40; //27 - S3 will have error from this
#define motorDriverPin2 41; //26 - S3 will have error from this
#define duration 5000; //1000 is a second

//Encoder
#define encoderPinA 34;
#define pulsesPerRevolution = 374; //may need adjusting

//Network Objects
APHandler ap(IPAddress(192,168,1,1), IPAddress(192,168,1,1), IPAddress(255,255,255,0));
WebServerHandler webHandler;
BridgeSystem* bridgeSystem = nullptr;

// ---------- STATE MACHINE ----------
enum BridgeState {
  MANUAL,
  AUTO,
};

BridgeState state;

void IRAM_ATTR onPulse();

void setup() {
    Serial.begin(115200);
    delay(100);

    bridgeSystem = new BridgeSystem();
    ap.begin();
    state = AUTO;
    bridgeSystem->gateF.init(servoPin);
    bridgeSystem->gateB.init(servoPin);

    // Ultrasonic setup
    pinMode(trigPinA, OUTPUT);
    pinMode(echoPinA, INPUT);
    pinMode(trigPinB, OUTPUT);
    pinMode(echoPinB, INPUT);
    
    bridgeSystem->mechanism.init(motorDriverPin1, motorDriverPin2, encoderPinA, duration, pulsesPerRevolution);

    xTaskCreatePinnedToCore(networkTask,
        "NetworkTask",
        16384,
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
    webHandler.handleClient(client, *bridgeSystem);
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

//TODO Add transitioning button states
void loop(){
  if(bridgeSystem->override.getButton() == 1) {
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
  delay(500);
}

void bridgeAuto() {
    int distanceA = bridgeSystem->ultra0.readUltrasonic(trigPinA, echoPinA);
    int distanceB = bridgeSystem->ultra1.readUltrasonic(trigPinB, echoPinB);

    printDist(distanceA, distanceB);
  
    // Detection condition (within 20cm)
    if (distanceA > 0 && distanceA <= 20) {
        bridgeSystem->gateF.close();
        bridgeSystem->gateF.servoClose();  // close gate slowly
    } else {
        bridgeSystem->gateF.open();
        bridgeSystem->gateF.servoOpen();  // close gate slowly
    }

     // Detection condition (within 20cm)
    if (distanceB > 0 && distanceB <= 20) {
        bridgeSystem->gateB.close();
        bridgeSystem->gateB.servoClose();  // close gate slowly
    } else {
        bridgeSystem->gateB.open();
        bridgeSystem->gateB.servoOpen();  // close gate slowly
    }

    //Motor Functionality
    if(mechanismState) {
        bridgeSystem->mechanism.raise();
        bridgeSystem->mechanism.raiseSequence();
    } else {
        bridgeSystem->mechanism.lower();
        bridgeSystem->mechanism.lowerSequence();
    }
}

void bridgeManual() {
  if(bridgeSystem->gateF.getButton() == 1){
      bridgeSystem->gateF.close();
      bridgeSystem->gateF.servoOpen();  // open gate slowly
  }else {
      bridgeSystem->gateF.open();
      bridgeSystem->gateF.servoClose();  // close gate slowly
  }

  if(bridgeSystem->gateB.getButton() == 1){
      bridgeSystem->gateB.close();
      bridgeSystem->gateB.servoOpen(); // open gate slowly
  }else {
      bridgeSystem->gateB.open();
      bridgeSystem->gateB.servoClose();  // close gate slowly
  }

  if(bridgeSystem->mechanism.getButton() == 1){
      bridgeSystem->mechanism.raise();
      bridgeSystem->mechanism.raiseSequence();
  }else {
      bridgeSystem->mechanism.lower();
      bridgeSystem->mechanism.lowerSequence();
  }
}

void printDist(int distanceA, int distanceB) {
    Serial.print("Sensor A: ");
    if (distanceA == -1) Serial.print("No echo");
    else {
        Serial.print(String(distanceA) + " cm  --> Object detected! ");
    }

    Serial.print(" | Sensor B: ");
    if (distanceB == -1) Serial.print("No echo");
    else {
        Serial.print(String(distanceB) + " cm  --> Object detected! ");
    }
    Serial.println();
}
