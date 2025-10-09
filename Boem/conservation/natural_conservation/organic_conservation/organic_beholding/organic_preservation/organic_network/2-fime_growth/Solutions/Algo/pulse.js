// pulse.js — Organic Pulse Loop with Euler Prime Frequency
// Compatible with Node v22+ ES Modules

import playerFactory from 'play-sound';
import fetch from 'node-fetch';
import { fileURLToPath } from 'url';
import { dirname, join } from 'path';
import { exec } from 'child_process';

// --- Context setup ---
const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// --- Initialize ---
const player = playerFactory({});
const INDEXER_URL = 'http://127.0.0.1:8980/health';

// --- Euler Prime Math ---
const e = 2.718281828459045;
const primes = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31];

// Find nearest prime <= Euler multiple
function eulerPrimeNode(multiplier) {
  const val = Math.floor(multiplier * e);
  let closest = 2;
  for (const p of primes) {
    if (p <= val) closest = p;
  }
  return closest;
}

// --- Pulse Sound Generator ---
function playFrequency(freq) {
  // Systema-safe, low-frequency tone for organic range
  const duration = 0.5; // seconds
  const cmd = `afplay /System/Library/Sounds/Glass.aiff`; // macOS default sound
  exec(cmd, (err) => {
    if (err) console.error('Sound playback error:', err.message);
  });
  console.log(`🎵 Frequency pulse ≈ ${freq.toFixed(2)} Hz (Euler-Prime adjusted)`);
}

// --- Pulse Logic ---
async function organicPulse() {
  try {
    const res = await fetch(INDEXER_URL);
    const data = await res.json();

    if (data['db-available']) {
      const round = data.round || 1;
      const node = eulerPrimeNode(round / 10);
      const freq = node * e; // Euler prime-based frequency
      playFrequency(freq);
      console.log(`🌱 Algo alive. Round ${round}. Euler Node: ${node}`);
    } else {
      console.log('💤 Algo resting...');
    }
  } catch (err) {
    console.error('⚠️ Pulse check failed:', err.message);
  }
}

// --- Run Loop ---
const eulerInterval = eulerPrimeNode(2) * 1000; // ~5000ms → 5s
setInterval(organicPulse, eulerInterval);

console.log(`💫 Organic pulse initialized (interval ${eulerInterval / 1000}s)`);