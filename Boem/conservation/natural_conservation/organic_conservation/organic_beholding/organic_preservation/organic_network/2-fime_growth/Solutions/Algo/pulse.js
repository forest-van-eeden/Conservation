// pulse.js — Organic Sound Feedback Loop
// Compatible with ES modules & Algorand Indexer (port 8980)

import playerFactory from 'play-sound';
import fetch from 'node-fetch';
import { fileURLToPath } from 'url';
import { dirname, join } from 'path';

// --- File system context ---
const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// --- Initialize player ---
const player = playerFactory({});

// --- Config ---
const INDEXER_URL = 'http://127.0.0.1:8980/health';
const SOUND_PATH = join(__dirname, 'sounds', 'pulse.wav');

// --- Organic Pulse Loop ---
async function organicPulse() {
  try {
    const res = await fetch(INDEXER_URL);
    const data = await res.json();

    // Basic feedback: if DB is available, play the organic pulse sound
    if (data['db-available']) {
      console.log('🌿 Algo is alive and syncing. Pulse active.');
      player.play(SOUND_PATH, (err) => {
        if (err) console.error('Sound error:', err);
      });
    } else {
      console.log('💤 Algo resting or disconnected.');
    }
  } catch (err) {
    console.error('⚠️ Pulse check failed:', err.message);
  }
}

// --- Loop every ~2e seconds (≈5.436s rounded down to 5s, Euler-prime adjusted) ---
setInterval(organicPulse, 5000);

console.log('💫 Organic pulse initialized — listening to Algo...');