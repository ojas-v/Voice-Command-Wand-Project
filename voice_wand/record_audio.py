import serial
import wave
import struct
import time
import os

# CONFIGURATION 
COM_PORT = 'COM4'       # ESP32 Port
BAUD_RATE = 921600
SAMPLE_RATE = 16000     # Must match ESP32
DURATION = 1            # Seconds per sample
OUTPUT_FOLDER = "datasets"
# ---------------------

def record_sample(label, filename):
    try:
        ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
        print(f"Opening {COM_PORT}...")
        time.sleep(2) # Wait for connection to stabilize
        
        print(f"Recording: {label} ({DURATION}s)... GO!")
        
        audio_data = []
        start_time = time.time()
        
        # Flush old data
        ser.reset_input_buffer()
        
        while (time.time() - start_time) < DURATION:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line.isdigit():
                    val = int(line)
                    # Center the audio (0-4095 -> -2048 to 2048)
                    val = val - 2048 
                    # Scale to 16-bit PCM range
                    val = val * 16 
                    audio_data.append(val)
        
        ser.close()
        
        # Save to WAV
        if not os.path.exists(OUTPUT_FOLDER):
            os.makedirs(OUTPUT_FOLDER)
            
        full_path = os.path.join(OUTPUT_FOLDER, filename)
        
        with wave.open(full_path, 'w') as wav_file:
            wav_file.setnchannels(1)      # Mono
            wav_file.setsampwidth(2)      # 2 bytes (16-bit)
            wav_file.setframerate(SAMPLE_RATE)
            wav_file.writeframes(struct.pack('<' + ('h' * len(audio_data)), *audio_data))
            
        print(f"Saved: {full_path} ({len(audio_data)} samples)")

    except Exception as e:
        print(f"Error: {e}")
        if 'ser' in locals() and ser.is_open:
            ser.close()

# MAIN MENU 
while True:
    print("\n DATA COLLECTOR ")
    print("1. Record 'MATRIX'")
    print("2. Record 'STOP'")
    print("3. Record 'NOISE'")
    print("4. Exit")
    choice = input("Select: ")
    
    if choice == '1':
        for i in range(30):
            input(f"Press Enter to record MATRIX sample #{i+1}...")
            record_sample("Matrix", f"matrix.{i+1}.wav")
    elif choice == '2':
        for i in range(30):
            input(f"Press Enter to record STOP sample #{i+1}...")
            record_sample("Stop", f"stop.{i+1}.wav")
    elif choice == '3':
        for i in range(30):
            input(f"Press Enter to record NOISE sample #{i+1}...")
            record_sample("Noise", f"noise.{i+1}.wav")
    elif choice == '4':
        break
