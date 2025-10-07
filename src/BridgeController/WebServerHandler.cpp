#include "WebServerHandler.h"

void WebServerHandler::handleClient(WiFiClient& client, BridgeSystem& system) {
  if (!client) return;
  header = "";
  unsigned long timeout = millis();

  //Read request until new line (simple and works for small packets)
  while (client.connected() && millis() - timeout < 2000) {
    if (client.available()) {
      char c = client.read();
      header += c;
      if (c == '\n') {
        if (header.indexOf("/sensor") >= 0) {
          sendSensorData(client, system);
        } else if (header.indexOf("/device") >= 0) {
          sendDeviceData(client, system);
        } else {
          sendResponse(client, system);
        }
        break;
      }
    }
    delay(1);
  }
  client.flush();
  client.stop();
}

void WebServerHandler::sendResponse(WiFiClient& client, BridgeSystem& system) {
  //Response Header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();

  // Apply command
  system.execute(header);

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

  // Web Page Heading
  client.println("<body><h1>Bridge12 Controller</h1>");

  client.println("<h2>Mechanism Controls</h2>");
  renderFlipButton(client, system.override);

  if (system.override.isOn()) {
    renderTransFlipButton(client, system.mechanism, "mechanismState");
    renderFlipButton(client, system.alarms);
    renderTransFlipButton(client, system.gates, "gatesState");
    renderRadioButton(client, system.trafficLights);
    renderRadioButton(client, system.bridgeLights);
  } else {
    client.println("<h3>Sensor Readings</h3>");
    client.println("<div id='sensorData'>Loading...</div>");

    client.println("<script>");
    client.println("function updateSensors() {");
    client.println("    fetch('/sensor')");
    client.println("    .then(response => response.json())");
    client.println("    .then(data => {");
    client.println("        document.getElementById('sensorData').innerHTML = ");
    client.println("            'Ultrasonic_Front: ' + data.ultrasonic0 + ' <br>' +");
    client.println("            'Ultrasonic_Back: ' + data.ultrasonic1 + ' <br>' +");
    client.println("            'PIR: ' + (data.pir ? 'Motion Detected' : 'No Motion');");
    client.println("    });");
    client.println("}");
    client.println("setInterval(updateSensors, 1000);");
    client.println("updateSensors();");
    client.println("</script>");
  }

  client.println("</body>");
  client.println("</html>");
  client.println();
}

void WebServerHandler::sendSensorData(WiFiClient& client, BridgeSystem& system) {
  // Response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  int ultra0int = system.ultra0.getDistance();
  int ultra1int = system.ultra1.getDistance();

  String ultra0str = (-1 == ultra0int) ? "No Echo" : String(ultra0int)+" cm";
  String ultra1str = (-1 == ultra1int) ? "No Echo" : String(ultra1int)+" cm";

  // JSON with sensor values
  String json = "{";
  json += "\"ultrasonic0\":\"" + ultra0str + "\",";
  json += "\"ultrasonic1\":\"" + ultra1str + "\",";
  json += "\"pir\":" + String(system.pir.isTriggered() ? 1 : 0);
  json += "}";

  client.println(json);
}

void WebServerHandler::sendDeviceData(WiFiClient& client, BridgeSystem& system) {
  // Response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();

  // JSON with device states
  String json = "{";
  json += "\"gatesState\":\"" + system.gates.getState() + "\",";
  json += "\"mechanismState\":\"" + system.mechanism.getState() + "\"";
  json += "}";

  client.println(json);
}

void WebServerHandler::renderFlipButton(WiFiClient& client, BridgeDevice& device) {
  String name = device.getName();
  String action = device.getAction();

  client.print("<p>");
  client.print(name);
  client.print(" - State ");
  client.print(device.getState());
  client.println("<p>");

  client.print("<p><a href=\"/");
  client.print(name);
  client.print("/");
  client.print(action);
  client.print("?ts=");
  client.print(millis());
  client.print("\"><button class=\"button\">");
  client.print(action);
  client.println("</button></a></p>");
}

void WebServerHandler::renderTransFlipButton(WiFiClient& client, BridgeDevice& device, String jsonKey) {
  String name = device.getName();
  String state = device.getState();

  if (device.isWorking()) {
    client.print("<p>");
    client.print(name);
    client.println("</p>");

    // Show temporary Transitioning message
    client.print("<div id='");
    client.print(name);
    client.print("Control'><p>");
    client.print(state);
    client.println("...</p></div>");

    // Enable JS polling until state changes
    client.println("<script>");
    client.print("function check");
    client.print(name);
    client.println("() {");
    client.println("  fetch('/device').then(r => r.json()).then(data => {");
    client.print("    let s = data.");
    client.print(jsonKey);
    client.println(";");
    client.println("    if (s === 'Raised' || s === 'Lowered') {");
    client.print("      clearInterval(" + name + "Poll);");
    client.println("      window.location.href = '/';");
    client.println("    }");
    client.println("  });");
    client.println("}");
    client.print("let ");
    client.print(name);
    client.println("Poll = setInterval(check" + name + ", 1000);");
    client.println("</script>");
  } else {
    renderFlipButton(client, device);
  }
}

void WebServerHandler::renderRadioButton(WiFiClient& client, BridgeDevice& device) {
  const int statesCount = 3;
  String name = device.getName();
  int currState = device.getStateNum();

  client.print("<p>");
  client.print(name);
  client.println("</p>");

  for (int i = 0; i < statesCount; i++) {
    String stateName = device.getAction(i);
    String buttonClass = (i == currState) ? "button" : "button2";

    client.print("<a href=\"/");
    client.print(name);
    client.print("/");
    client.print(stateName);
    client.print("?ts=");
    client.print(millis());
    client.print("\"><button class=\"");
    client.print(buttonClass);
    client.print(" radio\">");
    client.print(stateName);
    client.println("</button></a>");
  }
  client.println("<br>");
}
