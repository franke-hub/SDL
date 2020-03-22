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
//       HttpRequest.h
//
// Purpose-
//       HTTP request header container.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef HTTPREQUEST_H_INCLUDED
#define HTTPREQUEST_H_INCLUDED

#include <string>
#include "Properties.h"

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Socket;
class TextBuffer;

//----------------------------------------------------------------------------
//
// Class-
//       HttpRequest
//
// Purpose-
//       HTTP request header container
//
//----------------------------------------------------------------------------
class HttpRequest : public Properties { // Http request header container
//----------------------------------------------------------------------------
// HttpRequest::Attributes
//----------------------------------------------------------------------------
protected:
std::string            opCode;      // The operation code
std::string            opPath;      // The operation path
std::string            httpID;      // The HTTP identifier ("HTTP/n.n")
int                    major;       // The HTTP major version number
int                    minor;       // The HTTP minor version number
Socket&                socket;      // The associated Socket

//----------------------------------------------------------------------------
// HttpRequest::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~HttpRequest( void );            // Destructor
   HttpRequest(                     // Constructor
     Socket&           socket,      // Associated Socket
     const char*       request);    // The HTTP request

//----------------------------------------------------------------------------
// HttpRequest::Static methods
//----------------------------------------------------------------------------
public:
static bool                         // TRUE iff request is valid
   isValid(                         // Is this a valid HTTP request?
     const char*       request);    // The HTTP request in question

//----------------------------------------------------------------------------
// HttpRequest::Methods
//----------------------------------------------------------------------------
public:
inline std::string                  // The operation code
   getOpCode( void ) const          // Get operation code
{  return opCode;
}

inline std::string                  // The operation directory
   getOpDir( void ) const           // Get operation directory
{
   std::string result= "/";         // The subdirectory path
   int L= opPath.size();
   int X= 1;
   while( X > 0 && X < L )
   {
     X= opPath.find('/', X);
     if( X > 0 )
       result= opPath.substr(0, X++);
   }

   return result;
}

inline std::string                  // The (complete) operation path
   getOpPath( void ) const          // Get (complete) operation path
{  return opPath;
}

inline std::string                  // The HTTP identifier
   getHttpID( void ) const          // Get HTTP identifier
{  return httpID;
}

inline int                          // The major version number
   getMajor( void ) const           // Get major version number
{  return major;
}

inline int                          // The minor version number
   getMinor( void ) const           // Get minor version number
{  return minor;
}

inline Socket&                      // The associated Socket
   getSocket( void ) const          // Get associated Socket
{  return socket;
}

int                                 // Return code (0 OK)
   getText(                         // Retrieve the HTTP request data
     TextBuffer&       buff);       // APPEND to this request buffer
}; // class HttpRequest

#endif // HTTPREQUEST_H_INCLUDED
