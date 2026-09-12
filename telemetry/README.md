# Interfaz de control y telemetría — Deimonyag

Aplicación de escritorio en Python para diagnóstico y análisis del robot.

## Diseño

La aplicación tiene dos pestañas claramente separadas:

### 1. Pruebas

Uso exclusivo de banco/taller.

- Control manual del motor izquierdo.
- Control manual del motor derecho.
- Control de potencia de la EDF27.
- Botón STOP.
- Lectura en tiempo real de los 12 sensores.

Esta pestaña no está pensada para utilizarse durante una carrera.

### 2. Telemetría

Modo de solo lectura para uso durante una carrera.

- Lectura de los 12 sensores.
- Posición calculada de la línea.
- Error.
- Contribución proporcional P.
- Contribución derivativa D.
- Salida PID.
- PWM de ambos motores.
- Potencia de EDF.
- Tiempo de loop.
- Estado de carrera y pérdida de línea.
- Grabación de todos los datos en CSV.

En este modo la PC no modifica motores, EDF ni parámetros PID.

## Estado actual

La primera versión utiliza un backend simulado a 50 Hz para poder desarrollar y probar la interfaz sin conectar todavía el ESP32-C3.

La siguiente etapa será reemplazar el simulador por la comunicación Wi-Fi real con Deimonyag.

## Instalación en Windows

Desde Git Bash, PowerShell o CMD, dentro de la carpeta `telemetry`:

```bash
python -m venv .venv
```

Activar el entorno virtual en Git Bash:

```bash
source .venv/Scripts/activate
```

En PowerShell:

```powershell
.\.venv\Scripts\Activate.ps1
```

Instalar dependencias:

```bash
pip install -r requirements.txt
```

Ejecutar:

```bash
python app.py
```

## Archivos de telemetría

Las grabaciones se guardan automáticamente en:

```text
telemetry/logs/
```

con nombres del tipo:

```text
run_2026-09-12_17-30-00.csv
```

Cada fila contiene los 12 sensores, posición, error, P, D, PID, motores, EDF, tiempo de loop y estados principales.
