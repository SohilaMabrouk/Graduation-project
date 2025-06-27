// Required libraries
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- WiFi & MQTT Config ---
const char* ssid = "ERU ENG";
const char* password = "eru11111";
const char* loginURL = "https://eru.net/login";
const char* loginUsername = "191154";
const char* loginPassword = "6651";

const char* mqtt_server = "10.6.4.1";
WiFiClient espClient;
PubSubClient client(espClient);

// --- Display Config ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- Serial data ---
String serialData = "";
float hr = -1, spo2 = -1, glucose = -1, accel = -1;
int fallState = -1;
bool dataReceived = false;

// --- Buzzer Pin ---
const int BUZZER_PIN = 13;

void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  HTTPClient http;
  http.begin(espClient, loginURL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String postData = "username=" + String(loginUsername) + "&password=" + String(loginPassword);
  http.POST(postData);
  http.end();
}

void reconnectMQTT() {
  while (!client.connected()) {
    client.connect("ESP8266Client");
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Chroni");
  display.println("Connect");
  display.display();

  connectToWiFi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();

  while (Serial.available()) {
    char ch = Serial.read();
    if (ch == '\n') {
      parseAndPublish(serialData);
      serialData = "";
      dataReceived = true;
    } else {
      serialData += ch;
    }
  }

  // Update OLED display with current data
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.print("HR:");
  if (hr >= 0) display.print(hr, 0); else display.print("--");

  display.print(" SpO2:");
  if (spo2 >= 0) display.print(spo2, 0); else display.print("--");

  display.setCursor(0, 10);
  display.print("Glu:");
  if (glucose >= 0) display.print(glucose, 1); else display.print("--");

  display.print(" Acc:");
  if (accel >= 0) display.print(accel, 2); else display.print("--");

  display.setCursor(0, 20);
  if (fallState == 2) {
    display.print("Fall: Checking...");
  } else if (fallState == 3) {
    display.setTextColor((millis() / 500) % 2 == 0 ? BLACK : WHITE, WHITE);
    display.print("!! ALERT !!");
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    display.setTextColor(WHITE);
    display.print("Fall:");
    if (fallState >= 0) display.print(fallState); else display.print("--");
    digitalWrite(BUZZER_PIN, LOW);
  }
  display.display();
}

void parseAndPublish(String data) {
  if (!data.startsWith("S,")) return;
  data.remove(0, 2);

  float parsed_hr, parsed_spo2, parsed_glu, parsed_accel;
  int parsed_fallState;
  int parsed = sscanf(data.c_str(), "%f,%f,%f,%f,%d", &parsed_hr, &parsed_spo2, &parsed_glu, &parsed_accel, &parsed_fallState);

  if (parsed == 5) {
    hr = parsed_hr;
    spo2 = parsed_spo2;
    glucose = parsed_glu;
    accel = parsed_accel;
    fallState = parsed_fallState;

    client.publish("chroniconnect/patient001/hr", String(hr, 1).c_str());
    client.publish("chroniconnect/patient001/spo2", String(spo2, 1).c_str());
    client.publish("chroniconnect/patient001/glu", String(glucose, 1).c_str());
    client.publish("chroniconnect/patient001/accel", String(accel, 2).c_str());

    if (fallState == 3) {
      client.publish("chroniconnect/patient001/alert", "1");
    } else if (fallState == 0) {
      client.publish("chroniconnect/patient001/alert", "state:normal");
    }
    
  }

