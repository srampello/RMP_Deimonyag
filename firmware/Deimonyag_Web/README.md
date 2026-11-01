# Deimonyag_Web — interfaz web de pruebas y PID en tiempo real

El robot crea su propia red WiFi y sirve la interfaz desde la flash del ESP32-C3. No hace falta instalar nada en la PC ni en el celular: alcanza con un navegador.

```text
Red WiFi:   DEIMONYAG
Contraseña: deimonyag
Interfaz:   http://192.168.4.1
WebSocket:  ws://192.168.4.1:81  (datos en vivo y comandos)
```

## Cargar el firmware

1. Arduino IDE con **esp32 by Espressif Systems** 3.x.
2. Library Manager:
   - **WebSockets** de Markus Sattler (links2004)
   - **ArduinoJson** 7.x de Benoit Blanchon
3. Placa **ESP32C3 Dev Module** y **USB CDC On Boot: Enabled**.
4. Abrir `Deimonyag_Web.ino` y cargar. La librería `QTRMux` ya está dentro de la carpeta.

## Pestañas

| Pestaña | Qué hace |
|---|---|
| **Motores** | Cada motor de −255 a 255, mover juntos, volver a 0 al soltar. Succión EDF27 de 0 a 100 %, que se habilita solo con la casilla “robot sujeto”. |
| **Sensores** | 12 barras en vivo (crudo 0–4095 o calibrado 0–1000), posición de línea y error. Calibración a mano o girando sobre su eje, tabla mín/máx, tipo de línea. |
| **Botón** | Estado en vivo de GPIO21, contador, acción del botón (nada, calibrar o iniciar el PID). Control del LED de GPIO9. |
| **MicroStart** | Señal en vivo de GPIO20 con registro de cambios. Armado: START larga el PID y STOP lo frena y desarma. |
| **PID** | KP, KD y KI, velocidad base y en recta, detección de recta, arranque suave, límite de I y succión, todo en vivo con el robot andando. Gráficos de error, D, corrección y motores. Iniciar/detener y guardar en flash. |

El botón **STOP** de la cabecera (o la barra espaciadora) frena todo en cualquier pestaña.

## Control (base Holdmoss)

Mismo esquema que Holdmoss, campeón 2025:

- Posición con `readLine()` de 0 a 11000 y **centro en 5500**.
- `correccion = P·KP + D·KD + I·KI`, limitada a ±velocidad.
- Motores: `izq = vel + correccion`, `der = vel − correccion`, limitados de 0 a 255.
- Detección de recta: si |P| < umbral durante el tiempo de recta, sube a la velocidad de recta. El LED se enciende en recta.
- Arranque suave: sube 3 de velocidad cada 2 ms.
- Tocar el botón durante el PID siempre frena.

Los valores iniciales son los de Holdmoss reescalados al error de 12 sensores (±5500 en lugar de ±3500, o sea ×0,64): KP 0,024, KD 0,17, KI 0,00013. Hay que ajustarlos en pista, porque los motores, la frecuencia del loop y la succión son distintos.

## Seguridad

- **Watchdog manual:** si la página deja de comunicarse 0,6 s, los motores manuales y la succión se apagan.
- **PID iniciado desde la web:** se frena si se pierde la conexión (1,5 s sin latido o WebSocket cerrado).
- **PID iniciado por botón o MicroStart:** sigue corriendo aunque no haya web. Se frena con el botón, con el MicroStart o con STOP.
- El MicroStart siempre arranca desarmado después de un reset.
- `GPIO20` usa `INPUT_PULLDOWN`: sin módulo conectado lee STOP.
- Mientras corre el PID o la calibración, los mandos manuales de motores quedan bloqueados.

## Flash

- **Calibración:** se guarda automáticamente al terminar y se carga al encender.
- **Parámetros:** se guardan con “Guardar en flash”. “Valores por defecto” vuelve a los iniciales, pero no los guarda hasta tocar Guardar.

## Ajustes en el código

| Constante | Para qué |
|---|---|
| `MOTOR_IZQ_INVERTIDO` / `MOTOR_DER_INVERTIDO` | Si un motor gira al revés |
| `muxChannels` | Orden real de los sensores en el mux (Y2–Y13 en la placa actual) |
| `NUM_SAMPLES_PER_SENSOR` | Lecturas promediadas por sensor (3) |
| `WiFi.setTxPower(...)` | Descomentar si el WiFi de la Super Mini se corta seguido |

## Archivos

| Archivo | Contenido |
|---|---|
| `Deimonyag_Web.ino` | WiFi, servidor web, WebSocket, motores, sensores, PID, entradas |
| `web_page.h` | La página completa (HTML, CSS y JS). Se puede editar directo |
| `logos.h` | Logos RMP y Deimonyag en PNG, generados desde `images/logos/` |
| `QTRMux.h/.cpp` | Librería de sensores por mux (la misma del test 05) |

## Protocolo WebSocket

Todos los mensajes son JSON. Campo `t` = tipo.

**Robot → navegador**

- `hola` (al conectar): `fw`, `cfg{...}`, `cmin[12]`, `cmax[12]`.
- `s` (estado, 25 Hz libre / 50 Hz en PID):
  - `e`: estado (0 libre, 1 calibrando, 2 PID); `o`: origen (0 web, 1 botón, 2 MicroStart); `why`: motivo de parada.
  - `raw[12]` (fuera del PID), `cal[12]`, `pos`, `on`, `isc`.
  - `P`, `I`, `D`, `c`, `v`, `rec`.
  - `ml`, `mr`, `edf`.
  - `btn`, `bc`, `mic`, `mc`, `arm`, `led`.
  - `calp` (calibrando), `lu` (µs de loop), `hz` (loops por segundo).
- `aviso`: `msg`.

**Navegador → robot**

```json
{"t":"hb"}                                   latido cada 200 ms
{"t":"stop"}                                 frena todo
{"t":"mot","l":120,"r":-80}                  motores manuales
{"t":"edf","v":30}                           succión %
{"t":"cal","ms":5000,"gira":1,"vel":60}      calibrar
{"t":"calReset"}                             borrar calibración
{"t":"pid","run":1}                          iniciar (0 = detener)
{"t":"cfg","kp":0.03,"velBase":100}          cambiar parámetros en vivo
{"t":"arm","v":1}                            armar MicroStart
{"t":"guardar"}  {"t":"defecto"}  {"t":"cntReset"}
```
