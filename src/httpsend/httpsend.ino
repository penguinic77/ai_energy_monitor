#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "ConnLab";
const char* password = "LCCYCH507";
String serverPath = "http://10.118.126.244:5000/train";

void setup() {
  Serial.begin(115200); 

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}
void get_predict(){
  if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      http.begin(serverPath.c_str());
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    }
}

void loop() {
  get_predict();
  delay(2000);
}
