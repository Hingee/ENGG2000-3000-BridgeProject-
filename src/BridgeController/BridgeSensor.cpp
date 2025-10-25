#include "BridgeSensor.h"

// ---------- Bridge Sensor Parent Class ----------
BridgeSensor::BridgeSensor(String n) {
  name = n;
  mutex = xSemaphoreCreateMutex();
  assert(mutex);
}
String BridgeSensor::getName() {
  return name;
}

// ---------- Ultrasonic Implementation ----------
US::US()
  : BridgeSensor("Ultra-Sonic") {
  distance = 0;
}
int US::readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);  // 30ms timeout
  if (duration == 0) {
    updateDist(-1);
    return -1;  //no echo
  }

  int distance = duration * 0.034 / 2;  // convert to cm
  updateDist(distance);                 //Critical
  return distance;
}
void US::updateDist(int d) {
  xSemaphoreTake(mutex, portMAX_DELAY);
  distance = d;
  xSemaphoreGive(mutex);
}
int US::getDistance() {
  int temp;
  xSemaphoreTake(mutex, portMAX_DELAY);
  temp = distance;
  xSemaphoreGive(mutex);
  return temp;
}

// ---------- PIR Implementation ----------
PIR::PIR()
  : BridgeSensor("Passive Infrared Sensor") {
  isTriggerd = false;
  state = LOW;
}
void PIR::setTriggered(bool t) {
  xSemaphoreTake(mutex, portMAX_DELAY);
  isTriggerd = t;
  xSemaphoreGive(mutex);
}
bool PIR::isTriggered() {
  bool temp;
  xSemaphoreTake(mutex, portMAX_DELAY);
  temp = isTriggerd;
  xSemaphoreGive(mutex);
  return temp;
}
bool PIR::isNotTriggeredForSec(int n) {
  if(!isTriggered() && (millis() - lastTriggeredTime > n*1000)) return true;
  return false;
}
bool PIR::read(int pin) {
  if(millis() - lastReadingTime < 500 ) return isTriggered(); //Holds val for 0.5s
  
  lastReadingTime = millis();
  int val = digitalRead(pin);  // read input value
  
  if(val == HIGH && state == LOW) { //Motion Detected
    state == HIGH;
    lastTriggeredTime = millis();
    setTriggered(true);
    return true;
  }else if (state == HIGH) {
    state = LOW;
  }

  setTriggered(false);
  return false;
}
