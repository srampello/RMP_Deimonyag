// Interfaz web de Deimonyag (HTML + CSS + JS en un solo archivo).
// Se sirve en http://192.168.4.1 desde la flash del ESP32.
// Se puede editar directamente aca: todo lo que esta entre R"HTML( y )HTML"
// es la pagina tal cual.
#pragma once
#include <Arduino.h>

const char PAGINA_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1">
<meta name="theme-color" content="#070708">
<title>Deimonyag · RMP Robotics</title>
<style>
:root{
  --bg:#070708;--surface:#0d0d0f;--surface2:#141416;--surface3:#1b1b1e;--line:#252529;
  --text:#f7f7f8;--muted:#a8a8b0;--dim:#6c6c74;--red:#d21f2b;--red-hi:#f04b55;--red-dark:#7d0c14;--red-bg:#251013;
  --ok:#e9e9ec;--mono:ui-monospace,SFMono-Regular,Menlo,Consolas,monospace;
}
*{box-sizing:border-box}
html,body{margin:0;background:var(--bg);color:var(--text);font-family:system-ui,-apple-system,"Segoe UI",Roboto,Arial,sans-serif;font-size:15px;-webkit-tap-highlight-color:transparent}
button,input,select{font:inherit;color:inherit}
.wrap{max-width:1240px;margin:0 auto;padding:0 16px}

/* Cabecera */
header{position:sticky;top:0;z-index:20;background:rgba(7,7,8,.94);backdrop-filter:blur(8px);border-bottom:1px solid var(--line)}
.top{display:flex;align-items:center;gap:14px;height:76px}
.brand{display:flex;align-items:center;gap:12px;min-width:0}
.brand .rmp{height:40px;width:auto}
.brand .sep{width:1px;height:30px;background:var(--line)}
.brand .dem{height:58px;width:auto}
.status{margin-left:auto;display:flex;align-items:center;gap:8px}
.pill{display:inline-flex;align-items:center;gap:7px;padding:6px 10px;border-radius:999px;border:1px solid var(--line);background:var(--surface);font-size:11px;font-weight:700;letter-spacing:.09em;white-space:nowrap}
.pill i{width:8px;height:8px;border-radius:50%;background:var(--dim)}
.pill.on i{background:#fff;box-shadow:0 0 8px #fff}
.pill.run{background:var(--red-bg);border-color:#58181e;color:#ff8b92}
.pill.run i{background:var(--red-hi);box-shadow:0 0 10px var(--red-hi);animation:blink .6s infinite}
@keyframes blink{50%{opacity:.25}}
.stop{border:0;background:var(--red);color:#fff;font-weight:900;letter-spacing:.12em;padding:0 20px;height:42px;border-radius:10px;cursor:pointer;box-shadow:0 0 0 1px #ff5a64 inset,0 6px 20px rgba(210,31,43,.35)}
.stop:active{transform:translateY(1px);background:#b51722}

nav{display:flex;gap:2px;overflow-x:auto;scrollbar-width:none}
nav::-webkit-scrollbar{display:none}
nav button{background:none;border:0;padding:12px 16px 11px;color:var(--muted);font-weight:700;font-size:13px;letter-spacing:.08em;text-transform:uppercase;border-bottom:2px solid transparent;cursor:pointer;white-space:nowrap}
nav button:hover{color:var(--text)}
nav button.act{color:var(--text);border-bottom-color:var(--red)}

main.wrap{padding-top:22px;padding-bottom:40px}
.tab{display:none}
.tab.act{display:block}
h2{font-size:12px;letter-spacing:.14em;text-transform:uppercase;color:var(--muted);margin:0 0 14px;font-weight:700}
.grid{display:grid;gap:14px}
.g2{grid-template-columns:repeat(2,minmax(0,1fr))}
.g3{grid-template-columns:repeat(3,minmax(0,1fr))}
.pid-grid{grid-template-columns:minmax(300px,380px) minmax(0,1fr)}
.sens-grid{grid-template-columns:minmax(0,1fr) minmax(280px,360px)}
.card{background:var(--surface);border:1px solid var(--line);border-radius:14px;padding:18px;min-width:0}
.card.hl{border-color:#3a1418;background:linear-gradient(180deg,#120a0b,var(--surface) 60%)}
.note{color:var(--muted);font-size:13px;line-height:1.45;margin:10px 0 0}
.note b{color:var(--text)}
.row{display:flex;align-items:center;gap:10px;flex-wrap:wrap}
.sp{flex:1}
.mono{font-family:var(--mono)}
.big{font-family:var(--mono);font-size:44px;font-weight:700;line-height:1;letter-spacing:-.02em}
.kv{display:grid;grid-template-columns:repeat(auto-fill,minmax(112px,1fr));gap:8px}
.kv div{background:var(--surface2);border:1px solid var(--line);border-radius:10px;padding:10px 12px}
.kv span{display:block;font-size:10px;letter-spacing:.12em;text-transform:uppercase;color:var(--dim);margin-bottom:4px}
.kv b{font-family:var(--mono);font-size:19px;font-weight:600}

/* Controles */
.btn{border:1px solid var(--line);background:var(--surface2);padding:9px 14px;border-radius:9px;cursor:pointer;font-weight:600;font-size:13px}
.btn:hover{border-color:#3a3a40}
.btn:active{transform:translateY(1px)}
.btn.red{background:var(--red);border-color:var(--red);color:#fff}
.btn.ghost{background:none}
.btn:disabled{opacity:.4;cursor:not-allowed}
.btn.xl{width:100%;padding:16px;font-size:16px;font-weight:900;letter-spacing:.14em}
.seg{display:inline-flex;background:var(--surface2);border:1px solid var(--line);border-radius:10px;padding:3px;gap:3px}
.seg button{border:0;background:none;padding:7px 12px;border-radius:7px;color:var(--muted);font-weight:700;font-size:12px;letter-spacing:.06em;cursor:pointer}
.seg button.act{background:var(--text);color:#000}
input[type=range]{-webkit-appearance:none;appearance:none;width:100%;height:30px;background:none;margin:0}
input[type=range]::-webkit-slider-runnable-track{height:6px;border-radius:3px;background:var(--line)}
input[type=range]::-moz-range-track{height:6px;border-radius:3px;background:var(--line)}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:22px;height:22px;border-radius:50%;background:#fff;border:3px solid var(--red);margin-top:-8px}
input[type=range]::-moz-range-thumb{width:16px;height:16px;border-radius:50%;background:#fff;border:3px solid var(--red)}
input[type=range]:disabled{opacity:.35}
input[type=number],select{background:var(--surface2);border:1px solid var(--line);border-radius:8px;padding:8px 10px;width:100%;font-family:var(--mono)}
input[type=number]:focus,select:focus{outline:none;border-color:var(--red)}
label.chk{display:flex;align-items:center;gap:9px;cursor:pointer;font-size:14px}
label.chk input{width:18px;height:18px;accent-color:var(--red)}
.quick{display:flex;gap:6px;flex-wrap:wrap;margin-top:10px}
.quick .btn{padding:7px 10px;font-family:var(--mono);font-size:12px}

/* Motores */
.meter{height:10px;border-radius:5px;background:var(--surface3);position:relative;overflow:hidden;margin-top:12px}
.meter .mid{position:absolute;left:50%;top:0;bottom:0;width:1px;background:#3a3a40}
.meter .fill{position:absolute;top:0;bottom:0;background:var(--red)}
.meter.uni .fill{left:0}
.mlabel{display:flex;justify-content:space-between;font-size:11px;color:var(--dim);margin-top:6px;font-family:var(--mono)}

/* Sensores */
.bars{display:grid;grid-template-columns:repeat(12,1fr);gap:6px;align-items:end;height:220px;padding-top:6px}
.bar{position:relative;height:100%;background:var(--surface2);border-radius:6px;overflow:hidden;border:1px solid var(--line)}
.bar .f{position:absolute;left:0;right:0;bottom:0;background:linear-gradient(180deg,#fff,#cfcfd4)}
.bar.line .f{background:linear-gradient(180deg,var(--red-hi),var(--red))}
.blab{display:grid;grid-template-columns:repeat(12,1fr);gap:6px;margin-top:6px;text-align:center}
.blab div{font-size:10px;color:var(--dim);font-family:var(--mono)}
.blab div b{display:block;color:var(--text);font-size:11px;font-weight:600}
.track{position:relative;height:34px;background:var(--surface2);border:1px solid var(--line);border-radius:8px;margin-top:16px}
.track .c{position:absolute;left:50%;top:4px;bottom:4px;width:1px;background:#45454b}
.track .m{position:absolute;top:5px;bottom:5px;width:14px;margin-left:-7px;border-radius:4px;background:var(--red);box-shadow:0 0 12px rgba(240,75,85,.7);transition:left .05s linear}
.track.lost .m{background:var(--dim);box-shadow:none}
.prog{height:6px;background:var(--surface3);border-radius:3px;overflow:hidden;margin-top:12px}
.prog div{height:100%;width:0;background:var(--red)}
table{width:100%;border-collapse:collapse;font-family:var(--mono);font-size:12px}
th{font-weight:600;color:var(--dim);text-align:right;padding:5px 6px;font-size:10px;letter-spacing:.1em}
td{text-align:right;padding:4px 6px;border-top:1px solid var(--line)}
th:first-child,td:first-child{text-align:left}
.rng{height:6px;background:var(--surface3);border-radius:3px;position:relative;min-width:80px}
.rng div{position:absolute;top:0;bottom:0;background:#fff;border-radius:3px}

/* Boton / MicroStart */
.ind{width:150px;height:150px;border-radius:50%;margin:8px auto 14px;display:grid;place-items:center;border:3px solid var(--line);background:var(--surface2);font-weight:900;letter-spacing:.12em;font-size:14px;color:var(--muted);transition:all .08s}
.ind.on{border-color:var(--red-hi);background:radial-gradient(circle,var(--red) 0,var(--red-dark) 70%);color:#fff;box-shadow:0 0 40px rgba(210,31,43,.55)}
.center{text-align:center}
.led{width:26px;height:26px;border-radius:50%;background:#2a1013;border:2px solid #3a1418}
.led.on{background:var(--red-hi);border-color:#ff9aa1;box-shadow:0 0 18px var(--red-hi)}
.log{list-style:none;margin:0;padding:0;font-family:var(--mono);font-size:12px;max-height:260px;overflow:auto}
.log li{display:flex;justify-content:space-between;padding:7px 2px;border-top:1px solid var(--line)}
.log li b.s{color:var(--red-hi)}
.armed{border-color:var(--red)!important;box-shadow:0 0 0 1px var(--red) inset}

/* PID */
.param{display:grid;grid-template-columns:72px 1fr 34px 34px;gap:6px;align-items:center;margin-bottom:8px}
.param span{font-size:12px;font-weight:700;letter-spacing:.06em;color:var(--muted)}
.param .btn{padding:7px 0;text-align:center}
.param2{display:grid;grid-template-columns:110px 1fr 70px;gap:10px;align-items:center;margin-bottom:6px}
.param2 span{font-size:12px;color:var(--muted)}
.sub{font-size:11px;letter-spacing:.14em;text-transform:uppercase;color:var(--dim);margin:16px 0 8px;font-weight:700;border-top:1px solid var(--line);padding-top:14px}
.chart{width:100%;height:190px;display:block;background:var(--surface2);border:1px solid var(--line);border-radius:10px}
.legend{display:flex;gap:6px;flex-wrap:wrap;margin:10px 0 6px}
.legend button{display:flex;align-items:center;gap:6px;border:1px solid var(--line);background:var(--surface2);border-radius:999px;padding:4px 10px;font-size:12px;cursor:pointer;color:var(--muted)}
.legend button i{width:10px;height:3px;border-radius:2px}
.legend button.off{opacity:.35}
.strip{display:grid;grid-template-columns:repeat(12,1fr);gap:3px;margin-bottom:12px}
.strip div{height:16px;border-radius:3px;background:var(--surface3)}

/* Pie y avisos */
footer{border-top:1px solid var(--line);color:var(--dim);font-size:12px;padding:14px 0 28px}
footer .wrap{display:flex;gap:16px;flex-wrap:wrap}
footer b{color:var(--muted);font-weight:600}
.toast{position:fixed;left:50%;bottom:22px;transform:translateX(-50%) translateY(30px);opacity:0;background:var(--surface3);border:1px solid #3a3a40;border-left:3px solid var(--red);padding:12px 16px;border-radius:10px;font-size:14px;transition:all .25s;z-index:50;max-width:calc(100% - 32px);pointer-events:none}
.toast.show{opacity:1;transform:translateX(-50%) translateY(0)}
.off-overlay{display:none;position:fixed;inset:auto 0 0 0;background:var(--red-dark);color:#fff;text-align:center;font-size:13px;padding:8px;z-index:40;font-weight:600}
body.disc .off-overlay{display:block}

@media(max-width:900px){.g3{grid-template-columns:1fr 1fr}.pid-grid,.sens-grid{grid-template-columns:1fr}}
@media(max-width:640px){
  .g2,.g3{grid-template-columns:1fr}
  .brand .rmp{height:30px}.brand .dem{height:42px}.brand .sep{display:none}
  .pill.conn span{display:none}
  .stop{padding:0 14px;height:38px}
  .top{height:62px;gap:8px}
  .bars{height:170px;gap:3px}.blab{gap:3px}
  .big{font-size:36px}
}
</style>
</head>
<body class="disc">
<header>
  <div class="wrap top">
    <div class="brand">
      <img class="rmp" src="/rmp.png" alt="RMP Robotics">
      <div class="sep"></div>
      <img class="dem" src="/deimonyag.png" alt="Deimonyag">
    </div>
    <div class="status">
      <div class="pill conn" id="pConn"><i></i><span id="tConn">DESCONECTADO</span></div>
      <div class="pill" id="pEst"><i></i><span id="tEst">—</span></div>
      <button class="stop" id="bStop" title="Detener todo (barra espaciadora)">STOP</button>
    </div>
  </div>
  <nav class="wrap" id="nav">
    <button data-tab="motores" class="act">Motores</button>
    <button data-tab="sensores">Sensores</button>
    <button data-tab="boton">Botón</button>
    <button data-tab="microstart">MicroStart</button>
    <button data-tab="pid">PID</button>
  </nav>
</header>

<main class="wrap">

<!-- MOTORES -->
<section class="tab act" id="t-motores">
  <div class="card" style="margin-bottom:14px">
    <div class="row">
      <label class="chk"><input type="checkbox" id="mJuntos"> Mover juntos</label>
      <label class="chk"><input type="checkbox" id="mSoltar" checked> Volver a 0 al soltar</label>
      <div class="sp"></div>
      <button class="btn" id="mCero">Motores a 0</button>
    </div>
    <p class="note">Rango −255 a 255. Si la página deja de comunicarse por más de 0,6 s, el robot apaga los motores solo.</p>
  </div>
  <div class="grid g3">
    <div class="card">
      <h2>Motor izquierdo</h2>
      <div class="big" id="mlV">0</div>
      <input type="range" id="mlR" min="-255" max="255" value="0">
      <div class="quick" data-m="l"><button class="btn">-150</button><button class="btn">-80</button><button class="btn">0</button><button class="btn">80</button><button class="btn">150</button></div>
      <div class="meter"><div class="mid"></div><div class="fill" id="mlF"></div></div>
      <div class="mlabel"><span>atrás</span><span>salida real <b id="mlO">0</b></span><span>adelante</span></div>
    </div>
    <div class="card">
      <h2>Motor derecho</h2>
      <div class="big" id="mrV">0</div>
      <input type="range" id="mrR" min="-255" max="255" value="0">
      <div class="quick" data-m="r"><button class="btn">-150</button><button class="btn">-80</button><button class="btn">0</button><button class="btn">80</button><button class="btn">150</button></div>
      <div class="meter"><div class="mid"></div><div class="fill" id="mrF"></div></div>
      <div class="mlabel"><span>atrás</span><span>salida real <b id="mrO">0</b></span><span>adelante</span></div>
    </div>
    <div class="card hl">
      <h2>Succión · EDF27</h2>
      <div class="big"><span id="edfV">0</span><span style="font-size:22px;color:var(--muted)"> %</span></div>
      <label class="chk" style="margin-top:12px"><input type="checkbox" id="edfOk"> Robot sujeto y hélice libre</label>
      <input type="range" id="edfR" min="0" max="100" value="0" disabled>
      <div class="quick" id="edfQ"><button class="btn" disabled>0</button><button class="btn" disabled>15</button><button class="btn" disabled>30</button><button class="btn" disabled>50</button></div>
      <div class="meter uni"><div class="fill" id="edfF"></div></div>
      <div class="mlabel"><span>1000 µs</span><span>salida real <b id="edfO">0</b> %</span><span>2000 µs</span></div>
    </div>
  </div>
</section>

<!-- SENSORES -->
<section class="tab" id="t-sensores">
  <div class="grid sens-grid">
    <div class="card">
      <div class="row" style="margin-bottom:10px">
        <h2 style="margin:0">Lectura en vivo</h2><div class="sp"></div>
        <div class="seg" id="sModo"><button data-v="raw" class="act">Crudo</button><button data-v="cal">Calibrado</button></div>
      </div>
      <div class="bars" id="bars"></div>
      <div class="blab" id="blab"></div>
      <div class="track" id="track"><div class="c"></div><div class="m" id="trackM" style="left:50%"></div></div>
      <div class="mlabel"><span>Q1 · izquierda</span><span>centro 5500</span><span>Q12 · derecha</span></div>
      <div class="kv" style="margin-top:14px">
        <div><span>Posición</span><b id="sPos">—</b></div>
        <div><span>Error</span><b id="sErr">—</b></div>
        <div><span>Línea</span><b id="sOn">—</b></div>
        <div><span>Calibrado</span><b id="sIsc">—</b></div>
      </div>
      <p class="note">Crudo: 0–4095 del ADC. Con pull-up, <b>blanco = valor bajo</b> y negro o aire = valor alto. En rojo, los sensores que están viendo la línea.</p>
    </div>
    <div class="grid" style="align-content:start">
      <div class="card hl">
        <h2>Calibración</h2>
        <div class="param2"><span>Duración</span>
          <select id="calMs"><option value="3000">3 s</option><option value="5000" selected>5 s</option><option value="8000">8 s</option><option value="12000">12 s</option></select><span></span></div>
        <label class="chk" style="margin:10px 0"><input type="checkbox" id="calGira"> Girar sobre su eje (sin succión)</label>
        <div class="param2"><span>Velocidad giro</span><input type="range" id="calVel" min="20" max="150" value="60"><b class="mono" id="calVelV">60</b></div>
        <button class="btn red xl" id="calGo" style="margin-top:8px">CALIBRAR</button>
        <div class="prog"><div id="calP"></div></div>
        <p class="note">Sin girar: pasá la barra a mano por la línea y el fondo mientras dura. Se guarda en flash automáticamente.</p>
        <div class="sub">Tipo de línea</div>
        <div class="seg" id="sLinea"><button data-v="1">Blanca</button><button data-v="0">Negra</button></div>
      </div>
      <div class="card">
        <div class="row" style="margin-bottom:8px"><h2 style="margin:0">Mín / Máx</h2><div class="sp"></div><button class="btn ghost" id="calBorrar">Borrar</button></div>
        <table><thead><tr><th>Q</th><th>MÍN</th><th>MÁX</th><th style="width:40%">RANGO</th></tr></thead><tbody id="calT"></tbody></table>
      </div>
    </div>
  </div>
</section>

<!-- BOTON -->
<section class="tab" id="t-boton">
  <div class="grid g2">
    <div class="card center">
      <h2>Botón de estrategia · GPIO21</h2>
      <div class="ind" id="bInd">SUELTO</div>
      <div class="kv" style="max-width:340px;margin:0 auto">
        <div><span>Pulsaciones</span><b id="bCnt">0</b></div>
        <div><span>Última</span><b id="bLast">—</b></div>
      </div>
      <button class="btn ghost" id="bReset" style="margin-top:14px">Reiniciar contadores</button>
    </div>
    <div class="grid" style="align-content:start">
      <div class="card">
        <h2>Acción del botón</h2>
        <div class="seg" id="bAccion"><button data-v="0">Nada</button><button data-v="1">Calibrar</button><button data-v="2">Iniciar PID</button></div>
        <p class="note">Mientras corre el PID, tocar el botón <b>siempre frena</b> (igual que en Holdmoss). Guardá desde la pestaña PID para que quede en flash.</p>
      </div>
      <div class="card">
        <div class="row"><h2 style="margin:0">LED · GPIO9</h2><div class="sp"></div><div class="led" id="ledI"></div></div>
        <div class="seg" id="ledM" style="margin-top:12px"><button data-v="0">Auto</button><button data-v="1">Encendido</button><button data-v="2">Apagado</button></div>
        <p class="note">Auto: parpadeo lento esperando la web, destello corto conectado, rápido con MicroStart armado, fijo calibrando y encendido en recta durante el PID.</p>
      </div>
    </div>
  </div>
</section>

<!-- MICROSTART -->
<section class="tab" id="t-microstart">
  <div class="grid g2">
    <div class="card center" id="msCard">
      <h2>Señal MicroStart · GPIO20</h2>
      <div class="ind" id="msInd">STOP</div>
      <div class="kv" style="max-width:340px;margin:0 auto">
        <div><span>Cambios</span><b id="msCnt">0</b></div>
        <div><span>Estado</span><b id="msArmT">DESARMADO</b></div>
      </div>
      <button class="btn xl" id="msArm" style="margin-top:16px">ARMAR</button>
      <p class="note">Armado: <b>START</b> larga el PID con los parámetros actuales y <b>STOP</b> lo frena y desarma. Después de un reset del robot siempre arranca desarmado.</p>
    </div>
    <div class="card">
      <div class="row" style="margin-bottom:6px"><h2 style="margin:0">Registro</h2><div class="sp"></div><button class="btn ghost" id="msClr">Limpiar</button></div>
      <ul class="log" id="msLog"><li><span style="color:var(--dim)">Sin cambios todavía</span></li></ul>
    </div>
  </div>
</section>

<!-- PID -->
<section class="tab" id="t-pid">
  <div class="grid pid-grid">
    <div class="card">
      <h2>Parámetros · en vivo</h2>
      <div class="param"><span>KP</span><input type="number" id="kp" step="0.001"><button class="btn" data-s="kp:-0.001">−</button><button class="btn" data-s="kp:0.001">+</button></div>
      <div class="param"><span>KD</span><input type="number" id="kd" step="0.01"><button class="btn" data-s="kd:-0.01">−</button><button class="btn" data-s="kd:0.01">+</button></div>
      <div class="param"><span>KI</span><input type="number" id="ki" step="0.00001"><button class="btn" data-s="ki:-0.00001">−</button><button class="btn" data-s="ki:0.00001">+</button></div>
      <div class="sub">Velocidad</div>
      <div class="param2"><span>Base</span><input type="range" id="velBase" min="0" max="255"><b class="mono" id="velBaseV">0</b></div>
      <div class="param2"><span>Recta</span><input type="range" id="velRecta" min="0" max="255"><b class="mono" id="velRectaV">0</b></div>
      <label class="chk" style="margin:8px 0"><input type="checkbox" id="usarRecta"> Detección de recta</label>
      <div class="param2"><span>Umbral recta</span><input type="number" id="umbralRecta" step="50"><span class="mono" style="font-size:11px;color:var(--dim)">error</span></div>
      <div class="param2"><span>Tiempo recta</span><input type="number" id="tiempoRecta" step="10"><span class="mono" style="font-size:11px;color:var(--dim)">ms</span></div>
      <label class="chk" style="margin:8px 0"><input type="checkbox" id="arranqueSuave"> Arranque suave</label>
      <div class="sub">Avanzado</div>
      <div class="param2"><span>Límite I</span><input type="number" id="limiteI" step="1000"><span></span></div>
      <div class="param2"><span>Succión en PID</span><input type="range" id="edfPid" min="0" max="100"><b class="mono" id="edfPidV">0</b></div>
      <div class="param2"><span>Línea</span><div class="seg" id="pLinea"><button data-v="1">Blanca</button><button data-v="0">Negra</button></div><span></span></div>
      <div class="row" style="margin-top:16px">
        <button class="btn red" id="pGuardar">Guardar en flash</button>
        <button class="btn ghost" id="pDefecto">Valores por defecto</button>
      </div>
      <p class="note">Los cambios se aplican al instante, incluso con el robot andando. <b>Guardar</b> los deja para el próximo encendido.</p>
    </div>

    <div class="grid" style="align-content:start">
      <div class="card hl">
        <button class="btn red xl" id="pRun">INICIAR PID</button>
        <div class="strip" id="strip" style="margin-top:14px"></div>
        <div class="kv">
          <div><span>Posición</span><b id="pPos">—</b></div>
          <div><span>Error P</span><b id="pP">—</b></div>
          <div><span>Derivada D</span><b id="pD">—</b></div>
          <div><span>Integral I</span><b id="pI">—</b></div>
          <div><span>Corrección</span><b id="pC">—</b></div>
          <div><span>Velocidad</span><b id="pV">—</b></div>
          <div><span>Motor izq</span><b id="pML">—</b></div>
          <div><span>Motor der</span><b id="pMR">—</b></div>
          <div><span>Recta</span><b id="pRec">—</b></div>
          <div><span>Loop</span><b id="pHz">—</b></div>
        </div>
      </div>
      <div class="card">
        <div class="row">
          <h2 style="margin:0">Tiempo real</h2><div class="sp"></div>
          <div class="seg" id="cWin"><button data-v="5">5 s</button><button data-v="10" class="act">10 s</button><button data-v="20">20 s</button></div>
          <button class="btn" id="cPausa">Pausa</button>
        </div>
        <div class="legend" id="leg1"></div>
        <canvas class="chart" id="c1"></canvas>
        <div class="legend" id="leg2"></div>
        <canvas class="chart" id="c2"></canvas>
      </div>
    </div>
  </div>
</section>

</main>

<footer><div class="wrap">
  <span><b>RMP Robotics</b> · Deimonyag</span>
  <span>Firmware <b id="fFw">—</b></span>
  <span>Loop <b id="fLoop">—</b></span>
  <span>Barra espaciadora = STOP</span>
</div></footer>
<div class="off-overlay">Sin conexión con el robot · reconectando… (red WiFi DEIMONYAG)</div>
<div class="toast" id="toast"></div>

<script>
(()=>{
const $=id=>document.getElementById(id);
const N=12, CENTRO=5500;
let ws=null, conectado=false, cfg={}, ultimo=null;
let sModo='raw', ultimoWhy='', ultimoEstado=-1, ultimoMic=null, ultimoBc=0;

/* ---------- WebSocket ---------- */
function conectar(){
  const host=location.hostname||'192.168.4.1';
  try{ws=new WebSocket('ws://'+host+':81/');}catch(e){setTimeout(conectar,1000);return;}
  ws.onopen=()=>{conectado=true;document.body.classList.remove('disc');$('pConn').classList.add('on');$('tConn').textContent='CONECTADO';};
  ws.onclose=()=>{conectado=false;document.body.classList.add('disc');$('pConn').classList.remove('on');$('tConn').textContent='DESCONECTADO';setTimeout(conectar,1000);};
  ws.onerror=()=>{try{ws.close();}catch(e){}};
  ws.onmessage=ev=>{let m;try{m=JSON.parse(ev.data);}catch(e){return;}
    if(m.t==='s')estado(m);else if(m.t==='hola')hola(m);else if(m.t==='aviso')toast(m.msg);};
}
function send(o){if(ws&&ws.readyState===1)ws.send(JSON.stringify(o));}
setInterval(()=>send({t:'hb'}),200);

/* ---------- Avisos ---------- */
let toastT;
function toast(msg){const t=$('toast');t.textContent=msg;t.classList.add('show');clearTimeout(toastT);toastT=setTimeout(()=>t.classList.remove('show'),2800);}

/* ---------- Pestanas ---------- */
$('nav').addEventListener('click',e=>{const b=e.target.closest('button');if(!b)return;
  document.querySelectorAll('nav button').forEach(x=>x.classList.toggle('act',x===b));
  document.querySelectorAll('.tab').forEach(x=>x.classList.toggle('act',x.id==='t-'+b.dataset.tab));
  try{localStorage.setItem('tab',b.dataset.tab);}catch(e){}
  if(b.dataset.tab==='pid')redimensionar();
});
try{const t=localStorage.getItem('tab');if(t){const b=document.querySelector('nav button[data-tab="'+t+'"]');if(b)b.click();}}catch(e){}

function seg(id,fn){const el=$(id);el.addEventListener('click',e=>{const b=e.target.closest('button');if(!b)return;marcarSeg(id,b.dataset.v);fn(b.dataset.v);});}
function marcarSeg(id,v){$(id).querySelectorAll('button').forEach(x=>x.classList.toggle('act',x.dataset.v==String(v)));}

/* ---------- STOP ---------- */
function stopTodo(){send({t:'stop'});ponerMotores(0,0,true);setEdfUI(0);}
$('bStop').onclick=stopTodo;
document.addEventListener('keydown',e=>{if(e.code==='Space'&&!/INPUT|SELECT|TEXTAREA/.test(e.target.tagName)){e.preventDefault();stopTodo();}});

/* ---------- Motores ---------- */
let mL=0,mR=0,mEnvio=0,mPend=false;
function ponerMotores(l,r,enviar){
  mL=Math.round(l);mR=Math.round(r);
  $('mlR').value=mL;$('mrR').value=mR;$('mlV').textContent=mL;$('mrV').textContent=mR;
  if(enviar){const now=performance.now();if(now-mEnvio>50){mEnvio=now;send({t:'mot',l:mL,r:mR});}else if(!mPend){mPend=true;setTimeout(()=>{mPend=false;mEnvio=performance.now();send({t:'mot',l:mL,r:mR});},55);}}
}
['l','r'].forEach(k=>{
  const r=$('m'+k+'R');
  r.addEventListener('input',()=>{const v=+r.value;if($('mJuntos').checked)ponerMotores(v,v,true);else ponerMotores(k==='l'?v:mL,k==='r'?v:mR,true);});
  const soltar=()=>{if($('mSoltar').checked)ponerMotores($('mJuntos').checked?0:(k==='l'?0:mL),$('mJuntos').checked?0:(k==='r'?0:mR),true);};
  r.addEventListener('pointerup',soltar);r.addEventListener('touchend',soltar);
});
document.querySelectorAll('.quick[data-m]').forEach(q=>q.addEventListener('click',e=>{const b=e.target.closest('button');if(!b)return;const v=+b.textContent,k=q.dataset.m;
  if($('mJuntos').checked)ponerMotores(v,v,true);else ponerMotores(k==='l'?v:mL,k==='r'?v:mR,true);}));
$('mCero').onclick=()=>ponerMotores(0,0,true);
// Reenvio periodico mientras haya mando (mantiene vivo el watchdog del robot)
setInterval(()=>{if(mL||mR)send({t:'mot',l:mL,r:mR});},250);

function setEdfUI(v){$('edfR').value=v;$('edfV').textContent=v;}
$('edfOk').addEventListener('change',()=>{const ok=$('edfOk').checked;$('edfR').disabled=!ok;$('edfQ').querySelectorAll('button').forEach(b=>b.disabled=!ok);if(!ok){setEdfUI(0);send({t:'edf',v:0});}});
$('edfR').addEventListener('input',()=>{const v=+$('edfR').value;$('edfV').textContent=v;send({t:'edf',v});});
$('edfQ').addEventListener('click',e=>{const b=e.target.closest('button');if(!b||b.disabled)return;const v=+b.textContent;setEdfUI(v);send({t:'edf',v});});

function meterBi(el,v){const p=Math.abs(v)/255*50;el.style.left=(v>=0?50:50-p)+'%';el.style.width=p+'%';}

/* ---------- Sensores ---------- */
const bars=$('bars'),blab=$('blab'),strip=$('strip'),calT=$('calT');
for(let i=0;i<N;i++){
  bars.insertAdjacentHTML('beforeend','<div class="bar"><div class="f"></div></div>');
  blab.insertAdjacentHTML('beforeend','<div><b>0</b>Q'+(i+1)+'</div>');
  strip.insertAdjacentHTML('beforeend','<div></div>');
  calT.insertAdjacentHTML('beforeend','<tr><td>Q'+(i+1)+'</td><td>—</td><td>—</td><td><div class="rng"><div></div></div></td></tr>');
}
const barF=[...bars.querySelectorAll('.f')],barB=[...bars.children],labB=[...blab.querySelectorAll('b')],stripC=[...strip.children];
seg('sModo',v=>{sModo=v;});
function lineaVal(cal){return cfg.tipoLinea==1?1000-cal:cal;}
$('calVel').addEventListener('input',()=>$('calVelV').textContent=$('calVel').value);
$('calGo').onclick=()=>{send({t:'cal',ms:+$('calMs').value,gira:$('calGira').checked?1:0,vel:+$('calVel').value});};
$('calBorrar').onclick=()=>{if(confirm('¿Borrar la calibración guardada?'))send({t:'calReset'});};
function linea(v){cfg.tipoLinea=+v;marcarSeg('sLinea',v);marcarSeg('pLinea',v);send({t:'cfg',tipoLinea:+v});}
seg('sLinea',linea);seg('pLinea',linea);

function pintarCal(mn,mx){
  [...calT.rows].forEach((tr,i)=>{const a=mn[i],b=mx[i],ok=b>a;
    tr.cells[1].textContent=ok?a:'—';tr.cells[2].textContent=ok?b:'—';
    const d=tr.cells[3].firstChild.firstChild;d.style.left=(ok?a/4095*100:0)+'%';d.style.width=(ok?(b-a)/4095*100:0)+'%';});
}

/* ---------- Boton / LED ---------- */
seg('bAccion',v=>send({t:'cfg',accionBoton:+v}));
seg('ledM',v=>send({t:'cfg',led:+v}));
$('bReset').onclick=()=>send({t:'cntReset'});

/* ---------- MicroStart ---------- */
let armado=false;
$('msArm').onclick=()=>send({t:'arm',v:armado?0:1});
$('msClr').onclick=()=>{$('msLog').innerHTML='';};
function logMs(on){const li=document.createElement('li');const d=new Date();
  li.innerHTML='<span>'+d.toLocaleTimeString()+'.'+String(d.getMilliseconds()).padStart(3,'0')+'</span>'+(on?'<b class="s">START</b>':'<b>STOP</b>');
  const ul=$('msLog');if(ul.firstChild&&!ul.firstChild.querySelector('b'))ul.innerHTML='';ul.prepend(li);while(ul.children.length>30)ul.lastChild.remove();}

/* ---------- PID ---------- */
const campos=['kp','kd','ki','velBase','velRecta','umbralRecta','tiempoRecta','limiteI','edfPid'];
const checks=['usarRecta','arranqueSuave'];
let cfgT=null,cfgPend={};
function enviarCfg(k,v){cfg[k]=v;cfgPend[k]=v;clearTimeout(cfgT);cfgT=setTimeout(()=>{send(Object.assign({t:'cfg'},cfgPend));cfgPend={};},120);}
campos.forEach(k=>{const el=$(k);el.addEventListener('input',()=>{const v=parseFloat(el.value);if(isNaN(v))return;if($(k+'V'))$(k+'V').textContent=v;enviarCfg(k,v);});});
checks.forEach(k=>$(k).addEventListener('change',()=>enviarCfg(k,$(k).checked?1:0)));
document.querySelectorAll('[data-s]').forEach(b=>b.addEventListener('click',()=>{const[k,s]=b.dataset.s.split(':');const el=$(k);
  const dec=(s.split('.')[1]||'').length;let v=(parseFloat(el.value)||0)+parseFloat(s);if(v<0)v=0;v=+v.toFixed(dec);el.value=v;enviarCfg(k,v);}));
$('pGuardar').onclick=()=>send({t:'guardar'});
$('pDefecto').onclick=()=>{if(confirm('¿Volver a los valores por defecto? (no se guardan hasta tocar Guardar)'))send({t:'defecto'});};
$('pRun').onclick=()=>send({t:'pid',run:(ultimo&&ultimo.e===2)?0:1});

function cargarCfg(c){
  cfg=c;
  campos.forEach(k=>{if(c[k]!==undefined){$(k).value=c[k];if($(k+'V'))$(k+'V').textContent=c[k];}});
  checks.forEach(k=>$(k).checked=!!c[k]);
  marcarSeg('sLinea',c.tipoLinea);marcarSeg('pLinea',c.tipoLinea);marcarSeg('bAccion',c.accionBoton);marcarSeg('ledM',c.led);
  if(c.calMs)$('calMs').value=c.calMs;if(c.calVel){$('calVel').value=c.calVel;$('calVelV').textContent=c.calVel;}
}
function hola(m){cargarCfg(m.cfg);pintarCal(m.cmin,m.cmax);$('fFw').textContent=m.fw;}

/* ---------- Graficos ---------- */
const SERIES1=[{k:'P',n:'Error P',c:'#f04b55'},{k:'Dg',n:'D ×10',c:'#ffffff'}];
const SERIES2=[{k:'c',n:'Corrección',c:'#f04b55'},{k:'ml',n:'Motor izq',c:'#ffffff'},{k:'mr',n:'Motor der',c:'#8c8c94'}];
const buf=[];let ventana=10,pausa=false;
function leyenda(id,ser){const el=$(id);ser.forEach(s=>{s.on=true;const b=document.createElement('button');b.innerHTML='<i style="background:'+s.c+'"></i>'+s.n;b.onclick=()=>{s.on=!s.on;b.classList.toggle('off',!s.on);};el.appendChild(b);});}
leyenda('leg1',SERIES1);leyenda('leg2',SERIES2);
seg('cWin',v=>ventana=+v);
$('cPausa').onclick=()=>{pausa=!pausa;$('cPausa').textContent=pausa?'Seguir':'Pausa';};
const c1=$('c1'),c2=$('c2');
function redimensionar(){[c1,c2].forEach(c=>{const r=c.getBoundingClientRect();if(!r.width)return;const d=devicePixelRatio||1;c.width=r.width*d;c.height=r.height*d;});}
addEventListener('resize',redimensionar);
function dibujar(c,ser,lim,unidad){
  const ctx=c.getContext('2d'),W=c.width,H=c.height,d=devicePixelRatio||1;if(!W)return;
  ctx.clearRect(0,0,W,H);
  const padL=46*d,padR=8*d,padT=8*d,padB=8*d,w=W-padL-padR,h=H-padT-padB;
  ctx.font=(10*d)+'px ui-monospace,monospace';ctx.textAlign='right';ctx.textBaseline='middle';
  [-lim,-lim/2,0,lim/2,lim].forEach(v=>{const y=padT+h/2-v/lim*h/2;ctx.strokeStyle=v===0?'#45454b':'#222226';ctx.lineWidth=d;ctx.beginPath();ctx.moveTo(padL,y);ctx.lineTo(W-padR,y);ctx.stroke();ctx.fillStyle='#6c6c74';ctx.fillText(String(Math.round(v)),padL-6*d,y);});
  if(!buf.length)return;
  const tFin=buf[buf.length-1].ms,tIni=tFin-ventana*1000;
  ser.forEach(s=>{if(!s.on)return;ctx.strokeStyle=s.c;ctx.lineWidth=1.6*d;ctx.beginPath();let first=true;
    for(const p of buf){if(p.ms<tIni)continue;const x=padL+(p.ms-tIni)/(ventana*1000)*w;let v=p[s.k];if(v===undefined)continue;v=Math.max(-lim,Math.min(lim,v));const y=padT+h/2-v/lim*h/2;if(first){ctx.moveTo(x,y);first=false;}else ctx.lineTo(x,y);}
    ctx.stroke();});
}
function bucle(){if(!pausa&&$('t-pid').classList.contains('act')){dibujar(c1,SERIES1,CENTRO,'');dibujar(c2,SERIES2,255,'');}requestAnimationFrame(bucle);}
requestAnimationFrame(bucle);

/* ---------- Estado del robot ---------- */
const NOM=['LIBRE','CALIBRANDO','PID'],ORG=['web','botón','MicroStart'];
function fmt(n){return n===undefined?'—':(Math.round(n*10)/10).toString();}
function estado(m){
  ultimo=m;
  // cabecera
  const pe=$('pEst');pe.classList.toggle('run',m.e!==0);
  $('tEst').textContent=m.e===1?'CALIBRANDO '+(m.calp||0)+'%':m.e===2?'PID · '+ORG[m.o]:'LIBRE';
  if(m.why&&m.why!==ultimoWhy&&m.e===0)toast('Detenido: '+m.why);
  ultimoWhy=m.why;
  if(ultimoEstado!==m.e){
    if(ultimoEstado===1&&m.e===0)toast('Calibración lista y guardada');
    ultimoEstado=m.e;
    $('pRun').textContent=m.e===2?'DETENER PID':'INICIAR PID';
    $('pRun').classList.toggle('red',m.e!==2);
    $('calGo').disabled=m.e!==0;
  }
  $('fLoop').textContent=m.lu+' µs · '+m.hz+' Hz';

  // motores
  meterBi($('mlF'),m.ml);meterBi($('mrF'),m.mr);$('mlO').textContent=m.ml;$('mrO').textContent=m.mr;
  $('edfF').style.width=m.edf+'%';$('edfO').textContent=m.edf;
  if(m.e!==0&&(mL||mR))ponerMotores(0,0,false);

  // sensores
  const raw=m.raw,cal=m.cal;
  for(let i=0;i<N;i++){
    const lv=lineaVal(cal[i]);
    let v,p;
    if(sModo==='raw'&&raw){v=raw[i];p=v/4095;}else{v=cal[i];p=v/1000;}
    barF[i].style.height=(p*100)+'%';labB[i].textContent=v;
    barB[i].classList.toggle('line',m.isc&&lv>200);
    stripC[i].style.background=(m.isc&&lv>50)?'rgba(240,75,85,'+(0.25+lv/1333).toFixed(2)+')':'';
  }
  const tr=$('track');tr.classList.toggle('lost',!m.on);$('trackM').style.left=(m.pos/(CENTRO*2)*100)+'%';
  $('sPos').textContent=m.isc?m.pos:'—';$('sErr').textContent=m.isc?m.pos-CENTRO:'—';
  $('sOn').textContent=m.isc?(m.on?'SÍ':'PERDIDA'):'—';$('sIsc').textContent=m.isc?'SÍ':'NO';
  $('calP').style.width=(m.e===1?m.calp:0)+'%';

  // boton y LED
  $('bInd').classList.toggle('on',!!m.btn);$('bInd').textContent=m.btn?'PRESIONADO':'SUELTO';
  if(m.bc!==ultimoBc){if(m.bc>ultimoBc)$('bLast').textContent=new Date().toLocaleTimeString();ultimoBc=m.bc;}
  $('bCnt').textContent=m.bc;$('ledI').classList.toggle('on',!!m.led);

  // microstart
  $('msInd').classList.toggle('on',!!m.mic);$('msInd').textContent=m.mic?'START':'STOP';$('msCnt').textContent=m.mc;
  if(ultimoMic!==null&&ultimoMic!==m.mic)logMs(m.mic);ultimoMic=m.mic;
  armado=!!m.arm;$('msArmT').textContent=armado?'ARMADO':'DESARMADO';$('msArm').textContent=armado?'DESARMAR':'ARMAR';
  $('msArm').classList.toggle('red',armado);$('msCard').classList.toggle('armed',armado);

  // PID
  $('pPos').textContent=m.pos;$('pP').textContent=m.P;$('pD').textContent=m.D;$('pI').textContent=m.I;
  $('pC').textContent=fmt(m.c);$('pV').textContent=m.v;$('pML').textContent=m.ml;$('pMR').textContent=m.mr;
  $('pRec').textContent=m.rec?'SÍ':'NO';$('pHz').textContent=m.hz+' Hz';
  if(!pausa){buf.push({ms:m.ms,P:m.P,Dg:m.D*10,c:m.c,ml:m.ml,mr:m.mr});const lim=m.ms-21000;while(buf.length&&buf[0].ms<lim)buf.shift();}
}

redimensionar();
conectar();
})();
</script>
</body>
</html>
)HTML";
