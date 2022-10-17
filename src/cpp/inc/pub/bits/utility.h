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
//       bits/utility.h
//
// Purpose-
//       Internal use utilities, included separately
//
// Last change date-
//       2022/10/16
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
     size_t            size)        // Data length
{
   Trace::Record* record= Trace::trace();
   if( record) {
     Trace::Buffer<16> buff(addr, size);
     record->trace(ident, (uint32_t)size, buff.temp);
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
//      utility::should_not_occur
//
// Purpose-
//       Throw std::runtime_error("SHOULD NOT OCCUR");
//
//----------------------------------------------------------------------------
void
   should_not_occur(                // Throw "SHOULD NOT OCCUR" error
     int               line,        // Source file line
     const char*       file);       // Source file name
}  // namespace utility
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_BITS_UTILITY_H_INCLUDED
