// ============================================================
// DEIMONYAG - PRUEBA EDF27 CON BOTÓN
// ESP32-C3 Super Mini
// ============================================================

#define PIN_BOTON 21
#define PIN_EDF   10

// PWM típico para ESC
#define FRECUENCIA_ESC 50
#define RESOLUCION_ESC 14

// Pulso típico del ESC
#define ESC_MIN_US 1000
#define ESC_MAX_US 2000

// Potencia de prueba
#define POTENCIA_PRUEBA 25


uint32_t microsegundosADuty(uint16_t microsegundos) {
  const uint32_t dutyMaximo = (1UL << RESOLUCION_ESC) - 1;
  return ((uint32_t)microsegundos * dutyMaximo) / 20000;
}


void enviarPulsoESC(uint16_t microsegundos) {
  ledcWrite(PIN_EDF, microsegundosADuty(microsegundos));
}


void potenciaEDF(int porcentaje) {
  porcentaje = constrain(porcentaje, 0, 100);

  int pulso = map(
    porcentaje,
    0,
    100,
    ESC_MIN_US,
    ESC_MAX_US
  );

  enviarPulsoESC(pulso);
}


void setup() {
  pinMode(PIN_BOTON, INPUT_PULLUP);

  ledcAttach(
    PIN_EDF,
    FRECUENCIA_ESC,
    RESOLUCION_ESC
  );

  // Señal mínima para armar el ESC
  potenciaEDF(0);

  // Esperar armado
  delay(3000);
}


void loop() {
  bool botonPresionado = digitalRead(PIN_BOTON) == LOW;

  if (botonPresionado) {
    potenciaEDF(POTENCIA_PRUEBA);
  }
  else {
    potenciaEDF(0);
  }

  delay(10);
}
