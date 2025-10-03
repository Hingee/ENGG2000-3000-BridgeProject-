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
static int motorDriverPin1 = 40; //27 - S3 will have error from this
static int motorDriverPin2 = 41; //26 - S3 will have error from this
int duration = 5000; //1000 is a second
bool mechanismState = true; //true closed, false open

//Encoder
const int encoderPinA = 34;
volatile unsigned long pulseCount = 0;
const int pulsesPerRevolution = 374; //may need adjusting

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
    
    //Motor Setup
    pinMode(motorDriverPin1, OUTPUT);
    pinMode(motorDriverPin2, OUTPUT);
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, LOW);

    pinMode(encoderPinA, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(encoderPinA), onPulse, RISING);

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
        bridgeSystem->gateF.moveServoSmooth(0);  // close gate slowly
        bridgeSystem->gateF.close();
    } else {
        bridgeSystem->gateF.moveServoSmooth(90);  // close gate slowly
        bridgeSystem->gateF.open();
    }

     // Detection condition (within 20cm)
    if (distanceB > 0 && distanceB <= 20) {
        bridgeSystem->gateB.moveServoSmooth(0);  // close gate slowly
        bridgeSystem->gateB.close();
    } else {
        bridgeSystem->gateB.moveServoSmooth(90);  // close gate slowly
        bridgeSystem->gateB.open();
    }

    //Motor Functionality
    if(mechanismState) {
        bridgeSystem->mechanism.raise();
        MotorOpeningSequence();
    } else {
        bridgeSystem->mechanism.lower();
        MotorClosingSequence();
    }
}

void bridgeManual() {
  if(bridgeSystem->gateF.getButton() == 1){
      bridgeSystem->gateF.moveServoSmooth(90);  // open gate slowly
      bridgeSystem->gateF.close();
  }else {
      bridgeSystem->gateF.moveServoSmooth(0);  // close gate slowly
      bridgeSystem->gateF.open();
  }

  if(bridgeSystem->gateB.getButton() == 1){
      bridgeSystem->gateB.moveServoSmooth(90); // open gate slowly
      bridgeSystem->gateB.close();
  }else {
      bridgeSystem->gateB.moveServoSmooth(0);  // close gate slowly
      bridgeSystem->gateB.open();
  }

  if(bridgeSystem->mechanism.getButton() == 1){
      bridgeSystem->mechanism.raise();
      MotorOpeningSequence(); //Open bridge 5s
  }else {
      bridgeSystem->mechanism.lower();
      MotorClosingSequence(); //Close bridge 5s
  }
}

void MotorOpeningSequence(){ 
    if(!mechanismState) return;
    Serial.println("Opening sequence (Forward)");
    unsigned long startTime = millis();
    while(millis() - startTime < duration) {
        digitalWrite(motorDriverPin1, LOW); 
        digitalWrite(motorDriverPin2, HIGH); 
        printRPM();
        delay(1);  // Let the watchdog breathe
    }
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, LOW);
    mechanismState = false;
}

void MotorClosingSequence(){
    if(mechanismState) return;
    Serial.println("Closing sequence (Backwards)");
    unsigned long startTime = millis();
    while(millis() - startTime < duration) {
        digitalWrite(motorDriverPin1, HIGH);
        digitalWrite(motorDriverPin2, LOW);
        printRPM();
        delay(1);  // Let the watchdog breathe
    }
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, LOW);
    mechanismState = true;
}

void IRAM_ATTR onPulse() {
  pulseCount++; //count pulses
}

void printRPM() {
    //calculate and print RPM every second while motor runs
    static unsigned long lastTime = 0;
    unsigned long now = millis();
    if(now - lastTime >= 1000) {
        noInterrupts();
        unsigned long pulses = pulseCount;
        pulseCount = 0;
        interrupts();

        float revolutions = (float)pulses / pulsesPerRevolution;
        float rpm = revolutions * 60.0;

        Serial.print("Pulses/sec: ");
        Serial.print(pulses);
        Serial.print("  |  RPM: ");
        Serial.println(rpm, 2);

        lastTime = now;
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
