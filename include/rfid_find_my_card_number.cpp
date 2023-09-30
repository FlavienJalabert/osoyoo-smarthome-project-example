#include <SPI.h>
#include <RFID.h>
RFID rfid(48, 49); // D48--RFID module SDA pin & RFID module RST pin

void setup()
{
  Serial.begin(9600);
  SPI.begin();
  rfid.init();
}

void loop()
{
  // search card
  if (rfid.isCard())
  {
    Serial.println("Find the card!");
    // read serial number
    if (rfid.readCardSerial())
    {
      Serial.print("The card's number is  : ");
      Serial.print(rfid.serNum[0]);
      Serial.print(",");
      Serial.print(rfid.serNum[1]);
      Serial.print(",");
      Serial.print(rfid.serNum[2]);
      Serial.print(",");
      Serial.print(rfid.serNum[3]);
      Serial.print(",");
      Serial.print(rfid.serNum[4]);
      Serial.println(" ");
    }
    rfid.selectTag(rfid.serNum);
  }
  rfid.halt();
}