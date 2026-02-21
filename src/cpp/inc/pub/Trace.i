//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2026 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
// SPDX-License-Identifier: LGPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Trace.i
//
// Purpose-
//       Trace.h inline implementations
//
// Last change date-
//       2026/01/28
//
// Usage notes-
//       Trace.h includes this file. Use `#include "pub/Trace.h"`, not this.
//
// Implementation notes-
//       This is the __WORDSIZE == 64 implementation.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_TRACE_I_INCLUDED
#define _LIBPUB_TRACE_I_INCLUDED                               `

static_assert( __WORDSIZE == 64, "__WORDSIZE == 64 Required");

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Method-
//       pub::Trace::Record::trace
//       pub::Trace::Record_EX::trace
//
// Purpose-
//       Implement Trace.h Trace::Record::trace methods.
//       Implement Trace.h Trace::Record_EX::trace methods
//
//----------------------------------------------------------------------------
_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::Record::trace(            // Initialize with
     const char*       ident)       // This char[4] trace type identifier
{  set_clock();                     // Set the clock
   memcpy(this->ident, ident, sizeof(this->ident));

   set_cpuid();                     // Replace ident[0] with cpu id
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void                         // *DOES NOT MODIFY* value
   Trace::Record::trace(            // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     uint32_t          code)        // Trace code
{  unit= htobe32(code);

   trace(ident);
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void                         // *DOES NOT MODIFY* value
   Trace::Record::trace(            // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit)        // This char[4] trace unit identifier
{  memcpy(&this->unit, unit, sizeof(this->unit));

   trace(ident);
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::Record::trace(            // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     uint32_t          code,        // Trace code
     const char*       info)        // A char[16] informational Buffer
{  memcpy(value, info, sizeof(value));

   trace(ident, code);
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::Record::trace(            // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     opt_t             O0,          // Opt[0]
     opt_t             O1)          // Opt[1]
{
   *zx2z(value,0)= htobe64(O0.option);
   *zx2z(value,1)= htobe64(O1.option);

   trace(ident, unit);
}

#if 0
_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::Record::trace(            // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     intptr_t          I0,          // intptr[0]
     intptr_t          I1)          // intptr[1]
{
   *ix2i(value,0)= htobe64(I0);
   *ix2i(value,1)= htobe64(I1);

   trace(ident, unit);
}
#endif

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::Record::trace(            // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     opt_t             O0,          // Opt[0]
     opt_t             O1,          // Opt[1]
     opt_t             O2,          // Opt[2]
     opt_t             O3,          // Opt[3]
     opt_t             O4,          // Opt[4]
     opt_t             O5)          // Opt[5]
{
   *zx2z(value,2)= htobe64(O2.option);
   *zx2z(value,3)= htobe64(O3.option);
   *zx2z(value,4)= htobe64(O4.option);
   *zx2z(value,5)= htobe64(O5.option);

   Record::trace(ident, unit, O0, O1);
}

#if 0
_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::Record::trace(            // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     intptr_t          I0,          // intptr_t[0]
     intptr_t          I1,          // intptr_t[1]
     intptr_t          I2,          // intptr_t[2]
     intptr_t          I3,          // intptr_t[3]
     intptr_t          I4,          // intptr_t[4]
     intptr_t          I5)          // intptr_t[5]
{
   *ix2i(value,2)= htobe64(I2);
   *ix2i(value,3)= htobe64(I3);
   *ix2i(value,4)= htobe64(I4);
   *ix2i(value,5)= htobe64(I5);

   Record::trace(ident, unit, I0, I1);
}
#endif

//----------------------------------------------------------------------------
//
// Static method-
//       pub::Trace::trace
//
// Purpose-
//       Implement Trace.h's static Trace::trace methods.
//
// Implementation note-
//       These static methods allocate and initialize Trace Records, doing
//       (almost) nothing if Trace::table==nullptr.
//
//----------------------------------------------------------------------------
_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::trace(                    // Simple trace event
     const char*       ident)       // Trace identifier
{  Record* record= trace();
   if( record )
     record->trace(ident);
}

// Usage note: For a zero code, specify (int)0 to disambiguate between
// trace(ident, (char*)0). Using trace(ident, (char*)0) is discouraged.
_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::trace(                    // Simple trace event
     const char*       ident,       // Trace identifier
     uint32_t          code)        // Trace code
{  Record* record= trace();
   if( record )
     record->trace(ident, code);
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::trace(                    // Simple trace event
     const char*       ident,       // Trace identifier
     const char*       unit)        // Trace sub-identifier
{  Record* record= trace();
   if( record )
     record->trace(ident, unit);    // *DOES NOT MODIFY* value
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::trace(                    // I/O trace event
     const char*       ident,       // Trace identifier
     uint32_t          code,        // Trace code (Usually __LINE__)
     const char*       info)        // Trace info (16 characters max used)
{  Record* record= trace();
   if( record ) {
     new(record->value) Buffer<16>(info);
     record->trace(ident, code);    // *DOES NOT MODIFY* value
   }
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::trace(                    // Simple trace event
     const char*       ident,       // Trace identifier
     const char*       unit,        // Trace sub-identifier
     opt_t             O0,          // Opt[0]
     opt_t             O1)          // Opt[1]
{  Record* record= trace();
   if( record )
     record->trace(ident, unit, O0, O1);
}

#if 0
_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::trace(                    // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     intptr_t          I0,          // intptr[0]
     intptr_t          I1)          // intptr[1]
{  Record* record= trace();
   if( record )
     record->trace(ident, unit, I0, I1);
}
#endif

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void                         // Uses extended record
   Trace::trace(                    // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     opt_t             O0,          // Opt[0]
     opt_t             O1,          // Opt[1]
     opt_t             O2,          // Opt[2]
     opt_t             O3,          // Opt[3]
     opt_t             O4,          // Opt[4]
     opt_t             O5)          // Opt[5]
{  Record_EX* record= (Record_EX*)trace(sizeof(Record_EX));
   if( record )
     record->trace(ident, unit, O0, O1, O2, O3, O4, O5);
}

#if 0
_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::trace(                    // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     intptr_t          I0,          // intptr_t[0]
     intptr_t          I1,          // intptr_t[1]
     intptr_t          I2,          // intptr_t[2]
     intptr_t          I3,          // intptr_t[3]
     intptr_t          I4,          // intptr_t[4]
     intptr_t          I5)          // intptr_t[5]
{  Record_EX* record= (Record_EX*)trace(sizeof(Record_EX));
   if( record )
     record->trace(ident, unit, I0, I1, I2, I3, I4, I5);
}
#endif

//----------------------------------------------------------------------------
//
// Static method-
//       Trace::io_trace
//
// Purpose-
//       Implement Trace.h's static Trace::io_trace methods.
//
// Implementation note-
//       I/O data area initialization uses in-place Buffer constructors.
//
//----------------------------------------------------------------------------
_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::io_trace(                 // I/O trace event
     const char*       ident,       // A char[4] Trace type identifier
     const char*       unit,        // A char[4] trace unit identifier
     opt_t             O0,          // (Usually an object address)
     opt_t             O1,          // (Usually object information)
     const void*       data)        // Trace data (32 characters max used)
{  Record_EX* record= (Record_EX*)trace(sizeof(Record_EX));
   if( record ) {
     new(record->zx2z(record->value,2)) Buffer<32>((const char*)data);
     record->trace(ident, unit, O0, O1);
   }
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::io_trace(                 // I/O trace event
     const char*       ident,       // A char[4] Trace type identifier
     const char*       unit,        // A char[4] trace unit identifier
     opt_t             O0,          // (Usually an object address)
     opt_t             O1,          // (Usually object information)
     const void*       data,        // Trace data (32 characters max used)
     size_t            size)        // Actual data size
{  Record_EX* record= (Record_EX*)trace(sizeof(Record_EX));
   if( record ) {
     new(record->zx2z(record->value,2)) Buffer<32>((const char*)data, size);
     record->trace(ident, unit, O0, O1);
   }
}
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_TRACE_H_INCLUDED
