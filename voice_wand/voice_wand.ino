// ESP32 Audio Streamer - High Speed Version
// Hardware: ESP32 + MAX9814 Microphone
// Connection: MAX9814 OUT -> GPIO 36 (VP)

#define MIC_PIN 34

void setup() {
  // We use 921600 baud to handle the high volume of data (16,000 samples/sec)
  Serial.begin(921600);
  
  // serial connection a moment to stabilize
  delay(1000);
  Serial.println("ESP32 Audio Streamer Ready!");
}

void loop() {
  // Read raw value from Microphone (0-4095)
  int sample = analogRead(MIC_PIN);
  
  // Send value to computer
  Serial.println(sample);
  
  // Short delay to keep sampling rate roughly around 16kHz
  // 60us is usually a safe starting point
  delayMicroseconds(60); 
}
