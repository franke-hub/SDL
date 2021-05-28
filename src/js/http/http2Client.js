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
//       http2Client.js
//
// Purpose-
//       HTTP/2 Client
//
// Last change date-
//       2021/05/19
//
// Usage note-
//       From ~/obj/js/http: make http2Client
//
//----------------------------------------------------------------------------
'use strict';
const fs= require('fs');
const fp= fs.promises;
const http2= require('http2');
const os= require('os');

//----------------------------------------------------------------------------
// Constants and controls
//----------------------------------------------------------------------------
const CHECKER= true;                // Use data verification?
const MAXHTTP= 0x00040000;          // Largest allowed http response
var   VERBOSE= 1;                   // Verbosity: higher is more verbose

const
{  HTTP2_HEADER_METHOD
,  HTTP2_HEADER_PATH
,  HTTP2_HEADER_STATUS
} = http2.constants;

// Allow self-signed client certificate
process.env.NODE_TLS_REJECT_UNAUTHORIZED='0'; // (Local export=)

//----------------------------------------------------------------------------
// Initialize host and port
//----------------------------------------------------------------------------
var host= os.hostname();
if( process.env.USE_LOCALHOST )
   host= 'localhost';

var port= 8443;
if( process.argv.length > 2 )
   port= process.argv[2];
else if( process.env.PORT )
   port= process.env.PORT

//----------------------------------------------------------------------------
// Utility functions
//----------------------------------------------------------------------------
// Usage: after(ms_timeout).then(() => { /* Do something */ })
const after = t => new Promise(resolve => setTimeout(resolve, t));

function logger(text) {             // Write to Log
   console.log(text);               // (Console log);
}

//----------------------------------------------------------------------------
// Input data verifier
//----------------------------------------------------------------------------
async function do_FILE(path, want) {
   var result= null;
   if( path == '/' ) path= '/index.html';
   path= 'html' + path;

   return await fp.readFile(path, {encoding: 'utf8'})
   .then( (have) => {
     if( have == want ) {
       if( VERBOSE > 0 )
         console.log(`Verified OK: ${path}`);
     } else {
       console.log(`File(${path} verification error`);
       console.log('\nhave:'); console.log(have);
       console.log('\nwant:'); console.log(want);
     }
     return have;
   })
   .catch((err) => {
     console.log(`File(${path} read error: ${err}`);
     return '';
   })
}

//----------------------------------------------------------------------------
// Handle http response
//----------------------------------------------------------------------------
function do_RESP(req, head, opts) {
   var data= '';
   req.on('data', function(hunk) {
     if( data.length < MAXHTTP )
       data += hunk;
   })

   const code= head[HTTP2_HEADER_STATUS];
   const meth= opts[HTTP2_HEADER_METHOD];
   const path= opts[HTTP2_HEADER_PATH];
   if( CHECKER && meth == 'GET' && code == 200 ) {
     req.on('end', () => {
       do_FILE(path, data);
     })
   }

   if( VERBOSE > 0 ) {
     req.on('end', function() {
       if( data.length > MAXHTTP )
         data= `<<Response data error: length > ${MAXHTTP}>>\n`;

       logger('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>');
       const code= head[HTTP2_HEADER_STATUS];
       logger(`${code} do_RESP("${path}",${meth})`);

       for( var item in head ) {
         if( item[0] != ':' )
           logger(`${item}: "${head[item]}"`);
       }
       logger('');

       if( data.length > 0 )
         logger(data);
     });
   }
}

//----------------------------------------------------------------------------
// Initialize client
//----------------------------------------------------------------------------
var client;
try {
   client= http2.connect(`https://${host}:${port}`, {
     ca: fs.readFileSync('cert.pem')
   });
} catch(err) {
   console.log('Connection error:', err.message);
   process.exit(1);
}

client.on('error', (err) => console.error(err));

//----------------------------------------------------------------------------
// Handle http request
//----------------------------------------------------------------------------
function do_POST(path, data) {
   if( VERBOSE > 2 ) console.log('do_POST');

   var opts= {};
   opts[HTTP2_HEADER_METHOD]= 'POST';
   opts[HTTP2_HEADER_PATH]= path;
   const req= client.request(opts);
   req.on('error', function(mess) {
     logger('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>');
     logger(`do_POST("${path}","${data}") Error: ${mess.message}`);
   })

   req.on('response', function(head) {
     if( VERBOSE == 0 ) {
       const code= head[HTTP2_HEADER_STATUS];
       logger(`${code} do_POST("${path}",${data})`);
     }
     do_RESP(req, head, opts);
   })
   req.write(data);
   req.end();
}

function do_SEND(path, meth) {
   if( VERBOSE > 2 ) console.log('do_SEND');

   var opts= {};
   opts[HTTP2_HEADER_METHOD]= meth;
   opts[HTTP2_HEADER_PATH]= path;
   const req= client.request(opts);
   req.on('error', function(mess) {
     logger('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>');
     logger(`do_SEND("${path}",${meth}) Error: ${mess.message}`);
   })

   req.on('response', function(head, flag) {
     if( VERBOSE == 0 ) {
       const code= head[HTTP2_HEADER_STATUS];
       logger(`${code} do_SEND("${path}",${meth})`);
     }
     do_RESP(req, head, opts);
   })
   req.end();
}

//----------------------------------------------------------------------------
// Initiate client requests
//----------------------------------------------------------------------------
do_SEND('/index.html', 'GET');
do_SEND('/', 'HEAD');
do_POST('/post-test', 'This is the post data, by golly');
do_SEND('/403-test', 'GET');
do_SEND('/404-test', 'GET');
do_SEND('/405-test', 'MOVE');
do_SEND('/500-test/../', 'GET');

do_SEND('/small.html', 'GET');
do_SEND('/utf8.html', 'GET');

after(1000).then(() => {
   client.close();
})
