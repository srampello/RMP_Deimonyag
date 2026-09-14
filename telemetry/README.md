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

Los controles solo quedan habilitados cuando el backend está conectado y el modo confirmado es `TEST`.

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

En este modo la PC no modifica actuadores ni parámetros. La grabación solo puede iniciarse cuando el robot está conectado y el modo activo es `TELEMETRY`.

## Conexión real

Por defecto la aplicación usa el backend ESP32 mediante:

```text
SSID: DEIMONYAG
Password: deimonyag
WebSocket: ws://192.168.4.1:81
```

La URL puede cambiarse con la variable de entorno `DEIMONYAG_WS`.

## Ejecutar desde Python

Instalar dependencias:

```bash
pip install -r requirements.txt
```

Ejecutar:

```bash
python app.py
```

Para usar el simulador desde Git Bash:

```bash
DEIMONYAG_BACKEND=simulator python app.py
```

## Ejecutable de Windows

La aplicación completa —pestañas **Pruebas** y **Telemetría**— se empaqueta en un único ejecutable:

```text
Deimonyag_Control_Telemetry.exe
```

### Compilarlo en la PC

Hacer doble clic en:

```text
build_windows.bat
```

El script crea/usa `.venv`, instala dependencias y PyInstaller, y genera:

```text
telemetry/dist/Deimonyag_Control_Telemetry.exe
```

Para abrirlo rápidamente se puede usar:

```text
Abrir_Deimonyag.bat
```

El lanzador abre primero el `.exe` si existe; si no, intenta abrir `app.py` usando el entorno virtual.

### Compilación automática en GitHub

El workflow:

```text
.github/workflows/build-telemetry-windows.yml
```

compila el ejecutable en Windows y lo publica como artifact con el nombre:

```text
Deimonyag-Control-Telemetry-Windows
```

También puede ejecutarse manualmente desde la pestaña **Actions** de GitHub.

## Telemetría guardada

En desarrollo los CSV se guardan en:

```text
telemetry/logs/
```

En el ejecutable de Windows se crea una carpeta:

```text
logs/
```

junto a `Deimonyag_Control_Telemetry.exe`.

Opcionalmente se puede definir otra ubicación mediante la variable de entorno `DEIMONYAG_LOG_DIR`.

El protocolo completo está documentado en `docs/PROTOCOL_WIFI.md`.
