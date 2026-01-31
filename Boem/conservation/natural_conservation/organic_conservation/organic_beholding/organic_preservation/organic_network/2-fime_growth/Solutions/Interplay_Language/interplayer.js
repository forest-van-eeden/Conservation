const fs = require('fs');
const path = require('path');

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

    let currentGroup = null;

    lines.forEach(line => {
        // Detect 'start' or next file links
        if (line.trim().endsWith('.interplay')) {
            const nextFile = line.trim();
            console.log(`growing || Path found: moving to ${nextFile}`);
            // Restful delay to simulate organic growth
            setTimeout(() => walkPath(nextFile), 100); 
        }

        // Interplay Logic: No Tab = Code Group | Tab = Details
        if (line.trim() !== '' && !line.startsWith('\t') && !line.endsWith('.interplay')) {
            currentGroup = { head: line.trim(), details: [] };
            learnedGroups.push(currentGroup);
        } else if (line.startsWith('\t') && currentGroup) {
            currentGroup.details.push(line.trim());
        }
    });

    // Doing: Final report of the cell-by-cell learning
    if (fileName === 'doing.interplay') {
        console.log("doing || Interplay Saturated. Current Knowledge Base:");
        console.log(JSON.stringify(learnedGroups, null, 2));
    }
}

// Begin the walk
walkPath(currentFile);
