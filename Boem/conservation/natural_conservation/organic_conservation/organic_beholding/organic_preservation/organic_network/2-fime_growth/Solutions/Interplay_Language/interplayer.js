const fs = require('fs');

// The Organic Path defined in your format.interplay
let currentFile = 'format.interplay';
const learnedGroups = [];

console.log("start || ˈbu.ˈo͡ʊm Root Initialization");

function walkPath(fileName) {
    if (!fs.existsSync(fileName)) {
        console.log(`watching || Waiting for ${fileName} to manifest...`);
        return;
    }

    console.log(`looking || Accessing: ${fileName}`);
    const content = fs.readFileSync(fileName, 'utf8');
    const lines = content.split('\n');

    let currentEntry = null;

    lines.forEach(line => {
        // Path handling for sequential .interplay links
        if (line.trim().endsWith('.interplay')) {
            const nextFile = line.trim();
            console.log(`growing || Path found: moving to ${nextFile}`);
            // Organic delay for hardware stability
            setTimeout(() => walkPath(nextFile), 100); 
        }

        // Interplay Logic: 
        // No Tab = code (The primary group)
        // Tab = group (The details/supporting parts)
        if (line.trim() !== '' && !line.startsWith('\t') && !line.endsWith('.interplay')) {
            currentEntry = { code: line.trim(), group: [] };
            learnedGroups.push(currentEntry);
        } else if (line.startsWith('\t') && currentEntry) {
            currentEntry.group.push(line.trim());
        }
    });

    // Doing: Saturated output with your new terminology
    if (fileName === 'doing.interplay') {
        console.log("doing || Interplay Saturated. Current Knowledge Base:");
        console.log(JSON.stringify(learnedGroups, null, 2));
    }
}

walkPath(currentFile);
