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
//       HttpServer.cpp
//
// Purpose-
//       HttpServer implementation methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <sys/stat.h>
#include <com/Debug.h>
#include <com/istring.h>            // for memicmp

#include "Common.h"
#include "DateParser.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Properties.h"
#include "TextBuffer.h"

#include "HttpServer.h"

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
// Method-
//       HttpServer::~HttpServer
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   HttpServer::~HttpServer( void )  // Destructor
{
   IFHCDM( traceh("HttpServer(%p)::~HttpServer()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServer::HttpServer
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   HttpServer::HttpServer( void )   // Constructor
:  Interface()
{
   IFHCDM( traceh("HttpServer(%p)::HttpServer()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServer::serve
//
// Purpose-
//       Handle HTTP request/response
//
//----------------------------------------------------------------------------
void
   HttpServer::serve(               // Handle HTTP request/response
     HttpRequest&      Q,           // The HTTP request
     HttpResponse&     S)           // The HTTP response
{
   char                temp[4096];  // Working buffer

   #ifdef HCDM
       traceh("HttpServer(%p)::serve(%p,%p) %s\n", this, &Q, &S, Q.getOpPath().c_str());
   #else
       traceh("HttpServer::serve(%s)\n", Q.getOpPath().c_str());
   #endif

   //-------------------------------------------------------------------------
   // BRINGUP: Display the properties
   #if defined(BRINGUP) && TRUE
     traceh("Properties:\n");
     traceh("request.httpID: '%s'\n", Q.getHttpID().c_str());
     traceh("request.opCode: '%s'\n", Q.getOpCode().c_str());
     traceh("request.opPath: '%s'\n", Q.getOpPath().c_str());
     traceh("request.major: '%d'\n",  Q.getMajor());
     traceh("request.minor: '%d'\n",  Q.getMinor());

     Properties::Iterator it;
     for(it= Q.begin(); it != Q.end(); it++)
       traceh("\'%s\' = \'%s\'\n", it->first.c_str(), it->second.c_str());
     traceh("\n");
   #endif

   //-------------------------------------------------------------------------
   // Parse the request
   std::string httpID= Q.getHttpID();
   std::string opCode= Q.getOpCode();
   std::string opPath= Q.getOpPath();
   if( opCode == "GET" || opCode == "HEAD" )
   {
     //-----------------------------------------------------------------------
     // Look for special cases
     if( opPath == "/" )
       opPath= "/index.html";

     if( opPath.length() < 2 || opPath[0] != '/' || opPath[1] == '.' )
     {
       S.generateError(S.http403);
       return;
     }

     if( opPath == "/shutdown.html" )
       Common::get()->shutdown();

     //-----------------------------------------------------------------------
     // Serve the file
     if( int(opPath.find('/',1)) < 0 ) // If no subdirectory specified
       opPath= "/html" + opPath;    // Use 'html' subdirectory
     opPath= opPath.substr(1);      // Remove leading '/'

     struct stat filestat;
     int rc= stat(opPath.c_str(), &filestat);
     if( rc != 0 )
     {
       S.generateError(S.http404);
       return;
     }

     const char* C= Q.getProperty("If-Modified-Since");
     if( C != NULL )
     {
       if( memicmp(C, "Date: ", 6) == 0 )
         C += 6;

       time_t cacheTime= DateParser::parse(C);
       if( cacheTime >= filestat.st_mtime )
       {
         S.generateEmpty(S.http304);
         return;
       }
     }

     FILE* file= fopen(opPath.c_str(), "rb");
     if( file == NULL )
     {
       S.generateError(S.http404);
       return;
     }

     S.generateCode(S.http200);
     S.generateServer();
     S.put("Last-Modified: "); S.generateDate(filestat.st_mtime);
     if( opPath != "html/shutdown.html" )
       S.putln("max-age: 86400");
     S.putln("Content-Type: text/html");
     S.generateLength(filestat.st_size);
     S.generateDate();
     S.put("\r\n");

     if( opCode == "GET" )
     {
       for(;;)
       {
         unsigned L= fread(temp, 1, sizeof(temp), file);
         if( L <= 0 )
           break;

         S.put(temp, L);
       }
     }

     fclose(file);
   }
   else if( opCode == "TRACE" )
   {
     TextBuffer text;
     Properties::Iterator it;
     for(it= Q.begin(); it != Q.end(); it++)
     {
       S.put(it->first.c_str(), it->first.size());
       S.put(": ");
       S.put(it->second);
     }
     S.put("\r\n");
     Q.getText(text);

     S.generateCode(S.http200);
     S.generateServer();
     S.putln("Content-Type: message/http");
     S.generateLength(text.size());
     S.generateDate();
     S.put("\r\n");
     S.put(text.toString());
   }
   else
     S.generateError(S.http405);
}

