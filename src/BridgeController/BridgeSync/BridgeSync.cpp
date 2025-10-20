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
const int encoderPinA = 34;
volatile unsigned long pulseCount = 0;
const int pulsesPerRevolution = 700;

float globalSend = 0.0;
float reverseRevolutions = 0.0;
static unsigned long prev = 0;

void IRAM_ATTR onPulse() {
    pulseCount++;
}

void setup() {
    Serial.begin(115200); //originally bud9600
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

void syncCheck() {
    long prevAlt = 0;
    int endTime = 5000;

    //infinite loop
    while(true) {
        prevAlt = millis();
        while(millis() - prevAlt <= endTime) {
            runMotorClockwise();
            delay(1);
        }

        float forwardTotal = totalRotations;
        haltMotor();
        Serial.println();
        Serial.print("Opening sequence of ");
        Serial.print((millis() - prevAlt) / 1000);
        Serial.print(" seconds done in ");
        Serial.print(forwardTotal);
        Serial.println(" revolutions.");
        totalRotations = 0;
        prevAlt = millis();   
        delay(2000);

        Serial.println("");
        Serial.println("Lowering Bridge back to neutral state.");
        Serial.println("Check for consistency alignment.");

      //  float reversTYRevolutions = 0.0;
        float globalSend = 0.0;
        while(globalSend <= forwardTotal) {
            runMotorAntiClockwise();
            delay(1);
        }

        Serial.print("Bridge closed at position ");
        Serial.println(reverseRevolutions);

     //   reverseRevolutions = 0.0;
        haltMotor();
        endTime += 5000; //add 5 secs for next sequence
    }
}

void runMotorClockwise() {
    digitalWrite(motorDriverPin1, HIGH);
    digitalWrite(motorDriverPin2, LOW);

    if(millis() - prev >= 1000) {
        noInterrupts();
        unsigned long pulses = pulseCount;
        pulseCount = 0;
        interrupts();
        float revolutions = (float) pulses / pulsesPerRevolution;
        float rpm = revolutions * 60.0;
        totalRotations += revolutions; 
        Serial.print("Live RPM: ");
        Serial.print(rpm, 2);
        Serial.print(" | Revs Since Last Pulse: ");
        Serial.print(revolutions, 2);
        Serial.print(" | Total Revs: ");
        Serial.println(totalRotations, 2);
        prev = millis();  
    }
}

void runMotorAntiClockwise() {
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, HIGH);

    if(millis() - prev >= 1000) {
        noInterrupts();
        unsigned long pulses = pulseCount;
        pulseCount = 0;
        interrupts();
        reverseRevolutions = (float) pulses / pulsesPerRevolution;
        globalSend = 0.0;
    //    float revolutions = (float) pulses / pulsesPerRevolution;
        float rpm = reverseRevolutions * 60.0;
        totalRotations += reverseRevolutions; 
        Serial.print("Live RPM: ");
        Serial.print(rpm, 2);
        Serial.print(" | Revs Since Last Pulse: ");
        Serial.println(reverseRevolutions, 2);
        prev = millis(); 
        globalSend = reverseRevolutions; 
    }
}

void haltMotor() {
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, LOW);  
}
