#include "Credentials.h"
#include <MySQL_Generic.h>
#include <Adafruit_ADS1X15.h>

#if !(ESP8266 || ESP32)
#error This code is intended to run on the ESP8266/ESP32 platform! Please check your Tools->Board setting
#endif

#define MYSQL_DEBUG_PORT      Serial
#define _MYSQL_LOGLEVEL_      1

// Pin definitions
constexpr uint8_t
sensorPin = 2;
constexpr uint8_t
relayPin = 14;
constexpr uint8_t
relayPin2 = 12;

// Optional using hostname, and Ethernet built-in DNS lookup
constexpr bool USING_HOST_NAME = false;

#if USING_HOST_NAME
constexpr char server[] = "your_account.ddns.net"; // change to your server's hostname/URL
#else
constexpr IPAddress
server(000, 000, 000, 000); // change to your server's IP
#endif

constexpr uint16_t
server_port = 3306;
constexpr char default_database[] = "DATBASE"; // change to your database name
constexpr char default_table[] = "TABLE"; // change to your table name
constexpr char settings_table[] = "SETTINGSTABLE"; // change to your settings table name

MySQL_Connection conn((Client * ) & client);

Adafruit_ADS1115 ads1115;  // Construct an ads1115
long adc0;
long tankLvl_mm = 0;
int valveOpen = 0;
int waterLevelLimit;
int waterLevelHysteresis;
int waterBarrelInterval;

String sensor();

void setup();

String insertSQL();

void runInsert();

void runQuery();

void setup() {
    Serial.begin(115200);
    while (!Serial);

    ads1115.begin(0x48);  // Initialize ads1115 at address 0x49
    pinMode(sensorPin, OUTPUT);
    pinMode(relayPin, OUTPUT);
    pinMode(relayPin2, OUTPUT);
    digitalWrite(relayPin, LOW);

    MYSQL_DISPLAY1("\nStarting Basic_Insert_ESP on", ARDUINO_BOARD);
    MYSQL_DISPLAY(MYSQL_MARIADB_GENERIC_VERSION);

    MYSQL_DISPLAY1("Connecting to", ssid);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        MYSQL_DISPLAY0(".");
    }

    MYSQL_DISPLAY1("Connected to network. My IP address is:", WiFi.localIP());
    MYSQL_DISPLAY3("Connecting to SQL Server @", server, ", Port =", server_port);
    MYSQL_DISPLAY5("User =", user, ", PW =", password, ", DB =", default_database);
}

void loop() {
    MYSQL_DISPLAY("Connecting...");
    if (conn.connectNonBlocking(server, server_port, user, password) != RESULT_FAIL) {
        delay(1000);
        runQuery();
        runInsert();
        conn.close();                     // close the connection
    } else {
        MYSQL_DISPLAY("\nConnect failed. Trying again on next iteration.");
    }
    MYSQL_DISPLAY("\nSleeping...");
    MYSQL_DISPLAY("================================================");
}

String sensor() {
    adc0 = ads1115.readADC_SingleEnded(0);
    if (adc0 == 0) {
        adc0 = -1;
    }
    tankLvl_mm = 0.1897 * adc0 - 537.12; // 695mm = 6453
    int tankLvlLitre = tankLvl_mm * 3.14159265359;
    Serial.println(tankLvlLitre);

    if (tankLvlLitre < waterLevelLimit) {
        digitalWrite(relayPin, HIGH);
        digitalWrite(relayPin2, HIGH);
        valveOpen = 1;
    } else {
        if (tankLvlLitre > waterLevelLimit + waterLevelHysteresis) {
            digitalWrite(relayPin, LOW);
            digitalWrite(relayPin2, LOW);
            valveOpen = 0;
        }
    }
    return String(adc0) + "," + String(valveOpen);
}

String insertSQL() {
    return String("INSERT INTO ") + default_database + "." + default_table
           + " (measurement,valve_status) VALUES (" + sensor() + ")";
}

void runInsert() {
    MySQL_Query query_mem = MySQL_Query(&conn);
    if (conn.connected()) {
        MYSQL_DISPLAY(insertSQL());
        if (!query_mem.execute(insertSQL().c_str())) {
            MYSQL_DISPLAY("Insert error");
        } else {
            MYSQL_DISPLAY("Data Inserted.");
        }
    } else {
        MYSQL_DISPLAY("Disconnected from Server. Can't insert.");
    }
}

void runQuery() {
    MYSQL_DISPLAY("> Running SELECT");
    String query = String("SELECT * FROM ") + default_database + "." + settings_table;
    MYSQL_DISPLAY(query);
    MySQL_Query query_mem = MySQL_Query(&conn);
    if (!query_mem.execute(query.c_str())) {
        MYSQL_DISPLAY("Querying error");
        return;
    }

    column_names *cols = query_mem.get_columns();
    for (int f = 0; f < cols->num_fields; f++) {
        MYSQL_DISPLAY0(cols->fields[f]->name);
        if (f < cols->num_fields - 1) {
            MYSQL_DISPLAY0(",");
        }
    }

    MYSQL_DISPLAY();

    row_values *row = NULL;
    do {
        row = query_mem.get_next_row();
        if (row != NULL) {
            for (int f = 0; f < cols->num_fields; f++) {
                MYSQL_DISPLAY0(row->values[f]);
                if (f < cols->num_fields - 1) {
                    MYSQL_DISPLAY0(",");
                }
            }
            MYSQL_DISPLAY();
        }
    } while (row != NULL);
}
