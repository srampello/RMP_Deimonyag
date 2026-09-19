#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

// ============================================================
// DEIMONYAG - TEST ESP32-C3 SUPER MINI
// Wi-Fi + LED + botón
//
// LED externo: GPIO9
// Botón: GPIO21 a GND, usando INPUT_PULLUP
//
// PC:
//   SSID: DEIMONYAG
//   Password: deimonyag
//   WebSocket: ws://192.168.4.1:81
// ============================================================

static const char* WIFI_SSID = "DEIMONYAG";
static const char* WIFI_PASSWORD = "deimonyag";

static const uint8_t PIN_LED = 9;
static const uint8_t PIN_BUTTON = 21;
static const bool LED_ACTIVE_HIGH = true;

IPAddress AP_IP(192, 168, 4, 1);
IPAddress AP_GATEWAY(192, 168, 4, 1);
IPAddress AP_SUBNET(255, 255, 255, 0);

WebSocketsServer webSocket(81);

bool ledOn = false;
bool stableButtonPressed = false;
bool lastRawButtonPressed = false;
uint32_t lastButtonChangeMs = 0;
uint32_t lastHeartbeatMs = 0;

static const uint32_t DEBOUNCE_MS = 25;
static const uint32_t HEARTBEAT_MS = 1000;

void writeLed(bool on) {
  ledOn = on;
  const bool level = LED_ACTIVE_HIGH ? on : !on;
  digitalWrite(PIN_LED, level ? HIGH : LOW);
}

void sendJsonToClient(uint8_t client, JsonDocument& doc) {
  String output;
  serializeJson(doc, output);
  webSocket.sendTXT(client, output);
}

void broadcastJson(JsonDocument& doc) {
  String output;
  serializeJson(doc, output);
  webSocket.broadcastTXT(output);
}

void sendHello(uint8_t client) {
  JsonDocument doc;
  doc["type"] = "hello";
  doc["device"] = "Deimonyag ESP32-C3 Test";
  doc["protocol"] = 1;
  doc["mode"] = "TEST";
  sendJsonToClient(client, doc);
}

void sendIoStateToClient(uint8_t client) {
  JsonDocument doc;
  doc["type"] = "io";
  doc["button_pressed"] = stableButtonPressed;
  doc["led_on"] = ledOn;
  sendJsonToClient(client, doc);
}

void broadcastIoState() {
  JsonDocument doc;
  doc["type"] = "io";
  doc["button_pressed"] = stableButtonPressed;
  doc["led_on"] = ledOn;
  broadcastJson(doc);
}

void handleTextMessage(uint8_t client, uint8_t* payload, size_t length) {
  JsonDocument doc;
  if (deserializeJson(doc, payload, length)) {
    return;
  }

  const char* type = doc["type"] | "";

  if (strcmp(type, "hello") == 0) {
    sendHello(client);
    sendIoStateToClient(client);
    return;
  }

  if (strcmp(type, "ping") == 0) {
    JsonDocument response;
    response["type"] = "pong";
    sendJsonToClient(client, response);
    return;
  }

  if (strcmp(type, "mode") == 0) {
    JsonDocument response;
    response["type"] = "mode";
    response["mode"] = "TEST";
    sendJsonToClient(client, response);
    return;
  }

  if (strcmp(type, "led") == 0) {
    writeLed(doc["on"] | false);
    Serial.printf("[LED] %s\n", ledOn ? "ON" : "OFF");
    broadcastIoState();
    return;
  }

  // Para este test simple ignoramos control de motores/EDF y STOP.
  if (strcmp(type, "control") == 0 || strcmp(type, "stop") == 0) {
    return;
  }
}

void webSocketEvent(uint8_t client, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.printf("[WS] PC conectada. Cliente %u\n", client);
      sendHello(client);
      sendIoStateToClient(client);
      break;

    case WStype_DISCONNECTED:
      Serial.printf("[WS] PC desconectada. Cliente %u\n", client);
      break;

    case WStype_TEXT:
      handleTextMessage(client, payload, length);
      break;

    default:
      break;
  }
}

void updateButton() {
  const bool rawPressed = digitalRead(PIN_BUTTON) == LOW;

  if (rawPressed != lastRawButtonPressed) {
    lastRawButtonPressed = rawPressed;
    lastButtonChangeMs = millis();
  }

  if ((millis() - lastButtonChangeMs) >= DEBOUNCE_MS &&
      stableButtonPressed != rawPressed) {
    stableButtonPressed = rawPressed;

    Serial.printf("[BOTON] %s\n", stableButtonPressed ? "PRESIONADO" : "LIBRE");
    broadcastIoState();
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  writeLed(false);
  stableButtonPressed = digitalRead(PIN_BUTTON) == LOW;
  lastRawButtonPressed = stableButtonPressed;

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);

  if (!WiFi.softAP(WIFI_SSID, WIFI_PASSWORD)) {
    Serial.println("ERROR iniciando Access Point");
  }

  Serial.println();
  Serial.println("=== DEIMONYAG TEST ESP32-C3 ===");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);
  Serial.print("Password: ");
  Serial.println(WIFI_PASSWORD);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("WebSocket: ws://192.168.4.1:81");
  Serial.println("LED: GPIO9");
  Serial.println("Botón: GPIO21 -> GND (INPUT_PULLUP)");

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
  updateButton();

  if (millis() - lastHeartbeatMs >= HEARTBEAT_MS) {
    lastHeartbeatMs = millis();
    broadcastIoState();
  }
}
