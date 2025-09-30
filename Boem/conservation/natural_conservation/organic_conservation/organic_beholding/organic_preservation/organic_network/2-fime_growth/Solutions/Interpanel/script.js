import { startBreathing } from '../organic/breathing.js';

function openBook(book) {
  const container = document.getElementById('book-container');
  container.innerHTML = `<iframe src="../books/${book}" 
                          style="width:100%;height:91vh;border:none;"></iframe>`;
}

// Organic breathing background effect
window.addEventListener("DOMContentLoaded", () => {
  const frame = document.getElementById('frame');
  startBreathing(b => {
    const opacity = (b + 1) / 2; // normalize to 0..1
    frame.style.boxShadow = `0 0 ${Math.round(opacity * 17)}px #00ee82`;
  });
});

window.openBook = openBook;