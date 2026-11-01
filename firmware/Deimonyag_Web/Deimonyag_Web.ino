// ============================================================
// DEIMONYAG - Interfaz web de pruebas y PID en tiempo real
// RMP Robotics
//
// ESP32-C3 Super Mini
//  - Crea la red WiFi DEIMONYAG y sirve la interfaz en http://192.168.4.1
//  - WebSocket en el puerto 81 para datos en vivo y comandos
//  - Pestanas: Motores, Sensores, Boton, MicroStart, PID
//
// Base de control: Holdmoss (campeon 2025) -> PID + deteccion de recta
// + arranque suave, adaptado a 12 sensores (centro 5500).
//
// Librerias (Library Manager):
//  - WebSockets (Markus Sattler / links2004)
//  - ArduinoJson 7.x (Benoit Blanchon)
// Placa: ESP32C3 Dev Module, "USB CDC On Boot: Enabled"
// ============================================================

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>

#include "QTRMux.h"
#include "web_page.h"
#include "logos.h"

#define FIRMWARE_VERSION "web-1.0"

// ------------------------------------------------------------
// WiFi
// ------------------------------------------------------------
const char *WIFI_SSID     = "DEIMONYAG";
const char *WIFI_PASSWORD = "deimonyag";   // minimo 8 caracteres
IPAddress AP_IP(192, 168, 4, 1);
IPAddress AP_MASK(255, 255, 255, 0);

WebServer        server(80);
WebSocketsServer ws(81);

// ------------------------------------------------------------
// Pines
// ------------------------------------------------------------
#define PIN_MUX_S0   3
#define PIN_MUX_S1   2
#define PIN_MUX_S2   1
#define PIN_MUX_S3   0
#define PIN_MUX_SIG  4

#define PIN_MOTOR_IZQ_PWM  5
#define PIN_MOTOR_IZQ_DIR  6
#define PIN_MOTOR_DER_PWM  7
#define PIN_MOTOR_DER_DIR  8

#define PIN_LED        9
#define PIN_EDF       10
#define PIN_MICROSTART 20
#define PIN_BOTON     21

// Cambiar a true si un motor gira al reves
#define MOTOR_IZQ_INVERTIDO false
#define MOTOR_DER_INVERTIDO false

// ------------------------------------------------------------
// PWM
// ------------------------------------------------------------
#define MOTOR_PWM_FREQ  20000
#define MOTOR_PWM_BITS  8
#define ESC_PWM_FREQ    50
#define ESC_PWM_BITS    14
#define ESC_MIN_US      1000
#define ESC_MAX_US      2000

// ------------------------------------------------------------
// Sensores (mapeo real de la placa: Q1..Q12 en Y2..Y13)
// ------------------------------------------------------------
#define NUM_SENSORS             12
#define NUM_SAMPLES_PER_SENSOR  3
#define MUX_SETTLE_US           5
#define CENTRO_LINEA            ((NUM_SENSORS - 1) * 1000 / 2)   // 5500

const uint8_t muxSelPins[4] = {PIN_MUX_S0, PIN_MUX_S1, PIN_MUX_S2, PIN_MUX_S3};
const uint8_t muxChannels[NUM_SENSORS] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

QTRMux qtr(muxSelPins, PIN_MUX_SIG, muxChannels, NUM_SENSORS,
           NUM_SAMPLES_PER_SENSOR, MUX_SETTLE_US);

uint16_t sensorRaw[NUM_SENSORS];
uint16_t sensorCal[NUM_SENSORS];
int      position = CENTRO_LINEA;

// ------------------------------------------------------------
// Configuracion (se guarda en flash con "Guardar")
// Valores iniciales: los de Holdmoss reescalados al error de
// 12 sensores (+-5500 en vez de +-3500 -> x0.64)
// ------------------------------------------------------------
#define CFG_VERSION 1

struct Config {
  uint8_t  version;
  float    kp;
  float    kd;
  float    ki;
  int16_t  velBase;
  int16_t  velRecta;
  bool     usarRecta;
  int16_t  umbralRecta;
  int16_t  tiempoRecta;      // ms
  int32_t  limiteI;
  bool     arranqueSuave;
  uint8_t  edfPid;           // % de succion durante el PID
  uint8_t  tipoLinea;        // 1 blanca, 0 negra
  uint8_t  accionBoton;      // 0 nada, 1 calibrar, 2 iniciar/detener PID
  bool     microstartArmado;
};

Config cfg;

void configPorDefecto() {
  cfg.version          = CFG_VERSION;
  cfg.kp               = 0.024f;
  cfg.kd               = 0.17f;
  cfg.ki               = 0.00013f;
  cfg.velBase          = 90;
  cfg.velRecta         = 110;
  cfg.usarRecta        = true;
  cfg.umbralRecta      = 600;
  cfg.tiempoRecta      = 150;
  cfg.limiteI          = 18000;
  cfg.arranqueSuave    = true;
  cfg.edfPid           = 0;
  cfg.tipoLinea        = 1;
  cfg.accionBoton      = 0;
  cfg.microstartArmado = false;
}

Preferences prefs;

// ------------------------------------------------------------
// Estado general
// ------------------------------------------------------------
enum Estado : uint8_t { EST_LIBRE = 0, EST_CALIBRANDO = 1, EST_PID = 2 };
enum Origen : uint8_t { ORG_WEB = 0, ORG_BOTON = 1, ORG_MICROSTART = 2 };

Estado estado = EST_LIBRE;
Origen origenPid = ORG_WEB;
const char *motivoParada = "";

// Manual (pestana Motores)
int16_t manualIzq = 0;
int16_t manualDer = 0;
uint8_t edfActual = 0;
int16_t salidaIzq = 0;    // lo que realmente se aplica (manual o PID)
int16_t salidaDer = 0;

// Calibracion
uint32_t calInicioMs = 0;
uint32_t calDuracionMs = 5000;
bool     calGirando = false;
int16_t  calVelGiro = 60;

// PID
int32_t P = 0, D = 0, LAST = 0;
int32_t I = 0;
float   correccion = 0;
int16_t velActual = 0;     // velocidad objetivo actual (base o recta)
int16_t velRampa = 0;      // arranque suave
uint32_t ultimoRampaMs = 0;
bool     enRecta = false;
bool     rectaActiva = false;
uint32_t tiempoRecta = 0;

// Entradas
bool     botonPresionado = false;
bool     botonAnterior = false;
uint32_t botonCambioMs = 0;
uint32_t contadorBoton = 0;
bool     microstart = false;
bool     microstartAnterior = false;
uint32_t microstartCambioMs = 0;
uint32_t contadorMicrostart = 0;

// LED: 0 auto, 1 encendido, 2 apagado
uint8_t modoLed = 0;

// Comunicacion
uint8_t  clientes = 0;
uint32_t ultimoMensajeMs = 0;
uint32_t ultimoEnvioMs = 0;
#define WATCHDOG_MANUAL_MS  600     // sin mensajes -> motores manuales a 0
#define WATCHDOG_PID_WEB_MS 1500    // PID iniciado desde la web
#define PERIODO_ENVIO_LIBRE 40      // 25 Hz
#define PERIODO_ENVIO_PID   20      // 50 Hz

// Rendimiento
uint32_t loopUs = 0;
uint32_t loopsPorSeg = 0;
uint32_t contLoops = 0;
uint32_t ultimoConteoMs = 0;

// ============================================================
// Motores y EDF
// ============================================================
void setMotor(uint8_t pinPwm, uint8_t pinDir, int16_t v, bool invertido) {
  v = constrain(v, -255, 255);
  bool adelante = v >= 0;
  if (invertido) adelante = !adelante;
  digitalWrite(pinDir, adelante ? HIGH : LOW);
  ledcWrite(pinPwm, abs(v));
}

void setMotores(int16_t izq, int16_t der) {
  salidaIzq = constrain(izq, -255, 255);
  salidaDer = constrain(der, -255, 255);
  setMotor(PIN_MOTOR_IZQ_PWM, PIN_MOTOR_IZQ_DIR, salidaIzq, MOTOR_IZQ_INVERTIDO);
  setMotor(PIN_MOTOR_DER_PWM, PIN_MOTOR_DER_DIR, salidaDer, MOTOR_DER_INVERTIDO);
}

void setEdf(uint8_t porcentaje) {
  edfActual = constrain(porcentaje, 0, 100);
  uint16_t us = map(edfActual, 0, 100, ESC_MIN_US, ESC_MAX_US);
  const uint32_t maxDuty = (1UL << ESC_PWM_BITS) - 1UL;
  ledcWrite(PIN_EDF, (uint32_t)(((uint64_t)us * maxDuty) / 20000ULL));
}

void detenerTodo(const char *motivo) {
  estado = EST_LIBRE;
  manualIzq = 0;
  manualDer = 0;
  setMotores(0, 0);
  setEdf(0);
  motivoParada = motivo;
}

// ============================================================
// Flash (NVS)
// ============================================================
void cargarFlash() {
  configPorDefecto();
  prefs.begin("deimonyag", true);
  if (prefs.getBytesLength("cfg") == sizeof(Config)) {
    Config tmp;
    prefs.getBytes("cfg", &tmp, sizeof(Config));
    if (tmp.version == CFG_VERSION) cfg = tmp;
  }
  if (prefs.getBytesLength("cmin") == sizeof(uint16_t) * NUM_SENSORS &&
      prefs.getBytesLength("cmax") == sizeof(uint16_t) * NUM_SENSORS) {
    uint16_t mn[NUM_SENSORS], mx[NUM_SENSORS];
    prefs.getBytes("cmin", mn, sizeof(mn));
    prefs.getBytes("cmax", mx, sizeof(mx));
    qtr.setCalibration(mn, mx);
  }
  prefs.end();
  cfg.microstartArmado = false;   // nunca arrancar armado tras un reset
}

void guardarConfig() {
  prefs.begin("deimonyag", false);
  Config tmp = cfg;
  tmp.microstartArmado = false;
  prefs.putBytes("cfg", &tmp, sizeof(Config));
  prefs.end();
}

void guardarCalibracion() {
  prefs.begin("deimonyag", false);
  prefs.putBytes("cmin", qtr.calibratedMinimum, sizeof(uint16_t) * NUM_SENSORS);
  prefs.putBytes("cmax", qtr.calibratedMaximum, sizeof(uint16_t) * NUM_SENSORS);
  prefs.end();
}

// ============================================================
// Calibracion
// ============================================================
void iniciarCalibracion(uint32_t ms, bool girar, int16_t vel) {
  if (estado == EST_PID) return;
  manualIzq = manualDer = 0;
  setMotores(0, 0);
  setEdf(0);
  qtr.resetCalibration();
  calDuracionMs = constrain(ms, 1000UL, 15000UL);
  calGirando = girar;
  calVelGiro = constrain(vel, 20, 150);
  calInicioMs = millis();
  estado = EST_CALIBRANDO;
}

void pasoCalibracion() {
  uint32_t t = millis() - calInicioMs;
  if (calGirando) {
    // Gira a un lado y al otro para barrer la barra sobre la linea
    uint32_t fase = (t / 400) % 4;      // 0: der, 1: izq, 2: izq, 3: der
    int16_t v = (fase == 0 || fase == 3) ? calVelGiro : -calVelGiro;
    setMotores(v, -v);
  }
  qtr.calibrate();

  if (t >= calDuracionMs) {
    setMotores(0, 0);
    estado = EST_LIBRE;
    guardarCalibracion();
    enviarHola(255);    // manda la calibracion nueva a todos
  }
}

// ============================================================
// PID (estructura Holdmoss)
// ============================================================
void resetPID() {
  P = D = I = LAST = 0;
  correccion = 0;
  enRecta = false;
  rectaActiva = false;
}

void iniciarPID(Origen origen) {
  if (estado == EST_PID) return;
  if (!qtr.isCalibrated()) {
    motivoParada = "sin calibrar";
    return;
  }
  manualIzq = manualDer = 0;
  resetPID();
  origenPid = origen;
  velRampa = cfg.arranqueSuave ? 0 : cfg.velBase;
  ultimoRampaMs = millis();
  setEdf(cfg.edfPid);
  motivoParada = "";
  estado = EST_PID;
}

void actualizarRecta() {
  if (!cfg.usarRecta) {
    velActual = cfg.velBase;
    rectaActiva = false;
    return;
  }
  if (abs(P) < cfg.umbralRecta) {
    if (!enRecta) {
      tiempoRecta = millis();
      enRecta = true;
    }
    if (millis() - tiempoRecta > (uint32_t)cfg.tiempoRecta) {
      velActual = cfg.velRecta;
      rectaActiva = true;
    }
  } else {
    velActual = cfg.velBase;
    enRecta = false;
    rectaActiva = false;
  }
}

void pasoPID() {
  position = qtr.readLine(sensorCal, cfg.tipoLinea);
  P = (int32_t)position - CENTRO_LINEA;

  actualizarRecta();

  // Arranque suave: sube 3 cada 2 ms hasta la velocidad objetivo
  if (velRampa < velActual && millis() - ultimoRampaMs >= 2) {
    velRampa += 3;
    if (velRampa > velActual) velRampa = velActual;
    ultimoRampaMs = millis();
  }
  if (velRampa > velActual) velRampa = velActual;
  int16_t vel = velRampa;

  D = P - LAST;
  I += P;
  I = constrain(I, -cfg.limiteI, cfg.limiteI);
  LAST = P;

  correccion = (P * cfg.kp) + (D * cfg.kd) + (I * cfg.ki);
  correccion = constrain(correccion, -(float)vel, (float)vel);

  int16_t vIzq = constrain((int16_t)(vel + correccion), 0, 255);
  int16_t vDer = constrain((int16_t)(vel - correccion), 0, 255);
  setMotores(vIzq, vDer);
}

// ============================================================
// Entradas: boton y MicroStart
// ============================================================
void leerEntradas() {
  uint32_t ahora = millis();

  // Boton (activo en LOW) con antirrebote de 30 ms
  bool b = digitalRead(PIN_BOTON) == LOW;
  if (b != botonAnterior && ahora - botonCambioMs > 30) {
    botonAnterior = b;
    botonCambioMs = ahora;
    botonPresionado = b;
    if (b) {
      contadorBoton++;
      if (estado == EST_PID) {
        detenerTodo("boton");           // como Holdmoss: tocar el boton frena
      } else if (cfg.accionBoton == 1) {
        iniciarCalibracion(calDuracionMs, false, calVelGiro);
      } else if (cfg.accionBoton == 2) {
        iniciarPID(ORG_BOTON);
      }
    }
  }

  // MicroStart: HIGH = largar, LOW = parar
  bool m = digitalRead(PIN_MICROSTART) == HIGH;
  if (m != microstartAnterior && ahora - microstartCambioMs > 10) {
    microstartAnterior = m;
    microstartCambioMs = ahora;
    microstart = m;
    contadorMicrostart++;
    if (cfg.microstartArmado) {
      if (m) {
        iniciarPID(ORG_MICROSTART);
      } else if (estado == EST_PID) {
        detenerTodo("microstart");
        cfg.microstartArmado = false;   // hay que volver a armar
      }
    }
  }
}

void actualizarLed() {
  bool on;
  if (modoLed == 1)      on = true;
  else if (modoLed == 2) on = false;
  else {
    uint32_t t = millis();
    switch (estado) {
      case EST_CALIBRANDO: on = true; break;
      case EST_PID:        on = rectaActiva; break;          // como Holdmoss
      default:
        if (cfg.microstartArmado) on = (t / 100) % 2;        // armado: rapido
        else if (clientes == 0)   on = (t / 500) % 2;        // esperando web
        else                      on = (t % 2000) < 60;      // conectado
    }
  }
  digitalWrite(PIN_LED, on ? HIGH : LOW);
}

// ============================================================
// Envio de datos
// ============================================================
// Agrega texto con formato a un buffer
#define AP(...) do { if (n < (int)sizeof(buf)) n += snprintf(buf + n, sizeof(buf) - n, __VA_ARGS__); } while (0)

void enviarEstado() {
  char buf[900];
  int n = 0;

  AP("{\"t\":\"s\",\"ms\":%lu,\"e\":%u,\"o\":%u,\"why\":\"%s\"",
     (unsigned long)millis(), estado, origenPid, motivoParada);

  if (estado != EST_PID) {
    AP(",\"raw\":[");
    for (uint8_t i = 0; i < NUM_SENSORS; i++) AP(i ? ",%u" : "%u", sensorRaw[i]);
    AP("]");
  }
  AP(",\"cal\":[");
  for (uint8_t i = 0; i < NUM_SENSORS; i++) AP(i ? ",%u" : "%u", sensorCal[i]);
  AP("]");

  AP(",\"pos\":%d,\"on\":%u,\"isc\":%u", position, qtr.onLine() ? 1 : 0, qtr.isCalibrated() ? 1 : 0);
  AP(",\"P\":%ld,\"I\":%ld,\"D\":%ld,\"c\":%.1f,\"v\":%d,\"rec\":%u",
     (long)P, (long)I, (long)D, correccion, estado == EST_PID ? velRampa : 0, rectaActiva ? 1 : 0);
  AP(",\"ml\":%d,\"mr\":%d,\"edf\":%u", salidaIzq, salidaDer, edfActual);
  AP(",\"btn\":%u,\"bc\":%lu,\"mic\":%u,\"mc\":%lu,\"arm\":%u,\"led\":%u",
     botonPresionado ? 1 : 0, (unsigned long)contadorBoton,
     microstart ? 1 : 0, (unsigned long)contadorMicrostart,
     cfg.microstartArmado ? 1 : 0, digitalRead(PIN_LED));
  if (estado == EST_CALIBRANDO) {
    AP(",\"calp\":%lu", (unsigned long)((millis() - calInicioMs) * 100 / calDuracionMs));
  }
  AP(",\"lu\":%lu,\"hz\":%lu}", (unsigned long)loopUs, (unsigned long)loopsPorSeg);

  ws.broadcastTXT(buf, n);
}

void enviarHola(uint8_t cliente) {
  char buf[900];
  int n = 0;
  AP("{\"t\":\"hola\",\"fw\":\"%s\",\"n\":%u,\"centro\":%d", FIRMWARE_VERSION, NUM_SENSORS, CENTRO_LINEA);
  AP(",\"cfg\":{\"kp\":%.5f,\"kd\":%.5f,\"ki\":%.6f,\"velBase\":%d,\"velRecta\":%d,\"usarRecta\":%u",
     cfg.kp, cfg.kd, cfg.ki, cfg.velBase, cfg.velRecta, cfg.usarRecta ? 1 : 0);
  AP(",\"umbralRecta\":%d,\"tiempoRecta\":%d,\"limiteI\":%ld,\"arranqueSuave\":%u",
     cfg.umbralRecta, cfg.tiempoRecta, (long)cfg.limiteI, cfg.arranqueSuave ? 1 : 0);
  AP(",\"edfPid\":%u,\"tipoLinea\":%u,\"accionBoton\":%u,\"calMs\":%lu,\"calVel\":%d,\"led\":%u}",
     cfg.edfPid, cfg.tipoLinea, cfg.accionBoton, (unsigned long)calDuracionMs, calVelGiro, modoLed);
  AP(",\"cmin\":[");
  for (uint8_t i = 0; i < NUM_SENSORS; i++) AP(i ? ",%u" : "%u", qtr.calibratedMinimum[i]);
  AP("],\"cmax\":[");
  for (uint8_t i = 0; i < NUM_SENSORS; i++) AP(i ? ",%u" : "%u", qtr.calibratedMaximum[i]);
  AP("]}");

  if (cliente == 255) ws.broadcastTXT(buf, n);
  else                ws.sendTXT(cliente, buf, n);
}

void enviarAviso(uint8_t cliente, const char *texto) {
  char buf[160];
  int n = 0;
  AP("{\"t\":\"aviso\",\"msg\":\"%s\"}", texto);
  ws.sendTXT(cliente, buf, n);
}

// ============================================================
// Comandos desde la web
// ============================================================
void aplicarConfig(JsonObject o) {
  if (o["kp"].is<float>())          cfg.kp = o["kp"];
  if (o["kd"].is<float>())          cfg.kd = o["kd"];
  if (o["ki"].is<float>())          cfg.ki = o["ki"];
  if (o["velBase"].is<int>())       cfg.velBase = constrain((int)o["velBase"], 0, 255);
  if (o["velRecta"].is<int>())      cfg.velRecta = constrain((int)o["velRecta"], 0, 255);
  if (o["usarRecta"].is<int>())     cfg.usarRecta = (int)o["usarRecta"] != 0;
  if (o["umbralRecta"].is<int>())   cfg.umbralRecta = constrain((int)o["umbralRecta"], 0, CENTRO_LINEA);
  if (o["tiempoRecta"].is<int>())   cfg.tiempoRecta = constrain((int)o["tiempoRecta"], 0, 5000);
  if (o["limiteI"].is<long>())      cfg.limiteI = constrain((long)o["limiteI"], 0L, 1000000L);
  if (o["arranqueSuave"].is<int>()) cfg.arranqueSuave = (int)o["arranqueSuave"] != 0;
  if (o["edfPid"].is<int>()) {
    cfg.edfPid = constrain((int)o["edfPid"], 0, 100);
    if (estado == EST_PID) setEdf(cfg.edfPid);
  }
  if (o["tipoLinea"].is<int>())     cfg.tipoLinea = (int)o["tipoLinea"] ? 1 : 0;
  if (o["accionBoton"].is<int>())   cfg.accionBoton = constrain((int)o["accionBoton"], 0, 2);
  if (o["calMs"].is<long>())        calDuracionMs = constrain((long)o["calMs"], 1000L, 15000L);
  if (o["calVel"].is<int>())        calVelGiro = constrain((int)o["calVel"], 20, 150);
  if (o["led"].is<int>())           modoLed = constrain((int)o["led"], 0, 2);
}

void procesarMensaje(uint8_t cliente, uint8_t *payload, size_t len) {
  JsonDocument doc;
  if (deserializeJson(doc, payload, len)) return;

  ultimoMensajeMs = millis();
  const char *t = doc["t"] | "";

  if (!strcmp(t, "hb")) return;                      // latido

  if (!strcmp(t, "stop")) { detenerTodo("STOP web"); return; }

  if (!strcmp(t, "mot")) {
    if (estado != EST_LIBRE) { enviarAviso(cliente, "Motores bloqueados: PID o calibracion en curso"); return; }
    manualIzq = constrain((int)(doc["l"] | 0), -255, 255);
    manualDer = constrain((int)(doc["r"] | 0), -255, 255);
    setMotores(manualIzq, manualDer);
    return;
  }

  if (!strcmp(t, "edf")) {
    if (estado == EST_CALIBRANDO) return;
    if (estado == EST_PID) {
      cfg.edfPid = constrain((int)(doc["v"] | 0), 0, 100);
      setEdf(cfg.edfPid);
    } else {
      setEdf(constrain((int)(doc["v"] | 0), 0, 100));
    }
    return;
  }

  if (!strcmp(t, "cal")) {
    if (estado == EST_PID) { enviarAviso(cliente, "Detene el PID antes de calibrar"); return; }
    iniciarCalibracion(doc["ms"] | 5000, (int)(doc["gira"] | 0) != 0, doc["vel"] | 60);
    return;
  }

  if (!strcmp(t, "calReset")) {
    if (estado == EST_LIBRE) {
      qtr.resetCalibration();
      prefs.begin("deimonyag", false);
      prefs.remove("cmin");
      prefs.remove("cmax");
      prefs.end();
      enviarHola(255);
    }
    return;
  }

  if (!strcmp(t, "pid")) {
    if ((int)(doc["run"] | 0)) {
      if (estado == EST_CALIBRANDO) { enviarAviso(cliente, "Esperar a que termine la calibracion"); return; }
      iniciarPID(ORG_WEB);
      if (estado != EST_PID) enviarAviso(cliente, "No se puede iniciar: calibrar sensores primero");
    } else {
      detenerTodo("web");
    }
    return;
  }

  if (!strcmp(t, "cfg")) {
    aplicarConfig(doc.as<JsonObject>());
    return;
  }

  if (!strcmp(t, "arm")) {
    cfg.microstartArmado = (int)(doc["v"] | 0) != 0;
    if (cfg.microstartArmado && microstart) {
      enviarAviso(cliente, "Ojo: la senal de MicroStart ya esta en START. Larga al proximo flanco.");
    }
    return;
  }

  if (!strcmp(t, "guardar")) {
    guardarConfig();
    enviarAviso(cliente, "Configuracion guardada en flash");
    return;
  }

  if (!strcmp(t, "defecto")) {
    configPorDefecto();
    enviarHola(255);
    return;
  }

  if (!strcmp(t, "cntReset")) {
    contadorBoton = 0;
    contadorMicrostart = 0;
    return;
  }
}

void onWsEvento(uint8_t cliente, WStype_t tipo, uint8_t *payload, size_t len) {
  switch (tipo) {
    case WStype_CONNECTED:
      clientes = ws.connectedClients();
      ultimoMensajeMs = millis();
      enviarHola(cliente);
      break;

    case WStype_DISCONNECTED:
      clientes = ws.connectedClients();
      // Seguridad: si se va la web, se cortan los mandos manuales
      // y el PID que se inicio desde la web
      if (manualIzq || manualDer || (estado == EST_LIBRE && edfActual)) {
        detenerTodo("web desconectada");
      }
      if (estado == EST_PID && origenPid == ORG_WEB && clientes == 0) {
        detenerTodo("web desconectada");
      }
      break;

    case WStype_TEXT:
      procesarMensaje(cliente, payload, len);
      break;

    default:
      break;
  }
}

void watchdog() {
  uint32_t sinMensajes = millis() - ultimoMensajeMs;

  if (estado == EST_LIBRE && (manualIzq || manualDer || edfActual) && sinMensajes > WATCHDOG_MANUAL_MS) {
    detenerTodo("watchdog");
  }
  if (estado == EST_PID && origenPid == ORG_WEB && sinMensajes > WATCHDOG_PID_WEB_MS) {
    detenerTodo("watchdog");
  }
}

// ============================================================
// Servidor web
// ============================================================
void servirPagina() {
  server.sendHeader("Cache-Control", "no-cache");
  server.send_P(200, "text/html; charset=utf-8", PAGINA_HTML);
}

void servirLogoRmp() {
  server.sendHeader("Cache-Control", "max-age=86400");
  server.send_P(200, "image/png", (const char *)LOGO_RMP_PNG, LOGO_RMP_PNG_LEN);
}

void servirLogoDeimonyag() {
  server.sendHeader("Cache-Control", "max-age=86400");
  server.send_P(200, "image/png", (const char *)LOGO_DEIMONYAG_PNG, LOGO_DEIMONYAG_PNG_LEN);
}

// ============================================================
// Setup y loop
// ============================================================
void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(PIN_MICROSTART, INPUT_PULLDOWN);   // sin modulo -> LOW (no larga)
  pinMode(PIN_MOTOR_IZQ_DIR, OUTPUT);
  pinMode(PIN_MOTOR_DER_DIR, OUTPUT);

  ledcAttach(PIN_MOTOR_IZQ_PWM, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(PIN_MOTOR_DER_PWM, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(PIN_EDF, ESC_PWM_FREQ, ESC_PWM_BITS);
  setMotores(0, 0);
  setEdf(0);   // 1000 us: arma el ESC

  qtr.begin();
  cargarFlash();

  botonAnterior = botonPresionado = digitalRead(PIN_BOTON) == LOW;
  microstartAnterior = microstart = digitalRead(PIN_MICROSTART) == HIGH;

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_IP, AP_MASK);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(false);
  // Si el WiFi se corta seguido, en algunas Super Mini ayuda bajar la potencia:
  // WiFi.setTxPower(WIFI_POWER_8_5dBm);

  server.on("/", servirPagina);
  server.on("/rmp.png", servirLogoRmp);
  server.on("/deimonyag.png", servirLogoDeimonyag);
  server.onNotFound(servirPagina);
  server.begin();

  ws.begin();
  ws.onEvent(onWsEvento);

  Serial.println();
  Serial.println(F("DEIMONYAG - interfaz web"));
  Serial.print(F("WiFi: "));
  Serial.print(WIFI_SSID);
  Serial.print(F("  ->  http://"));
  Serial.println(WiFi.softAPIP());
}

void loop() {
  uint32_t t0 = micros();

  server.handleClient();
  ws.loop();
  leerEntradas();

  switch (estado) {
    case EST_PID:
      pasoPID();
      break;
    case EST_CALIBRANDO:
      pasoCalibracion();
      break;
    default:
      break;
  }

  watchdog();
  actualizarLed();

  uint32_t ahora = millis();
  uint32_t periodo = (estado == EST_PID) ? PERIODO_ENVIO_PID : PERIODO_ENVIO_LIBRE;
  if (clientes && ahora - ultimoEnvioMs >= periodo) {
    ultimoEnvioMs = ahora;
    if (estado == EST_LIBRE) {
      // Fuera del PID se leen los sensores solo para mostrarlos
      qtr.read(sensorRaw);
      position = qtr.readLine(sensorCal, cfg.tipoLinea);
    } else if (estado == EST_CALIBRANDO) {
      qtr.read(sensorRaw);
      qtr.readCalibrated(sensorCal);
    }
    enviarEstado();
  }

  loopUs = micros() - t0;
  contLoops++;
  if (ahora - ultimoConteoMs >= 1000) {
    loopsPorSeg = contLoops;
    contLoops = 0;
    ultimoConteoMs = ahora;
  }
}
