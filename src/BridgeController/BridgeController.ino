#include <ESP32_NOW.h>
#include <ESP32_NOW_Serial.h>
#include <Arduino.h>

#include "APHandler.h"
#include "WebServerHandler.h"
#include "BridgeSystem.h"

// ------------------ Pin Configuration (change as needed) ------------------
//Boat Lights
#define BL_RED 15
#define BL_YELLOW 2
#define BL_GREEN 4

//Pedestrian Lights
#define PL_RED 21
#define PL_GREEN 22

//Servo
#define SERVO_PIN_1 17  // GPIO13 for servo
#define SERVO_PIN_2 5   // GPIO12 for servo

// Ultrasonic Front
#define US_TRIG_PIN_F 13
#define US_ECHO_PIN_F 12
#define US_DIST_COND 20

// Ultrasonic Back
#define US_TRIG_PIN_B 14
#define US_ECHO_PIN_B 27  // 27 will give s3 errors

//Motor
#define MOTOR_PIN_1 25
#define MOTOR_PIN_2 33
#define ENCODER_PIN 26  //Chose an interruptable pin
#define PULSES_PER_REV 700

#define BUZZER_PIN 35
#define PIR_PIN 19

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
long timeDetected = 0;

// Encoder pulse counter (safe for ISR)
volatile unsigned long pulseCount = 0;
unsigned long prevPulseCount = 0;

// ---------- STATE MACHINES ----------
enum BridgeMode { MANUAL,
                  AUTO };
BridgeMode mode = AUTO;

enum BridgeState { OPENING,
                   PEDESTRIAN_GATE_OPEN,
                   IDLE_OPEN,
                   CLOSING,
                   IDLE_CLOSE,
                   PEDESTRIAN_GATE_CLOSE,
                   SAFETY_CHECK,
};
BridgeState state = IDLE_CLOSE;

//Forward Declarations
void networkTask(void* parameter);
void IRAM_ATTR onPulse();

void setup() {
  Serial.begin(115200);
  delay(100);

  bridgeSystem = new BridgeSystem(SERVO_PIN_1, SERVO_PIN_2,
                                  PL_RED, PL_GREEN,
                                  BL_RED, BL_YELLOW, BL_GREEN,
                                  BUZZER_PIN,
                                  MOTOR_PIN_1, MOTOR_PIN_2, ENCODER_PIN,
                                  PIR_PIN,
                                  US_TRIG_PIN_F, US_ECHO_PIN_F,
                                  US_TRIG_PIN_B, US_ECHO_PIN_B);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), onPulse, RISING);
  ap.begin();

  //Start Network Task pinned to core 0
  xTaskCreatePinnedToCore(networkTask,
                          "NetworkTask",
                          16384,
                          NULL,
                          1,
                          NULL,
                          0  //core 0
  );
  bridgeSystem->pedestrianLights.turnGreen();
  bridgeSystem->boatLights.turnRed();
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
    switch (state) {
      case OPENING:
        bridgeSystem->mechanism.incRev(pulses, PULSES_PER_REV);
        break;
      case CLOSING:
        bridgeSystem->mechanism.decRev(pulses, PULSES_PER_REV);
        break;
    }

    switch (mode) {
      case AUTO:
        if (bridgeSystem->override.isOn()) mode = MANUAL;
        bridgeAuto();
        break;
      case MANUAL:
        if (!bridgeSystem->override.isOn()) mode = AUTO;
        bridgeManual();
        break;
    }
  }
  delay(1);
}

void bridgeAuto() {
  bool boatDetected = false;
  int distF = bridgeSystem->ultraF.readUltrasonic();
  int distB = bridgeSystem->ultraB.readUltrasonic();
  bridgeSystem->pir.read();

  if ((distF > 0 && distF <= US_DIST_COND) || (distB > 0 && distB <= US_DIST_COND)) {
    boatDetected = true;
    timeDetected = millis();
  }
  switch (state) {
    case IDLE_CLOSE:
      //Has boat arrived
      if (boatDetected) {
        Serial.println("[AUTO]{IDLE_CLOSE} Begin Bridge Safety Check");
        bridgeSystem->pedestrianLights.turnRed();
        bridgeSystem->gates.closeNet();
        bridgeSystem->alarms.activate();
        state = PEDESTRIAN_GATE_CLOSE;
      }
      break;
    case PEDESTRIAN_GATE_CLOSE:
      if (gateClose()) {
        Serial.println("[AUTO]{PEDESTRIAN_GATE_CLOSE} Finished Closing Gates");
        bridgeSystem->pedestrianLights.turnRed();
        bridgeSystem->boatLights.turnRed();
        state = SAFETY_CHECK;
      }
      break;
    case SAFETY_CHECK:
      if (bridgeSystem->pir.isNotTriggeredForSec(3)) {
        Serial.println("[AUTO]{SAFETY_CHECK} Clear of pedestrians");
        bridgeSystem->pedestrianLights.turnRed();
        bridgeSystem->mechanism.raiseNet();
        state = OPENING;
      }
      break;
    case OPENING:
      if (bridgeSystem->mechanism.raiseHard()) {
        bridgeSystem->boatLights.turnGreen();
        postOpenSensorDelay = true;
        bridgeSystem->alarms.deactivate();
        state = IDLE_OPEN;
      }
      break;
    case IDLE_OPEN:
      //Are boats gone no detection for 5s
      if (millis() - timeDetected > 5000) {
        Serial.println("[AUTO]{IDLE_OPEN} No detection for 5s");
        bridgeSystem->alarms.activate();
        bridgeSystem->boatLights.turnRed();
        bridgeSystem->mechanism.lowerNet();
        state = CLOSING;
      }
      break;
    case CLOSING:
      if (bridgeSystem->mechanism.lowerHard()) {
        Serial.println("[AUTO]{CLOSING} Finished Closing");
        bridgeSystem->gates.openNet();
        state = PEDESTRIAN_GATE_OPEN;
      }
      break;
    case PEDESTRIAN_GATE_OPEN:
      if (gateOpen()) {
        Serial.println("[AUTO]{PEDESTRIAN_GATE_OPEN} Finished Opening Gates");
        bridgeSystem->pedestrianLights.turnGreen();
        bridgeSystem->alarms.deactivate();
        postOpenSensorDelay = true;
        state = IDLE_CLOSE;
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
    state = CLOSING;
  } else if (bridgeSystem->mechanism.getStateNum() == 2) {
    state = OPENING;
  }

  switch (state) {
    case OPENING:
      if (bridgeSystem->mechanism.raiseHard()) state = IDLE_OPEN;
      break;
    case CLOSING:
      if (bridgeSystem->mechanism.lowerHard()) state = IDLE_CLOSE;
      break;
  }
}

void IRAM_ATTR onPulse() {
  pulseCount++;
}

bool gateClose() {
  lastServoMoveTime = bridgeSystem->gates.closeHard(lastServoMoveTime, servoStepDelay);
  return bridgeSystem->gates.isIdle();
}

bool gateOpen() {
  lastServoMoveTime = bridgeSystem->gates.openHard(lastServoMoveTime, servoStepDelay);
  return bridgeSystem->gates.isIdle();
}
