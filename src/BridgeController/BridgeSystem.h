#ifndef BRIDGESYSTEM_H
#define BRIDGESYSTEM_H

#include <Arduino.h>

#include "BridgeDevice.h"
#include "BridgeSensor.h"

class BridgeSystem {
public:
  //Mechanisms
  Gate gates;
  Alarm alarms;
  Light pedestrianLights;
  Light boatLights;
  BridgeMechanism mechanism;
  Override override;

  //Sensors
  US ultraF;
  US ultraB;
  PIR pir;

  BridgeSystem(int servoPin1, int servoPin2,
              int pl_redPin, int pl_greenPin,
              int bl_redPin, int bl_yellowPin, int bl_greenPin,
              int alarmPin,
              int motorPin1, int motorPin2, int encoderPin,
              int pirPin,
              int us_trigPinF, int us_echoPinF,
              int us_trigPinB, int us_echoPinB);

  void execute(const String& cmd);
};

#endif
