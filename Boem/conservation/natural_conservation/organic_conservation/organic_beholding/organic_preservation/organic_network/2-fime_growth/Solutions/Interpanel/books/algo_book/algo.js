// Euler’s constant approximation
const E = 2.718;

// Small organic alphabet for name generation
const letters = "abcdefghijklmnopqrstuvwxyz";

// Generate a unique organic word
function generateName() {
  let length = 5 + (Math.floor(Math.random() * 3)); // 5–7 letters
  let name = "";
  for (let i = 0; i < length; i++) {
    let idx = Math.floor((Math.random() * letters.length + E) % letters.length);
    name += letters[idx];
  }
  return name.charAt(0).toUpperCase() + name.slice(1);
}

let indexerName = null;

// Canvas avatar
const canvas = document.getElementById("algo-avatar");
const ctx = canvas.getContext("2d");

function drawAvatar(flapping) {
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  ctx.strokeStyle = "#e66c02";
  ctx.lineWidth = 3;

  ctx.beginPath();
  let base = 116;
  let flap = flapping ? Math.sin(Date.now() / 233) * 11 : 0;
  ctx.moveTo(base - 20 - flap, base);
  ctx.lineTo(base, base - 40);
  ctx.lineTo(base + 20 + flap, base);
  ctx.lineTo(base, base + 40);
  ctx.closePath();
  ctx.stroke();
}

// Spirit meter update
function setSpirit(value) {
  document.getElementById("spirit-value").innerText = value ? "True" : "False";
}

// Button bindings
document.getElementById("spawn-indexer").onclick = () => {
  if (!indexerName) {
    indexerName = generateName();
    document.getElementById("indexer-value").innerText = indexerName;
  }
};

document.getElementById("check-transactions").onclick = async () => {
  if (!indexerName) {
    alert("Spawn the indexer first!");
    return;
  }

  try {
    // Call Algorand Indexer API (example: AlgoExplorer)
    const res = await fetch("https://algoindexer.algoexplorerapi.io/v2/transactions?limit=1");
    const data = await res.json();
    const tx = data.transactions[0];

    // Spirit = true if transaction has non-empty note
    const hasNote = tx.note !== undefined;
    setSpirit(hasNote);

    // Animate avatar on note
    drawAvatar(hasNote);

  } catch (err) {
    console.error("Algo check failed:", err);
  }
};

// Initial draw
drawAvatar(false);
