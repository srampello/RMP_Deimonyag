# RMP Deimonyag

Repositorio principal de **Deimonyag**, robot seguidor de línea de competición con sistema de succión.

<p align="center">
  <img src="images/deimonyag%20logo.png" alt="Logo Deimonyag" width="520">
</p>

<p align="center">
  <img src="images/deimonyag.png" alt="Deimonyag" width="700">
</p>

## Configuración actual

- ESP32-C3 Super Mini
- 12 × QRE1113GR + CD74HC4067
- 2 × IFX9201SG
- 2 × JSumo ProFast 12 V 3600 RPM
- EDF27 brushless + ESC
- LiPo 3S 450 mAh 75C XT30
- Matek MICRO BEC 6–30 V ajustado a 5 V
- MicroStart, botón de estrategia y LEDs indicadores

## Alimentación

- **VBAT / 3S:** motores de tracción y EDF/ESC
- **5 V:** Matek MICRO BEC → ESP32-C3 + LEDs IR de los QRE1113GR
- **3,3 V:** ESP32-C3 → CD74HC4067 + pull-ups de los QRE1113GR
- **GND:** común

## GPIO

| GPIO | Función |
|---:|---|
| 0 | MUX S3 |
| 1 | MUX S2 |
| 2 | MUX S1 |
| 3 | MUX S0 |
| 4 | MUX SIG / ADC |
| 5 | Motor 1 PWM |
| 6 | Motor 1 DIR |
| 7 | Motor 2 PWM |
| 8 | Motor 2 DIR |
| 9 | LED indicador |
| 10 | ESC / EDF27 |
| 20 | MicroStart |
| 21 | Botón de estrategia |

## Interfaz PC — control y telemetría

La carpeta [`telemetry/`](telemetry/) contiene la aplicación Python para:

- realizar pruebas de banco de motores, EDF27 y sensores;
- visualizar telemetría de sensores y PID durante una carrera en modo solo lectura;
- guardar archivos CSV de telemetría para análisis posterior.

La aplicación ya dispone de backend Wi‑Fi real por WebSocket para el ESP32-C3 y mantiene el simulador como alternativa de desarrollo.

## Firmware Wi‑Fi

La primera integración ESP32 ↔ PC se encuentra en:

[`firmware/Deimonyag_WiFi/`](firmware/Deimonyag_WiFi/)

El ESP32 crea el AP `DEIMONYAG` y expone el WebSocket en `ws://192.168.4.1:81`.

## Hardware — revisión 2026-09-12

Los esquemáticos de **Main Board** y **barra de 12 sensores** se visualizan desde el visor de GitHub Pages, evitando abrir directamente los archivos de soporte utilizados por la página.

- [Visor interactivo de hardware](https://srampello.github.io/RMP_Deimonyag/)
- [Página de hardware](docs/HARDWARE.md)

## Documentación

- [Conexiones actuales](docs/CONEXIONES.md)
- [Hardware](docs/HARDWARE.md)
- [Interfaz PC](telemetry/README.md)
- [Protocolo Wi‑Fi](docs/PROTOCOL_WIFI.md)
- [Changelog](CHANGELOG.md)
- [Visor interactivo](https://srampello.github.io/RMP_Deimonyag/)

## Estructura

```text
RMP_Deimonyag/
├── docs/
├── hardware/
│   ├── main-board/
│   ├── sensor-bar/
│   └── previews/
├── images/
├── firmware/
│   └── Deimonyag_WiFi/
├── telemetry/
│   ├── pages/
│   └── logs/
├── cad/
├── bom/
└── media/
```

El objetivo es mantener en un único repositorio los esquemáticos, documentación, firmware, CAD, BOM, telemetría y archivos de fabricación de Deimonyag.
