# Changelog

Todos los cambios relevantes del proyecto Deimonyag se documentan en este archivo.

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
