import cv2
import numpy as np
import time
from tensorflow.lite.python.interpreter import Interpreter

# Load model and labels
interpreter = Interpreter(model_path="model.tflite")
interpreter.allocate_tensors()

with open("labels.txt", "r") as f:
    labels = [line.strip() for line in f.readlines()]

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

# Input shape
height = input_details[0]['shape'][1]
width = input_details[0]['shape'][2]

cap = cv2.VideoCapture(0)

while True:
    ret, frame = cap.read()
    if not ret:
        continue

    # Preprocess input for model
    img = cv2.resize(frame, (width, height))
    input_data = np.expand_dims(img.astype(np.float32) / 255.0, axis=0)

    # Run inference
    interpreter.set_tensor(input_details[0]['index'], input_data)
    interpreter.invoke()
    output_data = interpreter.get_tensor(output_details[0]['index'])
    prediction = np.squeeze(output_data)

    confidence = np.max(prediction)
    predicted_label = labels[np.argmax(prediction)]

    # Determine what to show
    if confidence < 0.85:
        display_text = "No object detected"
    else:
        display_text = f"Detected: {predicted_label} ({confidence:.2f})"

    # Show prediction on screen
    cv2.putText(frame, display_text, (30, 50),
                cv2.FONT_HERSHEY_SIMPLEX, 0.9, (0, 255, 0), 2)
    cv2.imshow("Smart Gripper Vision Demo", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()