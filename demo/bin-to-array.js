const fs = require('fs');
const Path = require('path');

if (!process.argv[2]) {
    console.error('Usage: bin-to-array.js filename.bin');
    process.exit(1);
}

const file = fs.readFileSync(process.argv[2]);

console.log('{', Array.from(file).map(c => {
    c = c.toString(16);
    if (c.length === 0) c = '0' + c;
    return '0x' + c;
}).join(', '), '}');
