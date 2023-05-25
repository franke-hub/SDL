//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2023 Frank Eskesen.
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
//       2023/05/24
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
#include <stdint.h>                 // For uint32_t
#include <string.h>                 // For memset, memcpy, strcpy
#include <time.h>                   // For CLOCK_REALTIME
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
// TABLE_SIZE_MAX= 0x0'FFFF'FF00UL // Maximum allowed table size
// TABLE_SIZE_MIN= 0x0'0001'0000UL // Minimum allowed table size

,  USE_BIG_ENDIAN= true             // A bit slower but trace easier to read
,  WSIZE=          sizeof(void*)
}; // enum

static constexpr size_t TABLE_SIZE_MAX= 0x0'FFFF'FF00UL; // Maximum table size
static constexpr size_t TABLE_SIZE_MIN= 0x0'0001'0000UL; // Minimum table size

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
char                   value[2*sizeof(void*)]; // Data values (2 void*'s)

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

void
   set_cpuid( void );               // Replace ident[0] with CPU ID

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   trace(                           // Initialize with
     const char*       ident)       // This char[4] trace type identifier
{  set_clock();                     // Set the clock
   memcpy(this->ident, ident, sizeof(this->ident));
   set_cpuid();                     // Replace ident[0] with cpu id
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
   ((void const**)value)[0]= 0;
   ((void const**)value)[1]= 0;

   trace(ident);
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void
   trace(                           // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     const void*       W0)          // Word[0]
{  memcpy(&this->unit, unit, sizeof(this->unit));

   if( USE_BIG_ENDIAN ) {           // Use big_endian conversion?
     uintptr_t V0= uintptr_t(W0);
     for(unsigned i= WSIZE; i>0; i--) {
       value[i+0*WSIZE-1]= char(V0);
       V0 >>= 8;
     }
   } else {
     ((void const**)value)[0]= W0;
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
     const void*       W0,          // Word[0]
     const void*       W1)          // Word[1]
{  memcpy(&this->unit, unit, sizeof(this->unit));

   if( USE_BIG_ENDIAN ) {           // Use big_endian conversion?
     uintptr_t V0= uintptr_t(W0);
     uintptr_t V1= uintptr_t(W1);
     for(unsigned i= WSIZE; i>0; i--) {
       value[i+0*WSIZE-1]= char(V0);
       value[i+1*WSIZE-1]= char(V1);
       V0 >>= 8;
       V1 >>= 8;
     }
   } else {
     ((void const**)value)[0]= W0;
     ((void const**)value)[1]= W1;
   }

   trace(ident);
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
inline void                         // Expanded record
   trace(                           // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     const void*       W0,          // Word[0]
     const void*       W1,          // Word[1]
     const void*       W2,          // Word[2]
     const void*       W3= nullptr, // Word[3]
     const void*       W4= nullptr, // Word[4]
     const void*       W5= nullptr) // Word[5]
{  memcpy(&this->unit, unit, sizeof(this->unit));

   struct XR {                      // Extended Record
     char              ident[4];    // The trace type identifier
     uint32_t          unit;        // The trace unit identifier
     uint64_t          clock;       // The UTC epoch clock, in nanoseconds
     char              value[6*sizeof(void*)]; // Data values (6 void*'s)
   }; // struct XR

   struct XR* that= (struct XR*)this; // Extended Record
   if( USE_BIG_ENDIAN ) {           // Use big_endian conversion?
     uintptr_t V2= uintptr_t(W2);
     uintptr_t V3= uintptr_t(W3);
     uintptr_t V4= uintptr_t(W4);
     uintptr_t V5= uintptr_t(W5);
     for(unsigned i= WSIZE; i>0; i--) {
       that->value[i+2*WSIZE-1]= char(V2);
       that->value[i+3*WSIZE-1]= char(V3);
       that->value[i+4*WSIZE-1]= char(V4);
       that->value[i+5*WSIZE-1]= char(V5);
       V2 >>= 8;
       V3 >>= 8;
       V4 >>= 8;
       V5 >>= 8;
     }
   } else {
     ((void const**)that->value)[2]= W2;
     ((void const**)that->value)[3]= W3;
     ((void const**)that->value)[4]= W4;
     ((void const**)that->value)[5]= W5;
   }

   trace(ident, unit, W0, W1);
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

//----------------------------------------------------------------------------
//
// Static method-
//       trace
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
   trace(                           // Get trace record
     unsigned          size= sizeof(Record)) // Of this size
{  Record* record= (Record*)storage_if(size);
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
     const char*       info)        // Trace info (16 characters max used)
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
     const void*       W0)          // Word[0]
{  Record* record= trace();
   if( record )
     record->trace(ident, unit, W0);
   return record;
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline Record*
   trace(                           // Simple trace event
     const char*       ident,       // Trace identifier
     const char*       unit,        // Trace sub-identifier
     const void*       W0,          // Word[0]
     const void*       W1)          // Word[1]
{  Record* record= trace();
   if( record )
     record->trace(ident, unit, W0, W1);
   return record;
}

_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline Record*               // Expanded record
   trace(                           // Initialize with
     const char*       ident,       // This char[4] trace type identifier
     const char*       unit,        // This char[4] trace subtype identifier
     const void*       W0,          // Word[0]
     const void*       W1,          // Word[1]
     const void*       W2,          // Word[2]
     const void*       W3= nullptr, // Word[3]
     const void*       W4= nullptr, // Word[4]
     const void*       W5= nullptr) // Word[5]
{  Record* record= trace(sizeof(Record) + 4*sizeof(void*));
   if( record )
     record->trace(ident, unit, W0, W1, W2, W3, W4, W5);
   return record;
}
}; // class Trace
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_TRACE_H_INCLUDED
