#include <WiFi.h>
#include <WiFiClient.h>
#include <Arduino.h>

// WiFi credentials
const char* ssid = "Systems12_AP";
const char* password = "123456789";
IPAddress IP(192, 168, 1, 1); // Desired IP address for the AP
IPAddress Gateway(192, 168, 1, 1); // The IP address of the AP itself, often the same as the local IP
IPAddress Subnet(255, 255, 255, 0); // The subnet mask

// Set web server port number to 9090
WiFiServer server(9090);

WiFiClient client;

// Forward declarations
int send(String msg);
String receive();

void setup() {
  Serial.begin(115200);

  // Remove the password parameter, if you want the AP (Access Point) to be open
  if(WiFi.softAP(ssid, password)) {
    Serial.println("AP started successfully");
  } else {
    Serial.println("AP start failed!");
  }
  WiFi.softAPConfig(IP, Gateway, Subnet);

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  
  server.begin();
  Serial.println("Success");
}

void loop() {
    client = server.available();
    if(client) {
      Serial.println("Client made");
      receive(); //HELO
      send("OK");
      receive(); //AUTH
      send("OK");

      while (client.connected()) {
        receive();
        send("REDY");
        String msg = receive();

        if (msg == "STATRQ") {
          send("STAT . . . ."); //TODO Data
        }else if (msg == "EXEC") {
          send("OK");
          //TODO Execute
        }else if (msg == "QUIT") {
          send("QUIT");
          //Close Connection
          client.stop();
          Serial.println("Client disconnected.");
          Serial.println("");
        }
        
        delay(100);
      }
   }
}

int send(String msg) {
  if (client.connected()) {
    client.println(msg);
    Serial.print("Sent: ");
    Serial.println(msg);
    return 1;
  }
  return 0; // Not connected
}

String receive() { 
  String line = "";
  while(line.length() == 0) {
    line = client.readStringUntil('\n');
  }
  Serial.print("Received: ");
  Serial.println(line);
  return line;
}