# Arduino Osoyoo Smart Home Automation System

This Arduino project is a versatile home automation system that integrates various sensors and components to provide a range of functionalities, including access control, temperature monitoring, motion detection, and lighting control.

## Table of Contents

- [Features](#features)
- [Components Used](#components-used)
- [Installation](#installation)
- [Usage](#usage)
- [Adding Complexity](#adding-complexity)
- [Contributing](#contributing)
- [License](#license)

## Features

- RFID Access Control: Grant access using authorized RFID cards.
- Temperature and Humidity Monitoring: Measure and display real-time data.
- Motion Detection: Detect intruders and trigger alarms.
- Lighting Control: Automatically control lighting based on ambient light levels.
- Web Interface: Monitor system status and sensor readings via a web page.
- Fan Control: Automatically control a fan based on temperature.
- Servo-Operated Door: Open and close a door using a servo motor.
- Sound Alerts: Trigger sound alerts using a buzzer.
- RGB LED Status: Indicate system status with an RGB LED.
- WiFi Connectivity: Connect to a local Wi-Fi network for remote access.

## Components Used

The project uses the following components:

- Arduino board (e.g., Arduino Uno)
- RFID reader module
- DHT11 Temperature and Humidity Sensor
- PIR (Passive Infrared) Motion Sensor
- Ultrasonic Sensor (HC-SR04)
- Servo Motor
- RGB LED and individual LEDs
- Push Buttons
- Buzzer
- Light Sensor
- Lamp
- ESP8266 for Wi-Fi connectivity (Optional)

## Installation

1. Clone or download the project repository to your local machine.

2. Set up the Arduino IDE and install the necessary libraries mentioned in the code from the [lib folder](lib).

3. Connect the components to the Arduino board following the wiring diagram provided.

4. Upload the Arduino code to the board using the Arduino IDE.

5. Use the sketchs from [include floder](include) to setup the smart home correctly.

## Usage

1. Power on the Arduino board.

2. Monitor the system via the serial monitor or a web browser. Access the web interface using the IP address displayed in the serial monitor.

3. Use authorized RFID cards for access control. Configure the RFID tag in the code.

4. Observe temperature, humidity, and motion detection readings on the web interface and LCD.

5. Control the fan, lighting, and door through the push buttons and automation rules.

## Adding Complexity

To further enhance the complexity and functionality of this project, consider the following:

- Camera Integration for image/video capture.
- Voice Control for command recognition.
- Security Features such as facial recognition or notifications.
- IoT Integration with platforms like AWS IoT.
- Mobile App for remote control.
- Data Logging for long-term data analysis.
- Advanced AI for predictive maintenance or anomaly detection.
- Remote Firmware Updates for extending functionality.

## Contributing

Contributions are welcome! If you have ideas for improvements or new features, please open an issue or submit a pull request.

## License

This project is open-source and available under the [MIT License](LICENSE).
