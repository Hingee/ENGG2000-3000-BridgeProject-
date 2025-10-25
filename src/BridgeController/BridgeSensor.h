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
  int readUltrasonic(int trigPin, int echoPin);
  void updateDist(int d);
  int getDistance();
};

class PIR : public BridgeSensor {
protected:
  bool isTriggerd;
  int state;
  long lastReadingTime;
  long lastTriggeredTime;
  
public:
  PIR();
  bool isNotTriggeredForSec(int n);
  void setTriggered(bool t);
  bool isTriggered();
  bool read(int pin);
};

#endif
