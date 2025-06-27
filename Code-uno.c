#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "MAX30100_PulseOximeter.h"

// --- Objects ---
Adafruit_MPU6050 mpu;
PulseOximeter pox;

// --- Pins ---
const int BUTTON_PIN = 4;

// --- Sensor values ---
float accelMagnitude = 0;
float heartRate = 0;
float spo2 = 0;
float glucoseEstimate = 0;
bool hrSpo2Valid = false;

// --- Fall detection ---
enum FallState {
  NORMAL,
  IMPACT_DETECTED,
  IMMOBILITY_CHECK,
  FALL_CONFIRMED
};
FallState currentFallState = NORMAL;

// --- Fall detection tuning ---
const float IMPACT_THRESHOLD_G = 2.8;
const float IMMOBILITY_THRESHOLD_G = 1.3;
const unsigned long IMMOBILITY_DURATION_MS = 1500;
const float RESET_THRESHOLD_G = 1.8;
unsigned long lastFallStateChangeTime = 0;

// --- Timers ---
unsigned long lastMpuUpdate = 0;
const unsigned long MPU_UPDATE_INTERVAL = 50;
unsigned long lastSerialSendTime = 0;
const unsigned long SERIAL_SEND_INTERVAL = 500;

// --- Button handling ---
int lastButtonState = HIGH;
int buttonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 50;

// --- Beat callback ---
void onBeatDetected() {
  // optional
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Wire.begin();

  if (!mpu.begin()) {
    Serial.println(F("MPU6050 not found!"));
    while (1) delay(10);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  if (!pox.begin()) {
    Serial.println(F("MAX30100 not found!"));
    while (1) delay(10);
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_50MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.println(F("Setup complete."));
}

void loop() {
  unsigned long now = millis();

  if (now - lastMpuUpdate >= MPU_UPDATE_INTERVAL) {
    lastMpuUpdate = now;
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    accelMagnitude = sqrt(a.acceleration.x * a.acceleration.x +
                          a.acceleration.y * a.acceleration.y +
                          a.acceleration.z * a.acceleration.z) / 9.80665;
  }

  pox.update();
  heartRate = pox.getHeartRate();
  spo2 = pox.getSpO2();
  hrSpo2Valid = (heartRate > 0 && heartRate < 200 && spo2 > 0 && spo2 <= 100);

  if (hrSpo2Valid) {
    glucoseEstimate = 90 + ((heartRate - 60) * 0.5) - ((100 - spo2) * 0.3);
    glucoseEstimate = constrain(glucoseEstimate, 70, 180);
  } else {
    glucoseEstimate = 0;
  }

  switch (currentFallState) {
    case NORMAL:
      if (accelMagnitude > IMPACT_THRESHOLD_G) {
        currentFallState = IMPACT_DETECTED;
        lastFallStateChangeTime = now;
        Serial.println(F("State: IMPACT_DETECTED"));
      }
      break;

    case IMPACT_DETECTED:
      if (now - lastFallStateChangeTime > 1000) {
        if (accelMagnitude < IMMOBILITY_THRESHOLD_G) {
          currentFallState = IMMOBILITY_CHECK;
          lastFallStateChangeTime = now;
          Serial.println(F("State: IMMOBILITY_CHECK"));
        } else {
          currentFallState = NORMAL;
          Serial.println(F("State: NORMAL (false alarm)"));
        }
      }
      break;

    case IMMOBILITY_CHECK:
      if (accelMagnitude < IMMOBILITY_THRESHOLD_G) {
        if (now - lastFallStateChangeTime > IMMOBILITY_DURATION_MS) {
          currentFallState = FALL_CONFIRMED;
          lastFallStateChangeTime = now;
          Serial.println(F("State: FALL_CONFIRMED"));
        }
      } else {
        currentFallState = NORMAL;
        Serial.println(F("State: NORMAL (movement resumed)"));
      }
      break;

    case FALL_CONFIRMED:
      int reading = digitalRead(BUTTON_PIN);
      if (reading != lastButtonState) {
        lastDebounceTime = now;
      }
      if ((now - lastDebounceTime) > DEBOUNCE_DELAY) {
        if (reading == LOW && buttonState == HIGH) {
          currentFallState = NORMAL;
          Serial.println(F("Button pressed: Alert cleared."));
        }
        buttonState = reading;
      }
      lastButtonState = reading;
      break;
  }

  if (now - lastSerialSendTime >= SERIAL_SEND_INTERVAL) {
    lastSerialSendTime = now;
    Serial.print(F("S,"));
    Serial.print(hrSpo2Valid ? heartRate : 0.0);
    Serial.print(F(","));
    Serial.print(hrSpo2Valid ? spo2 : 0.0);
    Serial.print(F(","));
    Serial.print(glucoseEstimate, 1);
    Serial.print(F(","));
    Serial.print(accelMagnitude, 2);
    Serial.print(F(","));
    Serial.print(static_cast<int>(currentFallState));
    Serial.println();
  }
}
