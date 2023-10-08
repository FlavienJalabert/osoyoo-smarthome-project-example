// Include necessary libraries
#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include <RFID.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <DHT.h>
#include <WiFiEsp.h>

// Pin Definitions
const int SERVO_PIN = 3;      // Servo motor control
const int RED_LED = 4;        // Red LED indicator
const int PIR = 5;            // Passive Infrared Sensor
const int BUZZER = 6;         // Buzzer for sound
const int DHT11 = 10;         // DHT11 Temperature and Humidity Sensor
const int RED_PUSH_BTN = 11;  // Red Push Button
const int LIGHT_SENSOR = 13;  // Light Sensor
const int LAMP = A0;          // Lamp Control
const int BLUE_PUSH_BTN = A1; // Blue Push Button
const int MOTOR = A2;         // Motor Control
const int BLUE_PIN = 22;      // Blue LED control
const int GREEN_PIN = 23;     // Green LED control
const int RED_PIN = 24;       // Red LED control
const int TRIG_PIN = 25;      // Ultrasonic Sensor Trigger
const int ECHO_PIN = 26;      // Ultrasonic Sensor Echo

// Define timings
const unsigned long LONG_INTERVAL = 5000; // long interval for lasting events
const unsigned long SHORT_INTERVAL = 500; // short interval for refresh
const unsigned long INSTANT_INTERVAL = 5; // instant interval for servo sweep

// Define constants for intervals and thresholds
const unsigned long LCD_INTERVAL = SHORT_INTERVAL;
const unsigned long READING_INTERVAL = SHORT_INTERVAL;
const unsigned long SWITCH_LIGHT_INTERVAL = SHORT_INTERVAL;
const unsigned long HIGH_TEMP_INTERVAL = SHORT_INTERVAL;
const unsigned long RED_LED_STATUS_TIME = LONG_INTERVAL;
const unsigned long DOOR_OPEN_DURATION = LONG_INTERVAL;
const unsigned long SERVO_INTERVAL = INSTANT_INTERVAL;

// Servo parameters
const int SERVO_MIN_DEGREES = 20;  // Door opened
const int SERVO_MAX_DEGREES = 135; // Door closed
const int SERVO_DEGREES = 2;       // Servo step in each loop

// Define WiFi credentials
const char *SSID = "Your_SSID";
const char *PASSWORD = "Your_Password";

// Define RFID tag
byte myRFID[] = {110, 31, 128, 38, 215};

// Other variables
String httpResponseLines[5];
unsigned long currentMillis = 0;
unsigned long previousServoMillis = 0;
unsigned long previousReadingMillis = 0;
unsigned long previousLCDPrintMillis = 0;
unsigned long previousHighTempMillis = 0;
unsigned long redLEDStatusMillis = 0;
unsigned long doorOpenedTime = 0;
int servoPosition = SERVO_MAX_DEGREES;
int doorState = 0;         // 0 = door closed, 1 = door opening, -1 = door closing
bool togglePushBTN = true; // Toggles the state of the fan (true = running, false = stopped)
int maxTemp = 25;          // max temp to use as High Temp detection

// Objects for sensors and modules
dht dht11;
Servo head;
RFID rfid(48, 49); // D48--RFID module SDA pin, D49 RFID module RST pin
LiquidCrystal_I2C lcd(0x3f, 16, 2);
SoftwareSerial softSerial(A8, A9); // RX, TX pins for ESP module

WiFiEspServer server(80);

void setColor(int red, int green, int blue)
{ // sets the color for RGB LED
    analogWrite(RED_PIN, red);
    analogWrite(GREEN_PIN, green);
    analogWrite(BLUE_PIN, blue);
}

void printWebAddress()
{ // Prints the web adress to Serial
    IPAddress ip = WiFi.localIP();
    Serial.println("-------------------------------------------------------------------------------------------------");
    Serial.print("To see this page in action, open a browser to http://");
    Serial.println(ip);
    Serial.println("-------------------------------------------------------------------------------------------------");
}

void setup()
{
    // Initialize pins
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

    // initialize Serial communication with ESP and COM
    Serial.begin(9600);
    softSerial.begin(115200);

    softSerial.write("AT+CIOBAUD=9600\r\n");
    softSerial.write("AT+RST\r\n");
    softSerial.begin(9600);

    Serial.println("Initializing Arduino");

    // Initialize WiFi module
    WiFi.init(&softSerial);

    // Check for WiFi shield
    if (WiFi.status() == WL_NO_SHIELD)
    {
        Serial.println("WiFi shield not found.");
        while (true)
            ; // Halt the program
    }

    // Connect to WiFi network
    int status = WL_IDLE_STATUS;
    while (status != WL_CONNECTED)
    {
        Serial.print("Connecting to WiFi: ");
        Serial.println(SSID);
        status = WiFi.begin(SSID, PASSWORD);
        delay(10000); // Wait 10 seconds before retrying
    }

    Serial.println("Connected to WiFi network");
    printWebAddress();

    // start the server
    server.begin();

    // Initialize RFID module
    rfid.init();
    rfid.antennaOn();

    // Initialize Servo
    head.attach(SERVO_PIN);
    head.write(SERVO_MAX_DEGREES);
    digitalWrite(SERVO_PIN, LOW);

    // Set initial LED color to yellow (standby)
    setColor(240, 125, 0);

    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Bonjour");
    lcd.setCursor(0, 1);
    lcd.print("<Formateur>");

    Serial.println("Setup complete");
}

// Define functions for handling RFID, reading sensors, and other tasks
boolean compareRFID(byte x[], byte y[])
{
    for (int i = 0; i < 5; i++)
    {
        if (x[i] != y[i])
        {
            return false;
        }
    }
    return true;
}

void processClient(WiFiEspClient client)
{
    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
    // and a content-type so the client knows what's coming, then a blank line:
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();

    // the content of the HTTP response follows the header:
    client.print("<meta http-equiv=\"refresh\" content=\"5\"/>");
    for (unsigned int i = 0; i < 5; i++)
    {
        client.print(httpResponseLines[i]); // write to http response every sensors readings
    }
}

void handleButtonPresses()
{
    if (!digitalRead(RED_PUSH_BTN))
    {
        togglePushBTN = !togglePushBTN;
    }

    if (!digitalRead(BLUE_PUSH_BTN))
    {
        doorState = 1; // Open the door
    }
}

void checkRFID()
{
    if (rfid.readCardSerial())
    {
        if (compareRFID(rfid.serNum, myRFID))
        {
            setColor(0, 255, 0); // Green color
            lcd.clear();
            lcd.backlight();
            lcd.setCursor(3, 0);
            lcd.print("Bienvenue!");
            lcd.setCursor(0, 1);
            lcd.print("Acces autorise !");
            doorState = -1; // Close the door
        }
        else
        {
            setColor(255, 0, 0); // Red color (wrong RFID card)
        }
    }
    rfid.selectTag(rfid.serNum);
}

int readUltrasonicDistance()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(15);
    digitalWrite(TRIG_PIN, LOW);
    long echo_distance = pulseIn(ECHO_PIN, HIGH);
    echo_distance = echo_distance * 0.01657; // how far away is the object in cm
    // Serial.println((int)echo_distance);
    return echo_distance;
}

void printLCDReadings(float temperature, float humidity, int distance, bool lightStatus)
{
    if (currentMillis - previousLCDPrintMillis >= LCD_INTERVAL)
    {
        lcd.clear();
        if (distance < 20) // someone is close to the house
        {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("D:");
            lcd.setCursor(3, 0);
            lcd.print(distance);
            lcd.print("cm");
        }
        else
        {
            lcd.setCursor(0, 0);
            lcd.print("D:");
            lcd.setCursor(3, 0);
            lcd.print("__");
            lcd.print("cm");
        }
        lcd.setCursor(0, 1);
        if (lightStatus == true)
        {
            lcd.print("JOUR");
        }
        lcd.setCursor(8, 0);
        lcd.print("T:");
        lcd.setCursor(11, 0);
        lcd.print(temperature);
        lcd.print("C");
        lcd.setCursor(8, 1);
        lcd.print("H:");
        lcd.setCursor(11, 1);
        lcd.print(humidity);
        lcd.print("%");
        previousLCDPrintMillis = currentMillis;
    }
}

void readSensors()
{
    // Read RFID card
    if (rfid.isCard())
    {
        checkRFID();
    }
    else
    {
        setColor(240, 125, 0); // Standby color (yellow)
    }
    rfid.halt();

    // Read DHT sensor
    float temperature = dht11.temperature;
    float humidity = dht11.humidity;

    // Read ultrasonic distance
    int distance = readUltrasonicDistance();

    // Read light sensor
    int lightStatus = digitalRead(LIGHT_SENSOR);

    httpResponseLines[0] = (String) "<p>RFID status : " + rfid.isCard() + "</p>";
    httpResponseLines[1] = (String) "<p>Distance : " + distance + "</p>";
    httpResponseLines[2] = (String) "<p>T: " + temperature + "</p><p>H: " + humidity + "</p>";
    httpResponseLines[3] = (String) "<p>Motion Status : " + digitalRead(PIR) + "</p>";
    httpResponseLines[4] = (String) "<p>Light Status : " + lightStatus + "</p>";

    // Print sensor readings to LCD
    printLCDReadings(temperature, humidity, distance, lightStatus);
}

void updateDoorState()
{
    if (doorState == 1 && currentMillis - previousServoMillis >= SERVO_INTERVAL)
    {
        servoPosition -= SERVO_DEGREES;
        head.write(servoPosition);
        if (servoPosition <= SERVO_MIN_DEGREES)
        {
            doorState = -1; // Start closing the door
            doorOpenedTime = currentMillis;
        }
        previousServoMillis = currentMillis;
    }
    else if (doorState == -1 && currentMillis - doorOpenedTime >= DOOR_OPEN_DURATION)
    {
        servoPosition += SERVO_DEGREES;
        head.write(servoPosition);
        if (servoPosition >= SERVO_MAX_DEGREES)
        {
            doorState = 0; // Door is closed
        }
        previousServoMillis = currentMillis;
    }
}

void updateLampStatus()
{
    if (digitalRead(LIGHT_SENSOR) == 1)
    {
        digitalWrite(LAMP, HIGH); // night
    }
    else
    {
        digitalWrite(LAMP, LOW); // day
    }
}

void handlePIR()
{
    int motionStatus = digitalRead(PIR);
    if (motionStatus == HIGH)
    {
        digitalWrite(RED_LED, HIGH);
        analogWrite(BUZZER, 0); // Set desired volume (0-100)
        redLEDStatusMillis = currentMillis;
    }
    else
    {
        if (currentMillis - redLEDStatusMillis >= 5000)
        {
            digitalWrite(RED_LED, LOW);
        }
        digitalWrite(BUZZER, LOW);
    }
}

void handleHighTemperature()
{
    float temperature = dht11.temperature;
    if (temperature >= maxTemp)
    {
        if (togglePushBTN)
        {
            digitalWrite(MOTOR, HIGH); // Fan motor on
        }
        else
        {
            digitalWrite(MOTOR, LOW); // Fan motor off
        }
    }
    else
    {
        digitalWrite(MOTOR, LOW); // Fan motor off
    }
}

// Main loop of the program
void loop()
{
    WiFiEspClient client = server.available();
    if (client)
    {
        processClient(client);
    }

    // Record current system milli seconds
    currentMillis = millis();

    // Handle button presses
    handleButtonPresses();

    // Read sensors and perform actions
    if (currentMillis - previousReadingMillis >= READING_INTERVAL)
    {
        // Read sensors, print them to LCD and add them to the http client response string
        readSensors();

        // Handle PIR sensor activity
        handlePIR();
        previousReadingMillis = currentMillis;
    }

    // Update door state
    updateDoorState();

    // Update LED color based on light status
    updateLampStatus();

    // Handle high temperature conditions
    handleHighTemperature();

    // Keep the client responsive
    if (client && client.connected())
    {
        client.setTimeout(1); // Set a very short timeout for responsiveness
        client.read();        // Read a byte from the client to keep it responsive
    }

    Serial.println("Loop iteration complete. Current millis: " + String(currentMillis));
}