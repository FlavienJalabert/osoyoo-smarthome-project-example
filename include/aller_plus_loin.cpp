#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <WiFiEsp.h>
#include <SPI.h>
#include <dht.h>
#include <Servo.h>
#include <RFID.h>
#include <LiquidCrystal_I2C.h>
// #ifndef HAVE_HWSERIAL1
#include <SoftwareSerial.h>
SoftwareSerial softserial(A9, A8); // A9 to ESP_TX, A8 to ESP_RX by default
// #endif

// LIBS OBJECTS DEFINITIONS
dht DHT;
Servo head;
RFID rfid(48, 49); // D48--RFID module SDA pin、D49 RFID module RST pin

// PROJECTS PARAMETERS
WiFiEspServer server(80);                          // create the server on port 80 (change port here)
RingBuffer buf(16);                                // use a ring buffer to increase speed and reduce memory allocation of 16 bytes (change number of bytes here)
LiquidCrystal_I2C lcd(0x3f, 16, 2);                // set the LCD address to 0x27 or 0x3f
unsigned char my_rfid[] = {110, 31, 128, 38, 215}; // replace with your RFID serial number
char ssid[] = "SFR_EB78";                          // replace ****** with your network SSID (name)
char pass[] = "Marioluigi1242";                    // replace ****** with your network password
int maxTemp = 25;                                  // define the max temperature for high temperature actions

// DECLARE PINS
#define SERVO_PIN 3
#define RED_LED 4
#define PIR 5
#define BUZZER 6
#define DHT11 10
#define RED_PUSH_BTN 11
#define light_sensor 13
#define LAMP A0
#define MOTOR A2
#define bluePin 22  // B connected to digital pin 22
#define greenPin 23 // G connected to digital pin 23
#define redPin 24   // R connected to digital pin 24
#define Trig_PIN 25 // Trigger connected to digital pin 25 in ultrasonic pin
#define Echo_PIN 26 // Echo connected to digital pin 26 in ultrasonic pin

// CONST TIMINGS
const int LCDInterval = 1000;        // millisecs between LCD print values
const int readingInterval = 500;     // millisecs between readings
const int switchLightInterval = 500; // millisecs between light can be switched again
const int highTempInterval = 500;    // millisecs between checking if temp is high and turning on fan motor
const int redledStatusTime = 5000;   // millisecs during red led should be up after a motion has been read
const int doorOpenDuration = 5000;   // millisecs that the door must stay open before closing
const int servoInterval = 20;        // millisecs between servo moves

// the limits to servo movement
const int servoMinDegrees = 20;  // fully open
const int servoMaxDegrees = 135; // fully closed
const int servoDegrees = 2;      // degree step for servo movement

// VAR
unsigned long currentMillis = 0;          // current millis is set at each loop()
unsigned long previousServoMillis = 0;    // the time when the servo was last moved
unsigned long previousReadingMillis = 0;  // the time when the sensors were last read
unsigned long previousLCDPrintMillis = 0; // the time when the LCD was last updated
unsigned long previousHighTempMillis = 0; // the time when the motor was switched last
unsigned long redledStatusMillis = 0;     // the time when the redled was switched on last
unsigned long doorOpenedTime = 0;         // the time when the door was fully opened
int servoPosition = 135;                  // the current angle of the servo - starting at 135 (closed).
bool doorNeedOpen = 0;                    // 0 = door don't need open, -1 = door need open, 1 = door need close
String httpResponseLines[5];              // list of readings to be sent to HTTP page
int status = WL_IDLE_STATUS;              // Wifi status
long echo_distance;
int chkdht;
int distance = 0;
int lightStatus = 0;
int lastLightStatus = 1;
int motionStatus = 1;
bool togglePushBTN = true;

int watch()
{
  digitalWrite(Trig_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(Trig_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig_PIN, LOW);
  echo_distance = pulseIn(Echo_PIN, HIGH);
  echo_distance = echo_distance * 0.01657; // how far away is the object in cm
  return echo_distance;
}
void color(unsigned char red, unsigned char green, unsigned char blue) // the color generating function
{
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}
boolean compare_rfid(unsigned char x[], unsigned char y[]) // compare read RFID with our register
{
  for (int i = 0; i < 5; i++)
  {
    if (x[i] != y[i])
      return false;
  }
  return true;
}
void sendHttpResponse(WiFiEspClient client)
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
void doorOpenClose()
{
  if ((currentMillis - previousServoMillis >= servoInterval) && (currentMillis - doorOpenedTime >= doorOpenDuration))
  {
    if (abs(doorNeedOpen) == 1)
    {
      servoPosition = servoPosition + servoDegrees * doorNeedOpen; // servoDegrees is signed by doorNeedOpen 1 to open and -1 to close
      // make the servo move to the next position
      head.write(servoPosition);
    }
    if (servoPosition <= servoMinDegrees) // if door is opened
    {
      doorNeedOpen = 1;               // ask to close
      doorOpenedTime = currentMillis; // set the door opened time
    }
    if (servoPosition >= servoMaxDegrees)
    {
      doorNeedOpen = 0;             // ask to stop
      digitalWrite(SERVO_PIN, LOW); // turn off servo to save energy
    }
    previousServoMillis = currentMillis;
  }
}
void switchLight()
{
  if (lightStatus == 1)
  {                           // no light
    digitalWrite(LAMP, HIGH); // turning on lamp
    httpResponseLines[4] = "<p>NUIT</p>";
  }
  else
  {
    digitalWrite(LAMP, LOW); // Turning off lamp
    httpResponseLines[4] = "<p>JOUR</p>";
  }
}
void highTemp()
{
  if (currentMillis - previousHighTempMillis >= highTempInterval)
  {
    if (round(DHT.temperature) >= maxTemp) // high temperature
    {
      if (togglePushBTN)
      {
        digitalWrite(MOTOR, HIGH); // no stop btn
      }
      else
      {
        digitalWrite(MOTOR, LOW); // stop btn
      }
    }
    else
    {
      digitalWrite(MOTOR, LOW); // no more high temperature
    }
    previousHighTempMillis = currentMillis;
  }
}
void motionDetected()
{
  if (motionStatus == 0) // if no motion detected
  {
    digitalWrite(BUZZER, LOW);                                  // turn off buzzer
    if (currentMillis - redledStatusMillis >= redledStatusTime) // RED LED has been on for 5 seconds without movement detected
    {
      digitalWrite(RED_LED, LOW); // turn off RED LED
    }
  }
  else
  {
    analogWrite(BUZZER, 0);             // Set desired volume (0-100)
    digitalWrite(RED_LED, HIGH);        // turn on red led
    redledStatusMillis = currentMillis; // set timer for RED LED
  }
}
void checkRFID()
{
  if (rfid.readCardSerial())
  {
    if (compare_rfid(rfid.serNum, my_rfid))
    {
      color(0, 255, 0); // put RGB led color to green
      // write LCD access granted
      lcd.clear();
      lcd.backlight();
      lcd.setCursor(3, 0);
      lcd.print("Bienvenue!");
      lcd.setCursor(0, 1);
      lcd.print("Accès autorisé !");
      doorNeedOpen = -1; // ask open door
    }
    else // WRONG RFID card
    {
      color(255, 0, 0); // flash RGB led red
    }
  }
  rfid.selectTag(rfid.serNum); // get the RFID card serial number
}
void readings()
{
  if (currentMillis - previousReadingMillis >= readingInterval)
  {
    if (rfid.isCard())
    {
      checkRFID(); // if a card is read, we will check the serial number and do actions
    }
    else
    {
      color(240, 125, 0); // Put back RGB led to standby (yellow)
    }
    rfid.halt();
    chkdht = DHT.read11(DHT11);              // DHT read temp and humidity level
    distance = watch();                      // get ultrasonic distance in cm
    lightStatus = digitalRead(light_sensor); // Read light sensor
    if (lightStatus != lastLightStatus)      // if lightStatus is different from last lightStatus recorded
    {
      switchLight();                 // switch the lamp on or off
      lastLightStatus = lightStatus; // record lightStatus
    }
    motionStatus = digitalRead(PIR); // Read PIR
    previousReadingMillis = currentMillis;
  }

  httpResponseLines[0] = (String) "<p>RFID status : " + rfid.isCard() + "</p>";
  httpResponseLines[1] = (String) "<p>Distance : " + distance + "</p>";
  httpResponseLines[2] = (String) "<p>T: " + round(DHT.temperature) + "</p><p>H: " + round(DHT.humidity) + "</p>";
  httpResponseLines[3] = (String) "<p>Motion Status : " + motionStatus + "</p>";
}
void printLCDReadings()
{
  if (currentMillis - previousLCDPrintMillis >= LCDInterval)
  {
    // refresh LCD
    lcd.clear();
    if (distance < 20) // someone is under 20 cm in last reading
    {
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
    if (lightStatus == 1)
    {
      lcd.setCursor(0, 1);
      lcd.print("NUIT");
    }
    else
    {
      lcd.setCursor(0, 1);
      lcd.print("JOUR");
    }
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

    previousLCDPrintMillis = currentMillis;
  }
}
void printWebAddress()
{
  IPAddress ip = WiFi.localIP();
  // print where to go in the browser
  Serial.println("-------------------------------------------------------------------------------------------------");
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  Serial.println("-------------------------------------------------------------------------------------------------");
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
  softserial.begin(115200);
  softserial.write("AT+CIOBAUD=9600\r\n");
  softserial.write("AT+RST\r\n");
  softserial.begin(9600); // initialize serial for ESP module
  Serial.println("INITIALIZING ARDUINO");
  WiFi.init(&softserial); // initialize ESP module

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("WiFi shield not present !");
    // don't continue
    while (true)
      ;
  }
  // attempt to connect to WiFi network
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }
  Serial.println("You're connected to the network");
  printWebAddress();
  // start the web server on defined port
  server.begin();
  // setting udp http response values
  // httpResponseLines[3] = "<p>no motion detected</p>";
  // httpResponseLines[4] = "<p>JOUR</p>";
  httpResponseLines[5] = "<font color=GREEN><b>CONNECTION REUSSIE !</b></font><p>insérer le style ici</p>"; // Style for the HTTP page to monitor on browser

  lcd.init(); // Initialize LCD
  Serial.println("Initialized LCD");
  SPI.begin();      // Start up SPI IO
  rfid.init();      // Initialize RFID module
  rfid.antennaOn(); // turn on RFID antenna
  Serial.println("Initialized RFID module");
  head.attach(SERVO_PIN);      // Attach servo head to servo object
  head.write(servoMaxDegrees); // closing door by default
  delay(100);
  digitalWrite(SERVO_PIN, LOW);
  Serial.println("Closed the door by default");
  color(240, 125, 0);
  Serial.println("Turning RGB on to yellow by default for standby");
  lcd.backlight(); // open the backlight
  lcd.setCursor(0, 0);
  lcd.print("Bonjour");
  lcd.setCursor(0, 1);
  lcd.print("<Formateur>");
  lcd.clear();
}

void loop()
{
  WiFiEspClient client = server.available(); // listen for incoming clients
  if (client)                                // if you get a client,
  {
    buf.init(); // initialize the circular buffer
    while (client.connected())
    {                                 // loop while the client's connected
      currentMillis = millis();       // capture the latest value of millis()
      if (!digitalRead(RED_PUSH_BTN)) // record button pressed
      {
        togglePushBTN = !togglePushBTN; // toggle button status
      }
      // Do things
      readings();
      doorOpenClose();
      highTemp();
      motionDetected();
      printLCDReadings();
      // check connectivity
      if (client.available()) // if there's bytes to read from the client,
      {
        char c = client.read();       // read a byte, then
        buf.push(c);                  // push it to the ring buffer
        if (buf.endsWith("\r\n\r\n")) // if you get two new lines in a row it's the end of the request
        {
          sendHttpResponse(client); // send the response
          break;
        }
      }
    }
    client.stop(); // close the connection
  }
}