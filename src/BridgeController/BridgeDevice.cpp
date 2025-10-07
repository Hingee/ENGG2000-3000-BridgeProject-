#include "BridgeDevice.h"

//BridgeDevice Parent Class
BridgeDevice::BridgeDevice(String n, String* pa, String* ps, int pal) { 
    name = n; 
    state = 0; 
    possibleActions = pa;
    possibleStates = ps;
    possALen = pal;
    mutex = xSemaphoreCreateMutex();
    working = false;
    assert(mutex);

    if (mutex == NULL) {
      Serial.println("ERROR: mutex creation FAILED in BridgeDevice ctor!");
      // Optionally halt here so you can see the message:
      while (1) { delay(1000); }
    } else {
      Serial.println("BridgeDevice: mutex created OK for " + name);
    }
}
void BridgeDevice::setState(int s) { 
  if(s >= possALen) setWorking(true);
  else setWorking(false);
  
  xSemaphoreTake(mutex, portMAX_DELAY);
  state = s; 
  xSemaphoreGive(mutex);
}
String BridgeDevice::getState() { 
  String temp;
  xSemaphoreTake(mutex, portMAX_DELAY);
  temp = possibleStates[state]; 
  xSemaphoreGive(mutex);
  return temp;
}
int BridgeDevice::getStateNum() { 
  int temp;
  xSemaphoreTake(mutex, portMAX_DELAY);
  temp = state; 
  xSemaphoreGive(mutex);
  return temp;
}
String BridgeDevice::getName() { return name; }
String BridgeDevice::getAction(){ 
  int temp = getStateNum();
  if(temp >= possALen) return "-";
  return possibleActions[temp]; 
}
String BridgeDevice::getAction(int i){ 
  if(i >= possALen) return "-";
  return possibleActions[i]; 
}
void BridgeDevice::setWorking(bool w) {
  xSemaphoreTake(mutex, portMAX_DELAY);
  working = w; 
  xSemaphoreGive(mutex);
}
bool BridgeDevice::isWorking() {
  bool temp;
  xSemaphoreTake(mutex, portMAX_DELAY);
  temp = working;
  xSemaphoreGive(mutex);
  return temp;
}

//Gate Device Child Class
Gate::Gate(const String& n, String* actions, String* states, int len) : BridgeDevice(n,  actions, states, len) {}
void Gate::openNet() { setState(2); Serial.println("Raising Gates"); }
unsigned long Gate::openHard(unsigned long lastTime, int stepDelay) { 
  int targetPos = 90;
  if (gatePos == targetPos) {
    setState(0); 
    Serial.println("Raised Gates"); 
    return millis();
  }
  
  moveServoSmooth(targetPos,lastTime,stepDelay); 
  return millis();
}
void Gate::closeNet() { setState(3); Serial.println("Lowering Gates"); }
unsigned long Gate::closeHard(unsigned long lastTime, int stepDelay) { 
  int targetPos = 0;
  if (gatePos == targetPos) {
    setState(1); 
    Serial.println("Lowered Gates"); 
    return millis();
  }
  
  moveServoSmooth(targetPos,lastTime,stepDelay); 
  return millis();
}
void Gate::moveServoSmooth(int targetPos, unsigned long lastTime, int stepDelay) {
    if(millis() - lastTime >= stepDelay) {
      if (targetPos > gatePos) {
        gatePos++;
      } else {
        gatePos--;
      }
      servo1.write(gatePos);
      servo2.write(gatePos);
    }
    Serial.println(gatePos);
}
void Gate::init(int pin1, int pin2) {
    gatePos = 90;
    servo1.attach(pin1, 900, 2000);  
    servo1.write(gatePos);

    servo2.attach(pin2, 900, 2000);  
    servo2.write(gatePos);
}

//Alarm Device Child Class
Alarm::Alarm(const String& n, String* actions, String* states, int len) : BridgeDevice(n, actions, states, len) {}
void Alarm::activate()   { setState(0); Serial.println("Alarm On"); }
void Alarm::deactivate() { setState(1); Serial.println("Alarm Off"); }

//Light Device Child Class TODO implement RGY on light class
Light::Light(const String& n, String* states, int len) : BridgeDevice(n, states, states, len) {}
void Light::turnRed()  { setState(0); Serial.println(name + " Red"); }
void Light::turnGreen() { setState(1); Serial.println(name + " Green"); }
void Light::turnYellow() { setState(2); Serial.println(name + " Yellow"); }

//Bridge Mechanism Device Child Class
BridgeMechanism::BridgeMechanism(const String& n, String* actions, String* states, int len) : BridgeDevice(n, actions, states, len) {
    revolutionsCurrent = 0.0;
    revolutionsToOpen = 50.0;
}
void BridgeMechanism::raiseNet() { setState(2); Serial.println("Bridge Raising"); }
bool BridgeMechanism::raiseHard() { 
  Serial.println(revolutionsCurrent, 2);
  Serial.println(revolutionsToOpen, 2);
  if(revolutionsCurrent >= revolutionsToOpen) {
      haltMotor();
      setState(0); 
      
      Serial.println("Bridge Raised"); 
      return true;
  }
  
  //Run the motor clockwise 
  digitalWrite(motorDriverPin1, HIGH);
  digitalWrite(motorDriverPin2, LOW); 

  return false;
}
void BridgeMechanism::lowerNet() {  setState(3); Serial.println("Bridge Lowering"); }
bool BridgeMechanism::lowerHard() {
  Serial.println(revolutionsCurrent, 2);
  Serial.println(revolutionsToOpen, 2);  
  if(revolutionsCurrent <= 0.0) {
        haltMotor();
        setState(1); 
        
        Serial.println("Bridge Lowered"); 
        return true;
  }
  
  //Run the motor anti-clockwise
  digitalWrite(motorDriverPin1, LOW);
  digitalWrite(motorDriverPin2, HIGH);

  return false;
}
void BridgeMechanism::haltMotor() {
    Serial.println("System motor is idle.");
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, LOW);  
}
void BridgeMechanism::incRev(unsigned long p, int ppr) { 
//  revolutionsCurrent += (float) p / ppr; 
    revolutionsCurrent++;
}
void BridgeMechanism::decRev(unsigned long p, int ppr) { 
//  revolutionsCurrent -= (float) p / ppr;
    revolutionsCurrent--;
}
void BridgeMechanism::init(int mp1, int mp2, int encPin) {
    //Motor Setup
    pinMode(mp1, OUTPUT);
    pinMode(mp2, OUTPUT);
    digitalWrite(mp1, LOW);
    digitalWrite(mp2, LOW);

    pinMode(encPin, INPUT_PULLUP);

    motorDriverPin1 = mp1;
    motorDriverPin2 = mp2;
}

//Fake device to implement a flip override
Override::Override(const String& n, String* actions, String* states, int len) : BridgeDevice(n, actions, states, len) {}
void Override::on() { setState(1);}
void Override::off() { setState(0);}
bool Override::isOn() { return getStateNum()==1; }
