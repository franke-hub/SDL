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
//       PROTOTYPE: Curl Services desriptor
//
// Last change date-
//       2024/10/05
//
//----------------------------------------------------------------------------
#ifndef CURL_H_INCLUDED
#define CURL_H_INCLUDED

#include <pub/Dispatch.h>           // For pub::dispatch::Task

#include "Service.h"                // For Service, base class

//============================================================================
//
// Class-
//       Service_fetchURL
//
// Purpose-
//       Rate-limited web page fetch
//
//----------------------------------------------------------------------------
class Service_fetchURL : public Service {
//----------------------------------------------------------------------------
// Attributes
pub::dispatch::Task&   task;        // Our associated Task

//----------------------------------------------------------------------------
//
// Methods-
//       Service_fetchURL
//       ~Service_fetchURL
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
public:
   Service_fetchURL();              // (Default) constructor
   ~Service_fetchURL();             // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       Service_fetchURL::curl
//
// Purpose-
//       (Asynchronously) display a web page
//
//----------------------------------------------------------------------------
void
   curl(                            // Asynchronously display
     const char*       url);        // This URL (web page)

//----------------------------------------------------------------------------
//
// Method-
//       Service_fetchURL::fetch
//
// Purpose-
//       Fetch a URL
//
//----------------------------------------------------------------------------
std::string                         // The web page string
   fetch(                           // Fetch
     const char*       url);        // This URL (web page)
};
#endif // CURL_H_INCLUDED
