#include <Arduino.h>
#include <SPI.h>
#include <dht.h>
#include <Servo.h>
#include <RFID.h>
#include <LiquidCrystal_I2C.h>

dht DHT;
Servo head;
RFID rfid(48, 49);                                   // RFID module SDA pin and RST pin
LiquidCrystal_I2C lcd(0x27, 16, 2);                  // LCD address and size
unsigned char my_rfid[] = {163, 130, 226, 173, 110}; // Replace with your RFID value
int maxTemp = 25;

// Declare Pins
#define SERVO_PIN 3
#define RED_LED 4
#define PIR 5
#define BUZZER 6
#define DHT11 10
#define RED_PUSH_BTN 11
#define LIGHT_SENSOR 13
#define LAMP A0
#define BLUE_PUSH_BTN A1
#define MOTOR A2
#define BLUE_PIN 22  // B connected to digital pin 22
#define GREEN_PIN 23 // G connected to digital pin 23
#define RED_PIN 24   // R connected to digital pin 24
#define TRIG_PIN 25  // Trigger connected to digital pin 25 in ultrasonic pin
#define ECHO_PIN 26  // Echo connected to digital pin 26 in ultrasonic pin

long echo_distance;
int chkdht;
int distance = 0;
int lightStatus = 0;
int ledStatusTime = 0;
bool togglePushBTN = true;

void setupPins()
{
    pinMode(RED_LED, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(SERVO_PIN, OUTPUT);
    pinMode(DHT11, INPUT);
    pinMode(RED_PUSH_BTN, INPUT);
    pinMode(MOTOR, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LIGHT_SENSOR, INPUT);
    pinMode(PIR, INPUT);
    pinMode(LAMP, OUTPUT);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);
}

void initializeComponents()
{
    Serial.begin(9600);
    lcd.init();
    SPI.begin();
    head.attach(SERVO_PIN);
    rfid.init();
    rfid.antennaOn();
}

void setColor(unsigned char red, unsigned char green, unsigned char blue)
{
    analogWrite(RED_PIN, red);
    analogWrite(GREEN_PIN, green);
    analogWrite(BLUE_PIN, blue);
}

bool compareRFID(unsigned char x[], unsigned char y[])
{
    Serial.println("Comparing RFID with record");
    for (int i = 0; i < 5; i++)
    {
        if (x[i] != y[i])
            return false;
    }
    return true;
}

void initializeLCD()
{
    Serial.println("Initialized LCD");
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Bonjour");
    lcd.setCursor(0, 1);
    lcd.print("<Formateur>");
    delay(1000);
    lcd.clear();
}

void closeDoor()
{
    head.attach(SERVO_PIN);
    head.write(135);
    delay(400);
    head.detach();
    digitalWrite(SERVO_PIN, LOW);
    Serial.println("Closed the door by default");
}

void setup()
{
    setupPins();
    initializeComponents();
    closeDoor();
    setColor(240, 130, 0);
    Serial.println("Turning RGB on to yellow by default for standby");
    initializeLCD();
}

int readUltrasonicDistance()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(15);
    digitalWrite(TRIG_PIN, LOW);
    echo_distance = pulseIn(ECHO_PIN, HIGH);
    echo_distance = echo_distance * 0.01657; // Calculate distance in cm
    return echo_distance;
}

void displayDistance()
{
    distance = readUltrasonicDistance();
    lcd.setCursor(0, 0);
    lcd.print("D:");
    lcd.setCursor(3, 0);
    lcd.print(distance);
    lcd.print("cm");
}

void readDHT11()
{
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
}

void handleTemperature()
{
    if (DHT.temperature >= maxTemp)
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
}

void handleMotion()
{
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
        digitalWrite(RED_LED, HIGH);
        analogWrite(BUZZER, 0);
        ledStatusTime = 5;
    }
}

void handleLight()
{
    lightStatus = digitalRead(LIGHT_SENSOR);
    if (lightStatus == 1)
    {
        digitalWrite(LAMP, HIGH);
        lcd.setCursor(0, 1);
        lcd.print("NUIT");
    }
    else
    {
        digitalWrite(LAMP, LOW);
        lcd.setCursor(0, 1);
        lcd.print("JOUR");
    }
}

void loop()
{
    // Search for RFID card
    lcd.setCursor(6, 1);
    lcd.print(rfid.isCard());
    if (rfid.isCard())
    {
        delay(100);
        Serial.println("Found the card!");

        // Read serial number
        if (rfid.readCardSerial())
        {
            Serial.print("id=");
            for (int i = 0; i < 5; i++)
            {
                Serial.print(rfid.serNum[i]);
                Serial.print(' ');
            }
            Serial.println("Card Read successfully");
            if (compareRFID(rfid.serNum, my_rfid))
            {
                setColor(0, 255, 0);
                Serial.println("Match");
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
            else
            {
                setColor(255, 0, 0);
                Serial.println("Not Match");
            }
        }
        rfid.selectTag(rfid.serNum);
        setColor(240, 130, 0);
    }
    rfid.halt();

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

    displayDistance();
    readDHT11();

    if (!digitalRead(RED_PUSH_BTN))
    {
        togglePushBTN = !togglePushBTN;
    }

    handleTemperature();
    handleMotion();
    handleLight();
}
