function isPrime(num) {
  if (num < 2) return false;
  for (let i = 2; i * i <= num; i++) {
    if (num % i === 0) return false;
  }
  return true;
}

function nearestLowerPrime(num) {
  for (let n = Math.floor(num); n >= 2; n--) {
    if (isPrime(n)) return n;
  }
  return 2;
}

function eulerPrimeNodes(count = 12) {
  const e = Math.E;
  const nodes = [];
  for (let k = 1; k <= count; k++) {
    const value = k * e;
    const primeNode = nearestLowerPrime(value);
    nodes.push(primeNode);
  }
  return nodes;
}

console.log(eulerPrimeNodes(15));
