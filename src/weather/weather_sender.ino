#if !(ESP8266 || ESP32)
#error This code is intended to run on the ESP8266/ESP32 platform! Please check your Tools->Board setting
#endif

#include "Credentials.h"
#include <MySQL_Generic.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>

#define MYSQL_DEBUG_PORT      Serial
#define _MYSQL_LOGLEVEL_      1
#define USING_HOST_NAME     false
#define MYSQL_SERVER_PORT 3306
#define BME280_I2C_ADDRESS 0x76
#define LIGHT_SENSOR_SDA_PIN D2
#define LIGHT_SENSOR_SCL_PIN D1
#define WEATHER_DATA_TABLE "DATABASE.TABLE" // change this to your database.table name
#define SETTINGS_TABLE "DATABASE.TABLE" // change this to your database.table name

#if USING_HOST_NAME
char server[] = "your_account.ddns.net";
#else
IPAddress server(000, 000, 000, 000); // change this to your IP
#endif

Adafruit_BME280 bme;
BH1750 lightMeter;
MySQL_Connection conn((Client * ) & client);
MySQL_Query *query_mem;
MySQL_Query sql_query = MySQL_Query(&conn);

int weatherDataInterval_ms;

String sensor() {
    String strTemp = String(bme.readTemperature());
    String strPressure = String(bme.readPressure() / 100);
    String strHumidity = String(bme.readHumidity());
    String strBrightness = String(lightMeter.readLightLevel());


    String sendOutput = strBrightness + "," + strTemp + "," + strHumidity + "," + strPressure;
    return sendOutput;
}

String insertQuery() {
    // Query
    String INSERT_SQL = String("INSERT INTO ") + default_database + "." + default_table
                        + " (brightness,temperature,humidity,air_pressure) VALUES (" + sensor() + ")";
    return INSERT_SQL;
}

String selectQuery = String("SELECT weather_data_interval_ms FROM ") + default_database + "." + settings_table;


void setup() {
    Serial.begin(115200);
    while (!Serial);
    bme.begin(BME280_I2C_ADDRESS);
    Wire.begin(LIGHT_SENSOR_SDA_PIN, LIGHT_SENSOR_SCL_PIN);
    lightMeter.begin();
    MYSQL_DISPLAY1("\nWeatherdata", ARDUINO_BOARD);
    MYSQL_DISPLAY(MYSQL_MARIADB_GENERIC_VERSION);

    // Begin WiFi section
    MYSQL_DISPLAY1("Connecting to", ssid);

    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        MYSQL_DISPLAY0(".");
    }

    // print out info about the connection:
    MYSQL_DISPLAY1("Connected to network. My IP address is:", WiFi.localIP());

    MYSQL_DISPLAY3("Connecting to SQL Server @", server, ", Port =", server_port);
    MYSQL_DISPLAY5("User =", user, ", PW =", password, ", DB =", default_database);
}

// Helper function to run any query
void runQuery(String query) {
    MySQL_Query query_mem = MySQL_Query(&conn);
    if (!query_mem.execute(query.c_str())) {
        MYSQL_DISPLAY("Querying error");
    } else {
        query_mem.get_columns();
        row_values *row = NULL;
        do {
            row = query_mem.get_next_row();
            if (row != NULL) {
                weatherDataInterval_ms = atol(row->values[0]);
            }
        } while (row != NULL);
        query_mem.close();
    }
}

void runInsert(String query) {
    if (conn.connected()) {
        MYSQL_DISPLAY(query);
        runQuery(query);
        MYSQL_DISPLAY("Data Inserted.");
    } else {
        MYSQL_DISPLAY("Disconnected from Server. Can't insert.");
    }
}

void loop() {
    if (conn.connectNonBlocking(server, MYSQL_SERVER_PORT, user, password) != RESULT_FAIL) {
        delay(1000);
        runQuery(selectQuery);
        runInsert(insertQuery());
        conn.close();
    } else {
        MYSQL_DISPLAY("\nConnect failed. Trying again on next iteration.");
    }
    delay(weatherDataInterval_ms);
}
