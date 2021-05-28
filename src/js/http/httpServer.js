//----------------------------------------------------------------------------
//
//       Copyright (C) 2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       httpServer.js
//
// Purpose-
//       HTTP Server
//
// Last change date-
//       2021/05/19
//
// Usage note-
//       From ~/obj/js/http: make httpServer
//
//----------------------------------------------------------------------------
'use strict';
const dt= require('node-datetime');
const fs= require('fs');
const fp= fs.promises;
const http= require('http');
const os= require('os');

//----------------------------------------------------------------------------
// Constants and controls
//----------------------------------------------------------------------------
var   LOGFILE= 'log/httpServer.log'; // The log file name
//    LOGFILE= '';                  // Comment out to log
const MAXBODY= 0x00040000;          // Largest allowed html body
const PROGRAM= 'httpServer.js';     // The program name
const VERSION= '1.0.0';             // The program version
var   VERBOSE= 0;                   // Verbosity: higher is more verbose

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
var   log= null;                    // The log file handle, when opened

//----------------------------------------------------------------------------
// Default web pages
//----------------------------------------------------------------------------
const h1Version= `<h1 align="center">${PROGRAM}/${VERSION}</h1>\n`;

function _403page(from) {
   return  do_JOIN(
                [ '<html><head><title>NOT AUTHORIZED</title></head>'
                , '<body><h1 align="center">NOT AUTHORIZED</h1>'
                , `${from} Your improper request has been logged.`
                , '</body></html>' ]);
                }

function _404page() {
   return do_JOIN(
                [ '<html><head><title>NOT FOUND</title></head>'
                , '<body><h1 align="center">NOT FOUND</h1>'
                , 'The page you requested could not be found.'
                , '</body></html>' ]);
                }

function _405page(op) {
   return do_JOIN(
                [ '<html><head><title>NOT SUPPORTED</title></head>'
                , '<body><h1 align="center">NOT SUPPORTED</h1>'
                , `Method[${op}] is not supported.`
                , '</body></html>' ]);
                }

function _500page(oops) {
   return do_JOIN(
                [ '<html><head><title>Server Error</title></head>'
                , '<body><h1 align="center">Server Error</h1>'
                , `Diagnostic: ${oops}`
                , '</body></html>' ]);
                }

function _webpage(body) {
   return do_JOIN(
                [ `<html><head><title>${PROGRAM}/${VERSION}</title></head>`
                , '<meta http-equiv="Expires" content="0">'
                , '<meta http-equiv="CACHE-CONTROL" content="NO-CACHE">'
                , '<body>'
                , `${body}`
                , '</body></html>' ]);
                }

//----------------------------------------------------------------------------
// Initialize host and port
//----------------------------------------------------------------------------
var host= os.hostname();
if( process.env.USE_LOCALHOST )
   host= 'localhost';

var port= 8080;
if( process.argv.length > 2 )
   port= process.argv[2];
else if( process.env.PORT )
   port= process.env.PORT;

//----------------------------------------------------------------------------
// Utility functions
//----------------------------------------------------------------------------
// Usage: after(ms_timeout).then(() => { /* Do something */ })
const after = t => new Promise(resolve => setTimeout(resolve, t));

function do_HTML(req, res, code, html) {
   var buff= Buffer.from(html, 'utf-8');
   res.statusCode= code;
   log_request(req, res);           // (After res.end(), socket unusable)
   res.setHeader('Content-Type', 'text/html; charset=utf-8');
   res.setHeader('Content-Length', buff.length);
   if( req.method == 'HEAD' )
     res.end();
   else
     res.end(buff);
}

function do_JOIN(list) {
   var data= '';
   for( var line in list ) {
     data += list[line] + '\n';
   }
   return data;
}

function get_home() {               // Get home page
   return _webpage(h1Version + 'Home page');
}

function get_post() {               // Get post page
   return _webpage(h1Version + 'OK');
}

function log_request(req,res) {     // Log a request/response
   const code= res.statusCode;
   const http= `HTTP/${req.httpVersion}`;
   const meth= req.method.padEnd(4, ' ');
// const peer= req.connection.remoteAddress;
   const peer= req.socket.remoteAddress;
   const time= dt.create().format('d/n/Y H:M:S');

   logger(`${peer} [${time}] ${http} ${code} ${meth} ${req.url}`);
}

function logger(text) {             // Write to Log
   if( log ) {
     log.writeFile(text + '\n')
     .catch( err => {
         console.log(`File($LOGFILE) write error: ${err.message}`);
     })
   } else {
//   console.log(text);            // Log inactive, write to console
   }
}

//----------------------------------------------------------------------------
// Handle server requests
//----------------------------------------------------------------------------
async function do_FILE(path, from) {
   var result= null;
   if( path.indexOf('/../') >= 0 || path[0] != '/' )
     return [500, _500page('OOPSIE')];
   if( path == '/' ) path= '/index.html';
   path= 'html' + path;

   return await fp.readFile(path, {encoding: 'utf8'})
   .then((data) => {
     return [200, data];
   })
   .catch((err) => {
     if( err.code == 'ENOENT' )
       return [404, _404page()];

     else if( err.code == 'EACCES' || err.code == 'EISDIR' )
       return [403, _403page(from)];

     return [500, _500page(err.code)];
   })
}

function do_HGET(req, res) {         // (Handles both GET and HEAD methods)
   if( VERBOSE > 2 ) console.log('do_' + req.method);

   var path= req.url;
   do_FILE(path, req.socket.remoteAddress)
   .then((out) => {
     do_HTML(req, res, out[0], out[1]);
   })
   .catch((err) => {
     console.log(path, '.caught', err.message);
     do_HTML(req, res, 500, _500page('Internal error'));
   })
}

function do_POST(req, res) {
   if( VERBOSE > 2 ) console.log('do_POST');

   var text= '';
   req.on('data', function(data) {
     if( text.length < MAXBODY )
       text += data;
   })
   .on('end', function() {
     if( text.length > MAXBODY ) {
       do_HTML(req, res, 500, _500page('ue:TEL'));
       logger(`<<POST text error: length > ${MAXBODY}>>`);
     } else {
       do_HTML(req, res, 200, get_post());
       if( VERBOSE > 0 ) logger(`"${text}"`);
     }
   })
}

//----------------------------------------------------------------------------
// Activate the server, setting up event handlers
//----------------------------------------------------------------------------
const server= http.createServer();

server.on('close', () => {
   if( VERBOSE > 2 ) console.log('\nServer closed');
   if( log ) {
     log.close()
     .then( () => {
       if( VERBOSE > 2 ) console.log('Logger closed');
     })
     .catch( err => {
       console.log(`ERROR: Logger close: ${err.message}`);
     })
   }
   console.log('Server terminated');
   process.exit(0);
})

server.on('error', (err) => {
   console.log('\nServer error:', err.message);
   server.close();
})

server.on('request', (req, res) => {
   const method= req.method;
   if( method == 'POST' ) {
     do_POST(req, res);
   } else if( method == 'HEAD' || method == 'GET' ) {
     do_HGET(req, res);
   } else {
     do_HTML(req, res, 405, _405page(method));
   }
})

//----------------------------------------------------------------------------
// Activate logger and server listener
//----------------------------------------------------------------------------
if( LOGFILE ) {
   fp.open(LOGFILE, 'a', 0o600)
   .then( out => {
     log= out;
     logger('')
     logger('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>');
     const time= dt.create().format('d/n/Y H:M:S');
     logger(`>> [${time}] HTTP Server started`);
   })
   .then(() => {
     server.listen(port, host, () => {
       console.log(`Server running: http://${host}:${port}`)
     })
   })
   .catch( err => {
     console.log(`Activation failed: ${err.message}`);
     process.exit(1);
   })

} else {                            // If not logging
   try {
     server.listen(port, host, () => {
       console.log(`Server running: http://${host}:${port}`)
       console.log('<<Logging disabled>>');
     })
   } catch(err) {
     console.log(`Activation failed: ${err.message}`);
     process.exit(1);
   }
}

//----------------------------------------------------------------------------
// Wait for console interrupt
//----------------------------------------------------------------------------
process.stdin.resume();
process.on('SIGINT', () => {
   if( VERBOSE > 2 ) console.log('SIGINT');

   // Terminate after 30 seconds if normal termination doesn't complete
   after(30000).then(() => {
     console.log('Server terminated: timeout');
     process.exit(0);
   });

   // Normal termination sequence
   server.close();
})
