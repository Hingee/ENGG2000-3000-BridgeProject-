#include <ESP32_NOW.h>
#include <ESP32_NOW_Serial.h>

#include <Arduino.h>

#include "APHandler.h"
#include "WebServerHandler.h"
#include "BridgeSystem.h"

//Servo
#define servoPin1 13  // GPIO13 for servo
#define servoPin2 5  // GPIO5 for servo

// Ultrasonic sensor A (front)
#define trigPinA 17
#define echoPinA 16

// Ultrasonic sensor B (back)
#define trigPinB 5
#define echoPinB 18

//Motor specific
#define motorDriverPin1 40 //27 - S3 will have error from this
#define motorDriverPin2 41 //26 - S3 will have error from this

//Encoder
#define encoderPinA 34
#define pulsesPerRevolution 700 //may need adjusting
volatile unsigned long pulseCount = 0;
unsigned long prevPulseCount = 0;

//Network Objects
APHandler ap(IPAddress(192,168,1,1), IPAddress(192,168,1,1), IPAddress(255,255,255,0));
WebServerHandler webHandler;
BridgeSystem* bridgeSystem = nullptr;

//Ultrasonic Control Flow Variables
unsigned long lastServoMoveTime = 0;
int servoStepDelay = 15; // speed of motion (ms)

// Sensor System Reactivation Delay
bool postOpenSensorDelay = false;
int sensorDelay = 10000; //1000 per second
long prevTimeSensor;

// ---------- STATE MACHINE ----------
enum BridgeState {
  MANUAL,
  AUTO,
};
BridgeState state;

enum MechanismBridgeState { 
  OPENING, 
  CLOSING, 
  IDLE 
};
MechanismBridgeState mechanismState = IDLE;

enum SensorScanningState { 
  ULTRASONICDEFAULT, 
  PIRSAFETYCHECK, 
  IDLE_SENSOR 
};
SensorScanningState sensorState = ULTRASONICDEFAULT;

// System Refresh Interval (For poll systems to work accurately)
unsigned long lastRefresh = 0;
const unsigned long refreshInterval = 100; //every 500 ms

void IRAM_ATTR onPulse();

void setup() {
    Serial.begin(115200);
    delay(100);

    bridgeSystem = new BridgeSystem();
    ap.begin();
    state = AUTO;
    bridgeSystem->gates.init(servoPin1, servoPin2);

    // Ultrasonic setup
    pinMode(trigPinA, OUTPUT);
    pinMode(echoPinA, INPUT);
    pinMode(trigPinB, OUTPUT);
    pinMode(echoPinB, INPUT);
    
    bridgeSystem->mechanism.init(motorDriverPin1, motorDriverPin2, encoderPinA);
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

void loop(){
  if (millis() - lastRefresh >= refreshInterval) {
    lastRefresh = millis();

    //Encoder pulses
    noInterrupts();
    unsigned long currentPulses = pulseCount;
    interrupts();
    unsigned long pulses = currentPulses - prevPulseCount;
    prevPulseCount = currentPulses;

    //Count motor revolutions
    switch(mechanismState) {
        case OPENING:
          bridgeSystem->mechanism.incRev(pulses, pulsesPerRevolution);
          break;
        case CLOSING:
          bridgeSystem->mechanism.decRev(pulses, pulsesPerRevolution);
          break;
        case IDLE:
          break;
    }

    if(bridgeSystem->override.isOn()) {
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
  }
  delay(1);
}

void bridgeAuto() {
    int distA = bridgeSystem->ultra0.readUltrasonic(trigPinA, echoPinA);
    int distB = bridgeSystem->ultra1.readUltrasonic(trigPinB, echoPinB);

    switch(sensorState) {
      case ULTRASONICDEFAULT:
        // Detection condition (within 20cm)
        if ((distA > 0 && distA <= 20) || (distB > 0 && distB <= 20)) {
          Serial.println("Begin Bridge Safety Check");
          
          bridgeSystem->trafficLights.turnYellow();
          bridgeSystem->gates.closeNet();
          lastServoMoveTime = bridgeSystem->gates.closeHard(lastServoMoveTime, servoStepDelay);
          
          sensorState = PIRSAFETYCHECK;
        }     

        // Closing sequence condition
        if(postOpenSensorDelay && millis() - prevTimeSensor >= sensorDelay && mechanismState == IDLE && distA == -1 && distB == -1) {
          Serial.println("Begin Bridge Closing Sequence");
          postOpenSensorDelay = false;
          mechanismState = CLOSING;
          bridgeSystem->mechanism.lowerNet();
          
          bridgeSystem->gates.openNet();
          lastServoMoveTime = bridgeSystem->gates.openHard(lastServoMoveTime, servoStepDelay);
        }
        break;
      case PIRSAFETYCHECK:
        /*
        //Install PIR sensor code here:
        if(PIRdetection == false) {
          Serial.println("Begin Bridge Opening Sequence");
          trafficLights = RED;
          mechanismState = OPENING; 
          bridgeSystem->mechanism.raiseNet();
          sensorState = IDLE;
          targetPos = 0;
          bridgeSystem->gate.close();
        } else {
          Serial.println("Begin Bridge Closing Sequence");
          mechanismState = CLOSING;
          bridgeSystem->mechanism.lowerNet();
        }
        */
        break;
      case IDLE:
        Serial.println("SENSOR: Waiting for bridge state transition...");
        break;
    }

    //Motor Functionality
    switch(mechanismState) {
      case OPENING:
        if(bridgeSystem->mechanism.raiseHard()) {
          mechanismState = IDLE;
          postOpenSensorDelay = true;
          prevTimeSensor = millis();
        }
        break;
      case CLOSING:
        if(bridgeSystem->mechanism.lowerHard()) {
          mechanismState = IDLE;
          sensorState = ULTRASONICDEFAULT;
        }
        break;
    }
}

void bridgeManual() {
  if(bridgeSystem->gates.getStateNum() == 3){
      lastServoMoveTime = bridgeSystem->gates.closeHard(lastServoMoveTime, servoStepDelay);
  }else if(bridgeSystem->gates.getStateNum() == 2){
      lastServoMoveTime = bridgeSystem->gates.openHard(lastServoMoveTime, servoStepDelay);
  }
  
  if(bridgeSystem->mechanism.getStateNum() == 3){
      mechanismState = CLOSING;
  }else if(bridgeSystem->mechanism.getStateNum() == 2){
      mechanismState = OPENING;
  }

  switch(mechanismState) {
        case OPENING:
          if(bridgeSystem->mechanism.raiseHard()) mechanismState = IDLE;
          break;
        case CLOSING:
          if(bridgeSystem->mechanism.lowerHard()) mechanismState = IDLE;
          break;
    }
}

void IRAM_ATTR onPulse() {
  pulseCount++;
}
