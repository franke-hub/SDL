//----------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Curl.h
//
// Purpose-
//       PROTOTYPE: Curl Services descriptor
//
// Last change date-
//       2024/10/07
//
//----------------------------------------------------------------------------
#ifndef CURL_H_INCLUDED
#define CURL_H_INCLUDED

#include <pub/Dispatch.h>           // For pub::dispatch::Task

#include "Service.h"                // For Service, base class

//============================================================================
//
// Class-
//       Curl_service
//
// Purpose-
//       Rate-limited web page fetch
//
//----------------------------------------------------------------------------
class Curl_service : public Service {
public:
//----------------------------------------------------------------------------
// Attributes
pub::dispatch::Task&   task;        // Our associated Task

//----------------------------------------------------------------------------
//
// Class-
//       Curl_service::Item
//
// Purpose-
//       Task work Item
//
//----------------------------------------------------------------------------
class Item : public pub::dispatch::Item { // The task's work Item
public:
typedef pub::dispatch::Done         Done_t;
typedef pub::dispatch::Item         Item_t;

std::string            request;     // The web page to fetch
std::string            response;    // The web page content

pub::dispatch::Wait    _wait;       // Our Wait Done object

   Item(                            // Constructor
     std::string       url)         // The web page URL
:  Item_t(&_wait), request(url) {}

   Item(                            // Constructor
     std::string       url,         // The web page URL
     Done_t*           done)        // Replacement Done
:  Item_t(done), request(url) {}

void
   wait( void )                     // Wait for work Item completion
{  _wait.wait(); }
}; // class Curl_service::Item

//----------------------------------------------------------------------------
//
// Methods-
//       Curl_service
//       ~Curl_service
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   Curl_service();                  // (Default) constructor
   ~Curl_service();                 // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       Curl_service::async
//
// Purpose-
//       (Asynchronously) display a web page [rate limited, immediate return]
//
//----------------------------------------------------------------------------
void
   async(                           // Asynchronously display
     const char*       url);        // This URL (web page)

//----------------------------------------------------------------------------
//
// Method-
//       Curl_service::curl
//
// Purpose-
//       Fetch a URL [rate limited, delayed return]
//
//----------------------------------------------------------------------------
std::string                         // The web page string
   curl(                            // Fetch
     const char*       url);        // This URL (web page)
};
#endif // CURL_H_INCLUDED
