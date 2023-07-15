# Smart Data Project

Welcome to the Smart Data project repository! This repository contains two Arduino projects for collecting and storing weather data and water level measurements in a MySQL database.

## Repository Contents

- `weather_sender.ino`: This file contains the code for the Arduino weather data sender. It uses sensors such as the Adafruit BME280 (for temperature, humidity, and pressure) and the BH1750 (for brightness) to collect weather data. The code establishes a Wi-Fi connection, connects to a MySQL database, and periodically sends the collected weather data to the database.

- `waterlevel_sender.ino`: This file contains the code for the Arduino water level sensor. It utilizes an ADS1115 analog-to-digital converter (ADC) and a water level sensor to measure the water level in a cistern. The code reads the ADC values from the water level sensor, calculates the water level, and controls a relay for valve operation accordingly. The water level data is also inserted into the MySQL database at regular intervals.

## Getting Started

To get started with the Smart Data project, follow these steps:

1. Set up your Arduino board (ESP8266 or ESP32) and connect the necessary sensors as specified in the code comments.

2. Configure the Wi-Fi connection by modifying the `ssid` and `pass` variables in both files.

3. Set up the MySQL database by providing the server IP address, port, database name, username, and password in the code. Modify the corresponding variables accordingly.

4. Install any required libraries mentioned in the code, such as `MySQL_Generic`, `Adafruit_Sensor`, `Adafruit_BME280`, and `BH1750`.

5. Upload the `weather_sender.ino` code to your Arduino board and monitor the serial output for debugging and status information.

6. Upload the `waterlevel_sender.ino` code to your Arduino board and monitor the serial output for debugging and status information.

## Contributing

Contributions to the Smart Data project are welcome! If you find any issues or have any suggestions for improvements, please feel free to open a GitHub issue or submit a pull request.

## License

This project is licensed under the [MIT License](LICENSE).
