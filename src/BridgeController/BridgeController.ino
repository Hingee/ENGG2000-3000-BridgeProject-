#include <ESP32_NOW.h>
#include <ESP32_NOW_Serial.h>

#include <Arduino.h>
#include <ESP32Servo.h>

#include "APHandler.h"
#include "WebServerHandler.h"
#include "BridgeSystem.h"

//Bridge Motor Automation Initialisation
bool mechanismState = true; //true closed, false open
bool runningbridge = true;

//Motor specific
static int motorDriverPin1 = 27; //27 - S3 will have error from this
static int motorDriverPin2 = 26; //26 - S3 will have error from this
int duration = 5000; //1000 is a second
bool Direction = true; //true forward, false backwards

//Encoder
const int encoderPinA = 34;
volatile unsigned long pulseCount = 0;
const int pulsesPerRevolution = 374; //may need adjusting

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
    // Servo setup
    myServo.attach(servoPin, 900, 2000);  
    myServo.write(gatePos);

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
    distanceA = readUltrasonic(trigPinA, echoPinA);
    distanceB = readUltrasonic(trigPinB, echoPinB);

    Serial.print("Sensor A: ");
    if (distanceA == -1) Serial.print("No echo");
    else {
        bridgeSystem->ultra0.updateDist(distanceA); //Critical
        Serial.print(String(distanceA) + " cm  --> Object detected! ");
    }

    Serial.print(" | Sensor B: ");
    if (distanceB == -1) Serial.print("No echo");
    else {
        bridgeSystem->ultra1.updateDist(distanceB); //Critical
        Serial.print(String(distanceB) + " cm  --> Object detected! ");
    }

    // Detection condition (within 20cm)
    if ((distanceA > 0 && distanceA <= 20) || (distanceB > 0 && distanceB <= 20)) {
        moveServoSmooth(0);  // close gate slowly
        bridgeSystem->gate.close();
    } else {
        moveServoSmooth(90); // open gate slowly
        bridgeSystem->gate.open();
    }
    
    //Motor Functionality
    if(mechanismState) {
        MotorOpeningSequence();
        bridgeSystem->mechanism.lower();
    } else {
        MotorClosingSequence();
        bridgeSystem->mechanism.raise();
    }
}

void bridgeManual() {
  if(bridgeSystem->gate.getButton() == 1){
      moveServoSmooth(90);  // open gate slowly
  }else {
      moveServoSmooth(0);  // close gate slowly
  }

  if(bridgeSystem->mechanism.getButton() == 1){
      MotorOpeningSequence(); //Open bridge 5s
  }else {
      MotorClosingSequence(); //Close bridge 5s
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

void MotorOpeningSequence(){ 
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
