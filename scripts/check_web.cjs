const fs = require('fs');
const path = require('path');
const root = path.resolve(__dirname, '..');
const source = fs.readFileSync(path.join(root, 'src/web_ui.h'), 'utf8');
const html = source.split('R"HTML(')[1].split(')HTML";')[0];
new (require('vm').Script)(html.match(/<script>([\s\S]*?)<\/script>/)[1]);
fs.mkdirSync(path.join(root, 'preview'), { recursive: true });
fs.writeFileSync(path.join(root, 'preview/dashboard.html'), html);
console.log('JavaScript syntax OK. Static preview exported (requires device API for live data).');
