/* This was supposed to document a javascript bug, but turned out to
** document a javascript usage error instead.
** Oopsie: Strings with utf8 content have different lengths than Buffers.
*/

const fs = require('fs');
const http = require('http');

const HOST= 'localhost';
const PORT= 8080;

const data= fs.readFileSync('html/utf8.html').toString(); // Sample file

const server = http.createServer();
server.on('error', err => {
   console.log('on_error', err.message);
   server.close();
   process.exit(1);
});

var request= 0;
server.on('request', (req, res) => {
   var resp= data;
   if( (++request) % 2 )            // Alternate working/failing
     resp= Buffer.from(data);

   res.setHeader('Content-Type', 'text/html; charset=utf-8');
// res.setHeader('Content-Length', resp.length);
   res.end(resp);
});

server.listen(PORT, HOST, () => {
   console.log(`listening(${HOST}://${PORT})`);
});
