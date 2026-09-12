// ============================================================
// DEIMONYAG - PRUEBA DE MOTORES
// ESP32-C3 Super Mini + 2x IFX9201SG
// ============================================================

// Motor izquierdo
#define MOTOR_IZQ_PWM 5
#define MOTOR_IZQ_DIR 6

// Motor derecho
#define MOTOR_DER_PWM 7
#define MOTOR_DER_DIR 8

// PWM
#define FRECUENCIA_PWM 20000
#define RESOLUCION_PWM 8

// Velocidad de prueba: 0 a 255
#define VELOCIDAD_PRUEBA 80


void moverMotor(int pinPWM, int pinDIR, int velocidad) {
  // Limitar velocidad
  velocidad = constrain(velocidad, -255, 255);

  // Dirección
  if (velocidad >= 0) {
    digitalWrite(pinDIR, HIGH);
  }
  else {
    digitalWrite(pinDIR, LOW);
  }

  // PWM
  ledcWrite(pinPWM, abs(velocidad));
}


void detenerMotores() {
  ledcWrite(MOTOR_IZQ_PWM, 0);
  ledcWrite(MOTOR_DER_PWM, 0);
}


void setup() {
  pinMode(MOTOR_IZQ_DIR, OUTPUT);
  pinMode(MOTOR_DER_DIR, OUTPUT);

  // Configuración PWM
  ledcAttach(MOTOR_IZQ_PWM, FRECUENCIA_PWM, RESOLUCION_PWM);
  ledcAttach(MOTOR_DER_PWM, FRECUENCIA_PWM, RESOLUCION_PWM);

  detenerMotores();
}


void loop() {
  // ---------------------------
  // MOTOR IZQUIERDO ADELANTE
  // ---------------------------
  moverMotor(MOTOR_IZQ_PWM, MOTOR_IZQ_DIR, VELOCIDAD_PRUEBA);
  delay(2000);

  detenerMotores();
  delay(1000);

  // ---------------------------
  // MOTOR IZQUIERDO ATRÁS
  // ---------------------------
  moverMotor(MOTOR_IZQ_PWM, MOTOR_IZQ_DIR, -VELOCIDAD_PRUEBA);
  delay(2000);

  detenerMotores();
  delay(1000);

  // ---------------------------
  // MOTOR DERECHO ADELANTE
  // ---------------------------
  moverMotor(MOTOR_DER_PWM, MOTOR_DER_DIR, VELOCIDAD_PRUEBA);
  delay(2000);

  detenerMotores();
  delay(1000);

  // ---------------------------
  // MOTOR DERECHO ATRÁS
  // ---------------------------
  moverMotor(MOTOR_DER_PWM, MOTOR_DER_DIR, -VELOCIDAD_PRUEBA);
  delay(2000);

  detenerMotores();
  delay(2000);
}
