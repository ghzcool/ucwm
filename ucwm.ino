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
bool shouldRedirect = false;

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

void disconnectFromWiFi() {
  Serial.print("Disconnecting ... ");
  wifi_ssid = "";
  wifi_pass = "";
  saveWiFi();
  WiFi.disconnect();
  Serial.println("done");
  Serial.println("");
}

void saveWiFi() {
  Serial.print("Saving WiFi ... ");
  
  file = SPIFFS.open("wifi_ssid", "w+");
  file.write(wifi_ssid.c_str());
  file.close();
  file = SPIFFS.open("wifi_pass", "w+");
  file.write(wifi_pass.c_str());
  file.close();
  
  Serial.println("done");
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
  digitalWrite(LED_BUILTIN, 0);

  loadData();
  if(canConnectToWiFi()) {
    connectToWiFi();
  }
  startAP();
  startWebServer();
  
  Serial.print("Started");
  Serial.println("");
}

bool handleWebRequest(String header) {
  shouldRedirect = false;
  if (header.indexOf("GET /wconnect") >= 0) {
    int ssidIndex = header.indexOf("?wssid=");
    int passIndex = header.indexOf("&wpass=", ssidIndex);
    wifi_ssid = header.substring(ssidIndex + 7, passIndex);
    wifi_pass = header.substring(passIndex + 7, header.indexOf("&connect", passIndex + 7));
    if(canConnectToWiFi()) {
      saveWiFi();
      connectToWiFi();
    }
    shouldRedirect = true;
  } else if (header.indexOf("GET /wdisconnect") >= 0) {
    disconnectFromWiFi();
    shouldRedirect = true;
  }
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

            handleWebRequest(header);
            
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            if (shouldRedirect) {
              client.println("<script>window.location='/';</script>");
            }
            
            renderStyles(client);
            
            // Web Page Heading
            client.print("<body><h1>UCWM ");
            client.print(ver);
            client.println("</h1>");

            renderAPConnectionStatus(client, ssidAP, WiFi.softAPIP());
            
            client.println("<div class='block'>");
            client.println("<p><b>WiFi status:</b> " + (isWiFiConnected() ? labelConnected : labelNotConnected) + "</p>");
            if(isWiFiConnected()) {
              renderWiFiConnectedForm(client, wifi_ssid, WiFi.localIP());
            } else {
              renderWiFiConnectionForm(client);
            }
            client.println("</div>");

            renderConnectionStatus(client, webServerPort);
            
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
  }
}
