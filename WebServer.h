
void renderWiFiConnectionForm (WiFiClient client) {
  client.println("<form method='GET' name='wifiConnect' action='/wconnect'>");
  client.println("<label style='display: block;'>SSID:</label>");
  client.println("<input type='text' name='wssid' style='display: block; width: 100%;' />");
  client.println("<label style='display: block;'>Password:</label>");
  client.println("<input type='text' name='wpass' style='display: block; width: 100%;' />");
  client.println("<button type='submit' name='connect' style='display: inline-block; width: 50%;'>Connect</button>");
  client.println("</form>");
}

void renderWiFiConnectedForm (WiFiClient client, String wifi_ssid, IPAddress ip) {
  client.println("<form method='GET' name='wifiDisconnect' action='/wdisconnect'>");
  client.println("<p>SSID: " + wifi_ssid + "</p>");
  client.print("<p>IP: ");
  client.print(ip);
  client.println("</p>");
  client.println("<button type='submit' name='disconnect' style='display: inline-block; width: 50%;'>Disconnect</button>");
  client.println("</form>");
}

void renderAPConnectionStatus (WiFiClient client, String ssidAP, IPAddress softAPIP) {
  client.println("<div class='block'>");
  client.println("<p><b>WiFi Access Point</b></p>");
  client.println("<p>SSID: " + ssidAP + "</p>");
  client.print("<p>IP: ");
  client.print(softAPIP);
  client.println("</p>");
  client.println("</div>");
}

void renderConnectionStatus (WiFiClient client, int webServerPort) {
  client.println("<div class='block'>");
  client.println("<p><b>Server</b></p>");
  client.print("<p>HTTP port: ");
  client.print(webServerPort);
  client.println("</p>");
  client.println("<p>WebSocket port: not implemented</p>");
  client.println("</div>");
}

void renderStyles (WiFiClient client) {
  client.println("<style>");
  client.println("html { font-family: Helvetica; display: inline-block; margin: 0px auto;}");
  client.println("h1 {text-align: center; margin: 16px 0;}");
  client.println("p {margin: 0; line-height: 32px;}");
  client.println("input, button {margin: 4px 0; line-height: 24px; height: 24px;}");
  client.println(".block { width: 280px; margin: 0 auto 10px auto; padding: 5px 10px; border: 1px solid #c0c0c0; background-color: #fafafa;}");
  client.println("</style>");
}
