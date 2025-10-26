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
  int trigPin;
  int echoPin;

public:
  US(int tp, int ep);
  int readUltrasonic();
  void updateDist(int d);
  int getDistance();
};

class PIR : public BridgeSensor {
protected:
  bool isTriggerd;
  int state;
  long lastReadingTime;
  long lastTriggeredTime;
  int pin;
  
public:
  PIR(int p);
  bool isNotTriggeredForSec(int n);
  void setTriggered(bool t);
  bool isTriggered();
  bool read();
};

#endif
