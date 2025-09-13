#include "BridgeSensor.h"

//BridgeSensor Parent Class
BridgeSensor::BridgeSensor(String n) { name = n; }
String BridgeSensor::getName() {return name; }

//Ultra Sonic Sensor Child Class
US::US() : BridgeSensor("Ultra-Sonic") { distance = 0; }
void US::updateDist(int d) { distance = d; }
int US::getDistance() { return distance; }

//PIR Sensor Child Class
PIR::PIR() : BridgeSensor("Passive Infrared Sensor") { isTriggerd = false; }
void PIR::setTriggerd(boolean t) { isTriggerd = t; }
boolean PIR::isTriggered() { return isTriggerd; }
