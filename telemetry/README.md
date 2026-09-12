# Interfaz de control y telemetría — Deimonyag

Aplicación Python para diagnóstico, pruebas de banco y telemetría del robot.

## Pestañas

### Pruebas

Uso exclusivo de banco/taller:

- motor izquierdo;
- motor derecho;
- EDF27;
- STOP;
- lectura en tiempo real de los 12 sensores.

### Telemetría

Modo de solo lectura durante la carrera:

- 12 sensores;
- posición y error;
- P, D y salida PID;
- PWM de ambos motores;
- potencia EDF;
- tiempo de loop;
- estado de carrera y pérdida de línea;
- grabación CSV.

En este modo la PC no modifica actuadores ni parámetros.

## Conexión real

Por defecto la aplicación usa el backend ESP32 mediante:

```text
SSID: DEIMONYAG
Password: deimonyag
WebSocket: ws://192.168.4.1:81
```

Instalar dependencias:

```bash
pip install -r requirements.txt
```

Ejecutar:

```bash
python app.py
```

Para usar nuevamente el simulador desde Git Bash:

```bash
DEIMONYAG_BACKEND=simulator python app.py
```

## Telemetría guardada

Los CSV se guardan en:

```text
telemetry/logs/
```

El protocolo completo está documentado en `docs/PROTOCOL_WIFI.md`.
