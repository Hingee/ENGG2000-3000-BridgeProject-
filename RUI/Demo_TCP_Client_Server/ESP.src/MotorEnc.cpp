//consider change of speeds
//see if position can be tracked??

//ESP motor systems with encoder RPM calculation
//ESP-WROOM

//Universal from other subsystems or ESP (TEMP)
bool BridgeState = true; //true closed, false open
bool ESPcall = true; //standin for a send from the ESP main
bool runningBridge = true;

//Motor specific
static int motorDriverPin1 = 27; 
static int motorDriverPin2 = 26; 
int duration = 5000; //1000 is a second
bool Direction = true; //true forward, false backwards

//Encoder
const int encoderPinA = 34;
volatile unsigned long pulseCount = 0;
const int pulsesPerRevolution = 374; //may need adjusting

void IRAM_ATTR onPulse() {
  pulseCount++; //count pulses
}

void MotorOpeningSequence(){ 
    Serial.println("Opening sequence (Forward)");
    unsigned long startTime = millis();
    while(millis() - startTime < duration) {
        digitalWrite(motorDriverPin1, LOW); 
        digitalWrite(motorDriverPin2, HIGH); 
        printRPM();
    }
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, LOW);
    BridgeState = false;
}

void MotorClosingSequence(){
    Serial.println("Closing sequence (Backwards)");
    unsigned long startTime = millis();
    while(millis() - startTime < duration) {
        digitalWrite(motorDriverPin1, HIGH);
        digitalWrite(motorDriverPin2, LOW);
        printRPM();
    }
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, LOW);
    BridgeState = true;
}

void printRPM() {
    //calculate and print RPM every second while motor runs
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
    //if explodes put this into the while loop
}

void tempTestCall() {
    ESPcall = !ESPcall;
    delay(2000);
}

void setup() {
    Serial.begin(9600); //9600 works for encoder printing
    pinMode(motorDriverPin1, OUTPUT);
    pinMode(motorDriverPin2, OUTPUT);
    digitalWrite(motorDriverPin1, LOW);
    digitalWrite(motorDriverPin2, LOW);

    pinMode(encoderPinA, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(encoderPinA), onPulse, RISING);
}

void loop() {
    if(ESPcall) {
        if(BridgeState) {
            MotorOpeningSequence();
        } else {
            MotorClosingSequence();
        }
    }
    tempTestCall();
}