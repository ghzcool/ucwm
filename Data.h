#include <FS.h>

File file;

void setupData() {
  SPIFFS.begin();
}

// Get stored data
String getData(String name) {
  String data = "";
  if(SPIFFS.exists(name)) {
    file = SPIFFS.open(name, "r");
    while (file.available()) {
      data += (char)file.read();
    }
    file.close(); 
  }
  return data;
}

// Set stored data
void setData(String name, String data) {
  file = SPIFFS.open(name, "w+");
  file.write(data.c_str());
  file.close();
}
