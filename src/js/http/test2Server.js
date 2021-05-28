const http2= require('http2');
const fs= require('fs');

const INPFILE= 'html/utf8.html';

const HOST= 'localhost';
const PORT= 8443;

const data= fs.readFileSync(INPFILE).toString();

//----------------------------------------------------------------------------
// The minimal server
//----------------------------------------------------------------------------
const server= http2.createSecureServer(
   { key:  fs.readFileSync('privkey.pem')
   , cert: fs.readFileSync('cert.pem')
   });

server.listen(PORT, HOST, () => {
  console.log(`Server running: https://${HOST}:${PORT}/`)
})

var request= 0;
server.on('request', (req, res) => {
// diagnostics(req, res);

   var resp= data;
// if( (++request) % 2 )            // Alternate working/failing
     resp= Buffer.from(data);

   res.setHeader('Content-Type', 'text/html; charset=utf-8');
   res.setHeader('Content-Length', resp.length);
   res.end(resp);
})
