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
static String gateStates[] = {"Open", "Close", "Opening", "Closing"};
Gate::Gate(const String& n) : BridgeDevice(n, "Closed", 0, gateStates) {}
void Gate::open()  { 
    setState("Opening");
    setButton(2);
    Serial.print("Opened gate ");
    Serial.println(name);
}
void Gate::close() { 
    setState("Closing");
    setButton(3); 
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

void Gate::servoClose() {
    moveServoSmooth(0);
    setState("Closed");
    setButton(1); 
    Serial.print("Closed Gate");
    Serial.println(name);
}

void Gate::servoOpen() {
    moveServoSmooth(90);
    setState("Opened");
    setButton(0); 
    Serial.print("Opened Gate");
    Serial.println(name);
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
static String mechanismStates[] = {"Raise", "Lower", "Raising", "Lowering"};
BridgeMechanism::BridgeMechanism() : BridgeDevice("Bridge_Mechanism", "Lowered", 0, mechanismStates) {}
void BridgeMechanism::raise() { 
  setState("Raising");
  setButton(2); 
  Serial.println("Bridge Raising"); 
}
void BridgeMechanism::lower() { 
  setState("Lowering");
  setButton(3); 
  Serial.println("Bridge Lowering"); 
}

void BridgeMechanism::raiseSequence(){ 
    if(!mechanismState) return;
    Serial.println("Opening sequence (Forward)");
    unsigned long startTime = millis();
    while(millis() - startTime < duration) {
        digitalWrite(motorDriverPin1, LOW); 
        digitalWrite(motorDriverPin2, HIGH); 
        delay(1);  // Let the watchdog breathe
    }
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, LOW);
    mechanismState = false;

    setState("Raised");
    setButton(1); 
    Serial.println("Bridge Raised"); 
}

void BridgeMechanism::lowerSequence(){
    if(mechanismState) return;
    Serial.println("Closing sequence (Backwards)");
    unsigned long startTime = millis();
    while(millis() - startTime < duration) {
        digitalWrite(motorDriverPin1, HIGH);
        digitalWrite(motorDriverPin2, LOW);
        delay(1);  // Let the watchdog breathe
    }
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, LOW);
    mechanismState = true;

    setState("Lowered");
    setButton(0); 
    Serial.println("Bridge Lowered"); 
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

    mechanismState = true; //true closed, false open
    pulseCount = 0;
    duration = dur;
    pulsesPerRevolution = pulsePerRev;
}

//Fake device to implement a flip override
static String overrideStates[] = {"On", "Off"};
Override::Override() : BridgeDevice("Override", "Off", 0, overrideStates) {}
void Override::on() { state = "On"; setButton(1);}
void Override::off() { state = "Off"; setButton(0);}
