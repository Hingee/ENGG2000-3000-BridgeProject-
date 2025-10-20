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
#define US_DIST_COND 20

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
    switch (state) {
      case OPENING:
        bridgeSystem->mechanism.incRev(pulses, PULSES_PER_REV);
        break;
      case CLOSING:
        bridgeSystem->mechanism.decRev(pulses, PULSES_PER_REV);
        break;
    }

    if (bridgeSystem->override.isOn()) {
      mode = MANUAL;
    } else {
      mode = AUTO;
    }

    switch (mode) {
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
  long timeDetected = 0;
  switch (state) {
    case IDLE_CLOSE:
      //Has boat arrived
      if ((distA > 0 && distA <= US_DIST_COND) || (distB > 0 && distB <= US_DIST_COND)) {
        Serial.println("[AUTO]{IDLE_CLOSE} Begin Bridge Safety Check");
        bridgeSystem->trafficLights.turnYellow();
        bridgeSystem->gates.closeNet();
        bridgeSystem->alarms.on();
        gateState = PEDESTRIAN_GATE_CLOSE;
      }
      break;
    case PEDESTRIAN_GATE_CLOSE:
      if (gateClose()) {
        Serial.println("[AUTO]{PEDESTRIAN_GATE_CLOSE} Finished Closing Gates");
        bridgeSystem->trafficLights.turnRed();
        bridgeSystem->bridgeLights.turnRed();
        state = SAFETY_CHECK;
      }
      break;
    case SAFETY_CHECK:
      //DO PIR CHECK here
      if (!isTriggered()) {
        Serial.println("[AUTO]{SAFETY_CHECK} Clear of pedestrians");
        bridgeSystem->trafficLights.trunRed();
        bridgeSystem->mechanism.raiseNet();
        state = OPENING;
      }
      break;
    case OPENING:
      if (bridgeSystem->mechanism.raiseHard()) {
        bridgeSystem->bridgeLights.turnGreen();
        postOpenSensorDelay = true;
        timeDetected = millis();
        state = IDLE_OPEN;
      }
      break;
    case IDLE_OPEN:
      //Are boats gone no detection for 5s
      if ((distA > 0 && distA <= US_DIST_COND) || (distB > 0 && distB <= US_DIST_COND)) {
        timeDetected = milis();
      }
      if (millis() - timeDetected < 5000) {
        Serial.println("[AUTO]{IDLE_OPEN} No detection for 5s");
        bridgeSystem->alarms.on();
        bridgeSystem->bridgeLights.turnRed();
        bridgeSystem->mechanism.lowerNet();
        gateState = CLOSING;
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
        bridgeSystem->trafficLights.turnGreen();
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
