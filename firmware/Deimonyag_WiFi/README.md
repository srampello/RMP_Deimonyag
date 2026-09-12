# Deimonyag_WiFi

Firmware inicial para conectar Deimonyag con la aplicación Python mediante Wi‑Fi y WebSocket.

## Dependencias Arduino

Instalar desde el Library Manager:

- `ArduinoJson` 7.x
- `WebSockets` / `arduinoWebSockets` de Markus Sattler

Usar una versión actual de `esp32 by Espressif Systems` compatible con la API LEDC `ledcAttach()` / `ledcWrite()`.

## Red Wi‑Fi

Al iniciar, el ESP32-C3 crea:

```text
SSID: DEIMONYAG
Password: deimonyag
IP ESP32: 192.168.4.1
WebSocket: ws://192.168.4.1:81
```

La notebook debe conectarse a esa red antes de abrir la interfaz Python en modo real.

## Etapa actual

El firmware ya implementa:

- lectura RAW de los 12 QRE a través del CD74HC4067;
- control manual de ambos motores en modo `TEST`;
- control manual de EDF27/ESC en modo `TEST`;
- watchdog de 500 ms para pruebas de banco;
- WebSocket JSON;
- modo `TELEMETRY` de solo lectura;
- envío de telemetría a 100 Hz;
- bloqueo de comandos de actuadores durante telemetría/carrera.

Los valores `position`, `error`, `p`, `d`, `pid`, velocidades de carrera y estado de línea todavía deben ser alimentados por el futuro código real del seguidor mediante:

```cpp
publishRaceTelemetry(...);
```

Por eso, antes de integrar el PID real, la segunda pestaña ya recibirá los sensores reales pero los datos PID permanecerán en cero.

## GPIO utilizados

| GPIO | Función |
|---:|---|
| 0 | MUX S3 |
| 1 | MUX S2 |
| 2 | MUX S1 |
| 3 | MUX S0 |
| 4 | MUX SIG / ADC |
| 5 | Motor izquierdo PWM |
| 6 | Motor izquierdo DIR |
| 7 | Motor derecho PWM |
| 8 | Motor derecho DIR |
| 10 | ESC / EDF27 |

GPIO9 no se utiliza en esta primera etapa del firmware Wi‑Fi.

## Seguridad

El PID nunca depende del Wi‑Fi. Si la PC se desconecta durante `TEST`, los actuadores manuales se detienen. Si la PC se desconecta durante `TELEMETRY`, el firmware no modifica la carrera.

Antes de probar motores o EDF por primera vez, levantar el robot de la pista/mesa y verificar sentidos con valores bajos.
