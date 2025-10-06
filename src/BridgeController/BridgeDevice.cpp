#include "BridgeDevice.h"

//BridgeDevice Parent Class
BridgeDevice::BridgeDevice(String n, String initState, int b, String* ps) { 
    name = n; 
    state = initState; 
    buttonState = b; 
    possibleActions = ps;
    mutex = xSemaphoreCreateMutex();
    assert(mutex);
    signal = -1;

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
void BridgeDevice::signalAction(int s) {
    if(s == 1) setState("Raising");
    else if(s == 0) setState("Lowering");
    if(s != -1) setButton(-1);
    
    xSemaphoreTake(mutex, portMAX_DELAY);
    signal = s; 
    xSemaphoreGive(mutex);
}
int BridgeDevice::getSignal() { 
    int temp;
    xSemaphoreTake(mutex, portMAX_DELAY);
    temp = signal; 
    xSemaphoreGive(mutex);
    return temp; 
}
String BridgeDevice::getName() { return name; }
String BridgeDevice::getAction(int i){ return possibleActions[i]; }

//Gate Device Child Class
static String gateActions[] = {"Raise", "Lower"};
Gate::Gate(const String& n) : BridgeDevice(n, "Lowered", 0, gateActions) {}
void Gate::open()  { 
    setState("Raising");
    setButton(-1);
    Serial.println("Raising Gates");

    moveServoSmooth(90);
    
    setState("Raised");
    setButton(1); 
    Serial.println("Raised Gates");
}
void Gate::close() { 
    setState("Lowering");
    setButton(-1); 
    Serial.println("Lowering Gates");

    moveServoSmooth(0);
    
    setState("Lowered");
    setButton(0); 
    Serial.println("Lowered Gates");
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
    setState("Raising");
    setButton(-1); 
    Serial.println("Bridge Raising"); 

    raiseSequence();
    setState("Raised");
    setButton(1); 
    Serial.println("Bridge Raised");
}
void BridgeMechanism::lower() { 
    setState("Lowering");
    setButton(-1); 
    Serial.println("Bridge Lowering"); 
  
    lowerSequence();
    setState("Lowered");
    setButton(0); 
    Serial.println("Bridge Lowered");
}

void BridgeMechanism::raiseSequence(){ 
    Serial.println("Raising sequence (Forward)");
    unsigned long startTime = millis();
    while(millis() - startTime < duration) {
        digitalWrite(motorDriverPin1, LOW); 
        digitalWrite(motorDriverPin2, HIGH); 
        delay(1);  // Let the watchdog breathe
    }
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, LOW);
}

void BridgeMechanism::lowerSequence(){
    Serial.println("Closing sequence (Backwards)");
    unsigned long startTime = millis();
    while(millis() - startTime < duration) {
        digitalWrite(motorDriverPin1, HIGH);
        digitalWrite(motorDriverPin2, LOW);
        delay(1);  // Let the watchdog breathe
    }
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, LOW);
}

void IRAM_ATTR BridgeMechanism::onPulse() {
  pulseCount++; //count pulses
}

void BridgeMechanism::init(int mp1, int mp2, int encPin, int dur, int pulsePerRev) {
    //Motor Setup
    pinMode(mp1, OUTPUT);
    pinMode(mp2, OUTPUT);
    digitalWrite(mp1, LOW);
    digitalWrite(mp2, LOW);

    pinMode(encPin, INPUT_PULLUP);

    pulseCount = 0;
    duration = dur;
    pulsesPerRevolution = pulsePerRev;
    motorDriverPin1 = mp1;
    motorDriverPin2 = mp2;
}

//Fake device to implement a flip override
static String overrideStates[] = {"On", "Off"};
Override::Override() : BridgeDevice("Override", "Off", 0, overrideStates) {}
void Override::on() { state = "On"; setButton(1);}
void Override::off() { state = "Off"; setButton(0);}
