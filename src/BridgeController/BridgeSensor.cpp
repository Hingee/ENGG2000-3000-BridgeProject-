#include "BridgeSensor.h"

//BridgeSensor Parent Class
BridgeSensor::BridgeSensor(String n) { 
    name = n; 
    mutex = xSemaphoreCreateMutex();
    assert(mutex);
}
String BridgeSensor::getName() {return name; }

//Ultra Sonic Sensor Child Class
US::US() : BridgeSensor("Ultra-Sonic") { distance = 0; }
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

//PIR Sensor Child Class
PIR::PIR() : BridgeSensor("Passive Infrared Sensor") { isTriggerd = false; }
void PIR::setTriggerd(boolean t) { 
    xSemaphoreTake(mutex, portMAX_DELAY);
    isTriggerd = t; 
    xSemaphoreGive(mutex);
}
boolean PIR::isTriggered() { 
    boolean temp;
    xSemaphoreTake(mutex, portMAX_DELAY);
    temp = isTriggerd; 
    xSemaphoreGive(mutex);
    return temp;
}
