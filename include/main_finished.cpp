#include <Arduino.h>
#include <math.h>
#include <SPI.h>
#include <dht.h>
dht DHT;
#include <Servo.h>
Servo head;
#include <RFID.h>
RFID rfid(48, 49); // D48--RFID module SDA pin、D49 RFID module RST pin
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);                  // set the LCD address to 0x27 or 0x3f
unsigned char my_rfid[] = {163, 130, 226, 173, 110}; // replace with your RFID value

// DECLARE PINS
#define SERVO_PIN 3
#define RED_LED 4
#define PIR 5
#define BUZZER 6
#define DHT11 10
#define RED_PUSH_BTN 11
#define light_sensor 13
#define LAMP A0
#define BLUE_PUSH_BTN A1
#define MOTOR A2
#define bluePin 22  // B connected to digital pin 22
#define greenPin 23 // G connected to digital pin 23
#define redPin 24   // R connected to digital pin 24
#define Trig_PIN 25 // Trigger connected to digital pin 25 in ultrasonic pin
#define Echo_PIN 26 // Echo connected to digital pin 26 in ultrasonic pin

long echo_distance;
int chkdht;
int distance = 0;
int lightStatus = 0;
int ledStatusTime = 0;
bool togglePushBTN = true;

int watch()
{
    digitalWrite(Trig_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(Trig_PIN, HIGH);
    delayMicroseconds(15);
    digitalWrite(Trig_PIN, LOW);
    echo_distance = pulseIn(Echo_PIN, HIGH);
    echo_distance = echo_distance * 0.01657; // how far away is the object in cm
    // Serial.println((int)echo_distance);
    return echo_distance;
}
void color(unsigned char red, unsigned char green, unsigned char blue) // the color generating function
{
    analogWrite(redPin, red);
    analogWrite(greenPin, green);
    analogWrite(bluePin, blue);
}
boolean compare_rfid(unsigned char x[], unsigned char y[])
{
    Serial.println("Comparing rfid with record");
    for (int i = 0; i < 5; i++)
    {
        if (x[i] != y[i])
            return false;
    }
    return true;
}

void setup()
{
    pinMode(RED_LED, OUTPUT);     // initialize red led pin as an output
    pinMode(BUZZER, OUTPUT);      // initialize buzzer pin as an output
    pinMode(SERVO_PIN, OUTPUT);   // initialize servo motor as an output
    pinMode(DHT11, INPUT);        // initialize DHT11 pin as an input
    pinMode(RED_PUSH_BTN, INPUT); // initialize red push button pin as an input
    pinMode(MOTOR, OUTPUT);       // initialize motor pin as an output
    pinMode(LED_BUILTIN, OUTPUT); // initialize digital pin LED_BUILTIN as an output.
    pinMode(light_sensor, INPUT); // initialize light sensor pin as an input.
    pinMode(PIR, INPUT);          // initialize PIR sensor pin as an input.
    pinMode(LAMP, OUTPUT);        // initialize LAMP pin as an output.
    pinMode(Trig_PIN, OUTPUT);    // initialize digital pin Trig Pin as an output.
    pinMode(Echo_PIN, INPUT);     // initialize digital pin Echo Pin as an input.
    pinMode(redPin, OUTPUT);      // set the RGB redPin to be an output
    pinMode(greenPin, OUTPUT);    // set the RGB greenPin to be an output
    pinMode(bluePin, OUTPUT);     // set the RGB bluePin to be an output
    Serial.begin(9600);           // initialize serial for debugging
    lcd.init();                   // Initialize LCD
    Serial.println("Initialized LCD");
    SPI.begin(); // Start up SPI IO
    head.attach(SERVO_PIN);
    rfid.init();      // Initialize RFID module
    rfid.antennaOn(); // turn on RFID antenna
    Serial.println("Initialized RFID module");
    head.write(135); // closing door by default
    delay(400);
    head.detach();
    digitalWrite(SERVO_PIN, LOW);
    Serial.println("Closed the door by default");
    color(240, 130, 0);
    Serial.println("Turning RGB on to yellow by default for standby");
    lcd.backlight(); // open the backlight
    lcd.setCursor(0, 0);
    lcd.print("Bonjour");
    lcd.setCursor(0, 1);
    lcd.print("<Formateur>");
    delay(1000);
    lcd.clear();
}

void loop()
{
    // search card
    lcd.setCursor(8, 1);
    lcd.print(rfid.isCard());
    if ((bool)rfid.isCard())
    {
        delay(100);
        Serial.println("Find the card!");
        // read serial number
        if (rfid.readCardSerial())
        {
            Serial.print("id=");
            for (int i = 0; i < 5; i++)
            {
                Serial.print(rfid.serNum[i]);
                Serial.print(' ');
            }
            Serial.println("Card Read succesfully");
            if (compare_rfid(rfid.serNum, my_rfid))
            {
                color(0, 255, 0);
                Serial.println("match");
                lcd.clear();
                lcd.backlight();
                lcd.setCursor(3, 0);
                lcd.print("Bienvenue!");
                lcd.setCursor(0, 1);
                lcd.print("Vous Pouvez Entrer!");
                head.attach(SERVO_PIN);
                head.write(20);
                delay(3000);
                head.write(135);
                delay(400);
                head.detach();
                digitalWrite(SERVO_PIN, LOW);
                lcd.clear();
            }
            else // WRONG RFID card
            {
                color(255, 0, 0);
                Serial.println("NOT match");
            }
        }
        rfid.selectTag(rfid.serNum); // get the RFID card serial number
        color(240, 130, 0);          // Put back RGB led to standby
    }

    rfid.halt(); // Listen RFID module again in next loop

    if (!digitalRead(BLUE_PUSH_BTN))
    {
        head.attach(SERVO_PIN);
        head.write(20);
        delay(3000);
        head.write(135);
        delay(400);
        head.detach();
        digitalWrite(SERVO_PIN, LOW);
    }

    distance = watch(); // get supersonic distance in cm
    if (distance < 20)  // someone is close to the house
    {
        // Serial.println("detected presence under 20cm");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("D:");
        lcd.setCursor(3, 0);
        lcd.print(distance);
        lcd.print("cm");
    }
    else
    {
        // Serial.println("not detected any presence under 20cm");
        lcd.setCursor(0, 0);
        lcd.print("D:");
        lcd.setCursor(3, 0);
        lcd.print("__");
        lcd.print("cm");
    }

    // DHT read temp and humidity level
    chkdht = DHT.read11(DHT11);
    lcd.setCursor(8, 0);
    lcd.print("T:");
    lcd.setCursor(11, 0);
    lcd.print(round(DHT.temperature));
    lcd.print("C");
    lcd.setCursor(8, 1);
    lcd.print("H:");
    lcd.setCursor(11, 1);
    lcd.print(round(DHT.humidity));
    lcd.print("%");
    if (!digitalRead(RED_PUSH_BTN))
    {
        togglePushBTN = !togglePushBTN;
    }
    if (DHT.temperature >= 29)
    {
        if (togglePushBTN)
        {
            Serial.println("High temp detected, turning fan on");
            digitalWrite(MOTOR, HIGH);
        }
        else
        {
            Serial.println("High temp detected, but emergency stop triggered");
            digitalWrite(MOTOR, LOW);
        }
    }
    else
    {
        digitalWrite(MOTOR, LOW);
    }

    if (ledStatusTime > 0)
    {
        ledStatusTime--;
    }
    int motionStatus = digitalRead(PIR);
    if (motionStatus == 0)
    {
        if (ledStatusTime <= 0)
        {
            digitalWrite(RED_LED, LOW);
        }
        digitalWrite(BUZZER, LOW);
    }
    else
    {
        // Serial.println("Intruder detected");
        digitalWrite(RED_LED, HIGH);
        analogWrite(BUZZER, 0); // Set desired volume (0-100)
        ledStatusTime = 5;
    }

    lightStatus = digitalRead(light_sensor); // Read light sensor
    if (lightStatus == 1)
    {                             // no light
        digitalWrite(LAMP, HIGH); // turning on lamp
        lcd.setCursor(0, 1);
        lcd.print("NUIT");
    }
    else
    {
        digitalWrite(LAMP, LOW); // Turning off lamp
        lcd.setCursor(0, 1);
        lcd.print("JOUR");
    }
}