#include<WiFi.h>
#define DEVICE "ESP32"

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

const char ssid[]="ConnLab";
const char pwd[]="LCCYCH507";
#define INFLUXDB_URL "http://10.118.126.244:8086"
#define INFLUXDB_TOKEN "CvMuRWwO8ldZsQk-VspC_6yx6xZIWy1ObS6Hcl1QxnNycWozKl_HQ_JyXmd7yXvhOfgneL5jJoZgqYQy6sb0xg=="
#define INFLUXDB_ORG "lab"
#define INFLUXDB_BUCKET "ESP32"

#define TZ_INFO "PST8PDT"

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Data point
Point sensor("Energy_status");

#include <driver/adc.h>
#include "EmonLib.h"                  
EnergyMonitor emon1; 

hw_timer_t *timer = NULL;

void setup() {
 Serial.begin(115200);
 // Setup wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,pwd); 
  Serial.print("Connecting to wifi");
  while (WiFi.status()!=WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("");
  Serial.print("IP位址:");
  Serial.println(WiFi.localIP()); //讀取IP位址
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
  analogReadResolution(12);
  pinMode(34, INPUT);
  emon1.current(34, 0.5);

}

void loop(){
  timer = timerBegin(0, 80, true);
  timerWrite(timer, 1000000);
  int nowtime=timerReadSeconds(timer);
  while(nowtime!=60)
  {
    nowtime=timerReadSeconds(timer);
  }
  timerEnd(timer);
  float Irms = emon1.calcIrms(1676);
  sensor.clearFields();
  
  // Store measured value into point
  // Report RSSI of currently connected network
  sensor.addField("W", Irms*110.0);

  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(sensor.toLineProtocol());

  // If no Wifi signal, try to reconnect it
  if ((WiFi.RSSI() == 0) && (WiFi.status() != WL_CONNECTED)) {
    Serial.println("Wifi connection lost");
  }

  // Write point
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}
