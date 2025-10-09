import fetch from 'node-fetch';

const EULER = 2.718281828;
const PRIME_BASE = [2, 3, 5, 7, 11, 13, 17];
let tick = 0;

function nextPrimeTick() {
  const p = PRIME_BASE[tick % PRIME_BASE.length];
  tick++;
  return Math.floor(p * EULER * 100); // milliseconds
}

function renderWave(value) {
  const width = Math.round((value % 1) * 30) + 5;
  const wave = '~'.repeat(width);
  console.clear();
  console.log(`\nAlgo Pulse 🌿`);
  console.log(`Time: ${new Date().toLocaleTimeString()}`);
  console.log(`Connection Health: ${value.toFixed(4)}\n`);
  console.log(`#${wave}#\n`);
}

async function pulse() {
  try {
    const res = await fetch('http://127.0.0.1:8980/health');
    const data = await res.json();

    if (data && typeof data.round === 'number') {
      const val = (data.round % 100) / 100;
      renderWave(val);
    } else {
      renderWave(Math.random());
    }
  } catch (err) {
    console.log(`⚠️ Pulse check failed: ${err.message}`);
  } finally {
    setTimeout(pulse, nextPrimeTick());
  }
}

pulse();
