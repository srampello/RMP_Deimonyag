const files={main:'hardware/previews/SCH_Deimonyag_1-Main_Board_2026-09-12.svg',sensor:'hardware/previews/SCH_Deimonyag_2-12_Sensor_2026-09-12.svg'};
const stage=document.getElementById('svg-stage');
const img=document.getElementById('svg-image');
const hint=document.getElementById('hint');
const svgError=document.getElementById('svg-error');
let scale=1,x=0,y=0,drag=false,sx=0,sy=0;
function apply(){img.style.transform=`translate(calc(-50% + ${x}px),calc(-50% + ${y}px)) scale(${scale})`;document.getElementById('reset').textContent=Math.round(scale*100)+'%'}
function reset(){scale=1;x=0;y=0;apply()}
function show(kind){document.querySelectorAll('[data-view]').forEach(b=>b.classList.toggle('active',b.dataset.view===kind));svgError.classList.add('hidden');hint.textContent='Cargando esquemático…';img.src=files[kind];reset()}
img.addEventListener('load',()=>{svgError.classList.add('hidden');hint.textContent='Rueda del mouse: zoom · arrastrar: mover · doble clic: restablecer.'});
img.addEventListener('error',()=>{svgError.classList.remove('hidden');hint.textContent='No se pudo cargar el esquemático dentro del visor.'});
document.querySelectorAll('[data-view]').forEach(b=>b.addEventListener('click',()=>show(b.dataset.view)));
stage.addEventListener('wheel',e=>{e.preventDefault();scale=Math.min(12,Math.max(.2,scale*(e.deltaY<0?1.12:.89)));apply()},{passive:false});
stage.addEventListener('mousedown',e=>{drag=true;stage.classList.add('dragging');sx=e.clientX-x;sy=e.clientY-y});
window.addEventListener('mousemove',e=>{if(!drag)return;x=e.clientX-sx;y=e.clientY-sy;apply()});
window.addEventListener('mouseup',()=>{drag=false;stage.classList.remove('dragging')});
stage.addEventListener('dblclick',reset);
document.getElementById('plus').onclick=()=>{scale=Math.min(12,scale*1.2);apply()};
document.getElementById('minus').onclick=()=>{scale=Math.max(.2,scale/1.2);apply()};
document.getElementById('reset').onclick=reset;
show('main');
