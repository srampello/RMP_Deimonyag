// ============================================================
// DEIMONYAG - PRUEBA DE SENSORES (12x QRE1113GR + CD74HC4067)
// ESP32-C3 Super Mini
//
// Base: programacion de Holdmoss (QTRSensors) -> libreria QTRMux
//
// Comandos por Monitor Serie (115200, cualquier fin de linea):
//   r  -> valores crudos (0..4095)          [modo por defecto]
//   k  -> valores calibrados (0..1000)
//   l  -> posicion de linea + barra visual
//   c  -> calibrar 5 s (pasar la barra sobre linea y fondo)
//   m  -> mostrar min/max de calibracion
//   x  -> borrar calibracion
//   t  -> medir tiempo de lectura
//   d  -> diagnostico de orden: tapar/acercar un sensor por vez
//         y ver que Q reacciona (sirve para corregir muxChannels)
//   h  -> ayuda
// El boton de estrategia (GPIO21) tambien dispara la calibracion.
//
// Tip: en modo 'r' y 'k' la salida es compatible con el
// Serial Plotter del IDE (formato Q1:valor Q2:valor ...).
// ============================================================

#include "QTRMux.h"

// Sensores
#define NUM_SENSORS             12
#define NUM_SAMPLES_PER_SENSOR  4    // lecturas promediadas por sensor
#define MUX_SETTLE_US           5    // espera tras cambiar canal del mux

// MUX CD74HC4067
#define PIN_MUX_S0   3
#define PIN_MUX_S1   2
#define PIN_MUX_S2   1
#define PIN_MUX_S3   0
#define PIN_MUX_SIG  4

const uint8_t muxSelPins[4] = {PIN_MUX_S0, PIN_MUX_S1, PIN_MUX_S2, PIN_MUX_S3};

// Canal del mux de cada sensor, de izquierda (Q1) a derecha (Q12)
const uint8_t muxChannels[NUM_SENSORS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

QTRMux qtr(muxSelPins, PIN_MUX_SIG, muxChannels, NUM_SENSORS,
           NUM_SAMPLES_PER_SENSOR, MUX_SETTLE_US);

uint16_t sensorValues[NUM_SENSORS];
int position = 0;

// Line -
#define CENTRO_LINEA  ((NUM_SENSORS - 1) * 1000 / 2)   // 5500
#define TIPO_LINEA    1        // 1- Blanca  0- Negra

// Interfaz
const int LED   = 9;
const int BTN_1 = 21;          // boton de estrategia

// Monitor
#define PERIODO_PRINT_MS     50
#define TIEMPO_CALIBRACION   5000

enum Modo { MODO_RAW, MODO_CAL, MODO_LINEA, MODO_DIAG };
Modo modo = MODO_RAW;
unsigned long ultimoPrint = 0;

void setup() {
  Serial.begin(115200);
  // ESP32-C3 por USB nativo: esperar al monitor (max 2 s)
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 2000) { delay(10); }

  pinMode(LED, OUTPUT);
  pinMode(BTN_1, INPUT_PULLUP);
  digitalWrite(LED, LOW);

  qtr.begin();

  Serial.println();
  Serial.println(F("=== DEIMONYAG - PRUEBA DE SENSORES ==="));
  ayuda();
}

void loop() {
  leerComandos();

  if (botonPresionado(BTN_1)) {
    esperarSoltarBoton();
    calibrarSensores();
  }

  if (millis() - ultimoPrint < PERIODO_PRINT_MS) return;
  ultimoPrint = millis();

  switch (modo) {
    case MODO_RAW:
      qtr.read(sensorValues);
      imprimirValores("RAW");
      break;

    case MODO_CAL:
      if (!avisarSiNoCalibrado()) return;
      qtr.readCalibrated(sensorValues);
      imprimirValores("CAL");
      break;

    case MODO_LINEA:
      if (!avisarSiNoCalibrado()) return;
      position = qtr.readLine(sensorValues, TIPO_LINEA);
      imprimirLinea();
      break;

    case MODO_DIAG:
      qtr.read(sensorValues);
      imprimirDiagnostico();
      break;
  }
}

// Comandos serie
void leerComandos() {
  while (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 'r': case 'R': modo = MODO_RAW;   Serial.println(F("> Modo RAW"));        break;
      case 'k': case 'K': modo = MODO_CAL;   Serial.println(F("> Modo CALIBRADO"));  break;
      case 'l': case 'L': modo = MODO_LINEA; Serial.println(F("> Modo LINEA"));      break;
      case 'c': case 'C': calibrarSensores();        break;
      case 'm': case 'M': imprimirCalibracion();     break;
      case 'x': case 'X': qtr.resetCalibration(); Serial.println(F("> Calibracion borrada")); break;
      case 't': case 'T': medirTiempo();             break;
      case 'd': case 'D': modo = MODO_DIAG;
        Serial.println(F("> Modo DIAGNOSTICO: acercar algo blanco a UN sensor por vez, de izq a der"));
        break;
      case 'h': case 'H': case '?': ayuda();         break;
      default: break;   // ignora \r, \n, etc.
    }
  }
}

void ayuda() {
  Serial.println(F("Comandos: r=crudo  k=calibrado  l=linea  c=calibrar"));
  Serial.println(F("          m=min/max  x=borrar cal  t=tiempo  d=diagnostico  h=ayuda"));
  Serial.print  (F("Tipo de linea: "));
  Serial.println(TIPO_LINEA ? F("BLANCA") : F("NEGRA"));
}

// Impresion
void imprimirValores(const char *tag) {
  // Formato Serial Plotter: Q1:123 Q2:456 ...
  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    Serial.print('Q');
    Serial.print(i + 1);
    Serial.print(':');
    Serial.print(sensorValues[i]);
    Serial.print(i < NUM_SENSORS - 1 ? '\t' : '\n');
  }
  (void)tag;
}

void imprimirLinea() {
  // Barra: un caracter por sensor segun cuanto "ve" la linea
  const char escala[] = " .:-=+*#%@";
  char barra[NUM_SENSORS + 1];
  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    uint16_t v = TIPO_LINEA ? 1000 - sensorValues[i] : sensorValues[i];
    barra[i] = escala[v * 9 / 1000];
  }
  barra[NUM_SENSORS] = '\0';

  int error = position - CENTRO_LINEA;

  // Marcador de posicion en una regla de 23 caracteres
  char regla[24];
  for (uint8_t i = 0; i < 23; i++) regla[i] = '-';
  regla[11] = '|';
  int idx = (long)position * 22 / ((NUM_SENSORS - 1) * 1000);
  regla[constrain(idx, 0, 22)] = 'O';
  regla[23] = '\0';

  Serial.print('[');
  Serial.print(barra);
  Serial.print(F("]  "));
  Serial.print(regla);
  Serial.print(F("  pos="));
  Serial.print(position);
  Serial.print(F("  error="));
  Serial.print(error);
  if (!qtr.onLine()) Serial.print(F("  (LINEA PERDIDA)"));
  Serial.println();
}

void imprimirCalibracion() {
  Serial.println(F("Sensor  MIN   MAX   RANGO"));
  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    uint16_t mn = qtr.calibratedMinimum[i];
    uint16_t mx = qtr.calibratedMaximum[i];
    char buf[40];
    snprintf(buf, sizeof(buf), "Q%-5d %5u %5u %5d", i + 1, mn, mx, (int)mx - (int)mn);
    Serial.println(buf);
  }
}

// Diagnostico: muestra que sensores bajaron claramente respecto al resto.
// Si al tapar UN sensor reaccionan dos Q a la vez (ej. Q1 y Q3),
// hay una linea S0..S3 cortada o en corto, no es un tema de orden.
void imprimirDiagnostico() {
  // mediana aproximada: ordenar copia
  uint16_t tmp[NUM_SENSORS];
  memcpy(tmp, sensorValues, sizeof(tmp));
  for (uint8_t i = 1; i < NUM_SENSORS; i++) {
    uint16_t v = tmp[i]; int8_t j = i - 1;
    while (j >= 0 && tmp[j] > v) { tmp[j + 1] = tmp[j]; j--; }
    tmp[j + 1] = v;
  }
  uint16_t mediana = tmp[NUM_SENSORS / 2];
  uint16_t umbral  = mediana * 7 / 10;      // 30% por debajo del resto

  // Mapa: '#' = reacciona, '.' = no
  Serial.print(F("Q1["));
  bool alguno = false;
  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    bool r = sensorValues[i] < umbral;
    Serial.print(r ? '#' : '.');
    alguno |= r;
  }
  Serial.print(F("]Q12  "));

  if (!alguno) {
    Serial.println(F("(ninguno)"));
    return;
  }
  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    if (sensorValues[i] < umbral) {
      Serial.print('Q'); Serial.print(i + 1);
      Serial.print(F("(Y")); Serial.print(muxChannels[i]);
      Serial.print(F(")=")); Serial.print(sensorValues[i]); Serial.print(F("  "));
    }
  }
  Serial.println();
}

bool avisarSiNoCalibrado() {
  if (qtr.isCalibrated()) return true;
  static unsigned long ultimoAviso = 0;
  if (millis() - ultimoAviso > 1000) {
    Serial.println(F("Sin calibrar: enviar 'c' o presionar el boton"));
    ultimoAviso = millis();
  }
  return false;
}

void medirTiempo() {
  const int N = 200;
  unsigned long t0 = micros();
  for (int i = 0; i < N; i++) qtr.read(sensorValues);
  unsigned long dt = micros() - t0;
  Serial.print(F("> read() de 12 sensores: "));
  Serial.print(dt / N);
  Serial.print(F(" us  (~"));
  Serial.print(1000000UL * N / dt);
  Serial.println(F(" lecturas/s)"));
}

// Calibracion
void calibrarSensores() {
  Serial.println(F("> Calibrando 5 s... mover la barra sobre linea y fondo"));
  qtr.resetCalibration();
  digitalWrite(LED, HIGH);

  unsigned long t0 = millis();
  while (millis() - t0 < TIEMPO_CALIBRACION) {
    qtr.calibrate();
  }

  digitalWrite(LED, LOW);
  Serial.println(F("> Calibracion lista"));
  imprimirCalibracion();
}

// Boton
bool botonPresionado(int pin) {
  return digitalRead(pin) == LOW;
}

void esperarSoltarBoton() {
  while (botonPresionado(BTN_1)) delay(5);
  delay(100);
}
