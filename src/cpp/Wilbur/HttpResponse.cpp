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
//       HttpResponse.cpp
//
// Purpose-
//       HttpResponse implementation methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <com/Debug.h>

#include "Common.h"
#include "DateParser.h"
#include "Diagnostic.h"
#include "Global.h"

#include "HttpResponse.h"

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
// Internal data areas
//----------------------------------------------------------------------------
const char*            HttpResponse::http100= "100 Continue";
const char*            HttpResponse::http101= "101 Switching protocols";
const char*            HttpResponse::http102= "102 Processing";

const char*            HttpResponse::http200= "200 OK";
const char*            HttpResponse::http201= "201 Created";
const char*            HttpResponse::http202= "202 Accepted";
const char*            HttpResponse::http203= "203 Non-authoritative";
const char*            HttpResponse::http204= "204 No content";
const char*            HttpResponse::http205= "205 Reset content";
const char*            HttpResponse::http206= "206 Partial content";

const char*            HttpResponse::http300= "300 Multiple Choices";
const char*            HttpResponse::http301= "301 Moved Permanently";
const char*            HttpResponse::http302= "302 Found";
const char*            HttpResponse::http303= "303 See other";
const char*            HttpResponse::http304= "304 Not Modified";

const char*            HttpResponse::http400= "400 Bad Request";
const char*            HttpResponse::http401= "401 Unauthorized";
const char*            HttpResponse::http402= "402 Payment Required";
const char*            HttpResponse::http403= "403 Forbidden";
const char*            HttpResponse::http404= "404 Not Found";
const char*            HttpResponse::http405= "405 Method Not Allowed";

const char*            HttpResponse::http500= "500 Internal Server Error";
const char*            HttpResponse::http501= "501 Not Implemented";
const char*            HttpResponse::http502= "502 Bad Gateway";
const char*            HttpResponse::http503= "503 Service unavailable";
const char*            HttpResponse::http504= "504 Gateway timeout";
const char*            HttpResponse::http505= "505 Not Supported";

//----------------------------------------------------------------------------
//
// Method-
//       HttpResponse::~HttpResponse
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   HttpResponse::~HttpResponse( void )
{
   IFHCDM( logf("HttpResponse(%p)::~HttpResponse()\n", this); )

   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpResponse::HttpResponse
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   HttpResponse::HttpResponse(      // Constructor
     HttpRequest&      request)     // The HTTP request
:  Interface()
,  request(request)
,  buffer()
{
   IFHCDM( logf("HttpResponse(%p)::HttpResponse(%p)\n", this, &request); )
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpResponse::flush
//
// Purpose-
//       Flush the response buffer
//
//----------------------------------------------------------------------------
void
   HttpResponse::flush( void )      // Flush the reponse buffer
{
   IFHCDM( logf("HttpResponse(%p)::flush()\n", this); )

   if( buffer.size() > 0 )
   {
     Diagnostic::send(request.getSocket(), buffer.toChar(), buffer.size());
     buffer.reset();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpResponse::generate*
//
// Purpose-
//       Generate a packaged response line.
//
//----------------------------------------------------------------------------
void
   HttpResponse::generateCode(      // Generate the HTTP response code
     const char*       httpNNN)     // The http response code
{
   put(request.getHttpID()); put(" "); putln(httpNNN);
}

void
   HttpResponse::generateDate( void ) // Generate Date: response line
{
   generateDate(time(NULL));
}

void
   HttpResponse::generateDate(      // Generate Date: response line
     time_t            date)        // For this date/time
{
   put("Date: ");
   putln(DateParser::generate(date));
}

void
   HttpResponse::generateEmpty(     // Generate an empty HTTP response
     const char*       emptyID)     // The http error code
{
   reset();
   generateCode(emptyID);
   generateDate();
   put("\r\n");
}

void
   HttpResponse::generateError(     // Generate an error HTTP response
     const char*       errorID)     // The http error code
{
   Common&             common= *Common::get(); // Get the Common*
   TextBuffer          text;        // The descriptive text

   reset();
   text.put("<html><head><title>\r\n");
   text.put(common.wilbur); text.put("/"); text.put(common.global->VERSION_ID);
   text.put(" - Error Report</title></head>\r\n");
   text.put("<body>");
   text.put("<h1>"); text.put(errorID); text.put("</h1>\r\n");
   text.put("<br>Method: '"); text.put(request.getOpCode()); text.put("'\r\n");
   text.put("<br>Resource: '"); text.put(request.getOpPath()); text.put("'\r\n");
   text.put("<hr><h3>\r\n");
   text.put(common.wilbur); text.put("/"); text.put(common.global->VERSION_ID);
   text.put("</h3></body></html>\r\n");

   generateCode(errorID);
   generateServer();
   generateLength(text.size());
   generateDate();
   put("\r\n");
   put(text.toChar());
}

void
   HttpResponse::generateLength(    // Generate Content-Length: response line
     unsigned          length)      // The content length
{
   char                buffer[32];  // To generate the length

   sprintf(buffer, "%u", length);
   put("Content-Length: ");
   putln(buffer);
}

void
   HttpResponse::generateServer( void )           // Generate Server: response line
{
   Common&             common= *Common::get(); // Get the Common*

   put("Server: ");
   put(common.wilbur); put("/"); putln(common.global->VERSION_ID);
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpResponse::put/putln
//
// Purpose-
//       Append to HTTP response
//
//----------------------------------------------------------------------------
void
   HttpResponse::put(               // Append to HTTP response
     int               C)           // This character
{
   buffer.put(C);
}

void
   HttpResponse::put(               // Append to HTTP response
     const char*       S)           // This string
{
   buffer.put(S);
}

void
   HttpResponse::putln(             // Append to HTTP response
     const char*       S)           // This string + "\r\n"
{
   buffer.put(S);
   buffer.put("\r\n");
}

void
   HttpResponse::put(               // Append to HTTP response
     std::string       S)           // This string
{
   buffer.put(S);
}

void
   HttpResponse::putln(             // Append to HTTP response
     std::string       S)           // This string + "\r\n"
{
   buffer.put(S);
   buffer.put("\r\n");
}

void
   HttpResponse::put(               // Append to HTTP response
     const char*       A,           // This text
     unsigned          L)           // For this length
{
   buffer.put(A, L);
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpResponse::reset
//
// Purpose-
//       Reset the response buffer
//
//----------------------------------------------------------------------------
void
   HttpResponse::reset( void )      // Flush the reponse buffer
{
   IFHCDM( logf("HttpResponse(%p)::reset()\n", this); )

   buffer.reset();
}

