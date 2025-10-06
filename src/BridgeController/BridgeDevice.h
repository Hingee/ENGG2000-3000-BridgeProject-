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
    String* possibleActions;
    SemaphoreHandle_t mutex;
    int signal;
public:
    BridgeDevice(String n, String initState, int b, String* ps);
    void setState(String s); //Threadsafe
    String getState(); //Threadsafe
    void setButton(int b); //Threadsafe
    int getButton(); //Threadsafe
    String getName();
    String getAction(int i);
    void signalAction(int s); //Threadsafe
    int getSignal(); //Threadsafe
};

class Gate : public BridgeDevice {
public:
    Servo myServo;
    int gatePos;
    
    Gate(const String& n);
    void open();
    void close();
    void moveServoSmooth(int targetPos);
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
    volatile unsigned long pulseCount = 0;
    int duration;
    int pulsesPerRevolution;
    int motorDriverPin1;
    int motorDriverPin2;

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
