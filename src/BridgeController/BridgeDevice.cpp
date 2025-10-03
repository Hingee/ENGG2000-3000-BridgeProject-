#include "BridgeDevice.h"

//BridgeDevice Parent Class
BridgeDevice::BridgeDevice(String n, String initState, int b, String* ps) { 
    name = n; 
    state = initState; 
    buttonState = b; 
    possibleStates = ps;
    mutex = xSemaphoreCreateMutex();
    assert(mutex);

    if (mutex == NULL) {
      Serial.println("ERROR: mutex creation FAILED in BridgeDevice ctor!");
      // Optionally halt here so you can see the message:
      while (1) { delay(1000); }
    } else {
      Serial.println("BridgeDevice: mutex created OK for " + name);
    }
}
void BridgeDevice::setState(String s) { 
  xSemaphoreTake(mutex, portMAX_DELAY);
  state = s; 
  xSemaphoreGive(mutex);
}
String BridgeDevice::getState() { 
  String temp;
  xSemaphoreTake(mutex, portMAX_DELAY);
  temp = state; 
  xSemaphoreGive(mutex);
  return temp;
}
void BridgeDevice::setButton(int b) {  
    xSemaphoreTake(mutex, portMAX_DELAY);
    buttonState = b; 
    xSemaphoreGive(mutex);
}
int BridgeDevice::getButton() { 
    int temp;
    xSemaphoreTake(mutex, portMAX_DELAY);
    temp = buttonState; 
    xSemaphoreGive(mutex);
    return temp; 
}
String BridgeDevice::getName() { return name; }
String BridgeDevice::getPosState(int i){ return possibleStates[i]; }

//Gate Device Child Class
static String gateStates[] = {"Open", "Close"};
Gate::Gate(const String& n) : BridgeDevice(n, "Closed", 0, gateStates) {}
void Gate::open()  { 
    setState("Opened");
    setButton(1);
    Serial.print("Opened gate ");
    Serial.println(name);
}
void Gate::close() { 
    setState("Closed");
    setButton(0); 
    Serial.print("Closing gate ");
    Serial.println(name);
}
// Smooth servo movement
void Gate::moveServoSmooth(int targetPos) {
    if (gatePos == targetPos) return;
    
    if (targetPos > gatePos) {
        for (int pos = gatePos; pos <= targetPos; pos++) {
          myServo.write(pos);
          delay(15);  // speed of motion
        }
    } else {
        for (int pos = gatePos; pos >= targetPos; pos--) {
          myServo.write(pos);
          delay(15);  // speed of motion
        }
    }

    gatePos = targetPos;
}

void Gate::init(int pin) {
    gatePos = 90;
    myServo.attach(pin, 900, 2000);  
    myServo.write(gatePos);
}

//Alarm Device Child Class
static String alarmStates[] = {"On", "Off"};
Alarm::Alarm(const String& n) : BridgeDevice(n, "Off", 0, alarmStates) {}
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
void BridgeMechanism::raise() { 
  setState("Raised");
  setButton(1); 
  Serial.println("Bridge Raised"); 
}
void BridgeMechanism::lower() { 
  setState("Lowered");
  setButton(0); 
  Serial.println("Bridge Lowered"); 
}

//Fake device to implement a flip override
static String overrideStates[] = {"On", "Off"};
Override::Override() : BridgeDevice("Override", "Off", 0, overrideStates) {}
void Override::on() { state = "On"; setButton(1);}
void Override::off() { state = "Off"; setButton(0);}
