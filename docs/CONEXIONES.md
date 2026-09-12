# Conexiones existentes — Deimonyag

Documento de referencia rápida para las conexiones actualmente definidas.

## Alimentación

- Batería principal: LiPo 3S.
- VBAT / 3S alimenta directamente los bloques de potencia.
- Matek MICRO BEC 6–30 V configurado a 5 V alimenta:
  - pin 5V de la ESP32-C3 Super Mini;
  - rail de 5 V de los LEDs IR de la barra de sensores.
- El rail de 3,3 V de la ESP32-C3 alimenta:
  - CD74HC4067;
  - resistencias pull-up de los fototransistores QRE1113GR.
- Todos los GND son comunes.

## ESP32-C3 Super Mini

| GPIO | Función |
|---:|---|
| GPIO0 | MUX S3 |
| GPIO1 | MUX S2 |
| GPIO2 | MUX S1 |
| GPIO3 | MUX S0 |
| GPIO4 | MUX SIG / ADC |
| GPIO5 | M1 PWM |
| GPIO6 | M1 DIR |
| GPIO7 | M2 PWM |
| GPIO8 | M2 DIR |
| GPIO9 | LED indicador |
| GPIO10 | ESC / EDF27 |
| GPIO20 | MicroStart |
| GPIO21 | Botón de estrategia |

## Barra de sensores

- 12 × QRE1113GR.
- CD74HC4067 alimentado a 3,3 V.
- EN del CD74HC4067 a GND para dejarlo siempre habilitado.
- Canales Y0–Y11 usados para los sensores.
- Y12–Y15 a GND si permanecen sin uso.
- Salida común SIG del MUX hacia GPIO4.
- Q1 = extremo izquierdo.
- Q12 = extremo derecho.

### Cada QRE1113GR

- Pin 1, ánodo LED IR: +5 V a través de 150 Ω.
- Pin 2, cátodo LED IR: GND.
- Pin 3, colector fototransistor: señal Yx + pull-up de 10 kΩ a 3,3 V.
- Pin 4, emisor fototransistor: GND.

## Tracción

- 2 × IFX9201SG.
- Driver izquierdo:
  - PWM desde GPIO5.
  - DIR desde GPIO6.
- Driver derecho:
  - PWM desde GPIO7.
  - DIR desde GPIO8.
- VS de los drivers a VBAT / 3S.
- VSO a 3,3 V.
- DIS, SCK, CSN y SI a GND si no se utiliza SPI.
- OUT1 / OUT2 hacia cada motor.

## Succión

- EDF27 brushless.
- Alimentación desde LiPo 3S mediante ESC.
- Señal de control del ESC desde GPIO10.

## Interfaz

- Botón de estrategia en GPIO21.
- MicroStart en GPIO20.
- LED indicador en GPIO9.

## Geometría de la barra

- 12 sensores.
- Radio: 95 mm.
- Separación angular: 4,8°.
- Sensores orientados radialmente siguiendo el arco.
- Q6 y Q7 quedan simétricos respecto del eje central del robot.
