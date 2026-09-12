# RMP Deimonyag

Repositorio principal del robot **Deimonyag**, seguidor de línea de competición con sistema de succión.

## Estado actual

Proyecto en desarrollo. Esta rama contiene la documentación y los archivos base del diseño electrónico, mecánico y de firmware.

## Configuración definida

- **Microcontrolador:** ESP32-C3 Super Mini
- **Sensores de línea:** 12 × QRE1113GR en barra curva
- **Multiplexor:** CD74HC4067
- **Drivers de tracción:** 2 × IFX9201SG
- **Motores de tracción:** 2 × JSumo ProFast 12 V 3600 RPM
- **Sistema de succión:** EDF27 brushless + ESC
- **Batería:** LiPo 3S 450 mAh 75C XT30
- **Regulación de lógica:** Matek MICRO BEC 6–30 V ajustado a 5 V
- **Control de largada:** MicroStart
- **Interfaz:** botón de estrategia + LEDs indicadores

## Alimentación

- **VBAT / 3S:** alimentación directa de motores de tracción y sistema EDF/ESC.
- **5 V:** generados por el Matek MICRO BEC para la ESP32-C3 Super Mini y LEDs IR de los QRE1113GR.
- **3,3 V:** rail de la ESP32-C3 para CD74HC4067 y pull-ups de los fototransistores QRE1113GR.
- **GND:** común en todo el sistema.

## Mapa de GPIO actual

| GPIO | Función |
|---:|---|
| GPIO0 | MUX S3 |
| GPIO1 | MUX S2 |
| GPIO2 | MUX S1 |
| GPIO3 | MUX S0 |
| GPIO4 | MUX SIG / ADC |
| GPIO5 | Motor 1 PWM |
| GPIO6 | Motor 1 DIR |
| GPIO7 | Motor 2 PWM |
| GPIO8 | Motor 2 DIR |
| GPIO9 | LED indicador |
| GPIO10 | ESC / EDF27 |
| GPIO20 | MicroStart |
| GPIO21 | Botón de estrategia |

## Barra de sensores

Configuración mecánica actual:

- 12 sensores QRE1113GR
- Radio del arco: **95 mm**
- Separación angular: **4,8°**
- Q1 ubicado en el extremo izquierdo
- Q12 ubicado en el extremo derecho
- Q6 y Q7 simétricos respecto del eje central
- Sensores orientados radialmente siguiendo el arco

Configuración eléctrica por sensor:

- LED IR: 5 V mediante resistencia de 150 Ω
- Fototransistor: pull-up de 10 kΩ a 3,3 V
- Emisor del fototransistor a GND
- Señal de cada sensor hacia un canal del CD74HC4067

## Estructura del repositorio

```text
RMP_Deimonyag/
├── README.md
├── CHANGELOG.md
├── docs/
├── hardware/
│   ├── main-board/
│   └── sensor-bar/
├── firmware/
├── cad/
├── bom/
└── media/
```

## Objetivo

Mantener en un único repositorio el historial completo de Deimonyag: esquemáticos, PCB, documentación, firmware, CAD, BOM, archivos de fabricación y revisiones del proyecto.
