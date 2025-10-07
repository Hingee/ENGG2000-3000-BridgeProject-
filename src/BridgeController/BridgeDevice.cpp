#include "BridgeDevice.h"

// ---------- Bridge Device Parent Class ----------
BridgeDevice::BridgeDevice(const String& n, String* pa, String* ps, int pal, int psl) {
  name = n;
  state = 0;
  possibleActions = pa;
  possibleStates = ps;
  possALen = pal;
  possSLen = psl;
  working = false;

  mutex = xSemaphoreCreateMutex();
  assert(mutex);

  if (mutex == NULL) {
    Serial.println("[BridgeDevice] ERROR: mutex creation FAILED in BridgeDevice ctor!");
  } else {
    Serial.print("[BridgeDevice] mutex created OK for ");
    Serial.println(name);
  }
}

void BridgeDevice::setState(int s) {
  if (s >= possALen) setWorking(true);
  else setWorking(false);

  xSemaphoreTake(mutex, portMAX_DELAY);
  state = s;
  xSemaphoreGive(mutex);
}

String BridgeDevice::getState() {
  String temp;
  xSemaphoreTake(mutex, portMAX_DELAY);
  if (state >= 0 && state < possSLen) temp = possibleStates[state];
  else temp = "[BridgeDevice] Error: Unknown State";
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

String BridgeDevice::getName() const {
  return name;
}

String BridgeDevice::getAction() {
  int idx = getStateNum();
  if (idx >= possALen) return "[BridgeDevice] Error: Unknown Action";
  return possibleActions[idx];
}

String BridgeDevice::getAction(int idx) {
  if (idx >= possALen) return "[BridgeDevice] Error: Unknown Action i";
  return possibleActions[idx];
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

// ---------- Gate Implementation ----------
Gate::Gate(const String& n, String* actions, String* states, int aLen, int sLen)
  : BridgeDevice(n, actions, states, aLen, sLen) {}

void Gate::openNet() {
  setState(2);
  Serial.println("[Gate] Raising Gates");
}
unsigned long Gate::openHard(unsigned long lastTime, int stepDelay) {
  const int targetPos = 90;
  if (gatePos == targetPos) {
    setState(0);
    Serial.println("[Gate] Raised Gates");
    idle = true;
    return millis();
  }

  return moveServoSmooth(targetPos, lastTime, stepDelay);
}

void Gate::closeNet() {
  setState(3);
  Serial.println("[Gate] Lowering Gates");
}
unsigned long Gate::closeHard(unsigned long lastTime, int stepDelay) {
  const int targetPos = 0;
  if (gatePos == targetPos) {
    setState(1);
    Serial.println("[Gate] Lowered Gates");
    idle = true;
    return millis();
  }

  return moveServoSmooth(targetPos, lastTime, stepDelay);
}

unsigned long Gate::moveServoSmooth(int targetPos, unsigned long lastTime, int stepDelay) {
  unsigned long now = millis();
  idle = false;

  if (now - lastTime >= stepDelay) {
    if (targetPos > gatePos) gatePos++;
    else gatePos--;

    servo1.write(gatePos);
    servo2.write(gatePos);
    lastTime = now;
  }
  //Debugging
  Serial.print("[Gate] pos=");
  Serial.println(gatePos);
  return lastTime;
}
bool Gate::isIdle() {
  return idle;
}

void Gate::init(int pin1, int pin2) {
  idle = true;
  gatePos = 90;
  servo1.attach(pin1, 900, 2000);
  servo1.write(gatePos);

  servo2.attach(pin2, 900, 2000);
  servo2.write(gatePos);
}

// ---------- Alarm Implementation ----------
Alarm::Alarm(const String& n, String* actions, String* states, int len)
  : BridgeDevice(n, actions, states, len, len) {}
void Alarm::activate() {
  setState(0);
  Serial.println("[Alarm] Alarm On");
}
void Alarm::deactivate() {
  setState(1);
  Serial.println("[Alarm] Alarm Off");
}

// ---------- Light Implementation ----------
Light::Light(const String& n, String* states, int len)
  : BridgeDevice(n, states, states, len, len) {}
void Light::turnRed() {
  setState(0);
  Serial.print("[Light] ");
  Serial.print(name);
  Serial.println(" Red");
}
void Light::turnGreen() {
  setState(1);
  Serial.print("[Light] ");
  Serial.print(name);
  Serial.println(" Green");
}
void Light::turnYellow() {
  setState(2);
  Serial.print("[Light] ");
  Serial.print(name);
  Serial.println(" Yellow");
}

// ---------- BridgeMechanism Implementation ----------
BridgeMechanism::BridgeMechanism(const String& n, String* actions, String* states, int aLen, int sLen)
  : BridgeDevice(n, actions, states, aLen, sLen) {
  revolutionsCurrent = 0.0;
  revolutionsToOpen = 50.0;
}

void BridgeMechanism::raiseNet() {
  setState(2);
  Serial.println("[Mechanism] Raising");
}
bool BridgeMechanism::raiseHard() {
  Serial.print("[Mechanism] cur=");
  Serial.print(revolutionsCurrent);
  Serial.print(" target=");
  Serial.println(revolutionsToOpen);

  if (revolutionsCurrent >= revolutionsToOpen) {
    haltMotor();
    setState(0);
    Serial.println("[Mechanism] Raised");
    return true;
  }

  //Run the motor clockwise
  digitalWrite(motorDriverPin1, HIGH);
  digitalWrite(motorDriverPin2, LOW);
  return false;
}

void BridgeMechanism::lowerNet() {
  setState(3);
  Serial.println("[Mechanism] Lowering");
}
bool BridgeMechanism::lowerHard() {
  Serial.print("[Mechanism] cur=");
  Serial.print(revolutionsCurrent);
  Serial.print(" target=");
  Serial.println(revolutionsToOpen);

  if (revolutionsCurrent <= 0.0) {
    haltMotor();
    setState(1);
    Serial.println("[Mechanism] Lowered");
    return true;
  }

  //Run the motor anti-clockwise
  digitalWrite(motorDriverPin1, LOW);
  digitalWrite(motorDriverPin2, HIGH);
  return false;
}

void BridgeMechanism::haltMotor() {
  Serial.println("[Mechanism] Motor Idle.");
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
  motorDriverPin1 = mp1;
  motorDriverPin2 = mp2;

  //Motor Setup
  pinMode(motorDriverPin1, OUTPUT);
  pinMode(motorDriverPin2, OUTPUT);
  digitalWrite(motorDriverPin1, LOW);
  digitalWrite(motorDriverPin2, LOW);

  pinMode(encPin, INPUT_PULLUP);
}

// ---------- Override Implementation ----------
Override::Override(const String& n, String* actions, String* states, int len)
  : BridgeDevice(n, actions, states, len, len) {}
void Override::on() {
  setState(1);
}
void Override::off() {
  setState(0);
}
bool Override::isOn() {
  return getStateNum() == 1;
}
