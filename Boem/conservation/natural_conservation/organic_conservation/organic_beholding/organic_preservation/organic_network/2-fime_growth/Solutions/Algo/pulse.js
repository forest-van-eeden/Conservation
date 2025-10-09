// pulse.js
const fs = require('fs');
const path = require('path');
const blessed = require('blessed');
const contrib = require('blessed-contrib');
import player from 'play-sound';
const play = player();
const file = path.join(__dirname, 'algo_memory.txt');

// Create console UI
const screen = blessed.screen();
const box = blessed.box({
  top: 'center',
  left: 'center',
  width: '50%',
  height: '50%',
  content: '',
  tags: true,
  border: { type: 'line' },
  style: {
    fg: '#00ee82',
    border: { fg: '#e66c02' },
  },
});
screen.append(box);

function playTone(frequency) {
  // Sine tone synthesized via afplay using macOS
  const duration = 0.3; // seconds
  const file = `/System/Library/Sounds/Glass.aiff`; // lightweight tone
  play.play(file);
}

function readPulses() {
  try {
    const data = fs.readFileSync(file, 'utf8');
    const lines = data.split('\n').filter(Boolean);
    return lines.length;
  } catch {
    return 0;
  }
}

function animate() {
  const pulses = readPulses();
  const pulseSize = (pulses % 10) + 2;
  const circle = '•'.repeat(pulseSize);
  box.setContent(`{center}${circle}{/center}\n\nPulses sensed: ${pulses}`);
  screen.render();
  
  // Play tone every update
  const freq = 220 + ((pulses % 10) * 44); // 220–660 Hz range
  playTone(freq);
}

setInterval(animate, 2000);
screen.key(['escape', 'q', 'C-c'], () => process.exit(0));
animate();
