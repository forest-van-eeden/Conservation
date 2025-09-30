import { EULER } from './constants.js';

// Simple breathing loop based on Euler’s constant
export function startBreathing(callback) {
  let t = 0;
  const interval = setInterval(() => {
    const breath = Math.sin(t / EULER); // Euler-based rhythm
    callback(breath); // pass value (-1..1)
    t++;
  }, Math.round(EULER * 373)); // Euler * prime for timing

  return interval;
}

// Example usage:
// startBreathing(b => console.log("Breath:", b));
