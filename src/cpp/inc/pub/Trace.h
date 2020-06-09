//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2020 Frank Eskesen.
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
//       In memory trace object.
//
// Last change date-
//       2020/06/09
//
// Implementation notes-
//       Maximum table size of 4G (-32): 0x00ffffffe0, including header.
//       Applications are responsible for trace access control
//
// Usage-
//       struct Record : public pub::Trace::Record {
//         int32_t     ident;
//         :
//       };
//
//       Trace* trace= Trace::make(desired_size);
//
//       Record* record= (Record*)trace->allocate(sizeof(Record));
//         : // Initialize most of record
//         record->ident= *(uint32_t)(".xxx"); // Identify the record
//       // Always set the identifier last
//       // Identifier ".000" indicates record not initialized
//       // Identifier ".END" indicates wrap occurred
//
//----------------------------------------------------------------------------
#ifndef _PUB_TRACE_H_INCLUDED
#define _PUB_TRACE_H_INCLUDED

#include <stdint.h>                 // For uint32_t
#include "config.h"                 // For _PUB_NAMESPACE

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       Trace
//
// Purpose-
//       In memory trace object descriptor.
//
// Implementation notes-
//       The wrap counters are sequenced in big-endian order, but the
//       individual counters are machine-endian.
//
//----------------------------------------------------------------------------
class Trace {                       // Trace object
//----------------------------------------------------------------------------
// Trace::Attributes
//----------------------------------------------------------------------------
private:
uint32_t               top;         // Offset: Trace table origin
uint32_t               next;        // Offset: Next trace entry
uint32_t               bot;         // Offset: End of trace table
uint32_t               size;        // Available size of trace table
uint32_t               wrap[4];     // Wrap counters

public:
enum { ALIGNMENT= 32 };             // Required alignment
static Trace*          trace;       // Common Trace instance (Application controlled)

//----------------------------------------------------------------------------
// Trace::Enumerations and Typedefs
//----------------------------------------------------------------------------
public:
enum                                // Generic enum
{  MAXIMUM_SIZE=       0xffffffe0   // Maximum allowed size
,  MINIMUM_SIZE=       0x00010000   // Minimum allowed size
}; // enum

//----------------------------------------------------------------------------
// Trace::Record
//----------------------------------------------------------------------------
public:
struct Record {                     // A trace record
}; // struct Trace::Record

//----------------------------------------------------------------------------
// Trace::Constructors
//----------------------------------------------------------------------------
public:
   ~Trace( void ) {}                // Destructor

protected:
   Trace(                           // Constructor
     uint32_t          size);       // Size of trace area, including this

   Trace(const Trace&) = delete;    // Disallowed copy constructor
Trace& operator=(const Trace&) = delete; // Disallowed assignment operator

public:
// make() does NOT set the common Trace object. Set it separately.
static Trace*                       // The Trace object
   make(                            // Create a Trace object
     size_t            size);       // Of this length

// (Atomically) take the common Trace object, setting it to nullptr
static Trace*                       // The Trace object
   take( void );                    // Take the common Trace object

//----------------------------------------------------------------------------
// Trace::Methods
//----------------------------------------------------------------------------
public:
Record*                             // -> Trace record
   allocate(                        // Allocate a trace record
     uint32_t          size);       // Size of trace record

void
   dump( void ) const;              // Dump the trace table
}; // class Trace
}  // namespace _PUB_NAMESPACE
#endif // _PUB_TRACE_H_INCLUDED
