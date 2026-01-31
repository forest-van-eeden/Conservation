const fs = require('fs');

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
        const trimmed = line.trim();
        if (trimmed === '') return; // Skip empty rows

        // Path handling for sequential .interplay links
        if (trimmed.endsWith('.interplay')) {
            const nextFile = trimmed;
            console.log(`growing || Path found: moving to ${nextFile}`);
            setTimeout(() => walkPath(nextFile), 100); 
            return;
        }

        // IMPROVED LOGIC:
        // A line is a "code" header if it starts with "when" or "from" 
        // OR if it has NO leading whitespace.
        // Otherwise, it is part of the "group".
        if (trimmed.startsWith('when') || !line.startsWith('\t') && !line.startsWith('  ')) {
            currentEntry = { code: trimmed, group: [] };
            learnedGroups.push(currentEntry);
        } else if (currentEntry) {
            currentEntry.group.push(trimmed);
        }
    });

    if (fileName === 'doing.interplay') {
        console.log("doing || Interplay Saturated. Current Knowledge Base:");
        console.log(JSON.stringify(learnedGroups, null, 2));
    }
}

walkPath(currentFile);
