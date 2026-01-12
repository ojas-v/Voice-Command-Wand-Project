/*
 * PROJECT: ESP32 TinyML Voice Command Wand
 * AUTHOR: Ojas Vaidya
 * HARDWARE: ESP32 (DevKit V1), Analog Mic (MAX9814/9812), 5V Relay, Piezo Buzzer
 * * DESCRIPTION:
 * A "Headless" voice recognition system that uses Edge Impulse to detect keywords.
 * It toggles a relay and sends telemetry data to a Python dashboard over Serial.
 * * DEPENDENCIES:
 * - Requires the Edge Impulse Arduino Library exported from your project:
 * (Download as Arduino Library from Edge Impulse -> Deployment)
 */

#include <voice_wand_inferencing.h> 
#include <Arduino.h>

// PIN DEFINITIONS 
#define MIC_PIN      34
#define RELAY_PIN    26
#define BUZZER_PIN   27
#define LED_PIN      2   // Onboard Blue LED

// SETTINGS 
#define CONFIDENCE_THRESHOLD 0.80
#define SAMPLING_FREQ_HZ     16000 // 16kHz Audio
#define SAMPLING_PERIOD_US   (1000000 / SAMPLING_FREQ_HZ)

// GLOBALS
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
unsigned long lastTelemetry = 0;
bool relayState = false;

void setup() {
  Serial.begin(115200);
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(MIC_PIN, INPUT);

  // STARTUP SEQUENCE
  digitalWrite(RELAY_PIN, LOW);
  
  // Blink LED & Beep to confirm system boot
  for(int i=0; i<3; i++) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }

  Serial.println("SYSTEM READY: LISTENING...");
}

void loop() {
  
  // 1. SENSOR TELEMETRY
  // Sends environment data to dashboard. 
  // NOTE: Using simulated values for testing/demo purposes.
  if (millis() - lastTelemetry > 2000) {
      // Simulate Temp around 24C
      float t = 24.0 + ((random(0, 100) / 100.0) - 0.5); 
      // Simulate Humidity around 60%
      float h = 60.0 + ((random(0, 200) / 100.0) - 2.0);
      
      Serial.print("ENV:");
      Serial.print(t);
      Serial.print(",");
      Serial.println(h);
      
      lastTelemetry = millis();
  }

  // 2. SERIAL REMOTE CONTROL
  if (Serial.available()) {
      char cmd = Serial.read();
      if (cmd == 'T') toggleLight("REMOTE");
  }

  // 3. VOICE INFERENCE-
  
  // Record Audio
  // Using micros() for precise timing (better for AI than delayMicroseconds)
  for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i++) {
      int64_t start_time = micros();
      
      // Raw value conversion
      features[i] = (analogRead(MIC_PIN) - 2048) * 16.0; 

      // Wait until the exact sampling period has passed
      while ((micros() - start_time) < SAMPLING_PERIOD_US);
  }

  // Run Classification
  signal_t signal;
  numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  ei_impulse_result_t result;
  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

  if (res != EI_IMPULSE_OK) {
      Serial.println("ERR: Classifier failed");
      return;
  }

  // Check Results
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
      if (strcmp(result.classification[ix].label, "matrix") == 0) {
          if (result.classification[ix].value > CONFIDENCE_THRESHOLD) {
              Serial.println(">>> KEYWORD DETECTED: MATRIX <<<");
              toggleLight("VOICE");
          }
      }
  }
}

void toggleLight(String source) {
    relayState = !relayState;
    digitalWrite(RELAY_PIN, relayState);
    
    // Feedback: Short Beep + LED Flash
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100); // Keep short to avoid blocking the AI
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);

    Serial.println("ACTION: RELAY_TOGGLED_BY_" + source);
    
    // Small debounce to prevent double-triggering
    delay(500); 
}
