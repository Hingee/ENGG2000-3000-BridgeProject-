#include "WebServerHandler.h"

void WebServerHandler::handleClient(WiFiClient& client, BridgeSystem& system) {
  if (!client) return;

  header = "";
  unsigned long timeout = millis();

  //Read request until new line
  //WARNING: ONLY WORKS FOR SMALL PACKETS but makes it way faster
  while (client.connected() && millis() - timeout < 2000) {
    if (client.available()) {
      char c = client.read();
      header += c;
      if (c == '\n') {
        if (header.indexOf("/sensor") >= 0) sendSensorData(client, system);
        else if (header.indexOf("/device") >= 0) sendDeviceData(client, system);
        else sendResponse(client, system);
        break;
      }
    }
    delay(1);
  }
  client.flush();
  client.stop();
}

// ------------------------------ Responses ------------------------------
void WebServerHandler::sendResponse(WiFiClient& client, BridgeSystem& system) {
  system.execute(header);  //Execute command
  sendHTMLHeader(client);

  // Web Page Heading
  client.println("<body><h1>Bridge12 Controller</h1>");
  client.println("<h2>Mechanism Controls</h2>");
  renderFlipButton(client, system.override);

  if (system.override.isOn()) {
    renderTransFlipButton(client, system.mechanism, "mechanismState");
    renderFlipButton(client, system.alarms);
    renderTransFlipButton(client, system.gates, "gatesState");
    renderRadioButton(client, system.pedestrianLights);
    renderRadioButton(client, system.boatLights);
  } else {
    renderSensorSection(client);
  }
  client.println("</body></html>\n");
}

// ------------------------------ JSON DATA PACKETS  ------------------------------
void WebServerHandler::sendSensorData(WiFiClient& client, BridgeSystem& system) {
  sendJSONHeader(client);

  int ultraFint = system.ultraF.getDistance();
  int ultraBint = system.ultraB.getDistance();

  String ultraFstr = (-1 == ultraFint) ? "No Echo" : String(ultraFint) + " cm";
  String ultraBstr = (-1 == ultraBint) ? "No Echo" : String(ultraBint) + " cm";

  // JSON with sensor values
  String json = "{";
  json += "\"ultrasonicF\":\"" + ultraFstr + "\",";
  json += "\"ultrasonicB\":\"" + ultraBstr + "\",";
  json += "\"pir\":" + String(system.pir.isTriggered() ? 1 : 0);
  json += "}";

  client.println(json);
}

void WebServerHandler::sendDeviceData(WiFiClient& client, BridgeSystem& system) {
  sendJSONHeader(client);

  // JSON with device states
  String json = "{";
  json += "\"gatesState\":\"" + system.gates.getState() + "\",";
  json += "\"mechanismState\":\"" + system.mechanism.getState() + "\"";
  json += "}";

  client.println(json);
}

// ------------------------------ Device Controls  ------------------------------
void WebServerHandler::renderFlipButton(WiFiClient& client, BridgeDevice& device) {
  String name = device.getName();
  String action = device.getAction();

  client.printf("<p>%s - State %s</p>\n", name.c_str(), device.getState().c_str());
  client.printf("<p><a href=\"/%s/%s?ts=%lu\"><button class=\"button\">%s</button></a></p>\n",
                name.c_str(), action.c_str(), millis(), action.c_str());
}

void WebServerHandler::renderTransFlipButton(WiFiClient& client, BridgeDevice& device, String jsonKey) {
  if (!device.isWorking()) {
    renderFlipButton(client, device);
    return;
  }

  String name = device.getName();
  String state = device.getState();

  client.printf("<p>%s</p>\n", name.c_str());

  // Show temporary Transitioning message
  client.printf("<div id='%s Control'><p>%s...</p></div>\n", name.c_str(), state.c_str());

  // Enable JS polling until state changes
  client.println("<script>");
  client.printf("function check%s() {\n", name.c_str());
  client.println("  fetch('/device').then(r => r.json()).then(data => {");
  client.printf("    let s = data.%s;\n", jsonKey.c_str());
  client.println("    if (s === 'Raised' || s === 'Lowered') {");
  client.printf("      clearInterval(%sPoll);\n", name.c_str());
  client.println("      window.location.href = '/';");
  client.println("    }");
  client.println("  });");
  client.println("}");
  client.printf("let %sPoll = setInterval(check%s, 1000);\n", name.c_str(), name.c_str());
  client.println("</script>");
}

void WebServerHandler::renderRadioButton(WiFiClient& client, BridgeDevice& device) {
  const int statesCount = device.getNumStates();
  String name = device.getName();
  int currState = device.getStateNum();

  //TODO change to printf
  client.printf("<p>%s</p>\n", name.c_str());

  for (int i = 0; i < statesCount; i++) {
    String stateName = device.getAction(i);
    String buttonClass = (i == currState) ? "button" : "button2";

    client.printf("<a href=\"/%s/%s?ts=%lu\"><button class=\"%s radio\">%s</button></a>\n",
                  name.c_str(), stateName.c_str(), millis(), buttonClass.c_str(), stateName.c_str());
  }
  client.println("<br>");
}

// ------------------------------ Helper Functions ------------------------------
void WebServerHandler::sendHTMLHeader(WiFiClient& client) {
  //Response Header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();

  // HTML page
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<link rel=\"icon\" href=\"data:,\">");

  //CSS to Style Buttons
  client.println("<style>");
  client.println("html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align:center; width: 100vw; height: 100vh; overflow: hidden; }");
  client.println("h1 { font-size: 1.2em; margin: 5px 0; }");
  client.println("h2, h3 { font-size: 1em; margin: 3px 0; }");
  client.println("p { font-size: 1em; margin: 2px 0; }");
  client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 8px 15px;");
  client.println("text-decoration: none; font-size: 12px; margin: 2px; cursor: pointer; border-radius: 5px; }");
  client.println(".button2 {background-color: #555555;}");
  client.println("#sensorData { font-size: 0.9em; margin-top: 5px; }");
  client.println("</style></head>");
}
void WebServerHandler::renderSensorSection(WiFiClient& client) {
  client.println("<h3>Sensor Readings</h3>");
  client.println("<div id='sensorData'>Loading...</div>");

  //JavaScript Sensor display
  client.println("<script>");
  client.println("function updateSensors() {");
  client.println("    fetch('/sensor')");
  client.println("    .then(response => response.json())");
  client.println("    .then(data => {");
  client.println("        document.getElementById('sensorData').innerHTML = ");
  client.println("            'Ultrasonic_Front: ' + data.ultrasonicF + ' <br>' +");
  client.println("            'Ultrasonic_Back: ' + data.ultrasonicB + ' <br>' +");
  client.println("            'PIR: ' + (data.pir ? 'Motion Detected' : 'No Motion');");
  client.println("    });");
  client.println("}");
  client.println("setInterval(updateSensors, 500);");
  client.println("updateSensors();");
  client.println("</script>");
}
void WebServerHandler::sendJSONHeader(WiFiClient& client) {
  // Response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
}
