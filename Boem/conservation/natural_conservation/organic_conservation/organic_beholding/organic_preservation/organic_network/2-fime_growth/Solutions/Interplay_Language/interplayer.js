const fs = require('fs');

let currentFile = 'format.interplay';
const learnedGroups = [];

console.log("start || ˈbu.ˈo͡ʊm Root Initialization");

// Helper to format long lines with even indentation for MIUI readability
function formatLongLine(key, text, indentSize) {
    const prefix = `  "${key}": "`;
    const indent = " ".repeat(prefix.length);
    // This allows the terminal to handle the wrapping while we provide the visual anchor
    return `${prefix}${text}",`;
}

function walkPath(fileName) {
    if (!fs.existsSync(fileName)) return;

    console.log(`looking || Accessing: ${fileName}`);
    const content = fs.readFileSync(fileName, 'utf8');
    const lines = content.split('\n');

    let currentEntry = null;

    lines.forEach(line => {
        const trimmed = line.trim();
        if (trimmed === '' || trimmed.endsWith('.interplay')) {
            if (trimmed.endsWith('.interplay')) setTimeout(() => walkPath(trimmed), 100);
            return;
        }

        // Logic: No tab = code | Tab = group
        if (!line.startsWith('\t') && !line.startsWith('  ')) {
            currentEntry = { code: trimmed, group: [] };
            learnedGroups.push(currentEntry);
        } else if (currentEntry) {
            currentEntry.group.push(trimmed);
        }
    });

    if (fileName === 'doing.interplay') {
        console.log("doing || Interplay Saturated. Outputting Clean Matrix:");
        console.log("[");
        learnedGroups.forEach((entry, index) => {
            const isLast = index === learnedGroups.length - 1;
            console.log("  {");
            console.log(`    "code": "${entry.code}",`);
            console.log(`    "group": ${JSON.stringify(entry.group, null, 2).replace(/\n/g, '\n    ')}`);
            console.log(`  }${isLast ? "" : ","}`);
        });
        console.log("]");
    }
}

walkPath(currentFile);
