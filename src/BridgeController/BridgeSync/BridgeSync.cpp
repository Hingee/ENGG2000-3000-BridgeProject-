//Sync for Bridge Open/Close Runtimes

//IMPORTANT: This current program is written to be used to be manually modified to suit the required tests of the bridge
//This program is designed to run until termination, reading the number of rotations it takes to complete a cycle of the bridge
//When the motor is hitting its limit of bridge opening, shut off its power. (this could be changed to be implemented into the RUI if needed)

#include <Arduino.h>
#include <string>


//Run Types
std::string rSC = "runSyncCheck"; //Main
std::string rMC = "runMotorClockwise"; //Run continuously clockwise
std::string rMAC = "runMotorAntiClockwise"; //Run continuously anticlockwise

//System Run Type (MANUAL OVERRIDE) 
std::string runType = rSC;

//Motor specific
static int motorDriverPin1 = 27; 
static int motorDriverPin2 = 26; 
int duration = 5000; //1000 is a second
bool Direction = true; //true forward, false backwards

//Encoder
const int encoderPinA = 34;
volatile unsigned long pulseCount = 0;
const int pulsesPerRevolution = 374;

void IRAM_ATTR onPulse();

void setup() {
    Serial.begin(115200);
    delay(100);

    pinMode(motorDriverPin1, OUTPUT);
    pinMode(motorDriverPin2, OUTPUT);
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, LOW);

    pinMode(encoderPinA, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(encoderPinA), onPulse, RISING);
}

void loop() {
    switch (runType) {
    case ("runSyncCheck"):
        Serial.println("Begin Synchronization");
        syncCheck();
        break;
    case("runMotorClockwise"):
        Serial.println("Running Clockwise");
        digitalWrite(motorDriverPin1, LOW);
        digitalWrite(motorDriverPin2, HIGH);
        break;
    case("runMotorAntiClockwise"):
        Serial.println("Running Anti-Clockwise");
        digitalWrite(motorDriverPin1, HIGH);
        digitalWrite(motorDriverPin2, LOW);
        break;
    default:
        Serial.println("Choose a mode");
        break;
    }
}



void printRPM() {
    static unsigned long lastTime = 0;
    unsigned long now = millis();
    if(now - lastTime >= 1000) {
        noInterrupts();
        unsigned long pulses = pulseCount;
        pulseCount = 0;
        interrupts();

        float revolutions = (float)pulses / pulsesPerRevolution;
        float rpm = revolutions * 60.0;

        Serial.print("Pulses/sec: ");
        Serial.print(pulses);
        Serial.print("  |  RPM: ");
        Serial.println(rpm, 2);

        lastTime = now;
    }    
}
