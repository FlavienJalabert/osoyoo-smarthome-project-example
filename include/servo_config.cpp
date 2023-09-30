#include <Arduino.h>
#include <Servo.h>
Servo head;

#define SERVO_PIN 3 // servo connect to D11
#define LED 4       // LED connects to D2
#define BTN 11      // button connects to D3

void setup()
{
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(BTN, INPUT);

  Serial.begin(9600);

  head.attach(SERVO_PIN);

  head.write(135);
}

void loop()
{
  int buttonState = digitalRead(BTN);
  if (buttonState == LOW)
  {
    digitalWrite(LED, HIGH); // turn the LED on (HIGH is the voltage level)
    Serial.println("Open door");
    head.write(20);

    delay(2000);

    Serial.println("Close door");
    head.write(135);
    digitalWrite(LED, LOW); // turn the LED off by making the voltage LOW
  }
}
