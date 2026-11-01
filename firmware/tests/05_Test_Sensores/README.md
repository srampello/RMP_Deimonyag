# 05 — Test de sensores

Lectura de los 12 × QRE1113GR a través del CD74HC4067 por Monitor Serie (115200).

Incluye `QTRMux`, una librería con la misma API y lógica que QTRSensors de Pololu (base Holdmoss): `read()`, `calibrate()`, `readCalibrated()` (0–1000) y `readLine()` (0–11000, centro 5500).

## Uso

1. Arduino IDE → **USB CDC On Boot: Enabled** (ESP32-C3 Super Mini).
2. Abrir el Monitor Serie a 115200.
3. Comandos:

| Tecla | Acción |
|---|---|
| `r` | Valores crudos 0–4095 (por defecto) |
| `k` | Valores calibrados 0–1000 |
| `l` | Posición de línea + barra visual |
| `c` | Calibrar 5 s (también con el botón GPIO21) |
| `m` | Mín/máx de calibración |
| `x` | Borrar calibración |
| `t` | Tiempo de lectura de los 12 sensores |

Los modos `r` y `k` son compatibles con el Serial Plotter.

Con pull-up a 3,3 V: blanco → valor bajo, negro/aire → valor alto. `TIPO_LINEA 1` = línea blanca.
