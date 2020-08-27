/* Universal Configurable Wireless Module (UCWM)
 * by Pavel Pirogov
 * Started 13.08.2020
 */
 String ver = "0.0.2";

#include <ESP8266WiFi.h>
#include "WebServer.h"
#include "Data.h"

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

// Variables to store ssid, pass, api_key
String wifi_ssid = "";
String wifi_pass = "";
String api_key = "";

// Web server request timeout variables
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 1000;

// Label variables
String labelConnected = "Connected";
String labelNotConnected = "Not connected";

// Get stored data
void loadInitData() {
  Serial.print("Loading data ... ");
  
  setupData();
  wifi_ssid = getData("wifi_ssid");
  wifi_pass = getData("wifi_pass");
  api_key = getData("api_key");
  if (!api_key.length()) {
    generateApiKey();
    setData("api_key", api_key);
  }
  
  Serial.println("done");
  Serial.print("wifi_ssid: ");
  Serial.println(wifi_ssid);
  Serial.println("");
}

void generateApiKey() {
  api_key = String("") + random(0, 2147483647) + random(0, 2147483647) + random(0, 2147483647);
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
  
  setData("wifi_ssid", wifi_ssid);
  setData("wifi_pass", wifi_pass);
  
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
  randomSeed(analogRead(0));
  Serial.print("Starting UCWM ");
  Serial.println(ver);
  Serial.println("");
  
  // Initialize the output variables as outputs
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);

  loadInitData();
  if(canConnectToWiFi()) {
    connectToWiFi();
  }
  startAP();
  startWebServer();
  
  Serial.print("Started");
  Serial.println("");
}

void handleWebRequest(String header, bool isAdmin) {
  if(isAdmin) {
    if (header.indexOf("GET /wconnect") >= 0) {
      int ssidIndex = header.indexOf("?wssid=");
      int passIndex = header.indexOf("&wpass=", ssidIndex);
      wifi_ssid = decodeUriComponent(header.substring(ssidIndex + 7, passIndex));
      wifi_pass = decodeUriComponent(header.substring(passIndex + 7, header.indexOf("&connect", passIndex + 7)));
      if(canConnectToWiFi()) {
        saveWiFi();
        connectToWiFi();
      }
    } else if (header.indexOf("GET /wdisconnect") >= 0) {
      disconnectFromWiFi();
    } else if (header.indexOf("GET /generateapikey") >= 0) {
      generateApiKey();
      setData("api_key", api_key);
    }
  } else if (header.indexOf(String("API_KEY: ") + api_key) != -1) {
    // TODO: protected by API_KEY header
  }
}

void handleWebClient(WiFiClient client, String header) {
  Serial.print("Handling request ... ");
  IPAddress softAPIP = WiFi.softAPIP();
  bool isAdmin = header.indexOf(String("Host: ") + softAPIP[0] + String(".") + softAPIP[1] + String(".") + softAPIP[2] + String(".")) != -1;
  handleWebRequest(header, isAdmin);
  Serial.println("done");

  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  
  Serial.print("Rendering page ... ");
  
  // Display the HTML web page
  client.print("<!DOCTYPE html><html>");
  client.print("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.print("<link rel=\"icon\" href=\"data:,\">");
  
  renderStyles(client);

  client.print("</head><body>");

  // Web Page Heading
  client.print("<h1>UCWM ");
  client.print(ver);
  client.print("</h1>");

  renderAPConnectionStatus(client, ssidAP, softAPIP);
  
  client.print("<div class='block'>");
  client.print("<p><b>WiFi status:</b> " + (isWiFiConnected() ? labelConnected : labelNotConnected) + "</p>");
  if(isWiFiConnected()) {
    renderWiFiConnectedForm(client, wifi_ssid, WiFi.localIP(), isAdmin);
  } else if (isAdmin) {
    renderWiFiConnectionForm(client);
  }
  client.print("</div>");

  renderConnectionStatus(client, webServerPort, isAdmin, api_key);
  
  client.println("</body></html>");
  
  // The HTTP response ends with another blank line
  client.println();
  
  Serial.println("done");
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {
  
  // If a new client connects,
    Serial.println("New HTTP Client.");     // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    String header;
    int headerLinesAmount = 0;
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            break;
          } else {
            headerLinesAmount++;
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    if(headerLinesAmount > 0) {
      handleWebClient(client, header);
    }
    header = "";
    // Close the connection
    client.stop();
    Serial.println("HTTP Client disconnected.");
    Serial.println("");
  }
}
