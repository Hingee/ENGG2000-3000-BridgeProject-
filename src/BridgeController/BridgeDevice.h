#ifndef BRIDGEDEVICE_H
#define BRIDGEDEVICE_H

#include <Arduino.h>
#include <mutex>
#include <ESP32Servo.h>

class BridgeDevice {
protected:
    String name;
    String state;
    int buttonState;
    String* possibleStates;
    SemaphoreHandle_t mutex;
public:
    BridgeDevice(String n, String initState, int b, String* ps);
    void setState(String s);
    String getState();
    void setButton(int b);//Threadsafe
    int getButton();//Threadsafe
    String getName();
    String getPosState(int i);
};

class Gate : public BridgeDevice {
public:
    // Servo setup
    Servo myServo;
    int gatePos;  // Start open (upright)
    
    Gate(const String& n);
    void open();
    void close();
    void moveServoSmooth(int targetPos);
    void servoClose();
    void servoOpen();
    void init(int pin);
};

class Alarm : public BridgeDevice {
public:
    Alarm(const String& n);
    void activate();
    void deactivate();
};

class Light : public BridgeDevice {
public:
    Light(const String& n);
    void turnRed();
    void turnGreen();
    void turnYellow();
};

class BridgeMechanism : public BridgeDevice {
public:
    bool mechanismState = true; //true closed, false open
    volatile unsigned long pulseCount = 0;
    int duration;
    int pulsesPerRevolution;

    BridgeMechanism();
    void raise();
    void lower();
    void raiseSequence();
    void lowerSequence();
    void onPulse();
    void init(int mp1, int mp2, int encPin, int dur, int pulsePerRev);
};

//Fake device to implement a flip override
class Override : public BridgeDevice {
public:
    Override();
    void on();//Threadsafe
    void off();//Threadsafe
};

#endif
