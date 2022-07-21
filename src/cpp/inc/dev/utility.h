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
//       http/utility.h
//
// Purpose-
//       HTTP implementation utilities.
//
// Last change date-
//       2022/02/11
//
//----------------------------------------------------------------------------
#ifndef _PUB_HTTP_UTILITY_H_INCLUDED
#define _PUB_HTTP_UTILITY_H_INCLUDED

#include <pub/Trace.h>              // For pub::Trace

namespace pub::http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Hunk;                         // HTTP Data Hunk

//----------------------------------------------------------------------------
//
// Struct-
//       utility
//
// Purpose-
//       (Static) pub::http internal use method container
//
//----------------------------------------------------------------------------
struct utility {
using string = std::string;

//----------------------------------------------------------------------------
//
// Category-
//       NOP default functions
//
// Purpose-
//       std::function<...> defaults that do nothing
//
//----------------------------------------------------------------------------
struct f_data {
void operator()(const Hunk&) {}
}; // struct f_data

struct f_error {
void operator()(const string&) {}
}; // struct f_error

struct f_void {
void operator()( void ) {}
}; // struct f_void

//----------------------------------------------------------------------------
//
// Method-
//       pub::http::utility::iotrace
//
// Purpose-
//       Trace I/O operation
//
// Implementation note-
//       Performance critical path
//
//----------------------------------------------------------------------------
static inline void
   iotrace(const char* ident, const void* addr, size_t size)
{
   Trace::Record* record= Trace::trace();
   if( record) {
     Trace::Buffer<16> buff(addr, size);
     record->trace(ident, size, buff.temp);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::http::utility::not_coded_yet
//
// Purpose-
//       Throw std::runtime_error("NOT CODED YET");
//
//----------------------------------------------------------------------------
static int
   not_coded_yet(int, const char*);

//----------------------------------------------------------------------------
//
// Method-
//       pub::http::utility::report_error
//
// Purpose-
//       Display system error message, preserving errno
//
//----------------------------------------------------------------------------
static void
   report_error(int, const char*,
     const char*       op);         // Operation name

//----------------------------------------------------------------------------
//
// Method-
//       pub::http::utility::should_not_occur
//
// Purpose-
//       Display "SHOULD NOT OCCUR"  message, throw std::runtime_error
//
//----------------------------------------------------------------------------
static void
   should_not_occur(int, const char*);

//----------------------------------------------------------------------------
//
// Method-
//       pub::http::utility::visify_char
//
// Purpose-
//       Visify one character
//
//----------------------------------------------------------------------------
static string visify_char(int C);
}; // struct utility
} // namespace pub::utility::http
#endif // _PUB_HTTP_UTILITY_H_INCLUDED
