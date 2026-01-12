// /*
//  * ESP32 "Nuclear" Hardware Test
//  * Fixes: Slow I2C, New Pins, Forced Drivers
//  */
// #include <U8g2lib.h>
// #include <Wire.h>
// #include <DHT.h>

// 
// #define DHT_PIN 15    // Moved from 4 to 15 (Safer Pin)
// #define BUZZER_PIN 27
// #define RELAY_PIN 26

// SENSOR OBJECTS 
// DHT dht(DHT_PIN, DHT22);

// // --- OLED DRIVER (The "Stubborn Screen" Version) ---
// // We use a software buffer to ensure data integrity
// U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// void setup() {
//   Serial.begin(115200);
//   Serial.println("\n FINAL HARDWARE CHECK ");

//   // 1. Slow Down I2C (Critical for some 1.3" screens)
//   Wire.begin();
//   Wire.setClock(100000); // Drop to 100kHz standard speed

//   // 2. DHT TEST
//   dht.begin();
//   delay(1000); // Give it a second
//   float t = dht.readTemperature();
  
//   if (isnan(t)) {
//     Serial.println("DHT STILL FAILING.");
//     Serial.println("   -> move wire to GPIO 15?");
//     Serial.println("   -> a resistor between VCC and Data?");
//   } else {
//     Serial.print("DHT WORKING! Temp: ");
//     Serial.println(t);
//   }

//   // 3. OLED TEST
//   u8g2.begin();
//   u8g2.setContrast(255); // Force max brightness
  
//   u8g2.clearBuffer();
//   u8g2.setFont(u8g2_font_ncenB14_tr);
//   u8g2.drawStr(5, 20, "OLED FIX");
//   u8g2.drawStr(5, 45, "WORKING?");
//   u8g2.drawFrame(0,0,128,64);
//   u8g2.sendBuffer();
  
//   Serial.println("Display data sent.");
  
//   // 4. BUZZER CLICK
//   pinMode(BUZZER_PIN, OUTPUT);
//   digitalWrite(BUZZER_PIN, HIGH);
//   delay(50);
//   digitalWrite(BUZZER_PIN, LOW);
// }

// void loop() {
//   // Nothing here
// }

/*
 * ESP32 OLED DEFIBRILLATOR
 * Attempts to force-start the screen using Voltage Boosts
 */
#include <U8g2lib.h>
#include <Wire.h>

// PIN DEFINITIONS 
#define SDA_PIN 21
#define SCL_PIN 22
#define BUZZER_PIN 27

// DRIVER 1: Standard 1.3" (SH1106) 
U8G2_SH1106_128X64_NONAME_F_HW_I2C driver1(U8G2_R0, U8X8_PIN_NONE);

// DRIVER 2: The "Voltage Boost" (VCOMH0) 
// This is the magic fixer for dim/black screens
U8G2_SH1106_128X64_VCOMH0_F_HW_I2C driver2(U8G2_R0, U8X8_PIN_NONE);

// DRIVER 3: Standard 0.96" (SSD1306)
// Just in case it's a mislabeled screen
U8G2_SSD1306_128X64_NONAME_F_HW_I2C driver3(U8G2_R0, U8X8_PIN_NONE);

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- OLED DEFIBRILLATOR START ---");
  
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  // ATTEMPT 1: Standard SH1106
  Serial.println("Attempt 1: Standard SH1106...");
  driver1.begin();
  driver1.setContrast(255); // Max Brightness
  driver1.clearBuffer();
  driver1.setFont(u8g2_font_ncenB14_tr);
  driver1.drawStr(10, 40, "MODE 1");
  driver1.drawFrame(0,0,128,64);
  driver1.sendBuffer();
  beep();
  delay(3000);

  // ATTEMPT 2: Voltage Boost (VCOMH0)
  Serial.println("Attempt 2: Voltage Boost (VCOMH0)...");
  driver2.begin();
  driver2.setContrast(255);
  driver2.clearBuffer();
  driver2.setFont(u8g2_font_ncenB14_tr);
  driver2.drawStr(10, 40, "MODE 2!");
  driver2.drawStr(10, 60, "(BOOST)");
  driver2.drawFrame(0,0,128,64);
  driver2.sendBuffer();
  beep();
  delay(3000);

  // ATTEMPT 3: SSD1306 Driver
  Serial.println("Attempt 3: SSD1306 Driver...");
  driver3.begin();
  driver3.setContrast(255);
  driver3.clearBuffer();
  driver3.setFont(u8g2_font_ncenB14_tr);
  driver3.drawStr(10, 40, "MODE 3");
  driver3.drawFrame(0,0,128,64);
  driver3.sendBuffer();
  beep();
  delay(3000);
}

void beep() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(50);
  digitalWrite(BUZZER_PIN, LOW);
}
