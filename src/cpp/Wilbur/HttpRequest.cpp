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
//       HttpRequest.cpp
//
// Purpose-
//       HttpRequest implementation methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <com/Debug.h>
#include <com/Interval.h>
#include <com/Parser.h>
#include <com/Socket.h>
#include <com/Software.h>

#include "Common.h"
#include "Diagnostic.h"
#include "TextBuffer.h"
#include "HttpRequest.h"

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
//       retrieveLength
//
// Purpose-
//       Retrieve the Content-Length property
//
//----------------------------------------------------------------------------
static unsigned                     // The Content-Length property
   retrieveLength(                  // Get Congent-Length property
     HttpRequest&      request)     // The HTTP request
{
   unsigned            result= 0xffffffff; // Resultant

   const char* C= request.getProperty("Content-Length");
   if( C != NULL )
   {
     result= parseDec(C);
     if( *C != '\0' )
       result= 0xffffffff;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       subString
//
// Purpose-
//       Set a std::string from a character substring.
//
//----------------------------------------------------------------------------
static std::string                  // Resultant
   subString(                       // Set a std::string from a substring
     const char*       source,      // Source string
     unsigned          length)      // Source string length
{
   std::string result(source, length);
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpRequest::~HttpRequest
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   HttpRequest::~HttpRequest( void )
{
   IFHCDM( logf("HttpRequest(%p)::~HttpRequest()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpRequest::HttpRequest
//
// Purpose-
//       Constructor. (The request MUST be valid.)
//
//----------------------------------------------------------------------------
   HttpRequest::HttpRequest(        // Constructor
     Socket&           socket,      // Associated Socket
     const char*       request)     // The HTTP request
:  Properties()
,  opCode()
,  opPath()
,  httpID()
,  socket(socket)
{
   IFHCDM( logf("HttpRequest(%p)::HttpRequest(%s)\n", this, request); )

   while( *request == ' ' )         // Skip leading blanks
     request++;

   int x= 0;
   while( request[x] != ' ' && request[x] != '\0' )
     x++;
   opCode= subString(request, x);

   request += x;
   while( *request == ' ' )
     request++;

   x= 0;
   while( request[x] != ' ' && request[x] != '\0' )
     x++;
   opPath= subString(request, x);
   if( opPath == "" || opPath[0] != '/' )
     opPath.insert(0, "/");

   request += x;
   while( *request == ' ' )
     request++;

   httpID= request;

   request += 5;                    // Skip over "HTTP/"
   major= parseDec(request);
   request++;
   minor= parseDec(request);
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpRequest::isValid
//
// Purpose-
//       Determine whether HTTP request is valid
//
//----------------------------------------------------------------------------
bool
   HttpRequest::isValid(            // Is this a valid HTTP request?
     const char*       request)     // The HTTP request in question
{
   while( *request == ' ' )         // Skip leading blanks
     request++;

   int x=0;
   while( request[x] != ' ' && request[x] != '\0' )
     x++;
   std::string opCode= subString(request, x);
   if( opCode != "GET"
       && opCode != "PUT"
       && opCode != "POST"
       && opCode != "HEAD"
       && opCode != "DELETE"
       && opCode != "TRACE" )
     return false;

   request += x;
   while( *request == ' ' )
     request++;

   x= 0;
   while( request[x] != ' ' && request[x] != '\0' )
     x++;
   std::string opPath= subString(request, x);

   request += x;
   while( *request == ' ' )
     request++;

   std::string httpID= request;
   if( httpID != "HTTP/0.9"
       && httpID != "HTTP/1.0"
       && httpID != "HTTP/1.1" )
     return false;

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpRequest::getText
//
// Purpose-
//       Retrieve the HTTP request data
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   HttpRequest::getText(            // Retrieve the HTTP request
     TextBuffer&       buff)        // APPEND to this request buffer
{
   int                 result= 0;   // Resultant
   char                chunk[4096]; // Request chunk size

   unsigned length= retrieveLength(*this);
   Interval interval;
   for(;;)
   {
     int L= Diagnostic::recv(socket, chunk, sizeof(chunk), Socket::MO_NONBLOCK);
     if( L <= 0 )
     {
       Socket::SocketEC ec= socket.getSocketEC();
       if( ec == Software::EC_WOULDBLOCK || ec == Software::EC_AGAIN )
       {
         if( interval.stop() > 0.5 )
         {
           result= (-1);
           break;
         }
         Thread::sleep(0.1);
         continue;
       }

       result= (-2);
       break;
     }

     interval.start();
     buff.put(chunk, L);
     if( buff.size() >= length )
       break;
   }

   return result;
}

