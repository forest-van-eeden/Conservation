import { PRIMES } from './constants.js';

// Get closest prime to a number
export function closestPrime(n) {
  let diff = Infinity;
  let closest = n;
  for (let p of PRIMES) {
    const d = Math.abs(p - n);
    if (d < diff) {
      diff = d;
      closest = p;
    }
  }
  return closest;
}

// Example usage:
// console.log(closestPrime(100)); // 101
