#include <Servo.h>
#include "HX711.h"
#include <SoftwareSerial.h>

// --- Servo ---
Servo myServo;
Servo myServo2;

// --- HX711 (Load Cell) ---
#define DT 2
#define SCK 3
HX711 scale;
float calibration_factor = 681625.00;
long zero_factor = 121072;

// --- ปุ่มกด ---
#define BUTTON_PIN 4

// --- เซนเซอร์ก้ามปู ---
#define home 8

// --- SoftwareSerial สำหรับ TFT ---
SoftwareSerial tftSerial(10, 11); // RX, TX

void setup() {
  Serial.begin(9600); // Serial สำหรับคุยกับ Raspi
  tftSerial.begin(9600); // Serial สำหรับจอ TFT

  myServo.attach(9);
  myServo2.attach(6);
  pinMode(home, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  myServo.write(90);
  myServo2.write(90);

  // --- เริ่มต้น HX711 ---
  scale.begin(DT, SCK);
  scale.set_scale(calibration_factor);
  scale.tare(); // เคลียร์ค่าเริ่มต้น

}

void loop() {
  // รอการกดปุ่มเพื่อเริ่มชั่งน้ำหนัก
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(200); // debounce

    // เคลียร์ Serial Buffer ก่อนส่งข้อมูล เพื่อป้องกันข้อมูลค้าง
    while (Serial.available()) {
      Serial.read();
    }

    float weight = getWeight() * 1000; // แปลงเป็นกรัม
    
    // ส่งค่าน้ำหนักไปให้ Raspberry Pi แค่ 1 ครั้ง
    Serial.println(weight);

    // รอผลการจำแนกจาก Raspberry Pi (3 ค่า: class, weight, price)
    String input = "";
    unsigned long startTime = millis();
    while (Serial.available() == 0 && (millis() - startTime) < 5000) { // รอสูงสุด 5 วินาที
      // รอให้ข้อมูลมา
    }

    if (Serial.available() > 0) {
      input = Serial.readStringUntil('\n');
      input.trim();
    }

    if (input.length() > 0) {
      Serial.println(input);

      // แยกค่าเป็น class, weight, price
      int classID;
      float finalWeight, price;
      parseRaspiData(input, classID, finalWeight, price);

      // แสดงผลบนจอ TFT
      displayToTFT(classID, finalWeight, price);

      // ถ้า class != 0 -> ทำงาน servo
      if (classID != 0) {
        moveSlot(classID);
      } else {
        Serial.println("Warning: Class Other -> No Servo action");
      }
    } else {
      Serial.println("Error: Did not receive data from Raspberry Pi");
    }
  }
}

float getWeight() {
  scale.set_scale(calibration_factor);
  float reading = scale.get_units(10); // อ่านค่าเฉลี่ย 10 ครั้ง
  if (reading < 0) reading = 0;
  return reading;
}

void parseRaspiData(String data, int &classID, float &weight, float &price) {
  int firstComma = data.indexOf(',');
  int secondComma = data.indexOf(',', firstComma + 1);

  classID = data.substring(0, firstComma).toInt();
  weight = data.substring(firstComma + 1, secondComma).toFloat();
  price = data.substring(secondComma + 1).toFloat();
}

void displayToTFT(int classID, float weight, float price) {
  tftSerial.print(classID);
  tftSerial.print(",");
  tftSerial.print(weight, 2);
  tftSerial.print(",");
  tftSerial.println(price, 2);
}

void moveSlot(int type) {
  int angleStart;
  int angleReturn;

  switch (type) {
    case 1: angleStart = 53; angleReturn = 120; break; // กระป๋อง
    case 2: angleStart = 128; angleReturn = 60; break; // พลาสติก
    case 3: angleStart = 90; angleReturn = 120; break; // แก้ว
    default:
      Serial.println("Error: Invalid type!");
      return;
  }

  Serial.print("Servo1 moving to slot ");
  Serial.println(type);
  myServo.write(angleStart);
  delay(2000);
  myServo.write(90);
  delay(3000);

  Serial.println("Servo2 is working...");
  myServo2.write(50);
  delay(1000);
  myServo2.write(90);
  delay(500);

  Serial.println("Servo1 is returning...");
  myServo.write(angleReturn);

  while (true) {
    if (digitalRead(home) == HIGH) {
      Serial.println("Servo1 found object -> stopping at 90");
      myServo.write(90);
      break;
    }
    delay(10);
  }
}