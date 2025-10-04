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

// Network Variables
APHandler ap(IPAddress(192,168,1,1), IPAddress(192,168,1,1), IPAddress(255,255,255,0));
WebServerHandler webHandler;
BridgeSystem* bridgeSystem = nullptr;

// ---------- STATE MACHINES ----------
enum BridgeState {
  MANUAL,
  AUTO,
};
BridgeState state;

enum MechanismBridgeState {
  OPENING,
  CLOSING,
  IDLE,
};
MechanismBridgeState = mechanismState;
mechanismState = IDLE;

enum SensorScanningState {
  SCANNING,
  IDLE,
}
SensorScanningState = sensorState;
sensorState = SCANNING;

void setup() {
    Serial.begin(115200);
    delay(100);

    bridgeSystem = new BridgeSystem();
    ap.begin();
    state = AUTO;

    //Servo Setup
    myServo.attach(servoPin, 900, 2000);  
    myServo.write(gatePos);

    //Ultrasonic Setup
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

  //Encoder pulses since last counted
  noInterrupts();
  unsigned long pulses = pulseCount;
  pulseCount = 0;
  interrupts();

  //Count motor revolutions
  switch(mechanismState) {
      case OPENING:
        revolutionsCurrent += (float) pulses / pulsesPerRevolution;
        break;
      case CLOSING:
        revolutionsCurrent -= (float) pulses / pulsesPerRevolution;
        break;
      case IDLE:
        break;
    }

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
          Serial.println("Begin Bridge Safety Check");
          /*
          Install PIR sensor code here:
          trafficLights = YELLOW;
          if(PIRdetection == false) {
            Serial.println("Begin Bridge Opening Sequence");
            trafficLights = RED;
            mechanismState = OPENING;
            sensorState = IDLE;
            moveServoSmooth(0); 
            bridgeSystem->gate.close();
          }
          */ 
          moveServoSmooth(0); 
          bridgeSystem->gate.close();
          mechanismState = OPENING;
          sensorState = IDLE;
        } else {
          Serial.println("Begin Bridge Closing Sequence");
          mechanismState = CLOSING;

          //do these after done
          moveServoSmooth(90);
          bridgeSystem->gate.open();
        }     
        break;
      case IDLE:
        println("SENSOR: Waiting for bridge state transition...")
        break;
    }

    //Time-Based Functions (Polling) 

    //Sensor re-activation after bridge opened

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
      case IDLE:
        //bridgeSystem->mechanism.stop(); or whatever u want to call it
        haltMotor();
        break;
    }
}

void bridgeManual() {
  if(bridgeSystem->gate.getButton() == 1){
    moveServoSmooth(90);  // open gate slowly
  } else {
    moveServoSmooth(0);  // close gate slowly
  }

  if(bridgeSystem->mechanism.getButton() == 1){ //needs to be adjust for new state
    MotorOpeningSequence(); //Open bridge 5s
  } else {
    MotorClosingSequence(); //Close bridge 5s
  } /* else {
    haltMotor();
  }
*/
  
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
    if(revolutionsCurrent <= revolutionsToOpen) {
      Serial.println("MOTOR: Opening sequence completed.");
      haltMotor();
      mechanismState = IDLE;
      //set timer for reinitialisation of sensors
      return;
    }
    //Run the motor clockwise 
    digitalWrite(motorDriverPin1, HIGH);
    digitalWrite(motorDriverPin2, LOW); 
    delay(1);
}

void MotorClosingSequence(){
    if(mechanismState == IDLE || mechanismState == OPENING) return;
    if(revolutionsCurrent >= 0.0) {
      Serial.println("MOTOR: Closing sequence completed.")
      haltMotor();
      mechanismState = IDLE;
      sensorState = SCANNING;
      //any other calls based on return to normal position
      return;
    }
    //Run the motor anti-clockwise
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, HIGH);
    delay(1);
}

//New Motor Method
void haltMotor() {
  Serial.println("System motor is idle.");
  digitalWrite(motorDriverPin1, LOW);
  digitalWrite(motorDriverPin2, LOW); 
  delay(1);
}

void IRAM_ATTR onPulse() {
  pulseCount++;
}
