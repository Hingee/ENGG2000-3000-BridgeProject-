#ifndef BRIDGESYSTEM_H
#define BRIDGESYSTEM_H

#include <Arduino.h>

#include "BridgeDevice.h"
#include "BridgeSensor.h"

class BridgeSystem {
public:
    //Mechanisms
    Gate gateF;
    Gate gateB;
    Alarm alarm0;
    Alarm alarm1;
    Light trafficLights;
    Light bridgeLights;
    BridgeMechanism mechanism;
    Override override;

    //Sensors
    US ultra0;
    US ultra1;
    PIR pir;

    BridgeSystem();

    void execute(const String& cmd);
};

#endif
