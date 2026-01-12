import streamlit as st
import serial
import time
import datetime
import pandas as pd
import serial.tools.list_ports

# --- PAGE CONFIG ---
st.set_page_config(page_title="IoT Mission Control", page_icon="📡", layout="wide")

# --- CSS STYLING ---
st.markdown("""
    <style>
    .metric-card {
        background-color: #262730;
        padding: 20px;
        border-radius: 10px;
        border-left: 5px solid #00FF41;
        text-align: center;
        margin-bottom: 10px;
    }
    .big-stat { font-size: 36px; font-weight: bold; color: white; }
    .label { font-size: 14px; color: #aaaaaa; }
    </style>
    """, unsafe_allow_html=True)

# --- SESSION STATE INITIALIZATION ---
if 'ser' not in st.session_state:
    st.session_state.ser = None
if 'is_connected' not in st.session_state:
    st.session_state.is_connected = False
if 'temp_history' not in st.session_state:
    st.session_state.temp_history = []
if 'hum_history' not in st.session_state:
    st.session_state.hum_history = []
if 'logs' not in st.session_state:
    st.session_state.logs = []

# --- SIDEBAR: CONNECTION MANAGER ---
st.sidebar.title("🔌 Connection")

# Auto-detect ports
ports = [p.device for p in serial.tools.list_ports.comports()]
selected_port = st.sidebar.selectbox("Select Port", ports, index=0 if ports else None)
baud_rate = st.sidebar.selectbox("Baud Rate", [115200, 9600], index=0)

# Connect / Disconnect Logic
if not st.session_state.is_connected:
    if st.sidebar.button("CONNECT"):
        try:
            st.session_state.ser = serial.Serial(selected_port, baud_rate, timeout=0.1)
            st.session_state.is_connected = True
            st.rerun()
        except Exception as e:
            st.sidebar.error(f"Error: {e}")
else:
    if st.sidebar.button("DISCONNECT"):
        if st.session_state.ser:
            st.session_state.ser.close()
        st.session_state.is_connected = False
        st.rerun()

# --- REMOTE CONTROL ---
st.sidebar.subheader("Remote Override")
if st.session_state.is_connected:
    if st.sidebar.button("🚨 TOGGLE LIGHT"):
        try:
            st.session_state.ser.write(b'T')
            st.session_state.logs.insert(0, f"[{datetime.datetime.now().strftime('%H:%M:%S')}] 📤 Sent: Toggle Command")
        except:
            st.sidebar.error("Failed to send.")

# --- MAIN DASHBOARD LAYOUT ---
st.title("📡 ESP32 Neural Hub")

if not st.session_state.is_connected:
    st.info("Select your ESP32 Port and click CONNECT to start.")
    st.stop()

st.markdown(f"Status: **ONLINE** | Port: `{selected_port}`")

# 1. CREATE PLACEHOLDERS (These stay fixed on the screen)
col1, col2, col3, col4 = st.columns(4)
placeholder_temp = col1.empty()
placeholder_hum = col2.empty()
placeholder_status = col3.empty()
placeholder_last = col4.empty()

st.subheader("Environmental Trends")
chart_placeholder = st.empty()

st.subheader("📝 Event Log")
log_placeholder = st.empty()

# --- THE INFINITE LOOP (This updates data without reloading page) ---
while True:
    if st.session_state.ser and st.session_state.ser.in_waiting > 0:
        try:
            line = st.session_state.ser.readline().decode('utf-8').strip()
            
            if line:
                # 1. PARSE SENSOR DATA: "ENV:24.50,60.00"
                if line.startswith("ENV:"):
                    parts = line.split(":")[1].split(",")
                    if len(parts) == 2:
                        t, h = float(parts[0]), float(parts[1])
                        
                        # Update History
                        st.session_state.temp_history.append(t)
                        st.session_state.hum_history.append(h)
                        
                        # Keep charts clean (Last 50 points)
                        if len(st.session_state.temp_history) > 50:
                            st.session_state.temp_history.pop(0)
                            st.session_state.hum_history.pop(0)
                        
                        # UPDATE UI ELEMENTS DIRECTLY
                        placeholder_temp.markdown(f'<div class="metric-card"><div class="label">TEMP</div><div class="big-stat">{t:.1f}°C</div></div>', unsafe_allow_html=True)
                        placeholder_hum.markdown(f'<div class="metric-card"><div class="label">HUMIDITY</div><div class="big-stat">{h:.0f}%</div></div>', unsafe_allow_html=True)
                        
                        # Update Charts
                        df = pd.DataFrame({"Temp": st.session_state.temp_history, "Hum": st.session_state.hum_history})
                        chart_placeholder.line_chart(df)

                # 2. PARSE AI EVENT: "MATRIX"
                elif "MATRIX" in line:
                    timestamp = datetime.datetime.now().strftime('%H:%M:%S')
                    st.session_state.logs.insert(0, f"[{timestamp}] 🤖 Matrix Detected!")
                    placeholder_status.warning("AI TRIGGERED")

                # 3. PARSE LIGHT EVENT
                elif "LIGHT_TOGGLE" in line:
                    timestamp = datetime.datetime.now().strftime('%H:%M:%S')
                    st.session_state.logs.insert(0, f"[{timestamp}] 💡 Light Switched")
                    placeholder_status.success("SYSTEM READY")

                # Update Logs
                log_placeholder.text_area("Latest Events", "\n".join(st.session_state.logs[:8]), height=200)

        except Exception as e:
            pass
    
    # Small sleep to save CPU, but NO st.rerun()!
    time.sleep(0.05)