#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// Replace with your Wi-Fi credentials
const char* ssid = "Sumit";
const char* password = "12345678";

// Static IP Configuration
IPAddress local_IP(192, 168, 146, 100); // Static IP for NodeMCU
IPAddress gateway(192, 168, 146, 78);   // Same as your hotspot IP
IPAddress subnet(255, 255, 255, 0);     // Standard subnet mask

WiFiServer server(80); // Start an HTTP server on port 80

// RFID Pins
#define RST_PIN D1
#define SS_PIN D2
#define LED_RED_PIN D3   // Red LED pin
#define LED_GREEN_PIN D4 // Green LED pin
#define SERVO_PIN D8     // Servo pin (Gate control)

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
Servo gateServo;

// Store the last card UID
String lastCardUID = "";

// Servo state
bool isGateOpen = false;

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);
    delay(2000);

    // Initialize Wi-Fi connection with static IP
    WiFi.config(local_IP, gateway, subnet);
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi!");

    // Display the NodeMCU's IP Address
    Serial.print("NodeMCU IP Address: ");
    Serial.println(WiFi.localIP());

    // Display the Gateway (Phone) IP Address
    Serial.print("Gateway (Phone) IP Address: ");
    Serial.println(WiFi.gatewayIP());

    server.begin(); // Start the HTTP server

    // Initialize RFID, servo, and LEDs
    SPI.begin();
    mfrc522.PCD_Init();
    Serial.println("RFID reader initialized...");

    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    digitalWrite(LED_RED_PIN, HIGH);  // Gate closed by default (Red LED ON)
    digitalWrite(LED_GREEN_PIN, LOW); // Green LED OFF by default

    gateServo.attach(SERVO_PIN);
    gateServo.write(0); // Gate initially closed
}

void loop() {
    // Handle NFC scanning
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        handleNFC();
    }

    // Handle app-based control and respond with the IP address
    WiFiClient client = server.available();
    if (client) {
        handleAppControl(client);
    }
}

void handleNFC() {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println("Card UID: " + uid);

    // If the same card is scanned, toggle the gate
    if (uid == lastCardUID) {
        toggleGate();
    } else {
        lastCardUID = uid; // Update last scanned card
        Serial.println("New card detected. Try again!");
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}

void handleAppControl(WiFiClient client) {
    // Read HTTP request
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

    // Parse and handle the request
    if (request.indexOf("/GATE=OPEN") != -1) {
        if (!isGateOpen) toggleGate();
    } else if (request.indexOf("/GATE=CLOSE") != -1) {
        if (isGateOpen) toggleGate();
    } else if (request.indexOf("/IP") != -1) {
        // Send the IP address to the client
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");
        client.println();
        client.println("NodeMCU IP Address: " + WiFi.localIP().toString());
        return;
    }

    // Send HTTP response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("Command Received");
}

void toggleGate() {
    if (isGateOpen) {
        // Close the gate
        gateServo.write(0);              // Move servo to 0 degrees
        digitalWrite(LED_GREEN_PIN, LOW); // Turn OFF Green LED
        digitalWrite(LED_RED_PIN, HIGH);  // Turn ON Red LED
        Serial.println("Gate Closed");
    } else {
        // Open the gate
        gateServo.write(180);           // Move servo to 180 degrees
        digitalWrite(LED_GREEN_PIN, HIGH); // Turn ON Green LED
        digitalWrite(LED_RED_PIN, LOW);   // Turn OFF Red LED
        Serial.println("Gate Open");
    }
    isGateOpen = !isGateOpen; // Toggle gate state
}
