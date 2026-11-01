#include "QTRMux.h"

QTRMux::QTRMux(const uint8_t selPins[4], uint8_t sigPin,
               const uint8_t *channels, uint8_t numSensors,
               uint8_t samplesPerSensor, uint8_t settleUs) {
  for (uint8_t i = 0; i < 4; i++) _sel[i] = selPins[i];
  _sig = sigPin;

  if (numSensors > QTRMUX_MAX_SENSORS) numSensors = QTRMUX_MAX_SENSORS;
  _numSensors = numSensors;
  for (uint8_t i = 0; i < _numSensors; i++) _channels[i] = channels[i] & 0x0F;

  _samples   = samplesPerSensor < 1 ? 1 : samplesPerSensor;
  _settleUs  = settleUs;
  _maxValue  = 4095;            // ADC de 12 bits
  _lastValue = 0;
  _onLine    = false;
  resetCalibration();
}

void QTRMux::begin() {
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(_sel[i], OUTPUT);
    digitalWrite(_sel[i], LOW);
  }
  pinMode(_sig, INPUT);

#if defined(ARDUINO_ARCH_ESP32)
  analogReadResolution(12);
  analogSetPinAttenuation(_sig, ADC_11db);   // rango completo (~0..2,5 V en C3)
  _maxValue = 4095;
#else
  _maxValue = 1023;
#endif
}

void QTRMux::selectChannel(uint8_t ch) {
  digitalWrite(_sel[0], (ch >> 0) & 0x01);
  digitalWrite(_sel[1], (ch >> 1) & 0x01);
  digitalWrite(_sel[2], (ch >> 2) & 0x01);
  digitalWrite(_sel[3], (ch >> 3) & 0x01);
}

uint16_t QTRMux::readOne(uint8_t index) {
  if (index >= _numSensors) return 0;

  selectChannel(_channels[index]);
  if (_settleUs) delayMicroseconds(_settleUs);

  uint32_t acc = 0;
  for (uint8_t s = 0; s < _samples; s++) {
    acc += analogRead(_sig);
  }
  return (uint16_t)((acc + _samples / 2) / _samples);
}

void QTRMux::read(uint16_t *values) {
  for (uint8_t i = 0; i < _numSensors; i++) {
    values[i] = readOne(i);
  }
}

void QTRMux::resetCalibration() {
  for (uint8_t i = 0; i < QTRMUX_MAX_SENSORS; i++) {
    calibratedMinimum[i] = _maxValue;
    calibratedMaximum[i] = 0;
  }
  _calibrated = false;
}

void QTRMux::setCalibration(const uint16_t *minimos, const uint16_t *maximos) {
  for (uint8_t i = 0; i < _numSensors; i++) {
    calibratedMinimum[i] = minimos[i];
    calibratedMaximum[i] = maximos[i];
  }
  _calibrated = true;
}

void QTRMux::calibrate() {
  uint16_t v[QTRMUX_MAX_SENSORS];
  uint16_t maxThis[QTRMUX_MAX_SENSORS];
  uint16_t minThis[QTRMUX_MAX_SENSORS];

  // 10 lecturas: me quedo con el max y min de esta tanda
  for (uint8_t j = 0; j < 10; j++) {
    read(v);
    for (uint8_t i = 0; i < _numSensors; i++) {
      if (j == 0 || v[i] > maxThis[i]) maxThis[i] = v[i];
      if (j == 0 || v[i] < minThis[i]) minThis[i] = v[i];
    }
  }

  // Igual que QTR: el maximo se actualiza con el MINIMO de la tanda
  // y viceversa -> un pico aislado de ruido no arruina la calibracion.
  for (uint8_t i = 0; i < _numSensors; i++) {
    if (minThis[i] > calibratedMaximum[i]) calibratedMaximum[i] = minThis[i];
    if (maxThis[i] < calibratedMinimum[i]) calibratedMinimum[i] = maxThis[i];
  }
  _calibrated = true;
}

void QTRMux::readCalibrated(uint16_t *values) {
  read(values);

  for (uint8_t i = 0; i < _numSensors; i++) {
    uint16_t calmin = calibratedMinimum[i];
    uint16_t calmax = calibratedMaximum[i];

    int32_t x = 0;
    if (calmax > calmin) {
      x = ((int32_t)values[i] - calmin) * 1000L / (calmax - calmin);
    }
    if (x < 0)    x = 0;
    if (x > 1000) x = 1000;
    values[i] = (uint16_t)x;
  }
}

int QTRMux::readLine(uint16_t *values, bool whiteLine) {
  readCalibrated(values);

  uint32_t avg = 0;
  uint32_t sum = 0;
  _onLine = false;

  for (uint8_t i = 0; i < _numSensors; i++) {
    uint16_t value = values[i];
    if (whiteLine) value = 1000 - value;

    if (value > lineThreshold) _onLine = true;

    if (value > noiseThreshold) {
      avg += (uint32_t)value * (i * 1000UL);
      sum += value;
    }
  }

  if (!_onLine) {
    // Linea perdida: devuelvo el extremo del ultimo lado visto
    if (_lastValue < (_numSensors - 1) * 1000 / 2) return 0;
    return (_numSensors - 1) * 1000;
  }

  _lastValue = (int)(avg / sum);
  return _lastValue;
}
