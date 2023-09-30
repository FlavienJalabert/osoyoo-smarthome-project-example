#include <Arduino.h>
// #ifndef HAVE_HWSERIAL1
#include <SoftwareSerial.h>
SoftwareSerial softserial(A9, A8); // A9 to ESP_TX, A8 to ESP_RX by default
// #endif

#define RED_LED 4
#define BUZZER 6
#define RED_PUSH_BTN 11
#define MOTOR A2
#define redPin 24   // R connected to digital pin 24
#define greenPin 23 // G connected to digital pin 23
#define bluePin 22  // B connected to digital pin 22

void color(unsigned char red, unsigned char green, unsigned char blue) // the color generating function
{
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

void setup()
{
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RED_PUSH_BTN, INPUT);
  pinMode(MOTOR, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT); // initialize digital pin LED_BUILTIN as an output.
  pinMode(redPin, OUTPUT);      // set the redPin to be an output
  pinMode(greenPin, OUTPUT);    // set the greenPin to be an output
  pinMode(bluePin, OUTPUT);     // set the bluePin to be an output
  Serial.begin(9600);           // initialize serial for debugging
  softserial.begin(115200);
  softserial.write("AT+CIOBAUD=9600\r\n");
  softserial.write("AT+RST\r\n");

  analogWrite(RED_LED, 50); // Turning on red led
  digitalWrite(MOTOR, HIGH);
}

void loop()
{
  color(255, 0, 0);
  delay(1000);
  color(0, 255, 0);
  delay(1000);
  color(0, 0, 255);
  delay(1000);

  if (!digitalRead(RED_PUSH_BTN))
  {
    analogWrite(BUZZER, 5);
    delay(5000);
    digitalWrite(BUZZER, LOW);
  }
}