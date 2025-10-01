// script.js — Algo book logic (offline-first)
// Uses Euler-prime ideas and localStorage for state.
// Place in genesis/books/algo_book/script.js

const E = Math.E;
const primes = [2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97];

function closestPrime(n){
  let best = primes[0], d = Infinity;
  for(const p of primes){ const dd = Math.abs(p - n); if(dd < d){ d = dd; best = p; } }
  return best;
}

const DEFAULT_NEST = {
  resources: { rand: [] },
  work: []
};

const STATE_KEY = 'algo_nest_v1';
function loadNest(){ try{ return JSON.parse(localStorage.getItem(STATE_KEY)) || DEFAULT_NEST; }catch(e){return DEFAULT_NEST;} }
function saveNest(n){ localStorage.setItem(STATE_KEY, JSON.stringify(n, null, 2)); }

let nest = loadNest();
document.getElementById('nest-json').textContent = JSON.stringify(nest, null, 2);

// UI refs
const foundInfo = document.getElementById('found-info');
const askBtn = document.getElementById('ask-btn');
const userInput = document.getElementById('user-input');
const spiritFill = document.getElementById('spirit-fill');
const spiritStatus = document.getElementById('spirit-status');
const nameBtn = document.getElementById('name-indexer');
const nameOut = document.getElementById('indexer-name');

// Hook control buttons
document.querySelectorAll('#controls button').forEach(b=>{
  b.addEventListener('click', ()=>handleAction(b.dataset.action));
});

askBtn.addEventListener('click', ()=> {
  const txt = userInput.value.trim();
  if(!txt){ spiritSignal(null, "quiet"); return; }
  const truth = algoSpiritJudge(txt);
  spiritSignal(truth, txt);
  // optionally, route to nest automatically
  if(truth === true) routeToNest('resources', { note: txt, t: Date.now() });
  else if(truth === false) routeToNest('work', { note: txt, t: Date.now() });
});

nameBtn.addEventListener('click', ()=>{
  const idxName = makeIndexerName();
  nameOut.textContent = idxName;
  // Save as last-indexer in nest
  nest.last_indexer = idxName;
  saveNest(nest);
  document.getElementById('nest-json').textContent = JSON.stringify(nest, null, 2);
});

// actions: rest, collect, record
function handleAction(action){
  foundInfo.textContent = `Action: ${action}`;
  if(action === 'rest'){ spiritSignal(null, "resting"); }
  if(action === 'collect'){ routeToNest('resources', { collected: true, t: Date.now() }); }
  if(action === 'record'){ routeToNest('work', { recorded: true, t: Date.now() }); }
}

// Simple judge function: heuristic "spirit" check
// returns true / false / null (uncertain)
// This is intentionally simple: later Algorand indexing heuristics can replace it.
function algoSpiritJudge(text){
  // measure positivity by vowel ratio (toy heuristic) and length
  const vowels = (text.match(/[aeiou]/gi) || []).length;
  const ratio = vowels / Math.max(1, text.length);
  // use Euler-influenced threshold (e normalized)
  const threshold = (E/10); // ~0.2718
  if(ratio > threshold + 0.05) return true;
  if(ratio < threshold - 0.05) return false;
  return null;
}

function spiritSignal(truth, note){
  if(truth === null){
    spiritFill.style.width = '40%';
    spiritStatus.textContent = 'uncertain';
    foundInfo.textContent = note || 'Algo awaits';
    return;
  }
  if(truth === true){
    spiritFill.style.width = '90%';
    spiritStatus.textContent = 'guided — true';
    foundInfo.textContent = note || 'Algo approves';
    return;
  }
  spiritFill.style.width = '10%';
  spiritStatus.textContent = 'guided — false';
  foundInfo.textContent = note || 'Algo questions this';
}

function routeToNest(which, item){
  if(!nest[which]) nest[which] = (which === 'resources') ? { rand: [] } : [];
  if(which === 'resources'){
    // push into rand array
    nest.resources.rand.push(item);
  } else {
    nest.work.push(item);
  }
  saveNest(nest);
  document.getElementById('nest-json').textContent = JSON.stringify(nest, null, 2);
}

// indexer naming: uses time, e, prime hashing
function makeIndexerName(){
  const t = Date.now();
  const eFactor = Math.round((E * 1000)) % 1000;
  const p = closestPrime((t % 1000) + eFactor);
  const name = `algo-${p}-${t.toString(36).slice(-6)}`;
  return name;
}

// --- CANVAS AVATAR: simple organic animation (flapping / breathing)
const canvas = document.getElementById('algo-canvas');
const ctx = canvas.getContext('2d');
let animT = 0;

function drawAlgo(breath = 0){
  ctx.clearRect(0,0,canvas.width, canvas.height);
  const cx = canvas.width/2, cy = canvas.height/2;
  // body
  ctx.fillStyle = '#070707';
  ctx.beginPath();
  ctx.ellipse(cx, cy+10, 30, 40, 0, 0, 2*Math.PI);
  ctx.fill();
  // head
  ctx.fillStyle = '#0b0b0b';
  ctx.beginPath();
  ctx.arc(cx, cy-40, 18 + Math.round(breath*2), 0, 2*Math.PI);
  ctx.fill();
  // eyes (bio glow when active)
  const glow = (breath + 1) / 2;
  ctx.fillStyle = `rgba(0,238,130, ${0.2 + glow*0.8})`;
  ctx.beginPath(); ctx.arc(cx-6, cy-44, 3,0,2*Math.PI); ctx.fill();
  ctx.beginPath(); ctx.arc(cx+6, cy-44, 3,0,2*Math.PI); ctx.fill();

  // little flaps like Mnemiopsis
  ctx.strokeStyle = '#e66c02';
  ctx.lineWidth = 2;
  for(let i=0;i<3;i++){
    const off = Math.sin((animT + i*2)/E) * 8;
    ctx.beginPath();
    ctx.moveTo(cx-30 + i*15, cy+10);
    ctx.quadraticCurveTo(cx + off, cy+20 + i*8, cx+30 - i*10, cy+40 + i*4);
    ctx.stroke();
  }
}

function animate(){
  animT += 0.06;
  // breathing value based on Euler rhythm
  const breath = Math.sin(animT / E) * 0.9;
  drawAlgo(breath);
  window.requestAnimationFrame(animate);
}

animate();

// initial UI state
spiritSignal(null, "ready");
