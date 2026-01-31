const fs = require('fs');

let currentFile = 'format.interplay';
const learnedGroups = [];

console.log("start || ˈbu.ˈo͡ʊm Root Initialization");

// Function to handle the hanging indent for MIUI readability
function cleanLog(obj, depth = 0) {
    const space = "  ".repeat(depth);
    if (Array.isArray(obj)) {
        console.log(`${space}[`);
        obj.forEach((item, i) => {
            cleanLog(item, depth + 1);
            if (i < obj.length - 1) console.log(`${space}  },`);
        });
        console.log(`${space}]`);
    } else if (typeof obj === 'object') {
        console.log(`${space}{`);
        Object.keys(obj).forEach(key => {
            let val = obj[key];
            if (Array.isArray(val)) {
                console.log(`${space}  "${key}": [`);
                val.forEach((line, i) => {
                    // This creates the "Gutter" for wrapped text
                    const lineIndent = " ".repeat(6); 
                    console.log(`${space}${lineIndent}"${line}"${i < val.length - 1 ? ',' : ''}`);
                });
                console.log(`${space}  ]`);
            } else {
                console.log(`${space}  "${key}": "${val}",`);
            }
        });
    }
}

function walkPath(fileName) {
    if (!fs.existsSync(fileName)) return;

    const content = fs.readFileSync(fileName, 'utf8');
    const lines = content.split('\n');
    let currentEntry = null;

    lines.forEach(line => {
        const trimmed = line.trim();
        if (trimmed === '' || trimmed.endsWith('.interplay')) {
            if (trimmed.endsWith('.interplay')) setTimeout(() => walkPath(trimmed), 50);
            return;
        }

        // Logic: If the line doesn't start with a tab, it's a new CODE header
        if (!line.startsWith('\t') && !line.startsWith('  ')) {
            currentEntry = { code: trimmed, group: [] };
            learnedGroups.push(currentEntry);
        } else if (currentEntry) {
            currentEntry.group.push(trimmed);
        }
    });

    if (fileName === 'doing.interplay') {
        setTimeout(() => {
            console.log("doing || Interplay Saturated. Clean Matrix Output:");
            cleanLog(learnedGroups);
            console.log("  }"); // Close last object
            console.log("]");   // Close array
        }, 500);
    }
}

walkPath(currentFile);
