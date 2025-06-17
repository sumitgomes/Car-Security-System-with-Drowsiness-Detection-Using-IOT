🚗 Car Security System with Drowsiness Detection using IoT

![1f_pages-to-jpg-0001](https://github.com/user-attachments/assets/c5dc057d-1e03-48f9-8c82-7d6263895de6)


An advanced IoT-based car security system that ensures both driver safety and vehicle protection using a combination of drowsiness detection, buzzer/hazard alerts, RFID & biometric access, relay control, and audio playback — all orchestrated through an ESP32-based intelligent control unit.

📌 Project Highlights
💤 Drowsiness Detection Trigger
ESP32 receives an alert when the driver appears drowsy (detected externally via app or camera). It then activates hazard lights, buzzer, relay, and plays an alert message.

🔊 Voice Alert via I2S Audio
Audio .wav files are stored in SPIFFS and played through a speaker connected to an I2S DAC output when a drowsiness event occurs.

🚨 Hazard + Buzzer Alert System
The system turns on a hazard LED and a buzzer for 5 seconds to alert the driver and surrounding vehicles.

🔐 Relay Control
A relay can be used to disable the ignition or trigger external alarms during unsafe events.

📞 Emergency Contacts Storage
The system supports up to 3 emergency contacts, stored securely in EEPROM and manageable through a mobile/web interface.

🌐 ESP32 Wi-Fi Access Point + Web Server
Easily connect your mobile device to configure emergency contacts or manually test alerts.

ProtoType:-![IMG_20250508_103602_157](https://github.com/user-attachments/assets/6cc4366c-469b-4141-a541-a28a2b403e95)


OR

demonstration videos : https://drive.google.com/file/d/169zvuIrPbDb9XweUvsrs5P6hnLPzA1qd/view


🧠 How It Works
Driver's drowsiness is detected externally (via face/eye detection system or mobile app).

App sends HTTP GET request (/alert) to ESP32's static IP (192.168.4.1).

ESP32 triggers:

Hazard light (GPIO 5)

Buzzer (GPIO 4)

Relay (GPIO 18) to cut ignition

Audio alert via I2S using WAV file

After 5 seconds, system resets outputs.

Emergency contacts stored in EEPROM can be fetched or updated via /get and /save.

🛠️ Tech Stack
Component	Details
Microcontroller	ESP32 (Wroom 32 DevKit)
Alert Output	Buzzer, LED (Hazard), Relay
Audio Playback	I2S output via .wav files in SPIFFS
Storage	EEPROM for emergency contacts
Web Interface	ESP32 WebServer (AP Mode)
App Integration	MIT App Inventor for configuration
Language	C++ (Arduino IDE)

⚙️ Pin Configuration
   Function	GPIO Pin
   Buzzer	4
   Hazard LED	5
   Relay (Ignition)	18
   I2S Data	22
   I2S LRC	25
   I2S BCLK	26

🔗 API Endpoints
      Endpoint	Description
      /alert	Triggers buzzer, hazard, relay & audio
      /get Returns stored emergency contacts
      /save	Saves a new contact (name & number)

📸 Screenshots
Add photos or GIFs showing:

System in action (hazard + buzzer on drowsiness)

Audio playback

Web interface

App UI (MIT App Inventor)

🗂️ Folder Structure

📁 /esp32-code
   └── Phase_2.ino
📁 /audio
   └── alert.wav
📁 /ESP_8266
   └── ESP_8266.jpg
📁 /static
   └── background.mp4
📁 /templates
   └── index.html   
python.py
dlib-19.24.99-cp312-cp312-win_amd64.whl
shape_predictor_68_face_landmarks.dat
sound.mp3
README.md
LICENSE

🚀 How to Use
Upload Phase_2.ino to your ESP32 using Arduino IDE.

Connect:

Buzzer to GPIO 4

Hazard LED to GPIO 5

Relay to GPIO 18

I2S speaker (Data: 22, LRC: 25, BCLK: 26)

Format SPIFFS and upload alert.wav using Arduino IDE's data upload tool.

Connect your phone to the Wi-Fi ESP32_AP (Password: 12345678)

Access the web interface or app to trigger drowsiness alerts or update contacts.

📱 Mobile App
A custom Android app (MIT App Inventor) allows:

Viewing & updating emergency contacts

Testing the alert system

Sending drowsiness alert manually

(Optional: Add APK link or QR code)

✅ Applications
Smart Vehicle Safety Systems

Fleet Management Security

Public Transport Driver Monitoring

Accident Prevention Solutions

👤 Author
Sumit Gomes
💼 IoT | Embedded Systems | Security Projects
🔗 LinkedIn • GitHub

📄 License
This project is licensed under the MIT License — feel free to use and contribute!

