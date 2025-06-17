import cv2
import numpy as np
import dlib
from imutils import face_utils
from flask import Flask, render_template, Response, jsonify
import pygame
import threading
import requests
import time

# Initialize pygame mixer
pygame.mixer.init()

# Replace with your NodeMCU's IP address
NODEMCU_IP = "http://192.168.4.1"

# Initialize Flask app
app = Flask(__name__)

# Status variables for sound management
is_playing_sound = False
stop_sound = threading.Event()

# Initialize video capture
cap = cv2.VideoCapture(1) #"http://192.168.21.207:8080/video"
cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)  # Reduce buffering
cap.set(cv2.CAP_PROP_FPS, 30)  # Increase FPS

if not cap.isOpened():
    print("Error: Could not open video capture.")
    exit()

# Initialize dlib face detector and shape predictor
detector = dlib.get_frontal_face_detector()
predictor = dlib.shape_predictor("shape_predictor_68_face_landmarks.dat")

# Status variables
sleep = 0
drowsy = 0
active = 0
status = "Active :)"
color = (0, 255, 0)
sleep_start_time = None  # Track when eyes first close

def update_led_on_nodemcu(status):
    try:
        requests.post(f"{NODEMCU_IP}/set_led_status", json={"status": status})
    except Exception as e:
        print(f"Error communicating with NodeMCU: {e}")

# Function to play notification sound
def play_notification_sound():
    global is_playing_sound
    is_playing_sound = True
    pygame.mixer.music.load("sound.mp3")
    pygame.mixer.music.play(-1)  # Loop indefinitely
    stop_sound.wait()
    pygame.mixer.music.stop()
    is_playing_sound = False

def manage_sound(should_play):
    global is_playing_sound
    if should_play and not is_playing_sound:
        stop_sound.clear()
        threading.Thread(target=play_notification_sound, daemon=True).start()
    elif not should_play and is_playing_sound:
        stop_sound.set()

def compute(ptA, ptB):
    return np.linalg.norm(ptA - ptB)

def blinked(a, b, c, d, e, f):
    up = compute(b, d) + compute(c, e)
    down = compute(a, f)
    ratio = up / (2.0 * down)
    return 2 if ratio > 0.25 else 1 if 0.21 < ratio <= 0.25 else 0

# Background thread for drowsiness detection
def drowsiness_detection():
    global sleep, drowsy, active, status, color, sleep_start_time
    while True:
        ret, frame = cap.read()
        if not ret:
            print("Failed to grab frame.")
            continue

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        faces = detector(gray)
        for face in faces:
            landmarks = predictor(gray, face)
            landmarks = face_utils.shape_to_np(landmarks)

            left_blink = blinked(landmarks[36], landmarks[37], landmarks[38], landmarks[41], landmarks[40], landmarks[39])
            right_blink = blinked(landmarks[42], landmarks[43], landmarks[44], landmarks[47], landmarks[46], landmarks[45])

            if left_blink == 0 or right_blink == 0:  # Eyes closed
                if sleep_start_time is None:  # Start timer
                    sleep_start_time = time.time()

                elif time.time() - sleep_start_time >= 1:  # Check if [Variable(Time in second)] passed
                    sleep += 1
                    drowsy = 0
                    active = 0
                    if sleep > 3:
                        status, color = "SLEEPING !!!", (255, 0, 0)
                        update_led_on_nodemcu("sleeping")
                        manage_sound(True)
            else:  # Eyes open, reset timer
                sleep_start_time = None
                sleep = 0
                drowsy = 0
                active += 1
                if active > 3:
                    status, color = "Active :)", (0, 255, 0)
                    update_led_on_nodemcu("active")
                    manage_sound(False)

        time.sleep(0.05)  # Reduced delay for faster response

def generate_frames():
    while True:
        ret, frame = cap.read()
        if not ret:
            print("Failed to grab frame.")
            continue

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        faces = detector(gray)
        for face in faces:
            landmarks = predictor(gray, face)
            landmarks = face_utils.shape_to_np(landmarks)

            cv2.putText(frame, status, (100, 100), cv2.FONT_HERSHEY_SIMPLEX, 1.2, color, 3)
            for (x, y) in landmarks:
                cv2.circle(frame, (x, y), 1, (255, 255, 255), -1)

        ret, buffer = cv2.imencode('.jpg', frame)
        yield (b'--frame\r\n' b'Content-Type: image/jpeg\r\n\r\n' + buffer.tobytes() + b'\r\n\r\n')

@app.route('/')
def index():
    return render_template('index.html', status=status)

@app.route('/video')
def video():
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/status')
def status_api():
    return jsonify(status=status)

def run_flask():
    app.run(host='0.0.0.0', port=5000, threaded=True)

if __name__ == '__main__':
    threading.Thread(target=drowsiness_detection, daemon=True).start()
    threading.Thread(target=run_flask).start()
