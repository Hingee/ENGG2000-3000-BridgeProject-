#ifndef BRIDGESENSOR_H
#define BRIDGESENSOR_H

#include <Arduino.h>
#include <mutex>

class BridgeSensor {
protected:
    String name;
    SemaphoreHandle_t mutex;
public:
    BridgeSensor(String n);
    String getName();
};

class US : public BridgeSensor {
protected:
    int distance;
public:
    US();
    void updateDist(int d);
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
