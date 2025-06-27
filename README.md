# Chroni-Connect

**Smart IoT System for Chronic Disease Monitoring**  
*Supervised by Dr. Mohamed Ahmed Maher*  
**Project Team:** Rafat Ola, Ahmed Sohila, Hisham Baha, Mohamed Karim, Yasser Mohamed, Moamen Nasser, Sabri Waleed 

---

## Overview  
As chronic disease rates continue to climb worldwide—especially in developing countries like Egypt—there’s an urgent need for affordable, accessible smart-health solutions. **Chroni-Connect** is our innovative response: a secure, cloud-enabled IoT platform that empowers real-time monitoring of conditions such as diabetes, hypertension, and cardiovascular disorders via a wearable wristband and a physician dashboard :contentReference[oaicite:5]{index=5}.

## Key Features  
- **Continuous Real-Time Monitoring**  
  Tracks vital signs 24/7—SpO₂, heart rate, body temperature, non-invasive interstitial glucose estimate, and fall detection.  
- **Customizable Patient Interface**  
  Remote access for doctors, MPTC, Patient's close relatives, configurable per-patient dashboards and alerts.  
- **Flexible Subscription Tiers**  
  Basic / Advanced / Family plans to suit different care needs.  
- **Automated Alerts**  
  Instant notifications on dangerous readings or falls via MQTT → Node-RED dashboard.  
- **Rural & Underserved Focus**  
  Designed to bridge healthcare gaps in remote areas, paving the way for AI-driven predictive analytics :contentReference[oaicite:6]{index=6}.

## System Architecture  
1. **Wearable Health Band**  
   - Sensors: MAX30100 (SpO₂ + HR), MAX30205 (Temp), non-invasive glucose module, accelerometer for fall detection.  
2. **Secure Data Pipeline**  
   - Protocol: MQTT over TLS → cloud broker → Node-RED flow.  
3. **Dashboard & Alerts**  
   - Built with Node-RED; simulated and stress-tested in EVE-NG to ensure reliability.  
4. **Future AI Integration**  
   - Roadmap includes machine-learning models for early deterioration prediction.

## Technologies & Tools  
- **Microcontrollers:** ESP8266 / ESP32, Arduino Uno 
- **Sensors:** MAX30100, MAX30205, MPU6050  
- **Display:** OLED (I²C)  
- **Protocols:** MQTT, HTTP
- **Backend & Simulation:** Node-RED, EVE-NG  
- **Cloud:** MQTT-compatible broker (Mosquitto)

## Getting Started  
1. **Clone the repo**  
   ```bash
   git clone https://github.com/SohilaMabrouk/Graduation-project
   cd Graduation-project
