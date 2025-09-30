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
int totalRotations = 0; 

//Encoder
const int encoderPinA = 34;
volatile unsigned long pulseCount = 0;
const int pulsesPerRevolution = 700; //change this in main


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
        runMotorClockwise();
        break;
    case("runMotorAntiClockwise"):
        Serial.println("Running Anti-Clockwise");
        runMotorAntiClockwise();
        break;
    default:
        Serial.println("Choose a mode");
        break;
    }
}

void runMotorClockwise() { //need to change the pin order in main
    digitalWrite(motorDriverPin1, HIGH);
    digitalWrite(motorDriverPin2, LOW);
}

void runMotorAntiClockwise() {
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, HIGH);
}


void syncCheck() {
    long int time = millis();
    bool direction = true;
    int prevTime = 0;
    int endTime = 10000;

    //infinite loop
    while(true) {
        while(time - prev1 <= endTime) {
            if(direction) {runMotorClockwise();} 
            else {runMotorAntiClockwise();}
            if(time - prev2 <= 1000) { //every second check
                noInterrupts();
                unsigned long pulses = pulseCount;
                pulseCount = 0;
                interrupts();
                float revolutions = (float) pulses / pulsesPerRevolution;
                totalRotations += revolutions; 
                prev2 = time;
            } 
        }
        Serial.println("Opening sequence of " + time - prev1/1000 + " seconds done in " + totalRotations + " revolutions.");
        totalRotations = 0;
        direction = !direction
        prev1 = time;       

        //has to be a different part completely for reverse as we are counting down
        //the revolutions - this will be super close to the actual procedure.
        endTime += 10000;
    }
}