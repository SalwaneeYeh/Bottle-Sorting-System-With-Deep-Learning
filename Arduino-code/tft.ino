#include <SoftwareSerial.h>
#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>

// กำหนดพอร์ต SoftwareSerial
SoftwareSerial mySerial(10, 11); // RX, TX (พิน 10 = RX, 11 = TX)

// TFT LCD
MCUFRIEND_kbv tft;
#define LCD_WHITE 0xFFFF
#define LCD_BLACK 0x0000
#define LCD_BLUE 0x001F
#define LCD_RED 0xF800
#define LCD_GREEN 0x07E0

// ตัวแปร
float bottleWeight = 0.0;
int bottleType = 0;
float moneyEarned = 0.0;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  uint16_t id = tft.readID();
  tft.begin(id);
  tft.fillScreen(LCD_BLACK);
  tft.setRotation(1);

  // กำหนดคุณสมบัติข้อความ
 tft.setTextColor(LCD_WHITE, LCD_BLACK);
tft.setTextSize(2); // ขนาดตัวอักษร

// จอกว้างและสูง
int screenW = tft.width();
int screenH = tft.height();

// ----------------- ข้อความบรรทัดแรก -----------------
String line1 = "Ready to Use";
int textSize1 = 4; // ✅ ขนาดใหญ่
int textW1 = line1.length() * 12 * 2; // length * fontWidth * textSize
int x1 = (screenW - textW1) / 2;
int y1 = 120;  // กำหนด Y เองได้

tft.setTextSize(textSize1);
tft.setCursor(x1, y1);
tft.print(line1);

// ----------------- ข้อความบรรทัดสอง -----------------
String line2 = "Place the Bottle -> Press the Button";
int textSize2 = 2; // ✅ ขนาดใหญ่
int textW2 = line2.length() * 6 * 2;
int x2 = (screenW - textW2) / 2;
int y2 = y1 + 40; // เว้นห่างจากบรรทัดแรก

tft.setTextSize(textSize2);
tft.setCursor(x2, y2);
tft.print(line2);



  delay(2000);

  Serial.println("Waiting for data from board 1...");
}

void loop() {
  if (mySerial.available() > 0) {
    String input = mySerial.readStringUntil('\n');
    input.trim();
    
    Serial.print("TFT Received: ");
    Serial.println(input);

    // แยกเป็น 3 ค่า => type, weight, price
    int firstComma = input.indexOf(',');
    int secondComma = input.indexOf(',', firstComma + 1);

    if (firstComma > 0 && secondComma > firstComma) {
      String typeString = input.substring(0, firstComma);
      String weightString = input.substring(firstComma + 1, secondComma);
      String priceString = input.substring(secondComma + 1);

      bottleType = typeString.toInt();
      bottleWeight = weightString.toFloat();
      moneyEarned = priceString.toFloat();

      if (bottleType >= 0 && bottleType <= 3) {
        Serial.print(" Received: Type=");
        Serial.print(bottleType);
        Serial.print(" | Weight=");
        Serial.print(bottleWeight);
        Serial.print(" g | Price=");
        Serial.print(moneyEarned);
        Serial.println(" THB");

        displayData(bottleType, bottleWeight, moneyEarned);
      } else {
        Serial.println("Invalid type received.");
        // แก้ไขลำดับการเรียกใช้: สี, สี, ข้อความ
        displayMessage(LCD_RED, LCD_BLACK, "Invalid type!");
      }
    } else {
      Serial.println("Invalid format.");
      // แก้ไขลำดับการเรียกใช้: สี, สี, ข้อความ
      displayMessage(LCD_RED, LCD_BLACK, "Invalid format");
    }
  }
}

// =================== ฟังก์ชันแสดงผล ===================
void displayMessage(uint16_t textColor, uint16_t bgColor, String message) {
  tft.fillScreen(bgColor);
  tft.setCursor(20, 80);
  tft.setTextColor(textColor);
  tft.setTextSize(4);
  tft.print(message);
}

void displayData(int type, float weight, float price) {
  tft.fillScreen(LCD_BLACK);
  tft.setTextColor(LCD_WHITE);
  tft.setTextSize(4);

   // กำหนดระยะห่างระหว่างบรรทัด (Vertical Spacing) 
  // หาก setTextSize(4) ความสูงตัวอักษรประมาณ 32px เราใช้ 40px เป็นระยะห่าง
  const int LINE_HEIGHT = 40; 
  
  // คำนวณตำแหน่ง Y เริ่มต้นเพื่อให้ข้อมูล 3 บรรทัดอยู่ตรงกลาง
  // สมมติ: จอสูง 320px
  // ความสูงรวมของบล็อกข้อความ (3 บรรทัด * 40px) = 120px
  // Y_start = (tft.height() - 120) / 2
  int y_start = (tft.height() - (3 * LINE_HEIGHT)) / 2; 

  // ป้องกันค่า y_start เป็นลบในกรณีจอเล็ก
  if (y_start < 0) y_start = 0; 


  // ----------------------------------------------------
  // บรรทัดที่ 1: แสดงประเภทขวด (เริ่มต้นที่ y_start)
  tft.setCursor(20, y_start);
  tft.print("Type: ");
  if (type == 1) tft.print("Plastic");
  else if (type == 2) tft.print("Cans");
  else if (type == 3) tft.print("Glass");
  else tft.print("Can't sorted");

  // บรรทัดที่ 2: แสดงน้ำหนัก (เพิ่ม y_start ด้วย LINE_HEIGHT)
  tft.setCursor(20, y_start + LINE_HEIGHT);
  tft.print("weight: ");
  tft.print(weight, 2);
  tft.print(" g");

  // บรรทัดที่ 3: แสดงราคา (เพิ่ม y_start ด้วย 2 * LINE_HEIGHT)
  tft.setCursor(20, y_start + (2 * LINE_HEIGHT));
  tft.setTextColor(LCD_GREEN);
  tft.print("Price: ");
  tft.print(price, 2);
  tft.print(" Bath");
  // ----------------------------------------------------

}