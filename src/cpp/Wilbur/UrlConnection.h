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
//       UrlConnection.h
//
// Purpose-
//       Define a URL connection.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef URLCONNECTION_H_INCLUDED
#define URLCONNECTION_H_INCLUDED

#include <string>
#include <com/Socket.h>

#ifndef URL_H_INCLUDED
#include "Url.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       UrlConnection
//
// Purpose-
//       Define a URL connection.
//
//----------------------------------------------------------------------------
class UrlConnection {               // URL connection
//----------------------------------------------------------------------------
// UrlConnection::Attributes
//----------------------------------------------------------------------------
protected:
Socket                 socket;      // The connection Socket
Url                    url;         // The connection Url

public:
enum CC                             // Connect return codes
{  CC_OK= 0                         // No error
,  CC_INVALID_URL=  1               // Invalid URL
,  CC_UNKNOWN_HOST= 2               // Unknown host
,  CC_CANT_CONNECT= 3               // Connection refused
}; // enum RC

//----------------------------------------------------------------------------
// UrlConnection::Constructors
//----------------------------------------------------------------------------
public:
   ~UrlConnection( void );          // Destructor
   UrlConnection( void );           // Default constructor

   UrlConnection(                   // Constructor
     const Url&        url);        // For this URL

   UrlConnection(                   // Constructor
     const std::string&url);        // For this URL

//----------------------------------------------------------------------------
// UrlConnection::Accessors
//----------------------------------------------------------------------------
public:
Socket&                             // The Socket
   getSocket( void )                // Get Socket
{
   return socket;
}

const Url&                          // The URL
   getUrl( void ) const             // Get URL
{
   return url;
}

bool                                // TRUE iff connected
   isConnected( void ) const        // Is socket connected?
{
   return socket.isOpen();
}

int                                 // Return code, 0 OK
   setUrl(                          // Set URL
     const std::string&url)         // For this URL
{
   Url                 newUrl(url); // The new URL

   if( newUrl.getHost() != this->url.getHost() )
     disconnect();

   return this->url.setURI(url);
}

//----------------------------------------------------------------------------
// UrlConnection::Methods
//----------------------------------------------------------------------------
public:
int                                 // Return code, 0 OK
   connect( void );                 // Connect to server

void
   disconnect( void );              // Reset connection
}; // class UrlConnection

#endif // URLCONNECTION_H_INCLUDED
