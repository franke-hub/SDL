//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Url.h
//
// Purpose-
//       URL Parser.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef URL_H_INCLUDED
#define URL_H_INCLUDED

#include <string>

//----------------------------------------------------------------------------
//
// Class-
//       Url
//
// Purpose-
//       URL Parser.
//
// Examples-
//       Url url("HTTP://user@authority.com/foo/bar?query#fragment")
//         "user@authority.com" == url.getAuthority()
//         80                   == url.getDefaultPort()
//         "fragment"           == url.getFragment()
//         "authority.com"      == url.getHost()
//         "foo/bar"            == url.getPath()
//         (-1)                 == url.getPort()
//         "http"               == url.getProtocol()
//         "query"              == url.getQuery()
//         "user"               == url.getUserinfo()
//
//       Url url("ftp://auth.com:8080/foo/bar")
//         "auth.com:8080"      == url.getAuthority()
//         21                   == url.getDefaultPort()
//         ""                   == url.getFragment()
//         "auth.com"           == url.getHost()
//         "foo/bar"            == url.getPath()
//         8080                 == url.getPort()
//         "ftp"                == url.getProtocol()
//         ""                   == url.getQuery()
//         ""                   == url.getUserinfo()
//
//----------------------------------------------------------------------------
class Url {                         // URL Parser
//----------------------------------------------------------------------------
// Url::Attributes
//----------------------------------------------------------------------------
protected:
std::string            uri;         // The URI (iff valid)

public:
enum RC                             // Set return codes
{  RC_OK= 0                         // No error
,  RC_SYNTAX=  1                    // Generic syntax error
,  RC_NOPROTO= 2                    // No protocol specified
,  RC_NOHOST=  3                    // No host authority specified
}; // enum RC

//----------------------------------------------------------------------------
// Url::Constructors
//----------------------------------------------------------------------------
public:
   ~Url( void );                    // Destructor
   Url( void );                     // Default constructor

   Url(                             // Constructor
     const std::string&uri);        // Using this URI

public:                             // Allowed:
   Url(const Url&);                 // Copy constructor
Url& operator=(const Url&);         // Assignment operator
Url& operator=(const std::string&); // Assignment operator

//----------------------------------------------------------------------------
// Url::Accessors
//----------------------------------------------------------------------------
public:
std::string                         // The authority
   getAuthority( void ) const;      // Get authority

int                                 // The default authority port number, or (-1)
   getDefaultPort( void ) const;    // Get default authority port number

std::string                         // The fragment
   getFragment( void ) const;       // Get fragment

std::string                         // The authority host
   getHost( void ) const;           // Get authority host

std::string                         // The path
   getPath( void ) const;           // Get path

int                                 // The authority port number, or (-1)
   getPort( void ) const;           // Get authority port number, or (-1)

std::string                         // The protocol (lower case)
   getProtocol( void ) const;       // Get protocol (lower case)

std::string                         // The query
   getQuery( void ) const;          // Get query

std::string                         // The URI
   getURI( void ) const;            // Get URI

std::string                         // The authority user info
   getUserInfo( void ) const;       // Get authority user info

int                                 // Return code (0 OK)
   setURI(                          // Set the URI
     const std::string&uri);        // Using this URI

//----------------------------------------------------------------------------
// Url::Methods
//----------------------------------------------------------------------------
public:
void
   reset( void );                   // Reset the Url
}; // class Url

#endif // URL_H_INCLUDED
