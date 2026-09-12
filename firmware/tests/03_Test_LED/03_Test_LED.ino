// ============================================================
// DEIMONYAG - PRUEBA LED
// ESP32-C3 Super Mini
// ============================================================

#define PIN_LED 9


void setup() {
  pinMode(PIN_LED, OUTPUT);
}


void loop() {
  // Encender
  digitalWrite(PIN_LED, HIGH);
  delay(500);

  // Apagar
  digitalWrite(PIN_LED, LOW);
  delay(500);
}
