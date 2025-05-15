import cv2
import numpy as np
import time
import serial
from tensorflow.lite.python.interpreter import Interpreter

# === Model Setup ===
interpreter = Interpreter(model_path="model.tflite")
interpreter.allocate_tensors()

with open("labels.txt", "r") as f:
    labels = [line.strip() for line in f.readlines()]

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()
height = input_details[0]['shape'][1]
width = input_details[0]['shape'][2]

CONFIDENCE_THRESHOLD = 0.6
cap = cv2.VideoCapture(0)

# === Serial Setup ===
ser = serial.Serial("/dev/cu.usbmodem0005911822791", 115200)  # <- this is your port
last_sent = ""
send_interval = 1  # seconds
last_send_time = time.time()


while True:
    ret, frame = cap.read()
    if not ret:
        continue

    img = cv2.resize(frame, (width, height))
    input_data = np.expand_dims(img.astype(np.float32) / 255.0, axis=0)

    interpreter.set_tensor(input_details[0]['index'], input_data)
    interpreter.invoke()
    output_data = interpreter.get_tensor(output_details[0]['index'])
    prediction = np.squeeze(output_data)
    max_index = np.argmax(prediction)
    max_confidence = prediction[max_index]

    if max_confidence >= CONFIDENCE_THRESHOLD:
        predicted_label = labels[max_index]
    else:
        predicted_label = "None"

    # Send only if confidence is high enough and it's a known object
    if predicted_label in ["WOOD", "BALLOON", "METAL"]:
        current_time = time.time()
        if predicted_label != last_sent or (current_time - last_send_time) > send_interval:
            message = predicted_label + "\n"
            ser.write(message.encode('utf-8'))
            print(f"[SENT] {message.strip()}")
            last_sent = predicted_label
            last_send_time = current_time

    # Display
    cv2.putText(frame, f"Detected: {predicted_label}", (30, 50),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
    cv2.imshow("Gripper Vision", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
ser.close()
