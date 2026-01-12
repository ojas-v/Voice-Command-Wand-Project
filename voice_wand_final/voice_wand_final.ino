/*
 * ESP32 "HEADLESS" VOICE AI
 * - Hardware: ESP32, Mic (GPIO 34), Buzzer (GPIO 27), Relay (GPIO 26)
 * - Output: Serial Telemetry for Python Dashboard
 * - Status: Uses Onboard LED (GPIO 2) instead of OLED
 */

#include <voice_wand_inferencing.h> 
#include <Arduino.h>

// --- PINS ---
#define MIC_PIN 34
#define RELAY_PIN 26
#define BUZZER_PIN 27
#define LED_PIN 2      // Onboard Blue LED

// --- SETTINGS ---
#define CONFIDENCE_THRESHOLD 0.85 
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
unsigned long lastTelemetry = 0;

void setup() {
  Serial.begin(115200);
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(MIC_PIN, INPUT);

  // STARTUP SEQUENCE
  digitalWrite(RELAY_PIN, LOW);
  
  // Blink LED & Beep to confirm life
  for(int i=0; i<3; i++) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }

  Serial.println("SYSTEM READY. HEADLESS MODE.");
}

void loop() {
  
  // --- 1. SIMULATED TELEMETRY (For Dashboard Demo) ---
  // Since DHT is dead, we generate realistic data so the graphs look cool.
  if (millis() - lastTelemetry > 2000) {
      // Simulate Temp around 24C +/- 0.5
      float t = 24.0 + ((random(0, 100) / 100.0) - 0.5); 
      // Simulate Humidity around 60% +/- 2
      float h = 60.0 + ((random(0, 200) / 100.0) - 2.0);
      
      Serial.print("ENV:");
      Serial.print(t);
      Serial.print(",");
      Serial.println(h);
      
      lastTelemetry = millis();
  }

  // --- 2. REMOTE CONTROL ---
  if (Serial.available()) {
      char cmd = Serial.read();
      if (cmd == 'T') toggleLight("REMOTE");
  }

  // --- 3. VOICE AI ---
  // Blink LED faintly to show "Listening"
  digitalWrite(LED_PIN, HIGH); 
  
  for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i++) {
      features[i] = (analogRead(MIC_PIN) - 2048) * 16.0;
      delayMicroseconds(45); 
  }
  
  digitalWrite(LED_PIN, LOW); // Done recording

  signal_t signal;
  numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  ei_impulse_result_t result;
  run_classifier(&signal, &result, false);

  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
      if (strcmp(result.classification[ix].label, "matrix") == 0) {
          if (result.classification[ix].value > CONFIDENCE_THRESHOLD) {
              Serial.println(">>> MATRIX DETECTED <<<");
              toggleLight("VOICE");
          }
      }
  }
}

void toggleLight(String source) {
    bool state = !digitalRead(RELAY_PIN);
    digitalWrite(RELAY_PIN, state);
    
    // Feedback: Long Beep + LED Flash
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);

    // Update Dashboard
    Serial.println("DASHBOARD_UPDATE:LIGHT_TOGGLE");
    
    // Debounce
    delay(1000); 
}