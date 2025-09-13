#ifndef BRIDGESENSOR_H
#define BRIDGESENSOR_H

#include <Arduino.h>

class BridgeSensor {
protected:
    String name;
public:
    BridgeSensor(String n);
    String getName();
};

class US : public BridgeSensor {
protected:
    int distance;
public:
    US();
    void updateDist(int v);
    int getDistance();
};

class PIR : public BridgeSensor {
protected:
    boolean isTriggerd;
public:
    PIR();
    void setTriggerd(boolean t);
    boolean isTriggered();
};

#endif
