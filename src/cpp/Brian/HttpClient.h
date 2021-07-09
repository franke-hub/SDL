//----------------------------------------------------------------------------
//
//       Copyright (c) 2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HttpClient.h
//
// Purpose-
//       Define HTTP client objects. (( UNUSED ))
//
// Last change date-
//       2021/07/09
//
// Implementation notes-
//       argv[0] *INP* The input string
//       argv[1] *OUT* The command name
//
//----------------------------------------------------------------------------
#ifndef HTTPCLIENT_H_INCLUDED
#define HTTPCLIENT_H_INCLUDED

#include <string>
#include "Command.h"
#include "Service.h"

//----------------------------------------------------------------------------
//
// Class-
//       HttpClientService
//
// Purpose-
//       The HttpClientService handles HTTP requests.
//
//----------------------------------------------------------------------------
class HttpClientService : public Service { // The HTTP request handler
//----------------------------------------------------------------------------
// HttpClientService::Attributes
//----------------------------------------------------------------------------
protected:
// None defined

//----------------------------------------------------------------------------
// HttpClientService::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~HttpClientService( void ) {}    // Destructor
   HttpClientService( void )        // Constructor
:  Service("http") {}

   HttpClientService(const HttpClientService&) = delete; // Disallowed copy constructor
   HttpClientService& operator=(const HttpClientService&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// HttpClientService::Methods
//----------------------------------------------------------------------------
public:
virtual void
   work(                            // Handle
     Item*             item);       // This work Item
}; // class HttpClient
#endif // HTTPCLIENT_H_INCLUDED
