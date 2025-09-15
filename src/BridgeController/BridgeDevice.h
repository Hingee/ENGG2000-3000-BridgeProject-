#ifndef BRIDGEDEVICE_H
#define BRIDGEDEVICE_H

#include <Arduino.h>
#include <mutex>

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
    Gate();
    void open();
    void close();
};

class Alarm : public BridgeDevice {
public:
    Alarm();
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
    BridgeMechanism();
    void raise();
    void lower();
};

//Fake device to implement a flip override
class Override : public BridgeDevice {
public:
    Override();
    void on();//Threadsafe
    void off();//Threadsafe
};

#endif
