//----------------------------------------------------------------------------
//
//       Copyright (C) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       http/HTTP.h
//
// Purpose-
//       HTTP constants and utilities.
//
// Last change date-
//       2023/06/24
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HTTP_HTTP_H_INCLUDED
#define _LIBPUB_HTTP_HTTP_H_INCLUDED

#include <cstdlib>                  // For size_t
#include <string>                   // For std::string

#include "dev/bits/devconfig.h"     // For HTTP config controls

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace http {
//----------------------------------------------------------------------------
//
// Class-
//       HTTP
//
// Purpose-
//       HTTP constants and utilities
//
//----------------------------------------------------------------------------
class HTTP {                        // HTTP constants and utilities
//----------------------------------------------------------------------------
// HTTP::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef const char     CC;          // (Shorthand)
typedef std::string    string;

//----------------------------------------------------------------------------
// HTTP::Attributes
//----------------------------------------------------------------------------
protected:

//----------------------------------------------------------------------------
// HTTP::Static attributes
//----------------------------------------------------------------------------
public:

//----------------------------------------------------------------------------
// HTTP::HTTP text constants
//----------------------------------------------------------------------------
public:
// HTTP Header options
static constexpr CC*   HEADER_HOST= "HOST";
static constexpr CC*   HEADER_LENGTH= "Content-Length";
static constexpr CC*   HEADER_TYPE= "Content-Type";

// HTTP Methods
static constexpr CC*   METHOD_CONNECT= "CONNECT";
static constexpr CC*   METHOD_DELETE= "DELETE";
static constexpr CC*   METHOD_GET= "GET";
static constexpr CC*   METHOD_HEAD= "HEAD";
static constexpr CC*   METHOD_OPTIONS= "OPTIONS";
static constexpr CC*   METHOD_POST= "POST";
static constexpr CC*   METHOD_PUT= "PUT";
static constexpr CC*   METHOD_TRACE= "TRACE";

// HTTP Protocols
static constexpr CC*   PROTOCOL=    "PROTOCOL";  // Protocol specifier
static constexpr CC*   PROTOCOL_H0= "HTTP/1.0";  // HTTP/1.0 (clear text)
static constexpr CC*   PROTOCOL_H1= "HTTP/1.1";  // HTTP/1.1 (clear text)
static constexpr CC*   PROTOCOL_H2= "HTTP/2";    // HTTP/2   (clear text)
static constexpr CC*   PROTOCOL_S0= "HTTPS/1.0"; // HTTPS/1.0 (encrypted)
static constexpr CC*   PROTOCOL_S1= "HTTPS/1.1"; // HTTPS/1.1 (encrypted)
static constexpr CC*   PROTOCOL_S2= "HTTPS/2";   // HTTPS/2   (encrypted)

//----------------------------------------------------------------------------
// HTTP::Constructors, destructor
//----------------------------------------------------------------------------
   HTTP( void ) = delete;           // Static class, no constructor
   ~HTTP( void ) = delete;          // Static class, no destructor

//----------------------------------------------------------------------------
// HTTP::Accessor methods
//----------------------------------------------------------------------------
static const char* status_text(int); // Get (minimal) status text for code
}; // class HTTP
}  // namespace http
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_HTTP_HTTP_H_INCLUDED
