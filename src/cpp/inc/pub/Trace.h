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
//       Trace.h
//
// Purpose-
//       Trace table storage allocator.
//
// Last change date-
//       2026/01/28
//
// Usage notes-
//       The Trace object allocates storage sequentially from itself, wrapping
//       when a request cannot otherwise be satisfied. This is intended for use
//       as a trace table item allocator. Applications may find other uses.
//
//       The Trace object is thread-safe and process-safe. Trace tables may be
//       allocated in shared memory and shared between processes. However,
//       thread and process safety relies on a Trace Record's build completion
//       before allocation wraps and storage is reused. Shorter Record build
//       sequences and larger trace tables further reduce an already low
//       probability of table wrap storage collisions.
//
// Prerequisites-
//       Requires (and tests for) define of __WORDSIZE with value of 64.
//       This definition is provided with GCC compiler in stdint.h.
//       (__WORDSIZE == 32) NOT CODED YET
//
// Implementation notes-
//       Applications are responsible for trace table allocation and release.
//       The Trace object is contained within the trace table.
//       The entire trace table is initialized using Trace::make().
//
// Sample usage-
//       using _LIBPUB_NAMESPACE;
//       void* storage= malloc(desired_size); // Unaligned is OK. make trims
//       Trace::table= Trace::make(storage, desired_size);
//
//       : For a defined (standard) Trace Record
//       Trace::trace(".xxx", "yyyy", this, that); // this && that are (void*)
//       :
//       : or, for a non-standard Trace Record:
//       struct Record : public Trace::Record { // Your Record
//         : // Your data goes here
//       };
//       Record* record= (Record*)Trace::storage_if(sizeof(Record));
//       if( record ) {             // If trace is active
//         : // Initialize your Record
//         record->trace(".xxx");   // Set the trace identifier + clock
//       }
//       :
//       Trace::table= nullptr; free(Trace::table); // Done with trace table
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_TRACE_H_INCLUDED
#define _LIBPUB_TRACE_H_INCLUDED

#include <atomic>                   // For std::atomic_uint64_t, ...
#include <new>                      // For in-place operator new
#include <cstdint>                  // For uint32_t, __WORDSIZE, ...
#include <cstring>                  // For memset, memcpy, strcpy
#include <ctime>                    // For CLOCK_REALTIME

#include <endian.h>                 // For be64toh, ...

#include "utility.i"                // For conversion routines

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Anonymous struct-
//       opt_t
//
// Purpose-
//       The constructed trace option type.
//
//----------------------------------------------------------------------------
namespace {
struct opt_t {                      // Option type
size_t                 option;      // The option

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   opt_t(const uintptr_t i)
{  option= i; }

   opt_t(
     const char*       c)           // char* option (with blank fill)
{  size_t s= strlen(c);
   if( s > sizeof(option) )
     s= sizeof(option);
   memcpy((char*)&option, c, s);
   if( s < sizeof(option) )
     memset((char*)&option + s, ' ', sizeof(option) - s);
}

   opt_t(const void* v)
{  option= uintptr_t(v); }

   opt_t(std::nullptr_t)
{  option= 0; }
}; // struct opt_t
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Class-
//       Trace
//
// Purpose-
//       In memory trace object descriptor.
//
//----------------------------------------------------------------------------
class Trace {                       // Trace object
public:
//----------------------------------------------------------------------------
// Trace::Typedefs, enumerations, and constants
//----------------------------------------------------------------------------
enum                                // Generic enum
{  ALIGNMENT= 32                    // Table and Record alignment
// TABLE_SIZE_MAX= 0x0'FFFF'FF00UL  // Maximum allowed table size
// TABLE_SIZE_MIN= 0x0'0001'0000UL  // Minimum allowed table size

,  WSIZE=          sizeof(void*)
}; // enum

// Maximum/minimum trace table size
static constexpr size_t const TABLE_SIZE_MAX= 0x0'FFFF'FF00UL; // Maximum
static constexpr size_t const TABLE_SIZE_MIN= 0x0'0001'0000UL; // Minimum

enum FLAG_X                         // Flag[] indexes
{  X_HALT= 0                        // The HALT flag. If non-zero, halt
,  X_OFFSET= 3                      // Alignment offset adjustment
}; // enum FLAG_X

//----------------------------------------------------------------------------
// Trace::Attributes
//----------------------------------------------------------------------------
// Applications allocate and delete trace table storage, setting and clearing
// Trace::table.
static Trace*          table;       // Common Trace instance

// Applications can, but normally do not access these fields
std::atomic_uint32_t   next;        // Offset: Next trace entry
uint32_t               size;        // Offset: Size of trace table storage
uint8_t                flag[4];     // Control flags
uint8_t                user[4];     // (Available for application usage)
uint32_t               zero;        // Offset: Trace table origin
uint32_t               last;        // Offset: Last trace entry before wrap
uint64_t               wrap;        // The wrap counter

//----------------------------------------------------------------------------
// Trace::Record (POD: Plain Old Data)
//----------------------------------------------------------------------------
struct Record {                     // A standard (POD) trace record
char                   ident[4];    // The trace type identifier
uint32_t               unit;        // The trace unit identifier
uint64_t               clock;       // The UTC epoch clock, in nanoseconds
char                   value[16];   // User data area, (2 64-bit void*'s)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static size_t*                      // The indexed size_t*
   zx2z(                            // Indexed intptr* conversion
     char*             value,       // The data area
     unsigned          index)       // The intptr_t* index
{  return (size_t*)value + index; }

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   set_clock()                      // Set the clock
{
   struct timespec real_time;       // UTC time base
   clock_gettime(CLOCK_REALTIME, &real_time); // Get UTC time base
   clock= htobe64(i2i(real_time.tv_sec)<<32 | real_time.tv_nsec);
}

void
   set_cpuid( void );               // Replace ident[0] with CPU ID

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   trace(                           // Initialize with
     const char*       ident);      // This char[4] trace type identifier

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   trace(                           // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     uint32_t          code);       // Trace code

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   trace(                           // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit);       // This char[4] trace subtype identifier

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   trace(                           // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     uint32_t          code,        // Trace code
     const char*       info);       // A char[16] informational Buffer

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   trace(                           // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     opt_t             O0,          // Opt[0]
     opt_t             O1= nullptr); // Opt[1]

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void                         // Uses extended record
   trace(                           // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     opt_t             O0,          // Opt[0]
     opt_t             O1,          // Opt[1]
     opt_t             O2,          // Opt[2]
     opt_t             O3= nullptr, // Opt[3]
     opt_t             O4= nullptr, // Opt[4]
     opt_t             O5= nullptr); // Opt[5]
}; // struct Record

//----------------------------------------------------------------------------
// Trace::Record_EX (An extended Record)
//----------------------------------------------------------------------------
struct Record_EX : public Record {  // An extended Record
char                   extend[32];  // Data area extension
}; // struct Record_EX

//============================================================================
// The Trace::trace and Trace::io_trace static methods
//============================================================================
//
// Static methods-
//       pub::Trace::trace, pub::Trace::io_trace
//
// Purpose-
//       These are the "standard" (i.e. defined) Trace::trace methods.
//
// Implementation note-
//       These static methods allocate and initialize Trace Records, doing
//       (almost) nothing if Trace::table==nullptr.
//
//----------------------------------------------------------------------------
_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline Record*               // The trace record (uninitialized)
   trace(                           // Allocate a Trace::Record (of any length)
     unsigned          size= sizeof(Record)) // Of this length
{  return (Record*)storage_if(size); }

_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline void
   trace(                           // Simple trace event
     const char*       ident);      // Trace identifier

// Usage note: For a zero code, specify (int)0 to disambiguate between
// trace(ident, (char*)0). Using trace(ident, (char*)0) is discouraged.
_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline void
   trace(                           // Simple trace event
     const char*       ident,       // Trace identifier
     uint32_t          code);       // Trace code

_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline void
   trace(                           // Simple trace event
     const char*       ident,       // Trace identifier
     const char*       unit);       // Trace sub-identifier

_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline void
   trace(                           // I/O trace event
     const char*       ident,       // Trace identifier
     uint32_t          code,        // Trace code (Usually __LINE__)
     const char*       info);       // Trace info (16 characters max used)

_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline void
   trace(                           // Simple trace event
     const char*       ident,       // Trace identifier
     const char*       unit,        // Trace sub-identifier
     opt_t             O0,          // Opt[0]
     opt_t             O1= nullptr); // Opt[1]

_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline void                  // Uses extended record
   trace(                           // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     opt_t             O0,          // Opt[0]
     opt_t             O1,          // Opt[1]
     opt_t             O2,          // Opt[2]
     opt_t             O3= nullptr, // Opt[3]
     opt_t             O4= nullptr, // Opt[4]
     opt_t             O5= nullptr); // Opt[5]

// I/O Trace Methods - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline void
   io_trace(                        // I/O trace event
     const char*       ident,       // A char[4] Trace type identifier
     const char*       unit,        // A char[4] trace unit identifier
     opt_t             O0,          // (Usually an object address)
     opt_t             O1,          // (Usually object information)
     const void*       data);       // Trace data (32 characters max used)

_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline void
   io_trace(                        // I/O trace event
     const char*       ident,       // A char[4] Trace type identifier
     const char*       unit,        // A char[4] trace unit identifier
     opt_t             O0,          // (Usually an object address)
     opt_t             O1,          // (Usually object information)
     const void*       data,        // Trace data (32 characters max used)
     size_t            size);       // Actual data size

//----------------------------------------------------------------------------
// Trace::Buffer<size_t>, a temporary (completely filled) character string
//   (Used by Trace::trace and Trace::io_trace implementations)
//----------------------------------------------------------------------------
template<size_t SIZE>
struct Buffer {
char buffer[SIZE];                  // The temporary Buffer, '\0' padded

   Buffer( void ) = default;

   Buffer(const Buffer& copy)
{  memcpy(this->buffer, copy.buffer, SIZE); }

   Buffer(const char* info)
{  size_t size= strlen(info);
   if( size < SIZE ) {
     memcpy(buffer, info, size);
     memset(buffer+size, '\0', SIZE-size);
   } else {
     memcpy(buffer, info, SIZE);
   }
}

   Buffer(const char* info, size_t size)
{  if( size < SIZE ) {
     memcpy(buffer, info, size);
     memset(buffer+size, '\0', SIZE-size);
   } else {
     memcpy(buffer, info, SIZE);
   }
}

const char*                         // The char[SIZE] buffer
   get( void ) const                // Get the buffer
{  return buffer; }
}; // struct Buffer

//----------------------------------------------------------------------------
//
// Static method-
//       Trace::make
//
// Purpose-
//       Initialize the Trace table
//
// Usage notes-
//       Applications control the allocation and deletion of Trace table
//       storage. There is no alignment restriction on the storage area.
//       The resultant Trace object, however, is always ALIGNMENT aligned.
//       The Trace object is at the (aligned) beginning of the storage area.
//
//       This method DOES NOT set Trace::table. If desired, an application
//       may set this perhaps using Trace::table= make(addr, size);
//
//----------------------------------------------------------------------------
static Trace*                       // The Trace object
   make(                            // Create a Trace object from
     void*             addr,        // Storage area
     size_t            size);       // Storage length

//----------------------------------------------------------------------------
// Trace::Constructors/Destructor/Operators
//----------------------------------------------------------------------------
protected:                          // Applications MUST use make
   Trace(                           // Constructor
     uint32_t          size);       // Size of trace area, including *this
   Trace(const Trace&) = delete;    // Disallowed copy constructor

public:
   ~Trace( void ) {}                // Destructor

Trace& operator=(const Trace&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Trace::Debugging (Displays compile-time options)
//----------------------------------------------------------------------------
static void
   static_debug(const char* info= ""); // Static debugging information

//----------------------------------------------------------------------------
// Trace::Accessors
//----------------------------------------------------------------------------
bool is_active( void )              // Is trace active?
{  return flag[X_HALT] == 0; }

//----------------------------------------------------------------------------
// Trace::Methods
//----------------------------------------------------------------------------
// allocate: Allocate storage
_LIBPUB_HOT
void*                               // Resultant
   allocate(                        // Allocate a trace record
     uint32_t          size);       // of this length

// allocate_if: Allocate storagae, nullptr if is_active() == false
_LIBPUB_FLATTEN
_LIBPUB_HOT
void*                               // -> Trace record
   allocate_if(                     // Allocate a trace record
     uint32_t          size)        // of this length
{  if( is_active() ) return allocate(size); else return nullptr; }

// deactivate: Deactivate this Trace object
inline void
   deactivate( void )               // Suspend tracing
{  flag[X_HALT]= true; }            // is_active() now returns false

// dump: Create an unformatted hex dump file using Debug::tracef
//   Note: the global Debug lock is held while dumping.
void
   dump( void ) const;              // Dump the trace table

inline uint32_t                     // Offset of record
   offset(                          // Get offset of
     void*             record)      // This record
{  return uint32_t((char*)record - (char*)this); }

// reactivate: Reactivate this Trace object
inline void
   reactivate( void )               // Resume tracing
{  flag[X_HALT]= false; }           // is_active() now returns true

static void
   start( void );                   // Start tracing (if table present)

static void
   stop( void );                    // Stop tracing (if table present)

// storage_if: Static storage allocator (with status checking)
_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline void*                 // The storage, nullptr if inactive
   storage_if(                      // Conditionally allocate storage
     uint32_t          size)        // of this length
{  if( table && table->is_active() )
     return table->allocate(size);
   else
     return nullptr;
}
}; // class Trace
_LIBPUB_END_NAMESPACE
#include "Trace.i"                  // Include implementations
#endif // _LIBPUB_TRACE_H_INCLUDED
