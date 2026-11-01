# Changelog

Todos los cambios relevantes del proyecto Deimonyag se documentan en este archivo.

## [0.4] - 2026-11-01

### Agregado
- `firmware/Deimonyag_Web`: interfaz web servida por el robot (http://192.168.4.1)
  con logo y colores de RMP Robotics y pestañas Motores, Sensores, Botón,
  MicroStart y PID en tiempo real.
- PID con la estructura de Holdmoss (recta, arranque suave, freno por botón),
  parámetros en vivo y guardado en flash (NVS). Calibración guardada en flash.
- `QTRMux::setCalibration()` para cargar una calibración guardada.

### Eliminado
- Tests 01–04, `firmware/Deimonyag_WiFi`, la aplicación Python `telemetry/`,
  su workflow de compilación y `docs/PROTOCOL_WIFI.md` (reemplazados por la
  interfaz web; quedan en el historial de git).

## [0.3] - 2026-10-31

### Agregado
- `firmware/tests/05_Test_Sensores`: lectura de los 12 QRE1113GR por Monitor Serie
  (crudo, calibrado, posición de línea, calibración, tiempo y diagnóstico de canales).
- Librería `QTRMux`: misma API y lógica que QTRSensors (base Holdmoss) para sensores
  leídos a través del CD74HC4067.

### Verificado en placa
- Los 12 sensores funcionan con el firmware de prueba.
- Mapeo real del mux: Q1–Q12 en Y2–Y13 (difiere del esquemático, que indica Y0–Y11).
- Emisores en serie de a pares: una falla en Q12 apagaba también Q11. Corregido.

## [0.2] - 2026-09-12

### Cambiado
- Se reemplaza el regulador Mini360 por **Matek MICRO BEC 6–30 V**, configurado a 5 V.
- Se consolida la arquitectura de alimentación 3S / 5 V / 3,3 V.

### Definido
- ESP32-C3 Super Mini.
- 12 × QRE1113GR con CD74HC4067.
- Barra curva de sensores con radio 95 mm y separación angular 4,8°.
- 2 × IFX9201SG para tracción.
- 2 × JSumo ProFast 12 V 3600 RPM.
- EDF27 brushless + ESC.
- LiPo 3S 450 mAh 75C XT30.

### Documentación visual
- Se agregan exportaciones SVG de los dos esquemáticos de EasyEDA.
- Se agregan vistas PCB Top y Bottom en PDF vectorial.
- Se agrega `docs/HARDWARE.md` para visualizar esquemáticos y acceder a las PCB desde GitHub.
- Se mantiene `docs/hardware-viewer.html` preparado para navegación interactiva con zoom mediante GitHub Pages.

## [0.1]

### Inicial
- Primera arquitectura electrónica del robot.
- Primer mapa de GPIO y distribución de bloques.
