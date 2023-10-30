// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3
#include <driver/adc.h>
#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emon1;                   // Create an instance

void setup()
{  
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
  
  analogReadResolution(12);
  pinMode(34, INPUT);
  Serial.begin(9600);
  
  emon1.current(34, 0.5);             // Current: input pin, calibration.
}

void loop()
{
  float Irms = emon1.calcIrms(1676);  // Calculate Irms only
  Serial.print(" analog: ");
  Serial.println(analogRead(34));  
  Serial.print("kWh: ");
  Serial.print(Irms*110.0);	       // Apparent power
  Serial.print(" current: ");
  Serial.println(Irms);		       // Irms
  delay(1000);
}
