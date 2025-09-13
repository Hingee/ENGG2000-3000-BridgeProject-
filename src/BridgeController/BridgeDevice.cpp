#include "BridgeDevice.h"

//BridgeDevice Parent Class
BridgeDevice::BridgeDevice(String n, String initState, int b, String* ps) { 
    name = n; 
    state = initState; 
    buttonState = b; 
    possibleStates = ps;
}
void BridgeDevice::setState(String s) { state = s; }
String BridgeDevice::getState() { return state; }
void BridgeDevice::setButton(int b) { buttonState = b; }
int BridgeDevice::getButton() { return buttonState; }
String BridgeDevice::getName() { return name; }
String BridgeDevice::getPosState(int i){ return possibleStates[i]; }

//Gate Device Child Class
static String gateStates[] = {"Open", "Close"};
Gate::Gate() : BridgeDevice("Gate", "Closed", 0, gateStates) {}
void Gate::open()  { state = "Opened"; setButton(1); Serial.println("Gate Opened"); }
void Gate::close() { state = "Closed"; setButton(0); Serial.println("Gate Closed"); }

//Alarm Device Child Class
static String alarmStates[] = {"On", "Off"};
Alarm::Alarm() : BridgeDevice("Alarm", "Off", 0, alarmStates) {}
void Alarm::activate()   { state = "On"; setButton(1); Serial.println("Alarm On"); }
void Alarm::deactivate() { state = "Off"; setButton(0); Serial.println("Alarm Off"); }

//Light Device Child Class TODO implement RGY on light class
static String lightStates[] = {"Red", "Green", "Yellow"};
Light::Light(const String& n) : BridgeDevice(n, "Off",0, lightStates) {}
void Light::turnRed()  { state = "Red"; setButton(0); Serial.println(name + " Red"); }
void Light::turnGreen() { state = "Green"; setButton(1); Serial.println(name + " Green"); }
void Light::turnYellow() { state = "Yellow"; setButton(2); Serial.println(name + " Yellow"); }

//Bridge Mechanism Device Child Class
static String mechanismStates[] = {"Raise", "Lower"};
BridgeMechanism::BridgeMechanism() : BridgeDevice("Bridge_Mechanism", "Lowered", 0, mechanismStates) {}
void BridgeMechanism::raise() { state = "Raised"; setButton(1); Serial.println("Bridge Raised"); }
void BridgeMechanism::lower() { state = "Lowered"; setButton(0); Serial.println("Bridge Lowered"); }

//Fake device to implement a flip override
static String overrideStates[] = {"On", "Off"};
Override::Override() : BridgeDevice("Override", "Off", 0, overrideStates) {}
void Override::on() { state = "On"; setButton(1);}
void Override::off() { state = "Off"; setButton(0);}
