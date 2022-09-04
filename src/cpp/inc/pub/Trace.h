//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Trace.h
//
// Purpose-
//       Trace table storage allocator.
//
// Last change date-
//       2022/09/02
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
// Implementation notes-
//       Applications are responsible for trace table allocation and release.
//       The Trace object is contained within the trace table. The entire
//       trace table is initialized using make().
//
// Sample usage-
//       using _LIBPUB_NAMESPACE;
//       void* storage= malloc(desired_size); // Unaligned is OK. make trims
//       Trace::table= Trace::make(storage, desired_size);
//
//       struct Record : public Trace::Record { // Your Record
//         : // Your data goes here
//       };
//       :
//       Record* record= (Record*)Trace::storage_if(sizeof(Record));
//       if( record ) {             // If trace is active
//         : // Initialize your data
//         record->trace(".xxx");   // Set the trace identifier + clock
//       }
//       :
//       Trace::table= nullptr; free(Trace::table); // Done with trace table
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_TRACE_H_INCLUDED
#define _LIBPUB_TRACE_H_INCLUDED

#include <atomic>                   // For std::atomic_uint64_t, ...
#include <stdint.h>                 // For uint32_t
#include <string.h>                 // For memset, memcpy, strcpy
#include <arpa/inet.h>              // For htonl

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
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
,  TABLE_SIZE_MAX= 0x000FFFFF00UL   // Maximum allowed table size
,  TABLE_SIZE_MIN= 0x0000010000UL   // Minimum allowed table size

,  USE_BIG_ENDIAN= true             // Slower but trace easier to read
,  WSIZE=          sizeof(void*)
}; // enum

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
char                   value[16];   // Data values (For smallest Record)

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   set_clock()                      // Set the clock
{
   struct timespec     clock;       // UTC time base
   clock_gettime(CLOCK_REALTIME, &clock); // Get UTC time base

   if( USE_BIG_ENDIAN ) {           // Use big_endian clock value?
     uint64_t nsec= (clock.tv_sec << 32) | clock.tv_nsec;
     char* addr= (char*)&this->clock; // clock.tv_nsec: 0x00000000..0x3B9AC9FF
     for(int i= WSIZE; i>0; i--) {
       addr[i-1]= char(nsec);
       nsec >>= 8;
     }
   } else {
     this->clock= (clock.tv_sec * 1000000000) + clock.tv_nsec;
   }
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   trace(                           // Initialize with
     const char*       ident)       // This char[4] trace type identifier
{  set_clock();                     // Set the clock
   memcpy(this->ident, ident, sizeof(this->ident));
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   trace(                           // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     uint32_t          code)        // Trace code
{  unit= htonl(code);

   trace(ident);
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   trace(                           // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     uint32_t          code,        // Trace code
     const void*       info)        // If present, char[16] info
{  unit= htonl(code);
   memcpy(value, info, sizeof(value));

   trace(ident);
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   trace(                           // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit)        // This char[4] trace subtype identifier
{  memcpy(&this->unit, unit, sizeof(this->unit));

   trace(ident);
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   trace(                           // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     const void*       one)         // Word one
{  memcpy(&this->unit, unit, sizeof(this->unit));

   if( USE_BIG_ENDIAN ) {           // Use big_endian conversion?
     uintptr_t uone= uintptr_t(one);
     for(unsigned i= WSIZE; i>0; i--) {
       value[i      -1]= char(uone);
       uone >>= 8;
     }
   } else {
     ((void const**)value)[0]= one;
   }
   ((void const**)value)[1]= 0;

   trace(ident);
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   trace(                           // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     const void*       one,         // Word one
     const void*       two)         // Word two
{  memcpy(&this->unit, unit, sizeof(this->unit));

   if( USE_BIG_ENDIAN ) {           // Use big_endian conversion?
     uintptr_t uone= uintptr_t(one);
     uintptr_t utwo= uintptr_t(two);
     for(unsigned i= WSIZE; i>0; i--) {
       value[i      -1]= char(uone);
       value[i+WSIZE-1]= char(utwo);
       uone >>= 8;
       utwo >>= 8;
     }
   } else {
     ((void const**)value)[0]= one;
     ((void const**)value)[1]= two;
   }

   trace(ident);
}
}; // struct Record

//----------------------------------------------------------------------------
// Trace::Buffer, temporary character string storage area
//----------------------------------------------------------------------------
template<size_t N>                  // Buffer.temp *always* fully used
struct Buffer {
char                   temp[N];     // The temporary Buffer, '\0' padded

   Buffer(const void* info, size_t size)
{  if( size < N ) {
     memcpy(temp, info, size);
     memset(temp+size, '\0', N-size);
   } else {
     memcpy(temp, info, N);
   }
}

   Buffer(const void* info)
{  size_t size= strlen((const char*)info);
   if( size < N ) {
     memcpy(temp, info, size);
     memset(temp+size, '\0', N-size);
   } else {
     memcpy(temp, info, N);
   }
}
}; // struct Buffer

//----------------------------------------------------------------------------
// Trace::Destructor/Constructors/Operators
//----------------------------------------------------------------------------
   ~Trace( void ) {}                // Destructor

protected:                          // Applications MUST use make
   Trace(                           // Constructor
     uint32_t          size);       // Size of trace area, including *this

   Trace(const Trace&) = delete;    // Disallowed copy constructor
Trace& operator=(const Trace&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
//
// Method-
//       make
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
public:
static Trace*                       // The Trace object
   make(                            // Create a Trace object from
     void*             addr,        // Storage area
     size_t            size);       // Storage length

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

// deactivate: Globally deactivate this Trace object
inline void
   deactivate( void )               // Halt tracing
{  flag[X_HALT]= true; }            // is_active() now returns false

// dump: Create an unformatted hex dump file using Debug::tracef
//   Note: the global Debug lock is held while dumping.
void
   dump( void ) const;              // Dump the trace table

inline uint32_t                     // Offset of record
   offset(                          // Get offset of
     void*             record)      // This record
{  return uint32_t((char*)record - (char*)this); }

// storage_if: Static storage allocator (with status checking)
_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline void*                 // The storage, nullptr if inactive
   storage_if(                      // Conditionally allocate storage
     uint32_t          size)        // of this length
{  if( table )
     return table->allocate_if(size);
   else
     return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       trace
//
// Purpose-
//       Trace::Record helpers
//
//----------------------------------------------------------------------------
_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline Record*               // The trace record (uninitialized)
   trace(                           // Get trace record
     unsigned          size= 0)     // Of this extra size
{  size += unsigned(sizeof(Record));
   Record* record= (Record*)storage_if(size);
   return record;
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline Record*
   trace(                           // Simple trace event
     const char*       ident)       // Trace identifier
{  Record* record= trace();
   if( record )
     record->trace(ident);
   return record;
}

// Usage note: For a zero code, specify (int)0 to disambiguate between
// trace(ident, (char*)0). Using trace(ident, (char*)0) is discouraged.
_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline Record*
   trace(                           // Simple trace event
     const char*       ident,       // Trace identifier
     uint32_t          code)        // Trace code
{  Record* record= trace();
   if( record )
     record->trace(ident, code);
   return record;
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline Record*
   trace(                           // Simple trace event
     const char*       ident,       // Trace identifier
     uint32_t          code,        // Trace code
     const char*       info)        // Trace info (15 characters max)
{  Record* record= trace();
   if( record ) {
     Buffer<16> buff(info);
     record->trace(ident, code, buff.temp);
   }
   return record;
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline Record*
   trace(                           // Simple trace event
     const char*       ident,       // Trace identifier
     const char*       unit)        // Trace sub-identifier
{  Record* record= trace();
   if( record )
     record->trace(ident, unit);
   return record;
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline Record*
   trace(                           // Simple trace event
     const char*       ident,       // Trace identifier
     const char*       unit,        // Trace sub-identifier
     const void*       one)         // Word one
{  Record* record= trace();
   if( record )
     record->trace(ident, unit, one);
   return record;
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline Record*
   trace(                           // Simple trace event
     const char*       ident,       // Trace identifier
     const char*       unit,        // Trace sub-identifier
     const void*       one,         // Word one
     const void*       two)         // Word two
{  Record* record= trace();
   if( record )
     record->trace(ident, unit, one, two);
   return record;
}
}; // class Trace
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_TRACE_H_INCLUDED
