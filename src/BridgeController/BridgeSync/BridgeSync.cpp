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
//note to add a runUntil method (based on the amount of revolutions it needs to do to be manually reset if disconnected from power and it forgets its position)
}


void runMotorClockwise() { //need to change the pin order in main
    digitalWrite(motorDriverPin1, HIGH);
    digitalWrite(motorDriverPin2, LOW);

    long prev = 0;
    if(millis() - prev <= 1000) {
        noInterrupts();
        unsigned long pulses = pulseCount;
        pulseCount = 0;
        interrupts();
        float revolutions = (float) pulses / pulsesPerRevolution;
        float rpm = revolutions * 60.0;
        totalRotations += revolutions; 
        Serial.print("Live RPM: ");
        Serial.print(rpm, 2);
        Serial.print(" | Revolutions: ");
        Serial.println(revolutions, 2);
        prev = millis();  
    }
}

void runMotorAntiClockwise() {
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, HIGH);

    long prev = 0;
    if(millis() - prev <= 1000) {
        noInterrupts();
        unsigned long pulses = pulseCount;
        pulseCount = 0;
        interrupts();
        float revolutions = (float) pulses / pulsesPerRevolution;
        float rpm = revolutions * 60.0;
        totalRotations += revolutions; 
        Serial.print("Live RPM: ");
        Serial.print(rpm, 2);
        Serial.print(" | Revolutions: ");
        Serial.println(revolutions, 2);
        prev = millis();  
    }
}

void haltMotor() {
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, LOW);  
}

void syncCheck() {
    long prevAlt = 0;
    int endTime = 10000;

    //infinite loop
    while(true) {
        //initially run for 10 seconds
        while(millis() - prevAlt <= endTime) {
            runMotorClockwise();
        }

        haltMotor();
        Serial.println();
        Serial.print("Opening sequence of ");
        Serial.print((millis() - prevAlt) / 1000);
        Serial.print(" seconds done in ");
        Serial.print(totalRotations);
        Serial.println(" revolutions.");
        totalRotations = 0;
        prevAlt = millis();   
        delay(2000); //chill out for a sec

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
        totalRotations = 0; //reset count
        endTime += 10000; //add 10 secs for next sequence
    }
}