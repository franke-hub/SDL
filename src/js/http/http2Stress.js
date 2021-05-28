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
//       http2Stress.js
//
// Purpose-
//       HTTP/2 Client - stress test
//
// Last change date-
//       2021/05/19
//
// Usage note-
//       From ~/obj/js/http: make http2Stress
//
//----------------------------------------------------------------------------
'use strict';
const fs= require('fs');
const fp= fs.promises;
const http2= require('http2');
const os= require('os');
const perf= require('perf_hooks').performance; // For performance.now

//----------------------------------------------------------------------------
// Constants and controls
//----------------------------------------------------------------------------
const INPFILE= '/small.html';       // The test file

const CHECKER= true;                // Verify input data?
const MAXHTTP= 0x00040000;          // Largest allowed http response
const RUNTIME= 15000;               // Runtime, specified in milliseconds
var   VERBOSE= 0;                   // Verbosity: higher is more verbose

const
{  HTTP2_HEADER_METHOD
,  HTTP2_HEADER_PATH
,  HTTP2_HEADER_STATUS
} = http2.constants;

const want= fs.readFileSync('html'+INPFILE).toString(); // The expected data

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
var   operational= true;            // While test is running
var   curRequests= 0;               // Current running request count
const maxRequests= 32;              // Maximum running request count
var   totRequests= 0;               // Total completed request count

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

const origin= perf.now();           // Time origin
function delta() {                  // Time difference from origin
   return format((perf.now() - origin)/1000.0, 10, 6);
}

function format(value, width, precision) { // Format a number
   return value.toFixed(precision).padStart(width);
}

function logger(data) {
   console.log(delta(), data);
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

   if( code != 200 ) {

   }

   if( VERBOSE > 0 ) {
     req.on('end', function() {
       if( data.length > MAXHTTP )
         data= `<<Response data error: length > ${MAXHTTP}>>\n`;

       logger('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>');
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
function do_NEXT() {
   while( operational && curRequests < maxRequests ) {
     do_SEND(INPFILE, 'GET');
   }
}

function do_SEND(path, meth) {
   if( VERBOSE > 2 ) console.log('do_SEND');
   curRequests++;
   totRequests++;

   var opts= {};
   opts[HTTP2_HEADER_METHOD]= meth;
   opts[HTTP2_HEADER_PATH]= path;
   const req= client.request(opts);
   req.on('error', function(mess) {
     logger('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>');
     logger(`do_SEND("${path}",${meth}) Error: ${mess.message}`);
     process.exit(1);
   })

   req.on('response', function(head, flag) {
     const code= head[HTTP2_HEADER_STATUS];
     if( VERBOSE > 0 || code != 200 ) {
       console.log('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>');
       console.log(`${code} do_SEND("${path}",${meth})`);
       if( code != 200 )
         process.exit(1);
     }
     do_RESP(req, head, opts);
     curRequests--;
     do_NEXT();
   })
   req.end();
}

//----------------------------------------------------------------------------
// Run the test
//----------------------------------------------------------------------------
logger('Testing started');
do_NEXT();                          // Start the test (running maxRequests)
after(RUNTIME).then( () => {        // Run the timed test
   operational= false;
   totRequests -= curRequests;      // (Current tests don't count in total)
   logger('Testing complete');
   after(1000).then( () => {        // Allow running tests to complete
     console.log('');
     console.log(format(totRequests, 10, 0), 'Operations in'
                , RUNTIME/1000, 'seconds');
     const opsPerSecond= (totRequests/RUNTIME)*1000.0;
     console.log(format(opsPerSecond, 10, 1), 'Operations / second');

     console.log('');
     console.log(format(maxRequests, 10, 0), 'maxRequests');
     console.log(CHECKER.toString().padStart(10), 'CHECKER');
     client.close();
   })
})
