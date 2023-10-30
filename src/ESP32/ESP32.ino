/*************************************************************
                        Energy monitor
 *************************************************************/
#define DEVICE "ESP32"
#include <WiFi.h>
#include <HTTPClient.h>
const char* ssid = "ESP32";
const char* password = "12345678";

/*influxdb相關設定*/
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#define INFLUXDB_URL "http://10.118.126.244:8086"
#define INFLUXDB_TOKEN "" //填入token
#define INFLUXDB_ORG "lab"
#define INFLUXDB_BUCKET "ESP32"
#define TZ_INFO "PST8PDT"
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
Point sensor("Energy_status");

/*blynk相關設定*/
#define BLYNK_TEMPLATE_ID           " "
#define BLYNK_DEVICE_NAME           " "
#define BLYNK_AUTH_TOKEN            " "
// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
char auth[] = BLYNK_AUTH_TOKEN;
BlynkTimer timer;

/*電流感測相關設定*/
#include "ACS712.h"
#include <driver/adc.h>
ACS712  ACS(34, 3.3, 4095, 100);
/*電壓感測相關設定*/
#include "EmonLib.h"
#define vCalibration 83.3
EnergyMonitor emon;
/*LCD相關設定*/
#include <LiquidCrystal_I2C.h> 
#include <stdlib.h>
LiquidCrystal_I2C lcd(0x27,16,2);

int timer_power;
int timer_prediction;

//電源開關按鈕
BLYNK_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable
  int value = param.asInt();
  // Update state
  power_switch(value);
}
//執行預測按鈕
BLYNK_WRITE(V2)
{
  int value = param.asInt();
  if (value)
  {
    get_predict();
  }
}
// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

int count=0;
float watt=0;
float watt_average=0;
void check_power()
{
  count++;
  float average = 0;
  for (int i = 0; i < 100; i++)
  {
    average += ACS.mA_AC();
  }
  float mA = average / 100.0;
  watt=mA*110/1000;
  watt_average+=watt;
  if (count ==60)
  {
    watt_average=watt_average/60;
    upload_power();//一分鐘時上傳
    watt_average=0;//上傳完後清空
    count=0;
  }
  Serial.print("mA: ");
  Serial.println(mA);
  Serial.print("watt: ");
  Serial.println(watt);
  Blynk.virtualWrite(V3, watt);
}

void upload_power()
{
  sensor.clearFields();
  sensor.addField("W", watt_average);
  Serial.print("寫入資料: ");
  Serial.println(sensor.toLineProtocol());
  //開始上傳資料到資料庫
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}
//取得資料庫中的預測值
double pvalue;
double pvalue_kWh;
void check_prediction()
{
 String query = "from(bucket: \"ESP32\") \
    |> range(start: -30d) \
    |> filter(fn: (r) => r._measurement == \"Energy_status\" and r._field == \"prediction\") \
    |> last()";
 FluxQueryResult result = client.query(query);//start query
 Serial.print("Querying with: ");
 Serial.println(query);
 while(result.next())
 {
  pvalue = result.getValueByName("_value").getDouble();
  Serial.print("prediction:");
  Serial.println(pvalue);
 }
 //檢查是否有錯誤
 if(result.getError() != "") {
    Serial.print("Query result error: ");
    Serial.println(result.getError());
  }
 result.close();
 if(pvalue>2000)//超過閥值發送警報
 {Blynk.logEvent("alert", "警告!目前預估用電可能超過設定使用量，請注意電器狀況!!!");}
 pvalue_kWh=pvalue/60000;
 Blynk.virtualWrite(V1, pvalue_perhour);
 money_cost();
 timer.disable(timer_prediction);
}
//向server發送預測指令
void get_predict(){
  String serverPath = "http://10.118.126.244:5000/train";
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
//relay switch
int pflag=0;
void power_switch(int state)
{
  pflag=state;
  if(state)
  {digitalWrite(19,HIGH);}
  else
  {digitalWrite(19,LOW);}
}
void money_cost()
{
  double cost = 0;
  cost= pvalue/60000*1.63;//轉成度數並依台電計價
  Blynk.virtualWrite(V4, cost);
}
void LCD_print()
{
  char ebuff[20]="";
  char pbuff[20]="";
  String Energy_now= "Power:";
  dtostrf(watt, 4, 2, ebuff);
  Energy_now +=ebuff;
  Energy_now +="W";
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(Energy_now);
  String Predict_now= "Pred.:";
  dtostrf(pvalue_kWh, 4, 2, pbuff);
  Predict_now +=pbuff;
  Predict_now +="W";
  lcd.setCursor(0,1);
  lcd.print(Predict_now);
}

void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, password);
  pinMode(19, OUTPUT);//Relay pin
  /*電流檢測初始設定*/
  ACS.autoMidPoint();
  ACS.setmVperAmp(50);
  //ACS.setFormFactor(1.3);
  /*lCD初始設定*/
  lcd.init();
  lcd.backlight();
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  //檢查是否可連上InfluxDB
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  // Setup a function to be called every second
  timer.setInterval(2000L, LCD_print);//每2秒更新螢幕
  timer.setInterval(3600000L, get_predict);//每1小時自動一次執行預測
  timer_power=timer.setInterval(1000L, check_power);//每1秒檢測消耗
  timer_prediction=timer.setInterval(5000L, check_prediction);//每5秒檢測預測值
}
bool timer_flag=false;
void loop()
{
  Blynk.run();
  //開關沒打開消耗一律為0，關閉上傳與預測功能
  if(!pflag)
  {
    watt=0;
    pvalue=0;
    timer.disable(timer_power);
    timer.disable(timer_prediction);
    timer_flag=false;
  }
  else if(pflag && !timer_flag)
  {
    timer.enable(timer_power);
    timer_flag=true;
  }
  timer.run();
}
