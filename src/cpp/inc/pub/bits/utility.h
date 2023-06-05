//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       bits/utility.h
//
// Purpose-
//       Internal use utilities, included separately
//
// Last change date-
//       2023/06/04
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_BITS_UTILITY_H_INCLUDED
#define _LIBPUB_BITS_UTILITY_H_INCLUDED

#include <errno.h>                  // For errno
#include <pub/Trace.h>              // For pub::Trace

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace utility {
//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::checkstop
//
// Purpose-
//       Termination error reporting, throws std::runtime_error.
//
//----------------------------------------------------------------------------
[[noreturn]]
void
   checkstop(                       // Halt tracing, throw std::runtime_error
     int               line,        // Source line number
     const char*       file,        // Source file name
     const char*       mess);       // Error messsage

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::iotrace
//
// Purpose-
//       Trace I/O operation
//
//----------------------------------------------------------------------------
_LIBPUB_FLATTEN
_LIBPUB_HOT                         // Performance critical path
static inline void
   iotrace(                         // I/O internal trace
     const char*       ident,       // Trace identifier
     const void*       addr,        // Data address
     ssize_t           size)        // Data length
{
   if( size > 0 ) {
     Trace::Record* record= Trace::trace();
     if( record ) {
       Trace::Buffer<16> buff(addr, size);
       record->trace(ident, (uint32_t)size, buff.temp);
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::not_coded_yet
//
// Purpose-
//       Throw std::runtime_error("NOT CODED YET");
//
//----------------------------------------------------------------------------
[[noreturn]]
void
   not_coded_yet(                   // Throw "NOT CODED YET" error
     int               line,        // Source file line
     const char*       file);       // Source file name

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::report_error
//
// Purpose-
//       Display system error message, preserving errno
//
//----------------------------------------------------------------------------
void
   report_error(
     int               line,        // Source file line
     const char*       file,        // Source file name
     const char*       op);         // Operation name

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::report_exception
//
// Purpose-
//       Report (recoverable) exception information, including backtrace
//
//----------------------------------------------------------------------------
void
   report_exception(                // Report (recoverable exception
     std::string       what);       // Error message

//----------------------------------------------------------------------------
//
// Subroutine-
//      utility::report_unexpected
//
// Purpose-
//       Report "should not occur" recoverable error
//
//----------------------------------------------------------------------------
void
   report_unexpected(               // Recoverable "should not occur" message
     int               line,        // Source file line
     const char*       file);       // Source file name
}  // namespace utility
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_BITS_UTILITY_H_INCLUDED
