#include <ESP32_NOW.h>
#include <ESP32_NOW_Serial.h>

#include <Arduino.h>
#include <ESP32Servo.h>

#include "APHandler.h"
#include "WebServerHandler.h"
#include "BridgeSystem.h"

// Bridge Timer
long globalTime = millis();

// Motor Setup
static int motorDriverPin1 = 27; //27 - S3 will have error from this
static int motorDriverPin2 = 26; //26 - S3 will have error from this
float revolutionsToOpen = 50.0; //IMPORTANT VARIABLE (change based on BridgeSync estimation, and will need to be different for each bridge)
float revolutionsCurrent = 0.0; //current position in revs

// Motor Encoder
const int encoderPinA = 34;
volatile unsigned long pulseCount = 0;
const int pulsesPerRevolution = 700;

// Bridge Motor Automation States
enum MechanismBridgeState {
  OPENING,
  CLOSING,
  IDLE,
};
MechanismBridgeState = mechanismState;
mechanismState = IDLE;

// Servo Setup
Servo myServo;
int servoPin = 13;  // GPIO13 for servo

// Ultrasonic Sensor A (front)
#define trigPinA 17
#define echoPinA 16

// Ultrasonic Sensor B (back)
#define trigPinB 5
#define echoPinB 18

// Ultrasonic Variables
int distanceA, distanceB;
int gatePos = 90;  // Start open (upright)

// Sensor Automation States
enum SensorScanningState {
  SCANNING,
  IDLE,
}
SensorScanningState = sensorState;
sensorState = SCANNING;

// Network Variables
APHandler ap(IPAddress(192,168,1,1), IPAddress(192,168,1,1), IPAddress(255,255,255,0));
WebServerHandler webHandler;
BridgeSystem* bridgeSystem = nullptr;

// ---------- STATE MACHINE ----------
enum BridgeState {
  MANUAL,
  AUTO,
};
BridgeState state;

void setup() {
    Serial.begin(115200);
    delay(100);

    bridgeSystem = new BridgeSystem();
    ap.begin();
    state = AUTO;

    //Servo setup
    myServo.attach(servoPin, 900, 2000);  
    myServo.write(gatePos);

    //Ultrasonic setup
    pinMode(trigPinA, OUTPUT);
    pinMode(echoPinA, INPUT);
    pinMode(trigPinB, OUTPUT);
    pinMode(echoPinB, INPUT);
    
    //Motor Setup
    pinMode(motorDriverPin1, OUTPUT);
    pinMode(motorDriverPin2, OUTPUT);
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, LOW);

    //Encoder Setup
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
  } else {
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

    //Check revs (make this a method)
    noInterrupts();
    unsigned long pulses = pulseCount;
    pulseCount = 0;
    interrupts();
    revolutionsCurrent += (float) pulses / pulsesPerRevolution;

    switch(sensorState) {
      case SCANNING:

        //Ultrasonic update distance (any detection)
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

          //begin opening sequence
        } else {
          moveServoSmooth(90); // open gate slowly
          bridgeSystem->gate.open();

          //begin closing sequence after a period of time
        }     
        break;
      case IDLE:
        println("SENSOR: Waiting for bridge state transition...")
        break;
    }

    //polling states called from here
    
    //Motor Functionality
    switch(mechanismState) {
      case OPENING:
        bridgeSystem->mechanism.raise();
        MotorOpeningSequence();
        break;
      case CLOSING:
        bridgeSystem->mechanism.lower();
        MotorClosingSequence();
        break;
      case IDLE: //default case. Also can be treated as the HALT case to stop the bridge.
        //bridgeSystem->mechanism.stop(); or whatever u want to call it
        haltMotor();
        break;
    }
}

void bridgeManual() {
  if(bridgeSystem->gate.getButton() == 1){
      moveServoSmooth(90);  // open gate slowly
  }else {
      moveServoSmooth(0);  // close gate slowly
  }

  if(bridgeSystem->mechanism.getButton() == 1){ //needs to be adjust for new state
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
    int distance = duration * 0.034 / 2; // convert to cm
    if (duration == 0) return -1; // no echo
    return distance;
}

// Smooth servo movement
void moveServoSmooth(int targetPos) {
    if (gatePos == targetPos) return;

    if (targetPos > gatePos) { //Needs to be converting to poll from loop
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
    if(mechanismState == IDLE || mechanismState == CLOSING) return;
//  Serial.println("Initiate opening sequence."); //if polling this will run every time (move this to the loop)
    digitalWrite(motorDriverPin1, HIGH); //clockwise 
    digitalWrite(motorDriverPin2, LOW); 
    delay(1);  //Let the watchdog breathe
//need to alert other systems of the completion of the bridge, or a global method that constantly checks if the bridge is done. Maybe in these methods then.
}

void MotorClosingSequence(){
    if(mechanismState == IDLE || mechanismState == OPENING) return;
  //Serial.println("Initiate closing sequence");
    digitalWrite(motorDriverPin1, LOW); //anti-clockwise
    digitalWrite(motorDriverPin2, HIGH);
    delay(1);  // Let the watchdog breathe
}

//new method, may need to be added to the RUI
void haltMotor() {
  if(mechanismState == CLOSING || mechanismState == OPENING) return;
//"motor stopped"
  digitalWrite(motorDriverPin1, LOW);
  digitalWrite(motorDriverPin2, LOW); 
  delay(1);
}

void IRAM_ATTR onPulse() {
  pulseCount++;
}
