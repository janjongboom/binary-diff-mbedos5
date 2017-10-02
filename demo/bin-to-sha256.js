const fs = require('fs');
const Path = require('path');
const crypto = require('crypto');

if (!process.argv[2]) {
    console.error('Usage: bin-to-sha256.js filename.bin');
    process.exit(1);
}

const file = fs.readFileSync(process.argv[2]);

let hash = crypto.createHash('sha256').update(file).digest();

console.log(hash.toString('hex'));

console.log('{', Array.from(hash).map(c => '0x' + c.toString(16)).join(', '), '}');
