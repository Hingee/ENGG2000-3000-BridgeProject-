#include <ESP32_NOW.h>
#include <ESP32_NOW_Serial.h>
#include <Arduino.h>

#include "APHandler.h"
#include "WebServerHandler.h"
#include "BridgeSystem.h"

// ------------------ Pin Configuration (change as needed) ------------------
//Servo
#define SERVO_PIN_1 13  // GPIO13 for servo
#define SERVO_PIN_2 12  // GPIO12 for servo

// Ultrasonic Front
#define US_TRIG_PIN_F 17
#define US_ECHO_PIN_F 16

// Ultrasonic Back
#define US_TRIG_PIN_B 5
#define US_ECHO_PIN_B 18

//Motor
#define MOTOR_PIN_1 40  //S3 board will have error from this
#define MOTOR_PIN_2 41  //S3 board will have error from this

//Encoder
#define ENCODER_PIN 34      //Chose an interruptable pin
#define PULSES_PER_REV 700  //may need adjusting

//Network & System
APHandler ap(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
WebServerHandler webHandler;
BridgeSystem* bridgeSystem = nullptr;

// Timing & state
unsigned long lastServoMoveTime = 0;
int servoStepDelay = 15;  // ms between servo steps

// System Refresh Interval
unsigned long lastRefresh = 0;
const unsigned long refreshInterval = 100;  //every 500 ms

// Sensor reactivation delay after opening
bool postOpenSensorDelay = false;
int sensorDelay = 10000;  //1000 per second
long prevTimeSensor = 0;

// Encoder pulse counter (safe for ISR)
volatile unsigned long pulseCount = 0;
unsigned long prevPulseCount = 0;

// ---------- STATE MACHINES ----------
enum BridgeState { MANUAL,
                   AUTO };
BridgeState state = AUTO;

enum MechanismState { OPENING,
                      CLOSING,
                      IDLE };
MechanismState roadwayState = IDLE;
MechanismState gateState = IDLE;

enum SensorScanningState { ULTRASONICDEFAULT,
                           PIRSAFETYCHECK,
                           IDLE_SENSOR };
SensorScanningState sensorState = ULTRASONICDEFAULT;

bool gateCommandIssued = false;
bool mechCommandIssued = false;

//Forward Declarations
void IRAM_ATTR onPulse();
void networkTask(void* parameter);

void setup() {
  Serial.begin(115200);
  delay(100);

  bridgeSystem = new BridgeSystem();

  ap.begin();
  bridgeSystem->gates.init(SERVO_PIN_1, SERVO_PIN_2);

  //Ultrasonic
  pinMode(US_TRIG_PIN_F, OUTPUT);
  pinMode(US_ECHO_PIN_F, INPUT);
  pinMode(US_TRIG_PIN_B, OUTPUT);
  pinMode(US_ECHO_PIN_B, INPUT);

  bridgeSystem->mechanism.init(MOTOR_PIN_1, MOTOR_PIN_2, ENCODER_PIN);

  //Encoder Interrupt
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), onPulse, RISING);

  //Start Network Task pinned to core 0
  xTaskCreatePinnedToCore(networkTask,
                          "NetworkTask",
                          16384,
                          NULL,
                          1,
                          NULL,
                          0  //core 0
  );
}

//Network Loop Task
void networkTask(void* parameter) {
  for (;;) {
    WiFiClient client = ap.getClient();
    webHandler.handleClient(client, *bridgeSystem);
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void loop() {
  if (millis() - lastRefresh >= refreshInterval) {
    lastRefresh = millis();

    // Read encoder pulses atomically
    noInterrupts();
    unsigned long currentPulses = pulseCount;
    interrupts();

    unsigned long pulses = currentPulses - prevPulseCount;
    prevPulseCount = currentPulses;

    // Update revolutions depending on mechanism direction
    switch (roadwayState) {
      case OPENING:
        bridgeSystem->mechanism.incRev(pulses, PULSES_PER_REV);
        break;
      case CLOSING:
        bridgeSystem->mechanism.decRev(pulses, PULSES_PER_REV);
        break;
    }

    if (bridgeSystem->override.isOn()) {
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
  int distA = bridgeSystem->ultra0.readUltrasonic(US_TRIG_PIN_F, US_ECHO_PIN_F);
  int distB = bridgeSystem->ultra1.readUltrasonic(US_TRIG_PIN_B, US_ECHO_PIN_B);

  switch (sensorState) {
    case ULTRASONICDEFAULT:
      // Detection condition (within 20cm)
      if ((distA > 0 && distA <= 20) || (distB > 0 && distB <= 20)) {
        Serial.println("[AUTO]{ULTRASONICDEFAULT} Begin Bridge Safety Check");

        bridgeSystem->trafficLights.turnYellow();
        gateState = CLOSING;
      }

      // Closing sequence condition
      if (postOpenSensorDelay && millis() - prevTimeSensor >= sensorDelay && roadwayState == IDLE && distA == -1 && distB == -1) {
        Serial.println("[AUTO]{ULTRASONICDEFAULT} Begin Bridge Closing Sequence");
        postOpenSensorDelay = false;
        roadwayState = CLOSING;
      }
      break;
    case PIRSAFETYCHECK:
      /*
        //Install PIR sensor code here:
        if(PIRdetection == false) {
          Serial.println("[AUTO]{PIRSAFETYCHECK} Begin Bridge Opening Sequence");
          roadwayState = OPENING; 
          sensorState = IDLE;
          targetPos = 0;
        } else {
          Serial.println("[AUTO]{PIRSAFETYCHECK} Begin Bridge Closing Sequence");
          roadwayState = CLOSING;
        }
        */
      // Placeholder for PIR logic; fall back to ultrasonic default for now.
      // Implement PIR readings & decision here.
      // Temporary fallback to avoid getting stuck:
      sensorState = ULTRASONICDEFAULT;
      break;
    case IDLE:
      Serial.println("[AUTO]{IDLE} SENSOR: Waiting for bridge state transition...");
      break;
  }

  //Motor Functionality
  switch (roadwayState) {
    case OPENING:
      if (triggerMechRaise()) {
        roadwayState = IDLE;
        mechCommandIssued = false;
        postOpenSensorDelay = true;
        prevTimeSensor = millis();
      }
      break;
    case CLOSING:
      if (triggerMechLower()) {
        roadwayState = IDLE;
        mechCommandIssued = false;
        sensorState = ULTRASONICDEFAULT;
        gateState = OPENING;
      }
      break;
  }

  //Gate Functionality
  switch (gateState) {
    case OPENING:
      if (triggerGateOpen()) {
        gateState = IDLE;
        gateCommandIssued = false;
        bridgeSystem->trafficLights.turnGreen();
      }
      break;
    case CLOSING:
      if (triggerGateClose()) {
        gateState = IDLE;
        gateCommandIssued = false;
        sensorState = PIRSAFETYCHECK;
        bridgeSystem->trafficLights.turnRed();
      }
      break;
  }
}

void bridgeManual() {
  if (bridgeSystem->gates.getStateNum() == 3) {
    lastServoMoveTime = bridgeSystem->gates.closeHard(lastServoMoveTime, servoStepDelay);
  } else if (bridgeSystem->gates.getStateNum() == 2) {
    lastServoMoveTime = bridgeSystem->gates.openHard(lastServoMoveTime, servoStepDelay);
  }

  if (bridgeSystem->mechanism.getStateNum() == 3) {
    roadwayState = CLOSING;
  } else if (bridgeSystem->mechanism.getStateNum() == 2) {
    roadwayState = OPENING;
  }

  switch (roadwayState) {
    case OPENING:
      if (bridgeSystem->mechanism.raiseHard()) roadwayState = IDLE;
      break;
    case CLOSING:
      if (bridgeSystem->mechanism.lowerHard()) roadwayState = IDLE;
      break;
  }
}

void IRAM_ATTR onPulse() {
  pulseCount++;
}

bool triggerGateClose() {
  if (!gateCommandIssued) {
    bridgeSystem->gates.closeNet();
    gateCommandIssued = true;
  }
  lastServoMoveTime = bridgeSystem->gates.closeHard(lastServoMoveTime, servoStepDelay);
  return bridgeSystem->gates.isIdle();
}

bool triggerGateOpen() {
  if (!gateCommandIssued) {
    bridgeSystem->gates.closeNet();
    gateCommandIssued = true;
  }

  lastServoMoveTime = bridgeSystem->gates.closeHard(lastServoMoveTime, servoStepDelay);
  return bridgeSystem->gates.isIdle();
}

bool triggerMechLower() {
  if (!mechCommandIssued) {
    bridgeSystem->mechanism.lowerNet();
    mechCommandIssued = true;
  }
  return bridgeSystem->mechanism.lowerHard();
}

bool triggerMechRaise() {
  if (!mechCommandIssued) {
    bridgeSystem->mechanism.raiseNet();
    mechCommandIssued = true;
  }
  return bridgeSystem->mechanism.raiseHard();
}
