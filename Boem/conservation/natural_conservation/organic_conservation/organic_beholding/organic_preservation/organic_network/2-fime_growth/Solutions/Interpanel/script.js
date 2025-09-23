// Interpanel Core
console.log("Boem Interpanel ready.");

// Load library JSON and populate
fetch('interpanel.json')
  .then(response => response.json())
  .then(data => {
    const libraryDiv = document.getElementById('library');
    data.library.forEach(book => {
      const a = document.createElement('a');
      a.href = book.file;
      a.target = "_blank";
      a.className = "book";
      a.textContent = `📘 ${book.title} — ${book.description}`;
      libraryDiv.appendChild(a);
    });
  })
  .catch(err => {
    console.error("Failed to load interpanel.json:", err);
  });