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
//       utility.cpp
//
// Purpose-
//       Implement http/utility.h subroutines
//
// Last change date-
//       2022/07/21
//
// Implementation notes-
//       TODO: Verify usage
//
//----------------------------------------------------------------------------
#include <cstring>                  // For strerror
#include <stdexcept>                // For std::runtime_error
#include <errno.h>                  // For errno

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/utility.h>            // For pub::utility::visify

#include "pub/http/utility.h"       // For pub::http::utility, implemented

using namespace _LIBPUB_NAMESPACE;
using namespace _LIBPUB_NAMESPACE::debugging;
using pub::utility::visify;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  IODM= false                      // Input/Output Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more
}; // enum

namespace pub::http {               // Implementation namespace
//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::http::utility::not_coded_yet
//
// Purpose-
//       Throw std::runtime_error("NOT CODED YET");
//
//----------------------------------------------------------------------------
int
   utility::not_coded_yet(int line, const char* file)
{
   debugh("\n\n%4d %s ******** NOT CODED YET ********\n\n\n", line, file);
   throw std::runtime_error("NOT CODED YET");
   return -1;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::http::utility::report_error
//
// Purpose-
//       Display system error message, preserving errno
//
//----------------------------------------------------------------------------
void
   utility::report_error(int line, const char* file,
     const char*       op)          // Operation name
{
   int ERRNO= errno;
   debugf("%4d http::%s: %s failure: %d:%s\n", line, file, op, ERRNO
         , strerror(ERRNO));
   errno= ERRNO;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::http::utility::should_not_occur
//
// Purpose-
//       Display "SHOULD NOT OCCUR"  message
//
//----------------------------------------------------------------------------
void
   utility::should_not_occur(int line, const char* file)
{
   debugh("%4d %s HCDM: %s\n", line, file, "SHOULD NOT OCCUR");
   throw std::runtime_error("SHOULD NOT OCCUR");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::http::utility::visify_char
//
// Purpose-
//       Visify a character
//
//----------------------------------------------------------------------------
std::string
   utility::visify_char(int C)
{
   if( C ) {
     char buff[2]= {(char)C, '\0'};
     string S= buff;
     return visify(S);
   }

   static const string zero= "\\0";
   return zero;
}
}  // namespace pub::http
