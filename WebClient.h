#include <ESP8266HTTPClient.h>

String serverName = "http://10.100.1.252/";
String payload = "";
int httpResponseCode;
String res = "";

String sendGetRequest(String url) {
  HTTPClient http;
  
  Serial.print("HTTP GET: ");
  Serial.println(url);
    
  http.begin(url.c_str());
  
  httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  
  http.end();
  if (httpResponseCode > 0) {
    return String("ok");
  } else {
    return String("") + httpResponseCode;
  }
}

void setLightLevel(int lampId, int level) {
  if (level > 0) {
    res = sendGetRequest(serverName + String("y") + lampId + String("v") + level);
  } else {
    res = sendGetRequest(serverName + String("x") + lampId);
  }
}
