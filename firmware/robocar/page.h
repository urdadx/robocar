#pragma once

// The phone control page, served at /.
const char PAGE[] = R"page(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no,viewport-fit=cover">
<meta name="theme-color" content="#101614">
<title>RoboCar</title>
<style>
  :root { --bg:#101614; --panel:#18211e; --line:#2b3733; --fg:#e8eeeb; --muted:#8e9c97; --accent:#4f8ff0; --ok:#3fb67a; --bad:#e0564c; --warn:#e0a63c; }
  * { box-sizing:border-box; }
  html, body { height:100%; margin:0; }
  body { background:var(--bg); color:var(--fg); font:16px/1.4 system-ui,-apple-system,"Segoe UI",sans-serif;
    display:flex; flex-direction:column; user-select:none; -webkit-user-select:none; touch-action:none; overscroll-behavior:none;
    padding:env(safe-area-inset-top) max(16px,env(safe-area-inset-right)) env(safe-area-inset-bottom) max(16px,env(safe-area-inset-left)); }
  header { display:flex; align-items:center; gap:12px; padding:14px 0; }
  h1 { font-size:20px; margin:0; flex:1; letter-spacing:.02em; }
  .pill { display:inline-flex; align-items:center; gap:8px; font-size:14px; color:var(--muted); }
  .pill i { width:10px; height:10px; border-radius:50%; background:var(--warn); }
  .pill[data-state=online] i { background:var(--ok); }
  .pill[data-state=offline] i { background:var(--bad); }
  #rtt { font-variant-numeric:tabular-nums; color:var(--muted); font-size:14px; min-width:5ch; text-align:right; }
  main { flex:1; display:flex; flex-direction:column; align-items:center; justify-content:center; gap:28px; padding-bottom:16px; }
  @media (orientation:landscape) { main { flex-direction:row; gap:48px; } }
  #pad { position:relative; width:min(78vw,52vh,340px); aspect-ratio:1; border-radius:50%; background:var(--panel);
    border:2px solid var(--line); touch-action:none; cursor:grab; }
  @media (orientation:landscape) { #pad { width:min(70vh,44vw,340px); } }
  #pad::before, #pad::after { content:""; position:absolute; background:var(--line); }
  #pad::before { left:50%; top:12%; bottom:12%; width:2px; transform:translateX(-1px); }
  #pad::after { top:50%; left:12%; right:12%; height:2px; transform:translateY(-1px); }
  #knob { position:absolute; left:50%; top:50%; width:34%; aspect-ratio:1; margin:-17%; border-radius:50%;
    background:var(--accent); box-shadow:0 6px 20px rgba(0,0,0,.4); z-index:1; pointer-events:none; }
  #pad.active { border-color:var(--accent); cursor:grabbing; }
  .side { display:flex; flex-direction:column; gap:18px; width:min(100%,340px); }
  label { display:flex; justify-content:space-between; font-size:14px; color:var(--muted); }
  label b { color:var(--fg); font-variant-numeric:tabular-nums; }
  input[type=range] { width:100%; accent-color:var(--accent); height:32px; }
  #stop { font:inherit; font-weight:700; font-size:18px; letter-spacing:.06em; color:#fff; background:var(--bad);
    border:0; border-radius:14px; height:64px; cursor:pointer; touch-action:none; }
  #stop:active { filter:brightness(1.15); }
  #stop:focus-visible, input:focus-visible { outline:3px solid var(--accent); outline-offset:3px; }
  .hint { font-size:13px; color:var(--muted); margin:0; }
  #readout { font-size:13px; color:var(--muted); font-variant-numeric:tabular-nums; margin:0; }
</style>
</head>
<body>
<header>
  <h1>RoboCar</h1>
  <span id="rtt"></span>
  <span class="pill" id="conn" data-state="connecting"><i></i><span>Connecting</span></span>
</header>
<main>
  <div id="pad" aria-label="Drive joystick"><div id="knob"></div></div>
  <div class="side">
    <label for="speed">Top speed <b id="speedValue">70%</b></label>
    <input id="speed" type="range" min="20" max="100" step="5" value="70">
    <button id="stop" type="button">STOP</button>
    <p id="readout">Throttle 0 · Steer 0</p>
    <p class="hint">Drag the joystick, or use W A S D / arrow keys. Space stops. The car stops on its own if this page loses connection.</p>
  </div>
</main>
<script>
const pad = document.getElementById('pad'), knob = document.getElementById('knob');
const conn = document.getElementById('conn'), rttEl = document.getElementById('rtt');
const speed = document.getElementById('speed'), speedValue = document.getElementById('speedValue');
const readout = document.getElementById('readout');

let ws, throttle = 0, steer = 0, active = false;

function setConn(state, text) { conn.dataset.state = state; conn.lastElementChild.textContent = text; }

function connect() {
  ws = new WebSocket(`ws://${location.host}/ws`);
  ws.onopen = () => setConn('online', 'Connected');
  ws.onclose = () => { setConn('offline', 'Reconnecting'); rttEl.textContent = ''; setTimeout(connect, 1000); };
  ws.onmessage = e => {
    const [kind, value] = e.data.split(' ');
    if (kind === 'pong') rttEl.textContent = `${Math.round(performance.now() - Number(value))} ms`;
  };
}

function send(text) { if (ws && ws.readyState === WebSocket.OPEN) ws.send(text); }

function command() {
  const limit = Number(speed.value);
  return [Math.round(throttle * limit), Math.round(steer * limit)];
}

function setInput(t, s) {
  const wasActive = active;
  throttle = t; steer = s;
  active = t !== 0 || s !== 0;
  const [ct, cs] = command();
  readout.textContent = `Throttle ${ct} · Steer ${cs}`;
  if (active) send(`drive ${ct} ${cs}`);
  else if (wasActive) send('stop');
}

// Keep sending while driving so the car's 400 ms safety timeout never trips.
setInterval(() => { if (active) { const [ct, cs] = command(); send(`drive ${ct} ${cs}`); } }, 100);
setInterval(() => send(`ping ${Math.round(performance.now())}`), 1000);

// Joystick
function moveKnob(x, y) { knob.style.transform = `translate(${x}px, ${y}px)`; }
function fromPointer(e) {
  const r = pad.getBoundingClientRect(), radius = r.width / 2 - knob.offsetWidth / 4;
  let x = e.clientX - (r.left + r.width / 2), y = e.clientY - (r.top + r.height / 2);
  const d = Math.hypot(x, y);
  if (d > radius) { x *= radius / d; y *= radius / d; }
  moveKnob(x, y);
  const dead = 0.08, t = -y / radius, s = x / radius;
  setInput(Math.abs(t) < dead ? 0 : t, Math.abs(s) < dead ? 0 : s);
}
function releasePad() { pad.classList.remove('active'); moveKnob(0, 0); setInput(0, 0); }
pad.addEventListener('pointerdown', e => { e.preventDefault(); pad.setPointerCapture(e.pointerId); pad.classList.add('active'); fromPointer(e); });
pad.addEventListener('pointermove', e => { if (pad.hasPointerCapture(e.pointerId)) fromPointer(e); });
pad.addEventListener('pointerup', releasePad);
pad.addEventListener('pointercancel', releasePad);

// Keyboard
const held = new Set();
const keyMap = { ArrowUp:'f', w:'f', ArrowDown:'b', s:'b', ArrowLeft:'l', a:'l', ArrowRight:'r', d:'r' };
function fromKeys() { setInput((held.has('f') ? 1 : 0) - (held.has('b') ? 1 : 0), (held.has('r') ? 1 : 0) - (held.has('l') ? 1 : 0)); }
document.addEventListener('keydown', e => {
  if (e.key === ' ') { e.preventDefault(); held.clear(); setInput(0, 0); send('stop'); return; }
  const k = keyMap[e.key.length === 1 ? e.key.toLowerCase() : e.key];
  if (!k || document.activeElement === speed) return;
  e.preventDefault(); held.add(k); fromKeys();
});
document.addEventListener('keyup', e => {
  const k = keyMap[e.key.length === 1 ? e.key.toLowerCase() : e.key];
  if (k) { held.delete(k); fromKeys(); }
});

function stopAll() { held.clear(); releasePad(); send('stop'); }
document.getElementById('stop').addEventListener('pointerdown', e => { e.preventDefault(); stopAll(); });
document.getElementById('stop').addEventListener('click', stopAll);
window.addEventListener('blur', stopAll);
document.addEventListener('visibilitychange', () => { if (document.hidden) stopAll(); });

speed.addEventListener('input', () => { speedValue.textContent = `${speed.value}%`; });

connect();
</script>
</body>
</html>
)page";
