
//ESP motor systems

  //Universal from other subsystems or ESP (TEMP)
  bool BridgeState = true; //true closed, false open
  bool ESPcall = true; //temp for call from the ESP, replace with its method when developed
  
  //Motor specific
  static int motor1Pin1 = 27; 
  static int motor1Pin2 = 26; 
  static int enable1Pin = 14; 
  int duration = 5000; //how long the motor needs to run to open/close bridge
  bool Direction = true; //true forward, false backwards
  Serial.begin(115200); //get serial


void MotorOpeningSequence(){ 
    Serial.println("Opening sequence (Forward)");
    int tempDuration = duration;
    while(tempDuration>0) {
        digitalWrite(motor1Pin1, LOW); //p1 low and p2 high is forward
        digitalWrite(motor1Pin2, HIGH); 
        tempDuration--; }
    digitalWrite(motor1Pin1, LOW); //stops the motor after it has run
    digitalWrite(motor1Pin2, LOW);
}

void MotorClosingSequence(){
    Serial.println("Closing sequence (Backwards)");
    int tempDuration = duration;
    while(tempDuration>0) {
        digitalWrite(motor1Pin1, HIGH); //p1 high and p2 low is backward
        digitalWrite(motor1Pin2, LOW); 
        tempDuration--; }
    digitalWrite(motor1Pin1, LOW); //stops the motor after it has run
    digitalWrite(motor1Pin2, LOW);
}

//consider an emergency stop sequence

void setup() { //these setup pins should be established in the main when developed.
    pinMode(motor1Pin1, OUTPUT);
    pinMode(motor1Pin2, OUTPUT);
//    pinMode(enable1Pin, OUTPUT);
}

void loop() {
    if(ESPcall==true) {
        if (BridgeState==true) {
            MotorOpeningSequence();
        } else {
            MotorClosingSequence();
        }
    }
    //delay(2000);
}
