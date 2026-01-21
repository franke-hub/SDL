//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2024 Frank Eskesen.
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
//       2024/12/20
//
// Usage notes-
//       Trace.h includes this file and all prerequite includes.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_TRACE_I_INCLUDED
#define _LIBPUB_TRACE_I_INCLUDED

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
// Implementation notes-
//       This is the __WORDSIZE == 64 implementation.
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
inline void
   Trace::Record::trace(            // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     uint32_t          code)        // Trace code
{  unit= htobe32(code);
// memset(value, 0, sizeof(value)); // *DOES NOT MODIFY* value

   trace(ident);
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::Record::trace(            // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit)        // This char[4] trace subtype identifier
{  memcpy(&this->unit, unit, sizeof(this->unit));
// memset(value, 0, sizeof(value)); // *DOES NOT MODIFY* value

   trace(ident);
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::Record::trace(            // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     uint32_t          code,        // Trace code
     const void*       info)        // A char[16] informational Buffer
{  memcpy(value, info, sizeof(value));

   trace(ident, code);
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::Record::trace(            // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     const void*       W0,          // Word[0]
     const void*       W1)          // Word[1]
{
   *vx2v(value,0)= i2v(htobe64(v2i(W0)));
   *vx2v(value,1)= i2v(htobe64(v2i(W1)));

   trace(ident, unit);
}

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

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::Record::trace(            // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     const void*       W0,          // Word[0]
     const void*       W1,          // Word[1]
     const void*       W2,          // Word[2]
     const void*       W3,          // Word[3]
     const void*       W4,          // Word[4]
     const void*       W5)          // Word[5]
{
   *vx2v(value,2)= i2v(htobe64(v2i(W2)));
   *vx2v(value,3)= i2v(htobe64(v2i(W3)));
   *vx2v(value,4)= i2v(htobe64(v2i(W4)));
   *vx2v(value,5)= i2v(htobe64(v2i(W5)));

   Record::trace(ident, unit, W0, W1);
}

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
   Trace::trace(                    // I/O trace event
     const char*       ident,       // Trace identifier
     uint32_t          code,        // Trace code (Usually __LINE__)
     const char*       addr)        // Trace info (16 characters max used)
{  Record* record= trace();
   if( record ) {
     new(record->value) Buffer<16>(addr);
     record->trace(ident, code);    // *DOES NOT MODIFY* value
   }
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
   Trace::trace(                    // Simple trace event
     const char*       ident,       // Trace identifier
     const char*       unit,        // Trace sub-identifier
     const void*       W0,          // Word[0]
     const void*       W1)          // Word[1]
{  Record* record= trace();
   if( record )
     record->trace(ident, unit, W0, W1);
}

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

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::trace(                    // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     const void*       W0,          // Word[0]
     const void*       W1,          // Word[1]
     const void*       W2,          // Word[2]
     const void*       W3,          // Word[3]
     const void*       W4,          // Word[4]
     const void*       W5)          // Word[5]
{  Record_EX* record= (Record_EX*)trace(sizeof(Record_EX));
   if( record )
     record->trace(ident, unit, W0, W1, W2, W3, W4, W5);
}

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
     const char*       ident,       // A char[4] Trace identifier
     const char*       unit,        // A char[4] trace subtype identifier
     const void*       W0,          // (Normally the object address)
     const void*       W1,          // (Normally object information)
     const void*       addr,        // Trace info (32 characters max used)
     size_t            size)        // Actual info size
{  Record_EX* record= (Record_EX*)trace(sizeof(Record_EX));
   if( record ) {
     new(record->vx2v(record->value,2)) Buffer<32>(addr, size);
     record->trace(ident, unit, W0, W1);
   }
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   Trace::io_trace(                 // I/O trace event
     const char*       ident,       // A char[4] Trace identifier
     const char*       unit,        // A char[4] trace subtype identifier
     const void*       W0,          // (Normally the object address)
     const void*       W1,          // (Normally object information)
     const void*       addr)        // Trace info (32 characters max used)
{  Record_EX* record= (Record_EX*)trace(sizeof(Record_EX));
   if( record ) {
     new(record->vx2v(record->value,2)) Buffer<32>(addr);
     record->trace(ident, unit, W0, W1);
   }
}
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_TRACE_H_INCLUDED
