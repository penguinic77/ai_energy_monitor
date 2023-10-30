#include <LiquidCrystal_I2C.h> 
#include <stdlib.h>
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup()
{
  Serial.begin(115200);
  lcd.init(); // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);

  char buff[20]="";
  String Energy_now= "Power:";
  float kwh =20.35;
  dtostrf(kwh, 4, 2, buff);
  Energy_now +=buff;
  Energy_now +="W";
  Serial.print("Print:");
  Serial.println(Energy_now);
  lcd.print(Energy_now);
}

void loop()
{
}
