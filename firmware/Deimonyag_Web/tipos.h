// Tipos propios de Deimonyag_Web.
// Estan en un .h (y no en el .ino) porque el IDE de Arduino genera los
// prototipos de las funciones al principio del .ino, antes de cualquier
// enum o struct definido ahi, y entonces no los reconoce.
#pragma once
#include <Arduino.h>

#define CFG_VERSION 1

enum Estado : uint8_t { EST_LIBRE = 0, EST_CALIBRANDO = 1, EST_PID = 2 };
enum Origen : uint8_t { ORG_WEB = 0, ORG_BOTON = 1, ORG_MICROSTART = 2 };

// Configuracion que se guarda en flash con "Guardar"
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
