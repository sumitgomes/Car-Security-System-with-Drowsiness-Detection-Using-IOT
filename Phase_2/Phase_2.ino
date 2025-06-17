#include <WiFi.h>
#include <EEPROM.h>
#include <WebServer.h>
#include "SPIFFS.h"
#include "AudioFileSourceSPIFFS.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

#define EEPROM_SIZE 120  
const char *ssid = "ESP32_AP";
const char *password = "12345678";

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

const int LED_PIN = 5;
const int BUZZER_PIN = 4;
const int RELAY_PIN = 18;
bool isSleeping = false;
bool relayTriggered = false;
unsigned long previousMillis = 0;
unsigned long sleepStartTime = 0;
const long blinkInterval = 400;
const long sleepThreshold = 5000;
const long relayActiveTime = 3000;

#define I2S_BCLK  26  
#define I2S_LRC   25  
#define I2S_DOUT  22  

AudioGeneratorWAV *wav;
AudioFileSourceSPIFFS *file;
AudioOutputI2S *out;

void setup() {
    Serial.begin(115200);
    EEPROM.begin(EEPROM_SIZE);

    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RELAY_PIN, HIGH);

    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(ssid, password);
    Serial.println("WiFi AP started!");
    Serial.println(WiFi.softAPIP());

    server.on("/", handleRoot);
    server.on("/update", HTTP_POST, handleUpdate);
    server.on("/getAllContacts", HTTP_GET, handleGetAllContacts);
    server.on("/set_led_status", HTTP_POST, handleSetLedStatus);
    server.on("/get_ip", HTTP_GET, handleGetIP);
    server.on("/health", HTTP_GET, handleHealth);

    server.begin();

    if (!SPIFFS.begin(true)) Serial.println("SPIFFS Mount Failed!");

    out = new AudioOutputI2S();
    out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    out->SetGain(0.8);

    file = new AudioFileSourceSPIFFS("/sound.wav");
    wav = new AudioGeneratorWAV();
}

void loop() {
    server.handleClient();
    unsigned long currentMillis = millis();

    if (isSleeping) {
        if (currentMillis - previousMillis >= blinkInterval) {
            previousMillis = currentMillis;
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        }

        if (!relayTriggered) {
            if (sleepStartTime == 0) {
                sleepStartTime = currentMillis;  
            } else if (currentMillis - sleepStartTime >= sleepThreshold) {  
                triggerRelay();  
            }
        }
    } else {
        digitalWrite(LED_PIN, LOW);
        sleepStartTime = 0;  
        relayTriggered = false;
    }

    if (wav->isRunning()) wav->loop();
}

void handleRoot() {
    server.send(200, "text/plain", "ESP32 Emergency Contact & Sleep Detection API");
}

void handleUpdate() {
    if (server.hasArg("index") && server.hasArg("name") && server.hasArg("number")) {
        int index = server.arg("index").toInt();
        String name = server.arg("name");
        String phoneNumber = server.arg("number");

        if (index >= 1 && index <= 3 && isValidNumber(phoneNumber)) {
            saveContact(index, name, phoneNumber);
            server.send(200, "text/plain", "Contact " + String(index) + " Saved: " + name + ", " + phoneNumber);
        } else {
            server.send(400, "text/plain", "Invalid Input! Index 1-3, Name required, Number must be 10 digits.");
        }
    } else {
        server.send(400, "text/plain", "Missing Parameters! Use index, name & number.");
    }
}

void handleGetAllContacts() {
    String storedContacts = "";
    for (int i = 1; i <= 3; i++) {
        storedContacts += "Contact " + String(i) + ": " + readContact(i) + "\n";
    }
    server.send(200, "text/plain", storedContacts);
}

void handleSetLedStatus() {
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        Serial.println("Received: " + body);

        if (body.indexOf("sleeping") >= 0) {
            isSleeping = true;
            digitalWrite(BUZZER_PIN, HIGH);
            server.send(200, "application/json", "{\"success\": true, \"status\": \"sleeping\"}");
        } else if (body.indexOf("active") >= 0) {
            isSleeping = false;
            digitalWrite(LED_PIN, LOW);
            digitalWrite(BUZZER_PIN, LOW);
            server.send(200, "application/json", "{\"success\": true, \"status\": \"active\"}");
        } else if (body.indexOf("drowsy") >= 0) {
            isSleeping = false;
            digitalWrite(LED_PIN, LOW);
            digitalWrite(BUZZER_PIN, LOW);
            server.send(200, "application/json", "{\"success\": true, \"status\": \"drowsy\"}");
        } else {
            server.send(400, "application/json", "{\"error\": \"Invalid status value\"}");
        }
    } else {
        server.send(400, "application/json", "{\"error\": \"Missing request body\"}");
    }
}

void handleGetIP() {
    server.send(200, "application/json", "{\"ip\": \"" + WiFi.softAPIP().toString() + "\"}");
}

void handleHealth() {
    server.send(200, "application/json", "{\"status\": \"online\"}");
}

void saveContact(int index, String name, String phone) {
    int address = (index - 1) * 40;
    String data = name + "," + phone;
    EEPROM.writeString(address, data);
    EEPROM.commit();
    Serial.println("Saved Contact " + String(index) + ": " + data);
}

String readContact(int index) {
    int address = (index - 1) * 40;
    return EEPROM.readString(address);
}

bool isValidNumber(String num) {
    if (num.length() != 10) return false;
    for (char c : num) {
        if (!isDigit(c)) return false;
    }
    return true;
}

void triggerRelay() {
    Serial.println("Relay triggered!");
    digitalWrite(RELAY_PIN, LOW);
    delay(relayActiveTime);
    digitalWrite(RELAY_PIN, HIGH);
    relayTriggered = true;  

    Serial.println("Playing sound...");
    playWavFile();
}

void playWavFile() {
    file = new AudioFileSourceSPIFFS("/sound.wav");  // Reopen the file for playback
    wav->begin(file, out);  
}
