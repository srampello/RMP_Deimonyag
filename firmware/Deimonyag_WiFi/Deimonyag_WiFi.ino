#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

// ============================================================
// DEIMONYAG - Wi-Fi / WebSocket / banco / telemetría
// Primera etapa de integración con la aplicación Python.
//
// IMPORTANTE:
// - TEST: la PC puede controlar motores y EDF.
// - TELEMETRY: la PC queda en solo lectura.
// - El PID de carrera NO depende de la PC ni del Wi-Fi.
// ============================================================

// ------------------------------------------------------------
// Wi-Fi
// ------------------------------------------------------------
static const char* WIFI_SSID = "DEIMONYAG";
static const char* WIFI_PASSWORD = "deimonyag"; // mínimo 8 caracteres

IPAddress AP_IP(192, 168, 4, 1);
IPAddress AP_GATEWAY(192, 168, 4, 1);
IPAddress AP_SUBNET(255, 255, 255, 0);

WebSocketsServer webSocket(81);

// ------------------------------------------------------------
// Protocolo
// ------------------------------------------------------------
static const uint8_t PROTOCOL_VERSION = 1;

enum RobotMode {
  MODE_TEST,
  MODE_TELEMETRY
};

RobotMode robotMode = MODE_TEST;

// ------------------------------------------------------------
// GPIO - configuración actual Deimonyag
// ------------------------------------------------------------
// MUX CD74HC4067
static const uint8_t PIN_MUX_S3  = 0;
static const uint8_t PIN_MUX_S2  = 1;
static const uint8_t PIN_MUX_S1  = 2;
static const uint8_t PIN_MUX_S0  = 3;
static const uint8_t PIN_MUX_SIG = 4;

// IFX9201SG
static const uint8_t PIN_MOTOR_LEFT_PWM   = 5;
static const uint8_t PIN_MOTOR_LEFT_DIR   = 6;
static const uint8_t PIN_MOTOR_RIGHT_PWM  = 7;
static const uint8_t PIN_MOTOR_RIGHT_DIR  = 8;

// EDF27 / ESC
static const uint8_t PIN_EDF = 10;

// ------------------------------------------------------------
// PWM
// Arduino-ESP32 core 3.x: ledcAttach(pin, freq, resolution)
// ESP32-C3 admite hasta 14 bits de resolución LEDC.
// ------------------------------------------------------------
static const uint32_t MOTOR_PWM_FREQ = 20000;
static const uint8_t MOTOR_PWM_BITS = 8;

static const uint32_t ESC_PWM_FREQ = 50;
static const uint8_t ESC_PWM_BITS = 14;
static const uint16_t ESC_MIN_US = 1000;
static const uint16_t ESC_MAX_US = 2000;

// Cambiar a true si físicamente un motor queda invertido.
static const bool MOTOR_LEFT_INVERT = false;
static const bool MOTOR_RIGHT_INVERT = false;

// ------------------------------------------------------------
// Frecuencias de envío
// ------------------------------------------------------------
static const uint32_t TEST_SENSOR_PERIOD_US = 20000; // 50 Hz
static const uint32_t TELEMETRY_PERIOD_US   = 10000; // 100 Hz
static const uint32_t TEST_WATCHDOG_MS      = 500;

uint32_t lastTestSensorSendUs = 0;
uint32_t lastTelemetrySendUs = 0;
uint32_t lastPcPacketMs = 0;

// ------------------------------------------------------------
// Estado de actuadores manuales
// ------------------------------------------------------------
int16_t manualMotorLeft = 0;
int16_t manualMotorRight = 0;
uint8_t manualEdf = 0;

// ------------------------------------------------------------
// Snapshot de telemetría de carrera
// Este bloque será alimentado por el futuro código PID real.
// ------------------------------------------------------------
struct RaceTelemetry {
  int32_t position = 0;
  float error = 0.0f;
  float p = 0.0f;
  float d = 0.0f;
  float pid = 0.0f;
  int16_t motorLeft = 0;
  int16_t motorRight = 0;
  uint8_t edf = 0;
  uint32_t loopUs = 0;
  bool lineLost = false;
  bool running = false;
};

RaceTelemetry raceTelemetry;
uint16_t sensorRaw[12] = {0};

// ============================================================
// Utilidades
// ============================================================

const char* modeToString(RobotMode mode) {
  return mode == MODE_TEST ? "TEST" : "TELEMETRY";
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

void sendError(uint8_t client, const char* message) {
  JsonDocument doc;
  doc["type"] = "error";
  doc["message"] = message;
  sendJsonToClient(client, doc);
}

void sendHello(uint8_t client) {
  JsonDocument doc;
  doc["type"] = "hello";
  doc["device"] = "Deimonyag";
  doc["protocol"] = PROTOCOL_VERSION;
  doc["mode"] = modeToString(robotMode);
  sendJsonToClient(client, doc);
}

// ============================================================
// Sensores
// ============================================================

void selectMuxChannel(uint8_t channel) {
  digitalWrite(PIN_MUX_S0, (channel >> 0) & 0x01);
  digitalWrite(PIN_MUX_S1, (channel >> 1) & 0x01);
  digitalWrite(PIN_MUX_S2, (channel >> 2) & 0x01);
  digitalWrite(PIN_MUX_S3, (channel >> 3) & 0x01);
}

void readSensorsRaw() {
  for (uint8_t i = 0; i < 12; i++) {
    selectMuxChannel(i);
    delayMicroseconds(4);
    sensorRaw[i] = analogRead(PIN_MUX_SIG);
  }
}

// ============================================================
// Motores
// ============================================================

void setMotor(uint8_t pwmPin, uint8_t dirPin, int16_t value, bool invert) {
  value = constrain(value, -255, 255);

  bool forward = value >= 0;
  if (invert) {
    forward = !forward;
  }

  digitalWrite(dirPin, forward ? HIGH : LOW);
  ledcWrite(pwmPin, abs(value));
}

void applyManualMotors() {
  setMotor(PIN_MOTOR_LEFT_PWM, PIN_MOTOR_LEFT_DIR, manualMotorLeft, MOTOR_LEFT_INVERT);
  setMotor(PIN_MOTOR_RIGHT_PWM, PIN_MOTOR_RIGHT_DIR, manualMotorRight, MOTOR_RIGHT_INVERT);
}

// ============================================================
// EDF27 / ESC
// ============================================================

uint32_t pulseUsToDuty(uint16_t pulseUs) {
  const uint32_t maxDuty = (1UL << ESC_PWM_BITS) - 1UL;
  return (uint32_t)(((uint64_t)pulseUs * maxDuty) / 20000ULL);
}

void writeEscPulseUs(uint16_t pulseUs) {
  pulseUs = constrain(pulseUs, ESC_MIN_US, ESC_MAX_US);
  ledcWrite(PIN_EDF, pulseUsToDuty(pulseUs));
}

void setEdfPercent(uint8_t percent) {
  percent = constrain(percent, 0, 100);
  uint16_t pulseUs = map(percent, 0, 100, ESC_MIN_US, ESC_MAX_US);
  writeEscPulseUs(pulseUs);
}

// ============================================================
// Seguridad banco
// ============================================================

void stopManualActuators() {
  manualMotorLeft = 0;
  manualMotorRight = 0;
  manualEdf = 0;

  applyManualMotors();
  setEdfPercent(0);
}

bool controlAllowed() {
  return robotMode == MODE_TEST && !raceTelemetry.running;
}

void updateWatchdog() {
  if (robotMode != MODE_TEST) {
    return;
  }

  if (millis() - lastPcPacketMs > TEST_WATCHDOG_MS) {
    if (manualMotorLeft != 0 || manualMotorRight != 0 || manualEdf != 0) {
      stopManualActuators();
    }
  }
}

// ============================================================
// Salida de datos
// ============================================================

void sendSensorPacket() {
  readSensorsRaw();

  JsonDocument doc;
  doc["type"] = "sensors";
  doc["time_ms"] = millis();

  JsonArray sensors = doc["sensors"].to<JsonArray>();
  for (uint8_t i = 0; i < 12; i++) {
    sensors.add(sensorRaw[i]);
  }

  broadcastJson(doc);
}

void sendTelemetryPacket() {
  readSensorsRaw();

  JsonDocument doc;
  doc["type"] = "telemetry";
  doc["time_ms"] = millis();

  JsonArray sensors = doc["sensors"].to<JsonArray>();
  for (uint8_t i = 0; i < 12; i++) {
    sensors.add(sensorRaw[i]);
  }

  doc["position"] = raceTelemetry.position;
  doc["error"] = raceTelemetry.error;
  doc["p"] = raceTelemetry.p;
  doc["d"] = raceTelemetry.d;
  doc["pid"] = raceTelemetry.pid;
  doc["motor_left"] = raceTelemetry.motorLeft;
  doc["motor_right"] = raceTelemetry.motorRight;
  doc["edf"] = raceTelemetry.edf;
  doc["loop_us"] = raceTelemetry.loopUs;
  doc["line_lost"] = raceTelemetry.lineLost;
  doc["running"] = raceTelemetry.running;

  broadcastJson(doc);
}

// ============================================================
// Punto de integración con el futuro PID real
// Llamar a esta función desde el loop de control del seguidor.
// ============================================================

void publishRaceTelemetry(
  int32_t position,
  float error,
  float proportional,
  float derivative,
  float pid,
  int16_t motorLeft,
  int16_t motorRight,
  uint8_t edf,
  uint32_t loopUs,
  bool lineLost,
  bool running
) {
  raceTelemetry.position = position;
  raceTelemetry.error = error;
  raceTelemetry.p = proportional;
  raceTelemetry.d = derivative;
  raceTelemetry.pid = pid;
  raceTelemetry.motorLeft = motorLeft;
  raceTelemetry.motorRight = motorRight;
  raceTelemetry.edf = edf;
  raceTelemetry.loopUs = loopUs;
  raceTelemetry.lineLost = lineLost;
  raceTelemetry.running = running;

  // Si comienza una carrera, cualquier mando manual queda invalidado.
  if (running && robotMode == MODE_TEST) {
    stopManualActuators();
    robotMode = MODE_TELEMETRY;
  }
}

// ============================================================
// Protocolo WebSocket
// ============================================================

void handleTextMessage(uint8_t client, uint8_t* payload, size_t length) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error) {
    sendError(client, "invalid_json");
    return;
  }

  const char* type = doc["type"] | "";
  lastPcPacketMs = millis();

  if (strcmp(type, "hello") == 0) {
    sendHello(client);
    return;
  }

  if (strcmp(type, "ping") == 0) {
    JsonDocument response;
    response["type"] = "pong";
    sendJsonToClient(client, response);
    return;
  }

  if (strcmp(type, "mode") == 0) {
    const char* requested = doc["mode"] | "";

    if (strcmp(requested, "TELEMETRY") == 0) {
      stopManualActuators();
      robotMode = MODE_TELEMETRY;

      JsonDocument response;
      response["type"] = "mode";
      response["mode"] = "TELEMETRY";
      sendJsonToClient(client, response);
      return;
    }

    if (strcmp(requested, "TEST") == 0) {
      if (raceTelemetry.running) {
        sendError(client, "test_locked_while_running");
        return;
      }

      robotMode = MODE_TEST;
      stopManualActuators();

      JsonDocument response;
      response["type"] = "mode";
      response["mode"] = "TEST";
      sendJsonToClient(client, response);
      return;
    }

    sendError(client, "invalid_mode");
    return;
  }

  if (strcmp(type, "stop") == 0) {
    if (!controlAllowed()) {
      sendError(client, "control_locked_in_telemetry");
      return;
    }

    stopManualActuators();
    return;
  }

  if (strcmp(type, "control") == 0) {
    if (!controlAllowed()) {
      sendError(client, "control_locked_in_telemetry");
      return;
    }

    if (doc["motor_left"].is<int>()) {
      manualMotorLeft = constrain(doc["motor_left"].as<int>(), -255, 255);
    }

    if (doc["motor_right"].is<int>()) {
      manualMotorRight = constrain(doc["motor_right"].as<int>(), -255, 255);
    }

    if (doc["edf"].is<int>()) {
      manualEdf = constrain(doc["edf"].as<int>(), 0, 100);
    }

    applyManualMotors();
    setEdfPercent(manualEdf);
    return;
  }

  sendError(client, "unknown_message_type");
}

void webSocketEvent(uint8_t client, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.printf("[WS] Cliente %u conectado\n", client);
      sendHello(client);
      lastPcPacketMs = millis();
      break;

    case WStype_DISCONNECTED:
      Serial.printf("[WS] Cliente %u desconectado\n", client);
      // En TEST, una desconexión debe detener el banco.
      // En TELEMETRY, la carrera no se modifica.
      if (robotMode == MODE_TEST) {
        stopManualActuators();
      }
      break;

    case WStype_TEXT:
      handleTextMessage(client, payload, length);
      break;

    default:
      break;
  }
}

// ============================================================
// Setup / Loop
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(PIN_MUX_S0, OUTPUT);
  pinMode(PIN_MUX_S1, OUTPUT);
  pinMode(PIN_MUX_S2, OUTPUT);
  pinMode(PIN_MUX_S3, OUTPUT);
  pinMode(PIN_MUX_SIG, INPUT);

  pinMode(PIN_MOTOR_LEFT_DIR, OUTPUT);
  pinMode(PIN_MOTOR_RIGHT_DIR, OUTPUT);

  analogReadResolution(12);

  if (!ledcAttach(PIN_MOTOR_LEFT_PWM, MOTOR_PWM_FREQ, MOTOR_PWM_BITS)) {
    Serial.println("ERROR: PWM motor izquierdo");
  }

  if (!ledcAttach(PIN_MOTOR_RIGHT_PWM, MOTOR_PWM_FREQ, MOTOR_PWM_BITS)) {
    Serial.println("ERROR: PWM motor derecho");
  }

  if (!ledcAttach(PIN_EDF, ESC_PWM_FREQ, ESC_PWM_BITS)) {
    Serial.println("ERROR: PWM EDF/ESC");
  }

  stopManualActuators();

  // Mantener señal mínima para el armado normal del ESC.
  setEdfPercent(0);
  delay(1500);

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);

  if (!WiFi.softAP(WIFI_SSID, WIFI_PASSWORD)) {
    Serial.println("ERROR iniciando Access Point");
  }

  Serial.println();
  Serial.println("=== DEIMONYAG Wi-Fi ===");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("WebSocket: ws://192.168.4.1:81");

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  lastPcPacketMs = millis();
}

void loop() {
  webSocket.loop();
  updateWatchdog();

  const uint32_t nowUs = micros();

  if (robotMode == MODE_TEST) {
    if ((uint32_t)(nowUs - lastTestSensorSendUs) >= TEST_SENSOR_PERIOD_US) {
      lastTestSensorSendUs = nowUs;
      sendSensorPacket();
    }
  } else {
    if ((uint32_t)(nowUs - lastTelemetrySendUs) >= TELEMETRY_PERIOD_US) {
      lastTelemetrySendUs = nowUs;
      sendTelemetryPacket();
    }
  }

  // ----------------------------------------------------------
  // FUTURO:
  // Aquí se integrará el loop real del seguidor.
  // Ese loop calculará posición/error/P/D/PID y llamará a:
  // publishRaceTelemetry(...)
  // ----------------------------------------------------------
}
