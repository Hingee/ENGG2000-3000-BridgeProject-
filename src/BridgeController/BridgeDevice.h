#ifndef BRIDGEDEVICE_H
#define BRIDGEDEVICE_H

#include <Arduino.h>
#include <mutex>
#include <ESP32Servo.h>

class BridgeDevice {
protected:
  String name;
  int state;
  int possALen;
  int possSLen;
  String* possibleStates;
  String* possibleActions;
  bool working;
  SemaphoreHandle_t mutex;
public:
  BridgeDevice(const String& n, String* pa, String* ps, int pal, int psl);

  void setState(int s);  //Threadsafe
  String getState();     //Threadsafe
  String getName() const;
  String getAction();
  String getAction(int idx);
  int getStateNum();
  void setWorking(bool w);  //Threadsafe
  bool isWorking();         //Threadsafe
  int getNumStates();       //Threadsafe
};

class Gate : public BridgeDevice {
public:
  Servo servo1;
  Servo servo2;
  int gatePos;
  bool idle;

  Gate(const String& n, String* actions, String* states,  int aLen, int sLen);

  void openNet();
  unsigned long openHard(unsigned long lastTime, int stepDelay);

  void closeNet();
  unsigned long closeHard(unsigned long lastTime, int stepDelay);

  unsigned long moveServoSmooth(int targetPos, unsigned long lastTime, int stepDelay);
  bool isIdle();

  void init(int pin1, int pin2);
};

class Alarm : public BridgeDevice {
public:
  int pin;
  Alarm(const String& n, String* actions, String* states, int len);
  void init(int p);
  void activate();
  void deactivate();
};

class Light : public BridgeDevice {
public:
  int greenPin;
  int redPin;
  int yellowPin;
  
  Light(const String& n, String* states, int len);
  void init(int rPin, int yPin, int gPin);
  void init(int rPin, int gPin);
  void turnRed();
  void turnGreen();
  void turnYellow();
};

class BridgeMechanism : public BridgeDevice {
public:
  float revolutionsCurrent;
  float revolutionsToOpen;
  int motorDriverPin1;
  int motorDriverPin2;

  BridgeMechanism(const String& n, String* actions, String* states,  int aLen, int sLen);

  void raiseNet();
  bool raiseHard();

  void lowerNet();
  bool lowerHard();

  void haltMotor();

  void incRev(unsigned long p, int ppr);
  void decRev(unsigned long p, int ppr);

  void init(int mp1, int mp2, int encPin);
};

//Fake device to implement a flip override
class Override : public BridgeDevice {
public:
  Override(const String& n, String* actions, String* states, int len);
  void on();
  void off();
  bool isOn();
};

#endif
