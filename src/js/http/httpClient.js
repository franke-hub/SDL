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
//       httpClient.js
//
// Purpose-
//       HTTP Client
//
// Last change date-
//       2021/05/19
//
// Usage note-
//       From ~/obj/js/http: make httpClient
//
//----------------------------------------------------------------------------
'use strict';
const fs= require('fs');
const fp= fs.promises;
const http= require('http');
const os= require('os');

//----------------------------------------------------------------------------
// Constants and controls
//----------------------------------------------------------------------------
const CHECKER= true;                // Use data verification?
const MAXHTTP= 0x00040000;          // Largest allowed http response
var   VERBOSE= 1;                   // Verbosity: higher is more verbose

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
   port= process.env.PORT

//----------------------------------------------------------------------------
// Utility functions
//----------------------------------------------------------------------------
function logger(data) {             // Write to Log
   console.log(data);               // (Console log);
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
function do_RESP(res, path, meth) {
   res.on('error', function(err) {
     console.log(`do_RESP: ERROR: ${err.message}`);
   })

   var data= '';
   res.on('data', function(hunk) {
     if( data.length < MAXHTTP )
       data += hunk;
   })

   if( CHECKER && meth == 'GET' && res.statusCode == 200 ) {
     res.on('end', () => {
       do_FILE(path, data);
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
function do_POST(path, data) {
   if( VERBOSE > 2 ) console.log('do_POST');

   var opts= {host: host, port: port};
   opts['method']= 'POST';
   opts['path']= path;
   const req= http.request(opts);
   req.on('error', function(mess) {
     logger('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>');
     logger(`do_POST("${path}","${data}") Error: ${mess.message}`);
   })
   req.on('response', function(res) {
     if( VERBOSE > 0 )
       logger('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>');
     logger(`${res.statusCode} do_POST("${path}",${data})`);
     do_RESP(res);
   })
   req.write(data);
   req.end();
}

function do_SEND(path, meth) {
   if( VERBOSE > 2 ) console.log('do_SEND');

   var opts= {host: host, port: port};
   opts['method']= meth;
   opts['path']= path;
   const req= http.request(opts);

   req.on('error', function(mess) {
     console.log('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>');
     console.log(`do_SEND("${path}",${meth}) Error: ${mess.message}`);
   })

   req.on('response', function(res) {
     if( VERBOSE > 0 )
       logger('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>');
     logger(`${res.statusCode} do_SEND("${path}",${meth})`);
     do_RESP(res, path, meth);
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

do_SEND('/small.html', 'GET');      // Used in stress test
do_SEND('/utf8.html', 'GET');       // Regression test
