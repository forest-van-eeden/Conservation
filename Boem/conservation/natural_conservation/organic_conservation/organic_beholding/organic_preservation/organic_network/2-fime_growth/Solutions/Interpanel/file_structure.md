genesis/
├── books/                  # App "cartridges" (each is a book)
│   ├── opto.html           # First book (OPTO demo)
│   └── book_sample.html    # Placeholder for future books
│
├── interpanel/             # Core Interpanel framework
│   ├── index.html          # Genesis entry panel (Interpanel frame)
│   ├── style.css           # Interpanel-specific styling
│   ├── script.js           # Interpanel-specific JS logic
│   └── organic.css         # Organic Usability styles (Euler + prime, colors)
│
├── organic/                # Organic growth + shared logic
│   ├── breathing.js        # Euler breathing loop
│   ├── primes.js           # Prime helpers
│   └── constants.js        # Global organic parameters
│
├── main.js                 # Electron app entry point
├── package.json            # Project config
├── package-lock.json
└── node_modules/