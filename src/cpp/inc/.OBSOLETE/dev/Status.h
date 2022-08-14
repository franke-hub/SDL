//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       http/Status.h
//
// Purpose-
//       HTTP Status static object.
//
// Last change date-
//       2022/02/11
//
//----------------------------------------------------------------------------
#ifndef _PUB_HTTP_STATUS_H_INCLUDED
#define _PUB_HTTP_STATUS_H_INCLUDED

#include <string>                   // For std::string
#include <stdint.h>                 // For size_t

namespace pub::http {
//----------------------------------------------------------------------------
//
// Class-
//       Status                     // Currently in Server::reject
//
// Purpose-
//       Define the HTTP status codes and their brief explaination.
//
//----------------------------------------------------------------------------
class Status {                      // Http status code constants
public:
static const char*                  // Status description
   get_text(int);                   // Get text for error code
}; // class Status
} // namespace pub::http
#endif // _PUB_HTTP_STATUS_H_INCLUDED
