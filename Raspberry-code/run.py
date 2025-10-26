import cv2
import numpy as np
from tflite_runtime.interpreter import Interpreter
import threading
from collections import Counter
from PIL import Image
import time
import serial


# ===== CONFIG =====
model_path = "/home/project/model-v1.1/model-v1.1.tflite"
labels = ['cans', 'glass', 'other', 'plastic']
input_size = 224
num_images = 3
serial_port = "/dev/ttyAMC0"
baud_rate = 9600

# น้ำหนักและราคา
weight_ranges = {"plastic": (13, 30), "glass": (60, 500), "cans": (5, 13), "other": (0, 10)}
price_per_kg = {"cans": 45.0, "plastic": 7.0, "glass": 0.5, "other": 0.0}

# Load TFLite model
interpreter = Interpreter(model_path=model_path)
interpreter.allocate_tensors()
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

# Connect to Arduino
ser = serial.Serial(serial_port, baud_rate, timeout=1)
time.sleep(2)
print(f"Connected to Arduino on {serial_port}")

# ===== CAMERA THREAD =====
class CameraStream:
    def __init__(self, src=0):
        self.cap = cv2.VideoCapture(src)
        self.ret, self.frame = self.cap.read()
        self.lock = threading.Lock()
        self.running = True
        threading.Thread(target=self.update, daemon=True).start()

    def update(self):
        while self.running:
            ret, frame = self.cap.read()
            if ret:
                with self.lock:
                    self.ret, self.frame = ret, frame

    def read(self):
        with self.lock:
            return self.ret, self.frame.copy()

    def release(self):
        self.running = False
        self.cap.release()

cap_stream = CameraStream(0)

# ===== FUNCTIONS =====
def resize_with_padding(image_np, size=(input_size, input_size), fill=(0,0,0)):
    img = Image.fromarray(cv2.cvtColor(image_np, cv2.COLOR_BGR2RGB))
    img.thumbnail(size, Image.Resampling.LANCZOS)
    new = Image.new("RGB", size, fill)
    x = (size[0] - img.size[0]) // 2
    y = (size[1] - img.size[1]) // 2
    new.paste(img, (x, y))
    return cv2.cvtColor(np.array(new), cv2.COLOR_RGB2BGR)

def classify_image(image):
    padded_image = resize_with_padding(image)
    img_rgb = cv2.cvtColor(padded_image, cv2.COLOR_BGR2RGB)
    input_data = np.expand_dims(img_rgb.astype(np.float32)/255.0, axis=0)
    interpreter.set_tensor(input_details[0]['index'], input_data)
    interpreter.invoke()
    output_data = interpreter.get_tensor(output_details[0]['index'])
    idx = np.argmax(output_data)
    return labels[idx], output_data[0][idx]

def read_weight():
    while True:
        line = ser.readline().decode('utf-8').strip()
        if line:
            try:
                weight = float(line)
                print(f"[Arduino] Weight detected: {weight:.2f} g")
                return weight
            except ValueError:
                continue

def calculate_price(material_class, weight):
    price_kg = price_per_kg.get(material_class, 0)
    return round((weight / 1000) * price_kg, 2)

def send_result_to_arduino(material_class, weight, price):
    mapping = {"plastic":1, "cans":2, "glass":3, "other":0}
    number = mapping.get(material_class, 0)
    message = f"{number},{weight},{price}\n"
    ser.write(message.encode())
    print(f"[Raspberry Pi] Sent to Arduino: {message.strip()}")

# ===== MAIN LOOP =====
try:
    while True:
        print("\nWaiting for weight from Arduino...")
        weight = read_weight()

        if weight <= 0:
            continue

        print(f"\nCapturing {num_images} images for classification...")
        predictions = []

        for i in range(num_images):
            ret, frame = cap_stream.read()
            if not ret:
                continue

            label, conf = classify_image(frame)
            predictions.append(label)

        predicted_class = Counter(predictions).most_common(1)[0][0]
        final_class = predicted_class
        warning = False

        # weight validation
        if weight_ranges["glass"][0] <= weight <= weight_ranges["glass"][1]:
            final_class = "glass"
        elif predicted_class in ["plastic","cans"]:
            low, high = weight_ranges[predicted_class]
            if not (low <= weight <= high):
                warning = True
        elif predicted_class == "other":
            final_class = "other"

        price = calculate_price(final_class, weight) if final_class != "other" else 0.0
        print(f"\n[RESULT] Weight: {weight:.2f} g | Model: {predicted_class} | Final: {final_class.upper()}")
        if warning:
            print("[WARNING] Weight out of range but using model result.")

        send_result_to_arduino(final_class, weight, price)

except KeyboardInterrupt:
    print("\nExiting...")

cap_stream.release()
