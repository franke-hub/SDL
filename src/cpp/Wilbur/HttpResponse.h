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
//       HttpResponse.h
//
// Purpose-
//       HTTP response container.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef HTTPRESPONSE_H_INCLUDED
#define HTTPRESPONSE_H_INCLUDED

#include <string>
#include <sys/types.h>

#ifndef HTTPREQUEST_H_INCLUDED
#include "HttpRequest.h"            // And other prerequisites
#endif

#ifndef TEXTBUFFER_H_INCLUDED
#include "TextBuffer.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       HttpResponse
//
// Purpose-
//       HTTP response container
//
//----------------------------------------------------------------------------
class HttpResponse : public Interface { // Http response container
//----------------------------------------------------------------------------
// HttpResponse::Attributes
//----------------------------------------------------------------------------
protected:
HttpRequest&           request;     // The associated HttpRequest
TextBuffer             buffer;      // Response buffer

//----------------------------------------------------------------------------
// HttpResponse::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~HttpResponse( void );           // Destructor
   HttpResponse(                    // Constructor
     HttpRequest&      request);    // The HTTP request

//----------------------------------------------------------------------------
// HttpResponse::Internal data areas
//----------------------------------------------------------------------------
public:                             // Reponse codes
static const char*     http100;     // Continue
static const char*     http101;     // Switching protocols
static const char*     http102;     // Processing

static const char*     http200;     // OK
static const char*     http201;     // Created
static const char*     http202;     // Accepted
static const char*     http203;     // Non-authoritative information
static const char*     http204;     // No content
static const char*     http205;     // Reset content
static const char*     http206;     // Partial content

static const char*     http300;     // Multiple Choices
static const char*     http301;     // Moved Permanently
static const char*     http302;     // Found
static const char*     http303;     // See other
static const char*     http304;     // Not Modified

static const char*     http400;     // Bad Request
static const char*     http401;     // Unauthorized
static const char*     http402;     // Payment Required
static const char*     http403;     // Forbidden
static const char*     http404;     // Not Found
static const char*     http405;     // Method Not Allowed

static const char*     http500;     // Internal Server Error
static const char*     http501;     // Not Implemented
static const char*     http502;     // Bad Gateway
static const char*     http503;     // Service unavailable
static const char*     http504;     // Gateway timeout
static const char*     http505;     // HTTP Version Not Supported

//----------------------------------------------------------------------------
// HttpResponse::Methods
//----------------------------------------------------------------------------
public:
void
   generateCode(                    // Generate the HTTP response code line
     const char*       httpNNN);    // The http response code

void
   generateDate( void );            // Generate the "Date: " response line

void
   generateDate(                    // Generate the "Date: " response line
     time_t            date);       // For this date

void
   generateEmpty(                   // Generate an empty HTTP response
     const char*       emptyID);    // The http error code

void
   generateError(                   // Generate an error HTTP response
     const char*       errorID);    // The http error code

void
   generateLength(                  // Generate the "Length: " response line
     unsigned          length);     // For this length

void
   generateServer( void );          // Generate the "Server: " response line

inline HttpRequest&                 // The associated HttpRequest
   getRequest( void ) const         // Get associated HttpRequest
{  return request;
}

inline Socket&                      // The associated Socket
   getSocket( void ) const          // Get associated Socket
{  return request.getSocket();
}

virtual void
   flush( void );                   // Flush response buffer

virtual void
   put(                             // Put response character
     int               C);          // The character

virtual void
   put(                             // Put response string
     const char*       string);     // The string

virtual void
   putln(                           // Put response string + "\r\n"
     const char*       string);     // The string

virtual void
   put(                             // Put response string
     std::string       string);     // The string

virtual void
   putln(                           // Put response string + "\r\n"
     std::string       string);     // The string

virtual void
   put(                             // Put response text
     const char*       text,        // The text
     unsigned          size);       // The text length

virtual void
   reset( void );                   // Reset (empty) the response buffer
}; // class HttpRequest

#endif // HTTPRESPONSE_H_INCLUDED
