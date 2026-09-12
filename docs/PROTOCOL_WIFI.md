# Protocolo Wi‑Fi — Deimonyag

Versión inicial del protocolo entre la aplicación Python y el ESP32-C3.

## Transporte

- El ESP32 crea un punto de acceso Wi‑Fi propio.
- SSID por defecto: `DEIMONYAG`
- IP del ESP32: `192.168.4.1`
- WebSocket: `ws://192.168.4.1:81`
- Formato de mensajes: JSON UTF-8.
- Versión de protocolo: `1`.

La PC controla actuadores únicamente en `TEST`. En `TELEMETRY` la PC es de solo lectura.

## Modos

### TEST

Modo de banco/taller.

La PC puede:

- controlar motor izquierdo;
- controlar motor derecho;
- controlar EDF27;
- leer los 12 sensores;
- ejecutar STOP.

Existe un watchdog de seguridad: si el ESP32 deja de recibir tráfico de la aplicación durante 500 ms, los actuadores comandados desde la PC se llevan a cero.

### TELEMETRY

Modo de carrera y análisis.

La PC puede:

- recibir sensores;
- recibir posición, error, P, D y PID;
- recibir PWM de motores, potencia EDF y tiempo de loop;
- registrar los datos en CSV.

En este modo el firmware rechaza comandos de control de motores y EDF. La pérdida de Wi‑Fi o el cierre de la aplicación no debe modificar el comportamiento del robot durante la carrera.

## Mensajes PC → ESP32

### Handshake

```json
{"type":"hello","protocol":1}
```

### Cambio de modo

```json
{"type":"mode","mode":"TEST"}
```

```json
{"type":"mode","mode":"TELEMETRY"}
```

Al entrar en `TELEMETRY`, el ESP32 lleva a cero cualquier mando manual de banco antes de bloquear los comandos de actuadores.

### Motor izquierdo

```json
{"type":"control","motor_left":120}
```

Rango: `-255..255`.

### Motor derecho

```json
{"type":"control","motor_right":-90}
```

Rango: `-255..255`.

### EDF27

```json
{"type":"control","edf":65}
```

Rango: `0..100`.

### STOP

```json
{"type":"stop"}
```

Solo tiene efecto en `TEST`.

### Keepalive

```json
{"type":"ping"}
```

Se utiliza para mantener vivo el watchdog de banco.

## Mensajes ESP32 → PC

### Handshake

```json
{
  "type":"hello",
  "device":"Deimonyag",
  "protocol":1,
  "mode":"TEST"
}
```

### Lectura de sensores en TEST

```json
{
  "type":"sensors",
  "time_ms":12345,
  "sensors":[321,355,402,680,1380,3010,3650,2110,830,470,360,330]
}
```

Los valores iniciales son ADC RAW de 12 bits (`0..4095`). La inversión/calibración se realizará en el firmware de seguimiento, no en la capa de transporte.

### Telemetría de carrera

```json
{
  "type":"telemetry",
  "time_ms":182345,
  "sensors":[120,145,230,620,910,980,850,410,180,130,115,108],
  "position":5340,
  "error":-160.0,
  "p":-5.6,
  "d":2.4,
  "pid":-3.2,
  "motor_left":161,
  "motor_right":167,
  "edf":82,
  "loop_us":1520,
  "line_lost":false,
  "running":true
}
```

La frecuencia de telemetría es independiente de la frecuencia del PID. El control puede ejecutarse a una frecuencia alta dentro del ESP32 y enviar una muestra decimada a la PC.

### Respuesta keepalive

```json
{"type":"pong"}
```

### Error

```json
{"type":"error","message":"control_locked_in_telemetry"}
```

## Principio de seguridad

La interfaz Python nunca forma parte del lazo de control. El PID, lectura de sensores, control de motores y EDF durante una carrera permanecen ejecutándose localmente en el ESP32. El Wi‑Fi se usa únicamente para pruebas de banco o para sacar una copia de la telemetría.
