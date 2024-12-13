/*#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "cert.h"

const char* ssid = "Dialog 4G 044";
const char* wifiPassword = "c0Deb7c5";
int status = WL_IDLE_STATUS;

String FirmwareVer = "1.0.4";
#define URL_fw_Version "https://raw.githubusercontent.com/MadeeshaLakshan/ESP32_OTA_Github/refs/heads/main/version.txt"
#define URL_fw_Bin "https://raw.githubusercontent.com/MadeeshaLakshan/ESP32_OTA_Github/main/build/esp32.esp32.esp32/ESP32_FinalCodes_https.ino.bin"

unsigned long previousMillis = 0;  // will store last time update was checked
const long interval = 5000;        // interval at which to check for updates (milliseconds)

void setup() {
    Serial.begin(115200);
    Serial.print("Active Firmware Version: ");
    Serial.println(FirmwareVer);

    WiFi.begin(ssid, wifiPassword);

    Serial.print("Connecting to WiFi");
    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if (i++ == 10) {
            ESP.restart();
        }
    }
    Serial.println("\nConnected To WiFi");
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        checkForUpdate();
    }

    if (WiFi.status() != WL_CONNECTED) {
        reconnect();
    }
}

void checkForUpdate() {
    Serial.print("Checking for firmware updates... Current version: ");
    Serial.println(FirmwareVer);

    if (FirmwareVersionCheck()) {
        Serial.println("New firmware detected. Updating...");
        firmwareUpdate();
    } else {
        Serial.println("No new firmware available.");
    }
}

void reconnect() {
    int i = 0;
    status = WiFi.status();
    if (status != WL_CONNECTED) {
        WiFi.begin(ssid, wifiPassword);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
            if (i++ == 10) {
                ESP.restart();
            }
        }
        Serial.println("\nConnected to AP");
    }
}

void firmwareUpdate() {
    WiFiClientSecure client;
    client.setCACert(rootCACertificate);
    t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            break;

        case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            break;
    }
}

int FirmwareVersionCheck() {
    String payload;
    int httpCode;
    String FirmwareURL = URL_fw_Version;
    FirmwareURL += "?";
    FirmwareURL += String(rand());

    WiFiClientSecure* client = new WiFiClientSecure;

    if (client) {
        client->setCACert(rootCACertificate);
        HTTPClient https;

        if (https.begin(*client, FirmwareURL)) {
            Serial.print("[HTTPS] GET...\n");
            delay(100);
            httpCode = https.GET();
            delay(100);
            if (httpCode == HTTP_CODE_OK) {
                payload = https.getString();
            } else {
                Serial.print("Error Occurred During Version Check: ");
                Serial.println(httpCode);
            }
            https.end();
        }
        delete client;
    }

    if (httpCode == HTTP_CODE_OK) {
        payload.trim();
        if (payload.equals(FirmwareVer)) {
            Serial.printf("\nDevice is already on the latest firmware version: %s\n", FirmwareVer.c_str());
            return 0;
        } else {
            Serial.println(payload);
            Serial.println("New Firmware Detected");
            return 1;
        }
    }
    return 0;
}*/
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include <LittleFS.h> // Use LittleFS instead of LITTLEFS
#include "cert.h" // Ensure cert.h contains the rootCACertificate definition

String FirmwareVer = "1.0.1";
#define URL_fw_Version "https://raw.githubusercontent.com/MadeeshaLakshan/ESP32_OTA_Github/refs/heads/main/version.txt"
#define URL_fw_Bin "https://raw.githubusercontent.com/MadeeshaLakshan/ESP32_OTA_Github/main/build/esp32.esp32.esp32/ESP32_FinalCodes_https.ino.bin"

unsigned long previousMillis = 0;  // will store last time update was checked
const long interval = 5000;        // interval at which to check for updates (milliseconds)

// Function to initialize File System
void initFileSystem() {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount LittleFS, formatting...");
        if (LittleFS.format()) {
            Serial.println("LittleFS formatted successfully");
            if (!LittleFS.begin()) {
                Serial.println("Failed to mount LittleFS after formatting");
                return;
            }
        } else {
            Serial.println("Failed to format LittleFS");
            return;
        }
    }
}

// Function to save WiFi credentials
void saveCredentials(const char* ssid, const char* password) {
    File file = LittleFS.open("/wificredentials.txt", "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    file.println(ssid);
    file.println(password);
    file.close();
    Serial.println("Credentials saved");
}

// Function to load WiFi credentials
bool loadCredentials(String &ssid, String &password) {
    File file = LittleFS.open("/wificredentials.txt", "r");
    if (!file) {
        Serial.println("Failed to open file for reading");
        return false;
    }

    ssid = file.readStringUntil('\n');
    password = file.readStringUntil('\n');
    file.close();

    ssid.trim();
    password.trim();

    if (ssid.length() == 0 || password.length() == 0) {
        return false;
    }

    return true;
}

// Function to connect to WiFi
void connectToWiFi(const String& ssid, const String& password) {
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        Serial.print(".");
        delay(100);
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Failed to connect to WiFi");
    } else {
        Serial.println("Connected to WiFi");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }
}

void setup() {
    Serial.begin(115200);
    initFileSystem();
    
    // Save the defined credentials
    // Comment out this line after initial run to avoid overwriting credentials
    //saveCredentials("Dialog 4G 044", "c0Deb7c5");

    // Load and connect to WiFi
    String ssid, password;
    if (loadCredentials(ssid, password)) {
        Serial.println("Credentials loaded, attempting to connect");
        connectToWiFi(ssid, password);
    } else {
        Serial.println("Failed to load WiFi credentials");
    }
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        checkForUpdate();
    }
}

void checkForUpdate() {
    Serial.print("Checking for firmware updates... Current version: ");
    Serial.println(FirmwareVer);

    if (FirmwareVersionCheck()) {
        Serial.println("New firmware detected. Updating...");
        firmwareUpdate();
    } else {
        Serial.println("No new firmware available.");
    }
}

void firmwareUpdate() {
    WiFiClientSecure client;
    client.setCACert(rootCACertificate);
    t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            break;

        case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            break;
    }
}

int FirmwareVersionCheck() {
    String payload;
    int httpCode;
    String FirmwareURL = URL_fw_Version;
    FirmwareURL += "?";
    FirmwareURL += String(rand());

    WiFiClientSecure* client = new WiFiClientSecure;

    if (client) {
        client->setCACert(rootCACertificate);
        HTTPClient https;

        if (https.begin(*client, FirmwareURL)) {
            Serial.print("[HTTPS] GET...\n");
            delay(100);
            httpCode = https.GET();
            delay(100);
            if (httpCode == HTTP_CODE_OK) {
                payload = https.getString();
            } else {
                Serial.print("Error Occurred During Version Check: ");
                Serial.println(httpCode);
            }
            https.end();
        }
        delete client;
    }

    if (httpCode == HTTP_CODE_OK) {
        payload.trim();
        if (payload.equals(FirmwareVer)) {
            Serial.printf("\nDevice is already on the latest firmware version: %s\n", FirmwareVer.c_str());
            return 0;
        } else {
            Serial.println(payload);
            Serial.println("New Firmware Detected");
            return 1;
        }
    }
    return 0;
}


