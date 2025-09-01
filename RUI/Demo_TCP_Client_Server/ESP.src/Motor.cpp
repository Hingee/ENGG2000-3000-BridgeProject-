
//ESP motor systems

  //Universal from other subsystems or ESP (TEMP)
  bool BridgeState = true; //true closed, false open
  bool ESPcall = true; //standin for a send from the ESP main

  //Motor specific
  static int motor1Pin1 = 27; 
  static int motor1Pin2 = 26; 
  static int enable1Pin = 14; 
  int duration = 5000; //1000 is a second
  bool Direction = true; //true forward, false backwards

  //Note when this is running like this it will take priority on the ESP code,
  //if other functions need to be run during the open/close sequence they could be integrated into the loop
  //e.g. safety-check or signalling LEDs.

void MotorOpeningSequence(){ 
    Serial.println("Opening sequence (Forward)");
    int startTime = millis();
    while(millis() - startTime < duration) {
        digitalWrite(motor1Pin1, LOW); //p1 low and p2 high is forward
        digitalWrite(motor1Pin2, HIGH); 
    }
    digitalWrite(motor1Pin1, LOW); //stops the motor after it has run
    digitalWrite(motor1Pin2, LOW);
    BridgeState = false;
}

void MotorClosingSequence(){
    Serial.println("Closing sequence (Backwards)");
    int startTime = millis();
    while(millis() - startTime < duration) {
        digitalWrite(motor1Pin1, HIGH); //p1 high and p2 low is backward
        digitalWrite(motor1Pin2, LOW); 
    }
    digitalWrite(motor1Pin1, LOW); //stops the motor after it has run
    digitalWrite(motor1Pin2, LOW);
    BridgeState = true;
}

void tempTestCall() {
    ESPcall = !ESPcall;
    delay(2000);
}

//consider an emergency stop sequence

void setup() { //these setup pins should be established in the main when developed.
    Serial.begin(115200); //get serial
    pinMode(motor1Pin1, OUTPUT);
    pinMode(motor1Pin2, OUTPUT);
    pinMode(enable1Pin, OUTPUT);
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
