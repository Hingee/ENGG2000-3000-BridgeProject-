//Sync for Bridge Open/Close Runtimes

//IMPORTANT: This current program is written to be used to be manually modified to suit the required tests of the bridge
//This program is designed to run until termination, reading the number of rotations it takes to complete a cycle of the bridge
//When the motor is hitting its limit of bridge opening, shut off its power. (this could be changed to be implemented into the RUI if needed)

#include <Arduino.h>

//System Run Type (MANUAL OVERRIDE) 
int runType = 0;
    //0 = Run sync check (Main)
    //1 = Run continuously clockwise
    //2 = Run continuously anticlockwise

//Motor specific
static int motorDriverPin1 = 27; 
static int motorDriverPin2 = 26; 
int duration = 5000; //1000 is a second
float totalRotations = 0.0; 

//Encoder
const int encoderPinA = 34; //encoder won't be needed on main, only needed for trouble shooting
volatile unsigned long pulseCount = 0;
const int pulsesPerRevolution = 700;


void IRAM_ATTR onPulse() { //change in main
    pulseCount++;
}

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
    if (runType == 0) {
        Serial.println("Begin Synchronization");
        syncCheck();
    } else if (runType == 1) {
        Serial.println("Running Clockwise");
        runMotorClockwise();
    } else if (runType == 2) {
        Serial.println("Running Anti-Clockwise");
        runMotorAntiClockwise();
    } else {
        Serial.println("Choose a mode");
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

void haltMotor() {
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, LOW);  
}

void syncCheck() {
    long prev1 = 0;
    long prev2 = 0;
    int endTime = 10000;

    //infinite loop
    while(true) {
        while(millis() - prev1 <= endTime) {
            runMotorClockwise();
            if(millis() - prev2 <= 1000) {
                noInterrupts();
                unsigned long pulses = pulseCount;
                pulseCount = 0;
                interrupts();
                float revolutions = (float) pulses / pulsesPerRevolution;
                float rpm = revolutions * 60.0;
                totalRotations += revolutions; 
                Serial.print("Live RPM: ");
                Serial.print(rpm, 2);
                prev2 = millis();
            } 
        }

        haltMotor();
        Serial.print("Opening sequence of ");
        Serial.print((millis() - prev1) / 1000);
        Serial.print(" seconds done in ");
        Serial.print(totalRotations);
        Serial.println(" revolutions.");
        totalRotations = 0;
        prev1 = millis();   
        delay(2000);

        Serial.println("Lowering Bridge back to neutral state.");
        Serial.println("Check for consistency alignment.");

        //this part will be similar to new method for main
        float reverseRevolutions = 0.0;
        while(reverseRevolutions <= totalRotations) {
            runMotorAntiClockwise();
            noInterrupts();
            unsigned long pulses = pulseCount;
            pulseCount = 0;
            interrupts();
            reverseRevolutions += (float) pulses / pulsesPerRevolution;
        }

        haltMotor();
        totalRotations = 0;
        endTime += 10000; //add 10 secs to opening
    }
}