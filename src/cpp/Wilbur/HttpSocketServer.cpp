//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HttpSocketServer.cpp
//
// Purpose-
//       HttpSocketServer implementation methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <sys/stat.h>

#include <com/Debug.h>
#include <com/istring.h>
#include <com/Parser.h>

#include "Common.h"
#include "Diagnostic.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpServer.h"
#include "HttpServerPlugin.h"
#include "HttpServerPluginMap.h"
#include "Loader.h"
#include "Properties.h"

#include "HttpSocketServer.h"
using namespace std;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef BRINGUP
#undef  BRINGUP                     // If defined, BRINGUP Mode
#endif

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, Input/Output Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Subroutine-
//       parseDec
//
// Purpose-
//       Parse a string, extracting a decimal value.
//       Leading blanks are skipped.
//
// Returns-
//       Return (decimal value)
//       String (The value delimiter)
//
//----------------------------------------------------------------------------
static int                          // Return value
   parseDec(                        // Extract decimal value from string
     const char*&      C)           // -> String (updated)
{
   Parser parser(C);
   int result= parser.toDec();
   C= parser.getString();
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       strip
//
// Purpose-
//       Remove leading and trailing blanks from a string.
//
//----------------------------------------------------------------------------
static char*                        // -> Stripped string
   strip(                           // Strip a string
     char*             text)        // -> string (!MODIFIED!)
{
   int                 i;

   while( *text == ' ' )            // Remove leading blanks
     text++;

   i= strlen(text);
   for(i= strlen(text); i>0 && text[i-1] == ' '; i--)
     ;

   text[i]= '\0';
   return text;
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpSocketServer::~HttpSocketServer
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   HttpSocketServer::~HttpSocketServer( void )
{
   IFHCDM( logf("HttpSocketServer(%p)::~HttpSocketServer()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpSocketServer::HttpSocketServer
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   HttpSocketServer::HttpSocketServer( // Constructor
     Socket*           socket)      // Associated Socket
:  SocketServer(socket)
,  keepAlive(250)
{
   IFHCDM( logf("HttpSocketServer(%p)::HttpSocketServer(%p)\n", this, socket); )
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpSocketServer::work
//
// Purpose-
//       Handle next HTTP request/response
//
//----------------------------------------------------------------------------
int                                 // TRUE if no work available
   HttpSocketServer::work( void )   // Handle next HTTP request/response
{
   char buffer[1024];

   int rc= Diagnostic::recvLine(socket, buffer, sizeof(buffer), keepAlive);
   if( rc != 0 )
     return rc;

   if( HttpRequest::isValid(buffer) == FALSE )
     return TRUE;

   HttpRequest request(socket, buffer);
   for(;;)
   {
     int rc= Diagnostic::recvLine(socket, buffer, sizeof(buffer));
     if( rc != 0 )
       break;

     char* ptrN= buffer;
     if( *ptrN == '\0' )
       break;

     while( *ptrN == ' ' )
       ptrN++;

     char* ptrT= ptrN + 1;
     while( *ptrT != ':' && *ptrT != '\0' )
       ptrT++;

     if( *ptrT != '\0' )
     {
       *ptrT= '\0';
       ptrT= strip(ptrT+1);
     }

     request.setProperty(ptrN, ptrT);
   }

   HttpResponse response(request);
   if( Common::get()->getFSM() != Common::FSM_READY )
   {
     response.generateEmpty(response.http503); // Service unavailable
     return TRUE;
   }

   serve(request, response);

   //-------------------------------------------------------------------------
   // Update keepAlive
   const char* C= request.getProperty("Keep-Alive");
   if( C != NULL )
     keepAlive= parseDec(C);
   else
   {
     C= request.getProperty("Connection");
     if( C != NULL && stricmp(C, "keep-alive") == 0 )
       keepAlive= 125;
   }

   if( keepAlive > 5000 )           // Maximum (5 seconds)
     keepAlive= 5000;

   //-------------------------------------------------------------------------
   // Return, more work available
   return FALSE;
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpSocketServer::serve
//
// Purpose-
//       Handle HTTP request/response
//
//----------------------------------------------------------------------------
void
   HttpSocketServer::serve(         // Handle HTTP request/response
     HttpRequest&      Q,           // The HTTP request
     HttpResponse&     S)           // The HTTP response
{
   debugSetIntensiveMode();         // For Intensive HCDM

#if 0
   try {
         Loader  loader("./libHttpServer.so.1.0");
     HttpServer* server= dynamic_cast<HttpServer*>(loader.make());
     if( server != NULL )
       server->serve(Q, S);
     else
       S.generateError(S.http404);
     loader.take(server);

   } catch(const char* X) {
     logf("HttpSocketServer::serve Exception(%s)\n", X);
     S.generateError(S.http500);
   } catch(...) {
     logf("HttpSocketServer::serve Exception(...)\n");
     S.generateError(S.http500);
   }
#endif

   try {
     string path= Q.getOpPath();
     int L= path.size();
     if( L == 0 || path[0] != '/' )
     {
       S.generateError(S.http400);
       return;
     }

     //-----------------------------------------------------------------------
     // Look for '/' path delimiters. These may not be followed by '.'
     int X= 0;
     int W= 1;                      // Current offset
     while( W < L && X >= 0 )
     {
       if( path[W] == '.' )
       {
         S.generateError(S.http403); // Forbidden
         return;
       }

       X= path.find('/', W);
       if( X == W )                 // If "//" in path
       {
         S.generateError(S.http400); // Malformed
         return;
       }

       W= X+1;                      // Next offset
     }

     //-----------------------------------------------------------------------
     // Extract the subdirectory path
     string dir= "/";               // The default path
     X= path.find('/', 1);
     if( X > 0 )
       dir= path.substr(0, X);
     else
     {
       X= path.find('.', 0);
       if( X > 0 || path == "/" )
       {
         HttpServer server;         // Use HttpServer (serves files)
         server.serve(Q, S);
         return;
       }

       dir= path;                   // Terminating '/' omitted
     }

     //-----------------------------------------------------------------------
     // Using HTTP plugin
     Common& common= *Common::get();
     HttpServerPlugin* plugin= (HttpServerPlugin*)common.httpServerMap->getPlugin(dir);
     if( plugin == NULL )
     {
       S.generateError(S.http404);  // Not found
       return;
     }

     plugin->serve(Q, S);

   } catch(const char* X) {
     logf("HttpSocketServer::serve Exception(%s)\n", X);
     S.generateError(S.http500);
   } catch(...) {
     logf("HttpSocketServer::serve Exception(...)\n");
     S.generateError(S.http500);
   }
}

