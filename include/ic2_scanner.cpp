/* ___ ___ ___ _ _ ___ ___ ____ ___ ____
 * / _ /___)/ _ | | | |/ _ / _ / ___) _ |
 *| |_| |___ | |_| | |_| | |_| | |_| ( (__| |_| | | | |
 * ___/(___/ ___/ __ |___/ ___(_)____)___/|_|_|_|
 * (____/
 *In this sketch, we will show how to find the I2C address for the I2C device.
 * Tutorial URL https://osoyoo.com/2017/07/09/arduino-lesson-i2c-lcd1602-display/
 * Copyright (c) 2023 http://www.osoyoo.com
 */
#include <Arduino.h>
#include <Wire.h>

void setup()
{
  Wire.begin();

  Serial.begin(9600);
  Serial.println("nI2C Scanner");
}

void loop()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning…");

  nDevices = 0;
  for (address = 1; address < 127; address++)
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");

      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices foundn");
  else
    Serial.println("donen");

  delay(5000); // wait 5 seconds for next scan
}