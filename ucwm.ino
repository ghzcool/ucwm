/* Universal Configurable Wireless Module (UCWM)
 * by Pavel Pirogov
 * Started 13.08.2020
 */
 String ver = "0.0.1";

#include <ESP8266WiFi.h>
#include <FS.h>
#include "WebServer.h"

// WiFi AP Config
IPAddress local_IP(192,168,0,1);
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);
String ssidAP = "UCWM_AP";
String passAP = "";

// WiFi Config
int connectionAttempts = 10;
int connectionAttemptDelay = 1000;
int connectionAttempt = 0;

// Web server port
int webServerPort = 80;
WiFiServer server(webServerPort);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String outputLedState = "off";

// Variables to store ssid, pass and file for SPIFFS
String wifi_ssid = "";
String wifi_pass = "";
File file;

// Web server request timeout variables
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// Label variables
String labelConnected = "Connected";
String labelNotConnected = "Not connected";

// Get stored data
void loadData() {
  Serial.print("Loading SPIFFS data ... ");
  SPIFFS.begin();
  if(SPIFFS.exists("wifi_ssid")) {
    file = SPIFFS.open("wifi_ssid", "r");
    wifi_ssid = "";
    while (file.available()) {
      wifi_ssid += (char)file.read();
    }
    file.close(); 
  }
  if(SPIFFS.exists("wifi_pass")) {
    file = SPIFFS.open("wifi_pass", "r");
    wifi_pass = "";
    while (file.available()) {
      wifi_pass += (char)file.read();
    }
    file.close(); 
  }
  Serial.println("done");
  Serial.print("wifi_ssid: ");
  Serial.println(wifi_ssid);
  Serial.println("");
}

// Start WiFi AP
void startAP() {
  Serial.print("Configuring WiFi AP ... ");
  if (WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println("done");
  } else {
    Serial.println("failed");
  }
  Serial.print("Starting WiFi AP ... ");
  if(WiFi.softAP(ssidAP, passAP)) {
    Serial.println("done");
    Serial.print("WiFi AP IP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("failed");
  }
  Serial.println("");
}

// Connect to Wi-Fi network with SSID and password
void connectToWiFi() {
  Serial.print("Connecting to WiFi .");
  WiFi.begin(wifi_ssid, wifi_pass);
  connectionAttempt = 0;
  while (!isWiFiConnected() && connectionAttempt < connectionAttempts) {
    delay(connectionAttemptDelay);
    connectionAttempt ++;
    Serial.print(".");
  }
  if(!isWiFiConnected()) {
    WiFi.disconnect();
    Serial.println(" failed");
  } else {
    Serial.println(" done");
    Serial.print("WiFi IP: ");
    Serial.println(WiFi.localIP());
  }
  Serial.println("");
}

bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

bool canConnectToWiFi() {
  return wifi_ssid.length() > 0 && wifi_pass.length() > 0;
}

// Start Web server
void startWebServer() {
  Serial.print("Starting web server ... ");
  
  server.begin();
  
  Serial.println("done");
  Serial.print("Listening port ");
  Serial.println(webServerPort);
  Serial.println("");
}

void setup() {
  Serial.begin(9600);
  Serial.print("Starting UCWM ");
  Serial.println(ver);
  Serial.println("");
  
  // Initialize the output variables as outputs
  pinMode(LED_BUILTIN, OUTPUT);
  // Set outputs to LOW
  digitalWrite(LED_BUILTIN, LOW);

  loadData();
  if(canConnectToWiFi()) {
    connectToWiFi();
  }
  startAP();
  startWebServer();
  
  Serial.print("Started");
  Serial.println("");
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {
  
  // If a new client connects,
    Serial.println("New HTTP Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /led/on") >= 0) {
              Serial.println("GPIO 4 on");
              outputLedState = "on";
              digitalWrite(LED_BUILTIN, LOW);
            } else if (header.indexOf("GET /led/off") >= 0) {
              Serial.println("GPIO 4 off");
              outputLedState = "off";
              digitalWrite(LED_BUILTIN, HIGH);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto;}");
            client.println("h1 {text-align: center; margin: 16px 0;}");
            client.println("p {margin: 0; line-height: 32px;}");
            client.println(".block { width: 280px; margin: 0 auto 10px auto; padding: 5px 10px; border: 1px solid #c0c0c0; background-color: #fafafa;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");

            
            // Web Page Heading
            client.print("<body><h1>UCWM ");
            client.print(ver);
            client.println("</h1>");
            
renderWebPage(client);

            // Display connection status
            client.println("<div class='block'>");
            client.println("<p><b>WiFi Access Point</b></p>");
            client.println("<p>SSID: " + ssidAP + "</p>");
            client.print("<p>IP: ");
            client.print(WiFi.softAPIP());
            client.println("</p>");
            client.println("</div>");
            
            client.println("<div class='block'>");
            client.println("<p><b>WiFi status:</b> " + (isWiFiConnected() ? labelConnected : labelNotConnected) + "</p>");
            if(isWiFiConnected) {
              client.println("<p>SSID: " + wifi_ssid + "</p>");
              client.print("<p>IP: ");
              client.print(WiFi.localIP());
              client.println("</p>");
            }
            client.println("</div>");
            
            client.println("<div class='block'>");
            client.println("<p><b>Server</b></p>");
            client.print("<p>HTTP port: ");
            client.print(webServerPort);
            client.println("</p>");
            client.println("<p>WebSocket port: not implemented</p>");
            client.println("</div>");
               
            // Display current state, and ON/OFF buttons for LED 
            client.println("<p>LED - State " + outputLedState + "</p>");
            // If the output4State is off, it displays the ON button       
            if (outputLedState=="off") {
              client.println("<p><a href=\"/led/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/led/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("HTTP Client disconnected.");
    Serial.println("");

    file = SPIFFS.open("wifi_ssid", "w+");
    file.write("ssidname");
    file.close();
    
    file = SPIFFS.open("wifi_pass", "w+");
    file.write("password");
    file.close();
  }
}
