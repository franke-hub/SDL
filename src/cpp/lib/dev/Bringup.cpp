//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Bringup.cpp
//
// Purpose-
//       Temporary implementation placeholder.
//
// Last change date-
//       2022/02/11
//
//----------------------------------------------------------------------------
#include <memory>                   // For std::shared_ptr
#include <new>                      // For std::bad_alloc
#include <cstring>                  // For memset
#include <ostream>                  // For std::ostream
#include <stdexcept>                // For std::out_of_range, ...
#include <string>                   // For std::string

#include <assert.h>                 // For assert()
#include <errno.h>                  // For errno
#include <stdio.h>                  // For fprintf()
#include <stdint.h>                 // For integer types
#include <arpa/inet.h>              // For inet_ntop()

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/utility.h>            // For pub::utility::to_string()

// #include "pub/http/Client.h"     // For pub::http::Client
// #include "pub/http/Options.h"    // For pub::http::Options, implemented
// #include "pub/http/Request.h"    // For pub::http::Request
// #include "pub/http/Response.h"   // For pub::http::Response
// #include "pub/http/utility.h"    // For pub::http utilities

using namespace _PUB_NAMESPACE;
using namespace _PUB_NAMESPACE::debugging;
using std::string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
}; // enum

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------

namespace pub::http {               // Implementation namespace
}  // namespace pub::http
