// ============================================================
// QTRMux - Libreria tipo QTRSensors para sensores analogicos
// leidos a traves de un multiplexor CD74HC4067 (16 canales).
//
// Pensada para Deimonyag (ESP32-C3 + 12x QRE1113GR), pero sirve
// para cualquier cantidad de sensores (hasta 16) en un mux 4 bits.
//
// Misma logica que QTRSensors de Pololu (Holdmoss):
//   read()           -> valores crudos del ADC (0..4095 en ESP32)
//   calibrate()      -> guarda min/max por sensor
//   readCalibrated() -> valores 0..1000 (0 = min, 1000 = max)
//   readLine()       -> posicion 0 .. (N-1)*1000
//
// Con pull-up al colector (QRE1113GR):
//   superficie blanca (mucha reflexion) -> valor BAJO
//   superficie negra / aire             -> valor ALTO
// Igual que la QTR analogica: linea blanca -> readLine(..., true)
// ============================================================
#ifndef QTRMUX_H
#define QTRMUX_H

#include <Arduino.h>

#define QTRMUX_MAX_SENSORS 16

class QTRMux {
public:
  // selPins:  {S0, S1, S2, S3}
  // sigPin:   salida comun del mux (pin ADC)
  // channels: canal del mux (Y0..Y15) de cada sensor, de izquierda a derecha
  // numSensors: cantidad de sensores (<= 16)
  // samplesPerSensor: lecturas promediadas por sensor
  // settleUs: microsegundos de espera tras cambiar de canal
  QTRMux(const uint8_t selPins[4], uint8_t sigPin,
         const uint8_t *channels, uint8_t numSensors,
         uint8_t samplesPerSensor = 4, uint8_t settleUs = 5);

  // Configura pines y ADC. Llamar en setup().
  void begin();

  // Valores crudos (promedio de samplesPerSensor lecturas).
  void read(uint16_t *values);

  // Lee un solo sensor crudo (indice 0..numSensors-1).
  uint16_t readOne(uint8_t index);

  // Una pasada de calibracion (10 lecturas, igual que QTR).
  // Llamar muchas veces moviendo los sensores sobre linea y fondo.
  void calibrate();

  // Borra la calibracion.
  void resetCalibration();

  // Carga una calibracion guardada (por ejemplo desde flash/NVS).
  void setCalibration(const uint16_t *minimos, const uint16_t *maximos);

  // true si ya se llamo a calibrate() al menos una vez.
  bool isCalibrated() const { return _calibrated; }

  // Valores calibrados 0..1000.
  void readCalibrated(uint16_t *values);

  // Posicion de linea 0..(N-1)*1000. whiteLine = true para linea blanca.
  // Si se pierde la linea devuelve el extremo del ultimo lado visto.
  int readLine(uint16_t *values, bool whiteLine = false);

  // true si en la ultima readLine() algun sensor vio linea.
  bool onLine() const { return _onLine; }

  uint8_t  numSensors() const { return _numSensors; }
  uint16_t maxValue()   const { return _maxValue; }

  // Calibracion (accesible para imprimir o guardar en flash).
  uint16_t calibratedMinimum[QTRMUX_MAX_SENSORS];
  uint16_t calibratedMaximum[QTRMUX_MAX_SENSORS];

  // Umbrales de readLine (mismos valores por defecto que QTR).
  uint16_t lineThreshold  = 200;  // por encima: "ve linea"
  uint16_t noiseThreshold = 50;   // por debajo: no suma al promedio

private:
  void selectChannel(uint8_t ch);

  uint8_t  _sel[4];
  uint8_t  _sig;
  uint8_t  _channels[QTRMUX_MAX_SENSORS];
  uint8_t  _numSensors;
  uint8_t  _samples;
  uint8_t  _settleUs;
  uint16_t _maxValue;
  bool     _calibrated;
  bool     _onLine;
  int      _lastValue;
};

#endif
