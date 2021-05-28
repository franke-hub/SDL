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
//       httpStress.js
//
// Purpose-
//       HTTP Client - stress test
//
// Last change date-
//       2021/05/19
//
// Usage note-
//       From ~/obj/js/http: make httpStress
//
//----------------------------------------------------------------------------
'use strict';
const fs= require('fs');
const http= require('http');
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

const want= fs.readFileSync('html'+INPFILE).toString(); // The expected data

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
var   operational= true;            // While test is running
var   curRequests= 0;               // Current running request count
const maxRequests= 32;              // Maximum running request count
var   totRequests= 0;               // Total completed request count

//------------------------------------------------------------- ---------------
// Initialize host and port
//----------------------------------------------------------------------------
var host= os.hostname();
if( process.env.USE_LOCALHOST )
   host= 'localhost';

var port= 8080;
if( process.argv.length > 2 )
   port= process.argv[2];
else if( process.env.PORT )
   port= process.env.PORT

//----------------------------------------------------------------------------
// Utility functions
//----------------------------------------------------------------------------
// ES6 idiom: after(ms_timeout).then(() => { /* Do something */ })
const after = t => new Promise(resolve => setTimeout(resolve, t));

const origin= perf.now();           // Time origin
function delta() {                  // Time difference from origin
   return format((perf.now() - origin)/1000.0, 10, 6);
}

function format(value, width, precision) { // Format a number
   return value.toFixed(precision).padStart(width);
}

function logger(data) {
// var join = data[0];
// for(var i= 1; i < data.length; i++) {
//   var item= data[i];
//   if( Array.isArray(item) )
//     join = join + ' [' + item + ']';
//   else
//     join = join + ' ' + data[i];
// }
// console.log(delta(), join);
   console.log(delta(), data);
}

//----------------------------------------------------------------------------
// Handle http response
//----------------------------------------------------------------------------
function do_RESP(res, path) {
   res.on('error', function(err) {
     console.log(`do_RESP: ERROR: ${err.message}`);
   })

   var data= '';
   res.on('data', function(hunk) {
     if( data.length < MAXHTTP )
       data += hunk;
   })

   if( CHECKER ) {
     res.on('end', () => {
       if( data == want )
         return;

       console.log('\nData verification error');
       console.log('\nLengths (sock/file):', data.length, want.length);
       var max= data.length;
       var min= data.length;
       if( want.length > max )
         max= want.length
       if( want.length < min )
         min= want.length

       var ok= true;
       for(var i=0; i<min; i++) {
         var T= data[i];
         var W= want[i]
         if( T == W )
           console.log(`[${i}] OK: "${T}"`, T.charCodeAt(0))
         else {
           ok= false;
           console.log(`[${i}] NG: "${T}" Want "${W}"`
                      , T.charCodeAt(0), W.charCodeAt(0));
           break;
         }
       }
       if( ok ) {
         var big= 'sock';
         if( want.length > data.length ) {
           big= 'file';
           data= want;
         }
         for(var i=min; i<max; i++) {
           var T= data[i];
           console.log(`[${i}] ${big}: "${T}"`)
         }
       }

       process.exit(1);
     })
   }

   if( VERBOSE > 0 ) {
     res.on('end', function() {
       if( data.length > MAXHTTP )
         data= `<<Response data error: length > ${MAXHTTP}>>\n`;

       var headers= res.headers;
       for( var item in headers )
         logger(`${item}: "${headers[item]}"`);
       logger('');

       if( data.length > 0 )
         logger(data);
     });
   }
}

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

   var opts= {host: host, port: port};
   opts['method']= meth;
   opts['path']= path;
   const req= http.request(opts);

   req.on('error', function(mess) {
     console.log('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>');
     console.log(`do_SEND("${path}",${meth}) Error: ${mess.message}`);
     process.exit(1);
   })

   req.on('response', function(res) {
     if( VERBOSE > 0 || res.statusCode != 200 ) {
       console.log('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>');
       console.log(`${res.statusCode} do_SEND("${path}",${meth})`);
       if( res.statusCode != 200 )
         process.exit(1);
     }
     do_RESP(res);
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
   })
})
