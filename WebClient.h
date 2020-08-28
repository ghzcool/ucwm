#include <ESP8266HTTPClient.h>

String serverName = "http://192.168.1.106:80/";
String payload = "";
int httpResponseCode;

String sendGetRequest(String url) {
  HTTPClient http;
  
  Serial.print("HTTP GET: ");
  Serial.println(url);
    
  http.begin(url.c_str());
  
  httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
    Serial.println(payload);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  
  http.end();
  if (httpResponseCode > 0) {
    return payload;
  } else {
    return String("") + httpResponseCode;
  }
}

void setLightLevel(int lampId, int level) {
  if (level > 0) {
    sendGetRequest(serverName + String("y") + lampId + String("v") + level);
  } else {
    sendGetRequest(serverName + String("n") + lampId);
  }
}
