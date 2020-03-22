//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
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
//       Trace object.
//
// Last change date-
//       2007/01/01
//
// Usage-
//       #include <new>;            // For in-place operator new
//       void* addr= malloc(desired_size);
//       Trace* trace= new(addr) Trace(desired_size);
//
//----------------------------------------------------------------------------
#ifndef TRACE_H_INCLUDED
#define TRACE_H_INCLUDED

#include <stdint.h>

//----------------------------------------------------------------------------
//
// Class-
//       Trace
//
// Purpose-
//       Trace object descriptor.
//
//----------------------------------------------------------------------------
class Trace {                       // Trace object
//----------------------------------------------------------------------------
// Trace::Attributes
//----------------------------------------------------------------------------
private:
uint32_t               next;        // Offset: Next trace entry
uint32_t               top;         // Offset: Trace table origin
uint32_t               bot;         // Offset: End of trace table
uint32_t               size;        // Available size of trace table
uint32_t               wrap[4];     // Wrap counter

//----------------------------------------------------------------------------
// Trace::Enumerations and Typedefs
//----------------------------------------------------------------------------
public:
enum                                // Generic enum
{  MinimumSize=               65536 // Minimum allowed size
}; // enum

typedef unsigned int Size;          // Size

//----------------------------------------------------------------------------
// Trace::Record
//----------------------------------------------------------------------------
public:
struct Record                       // A trace record
{
}; // struct Trace::Record

//----------------------------------------------------------------------------
// Trace::Constructors
//----------------------------------------------------------------------------
public:
   ~Trace( void );                  // Destructor
   Trace( void );                   // Default constructor

   Trace(                           // Constructor
     Size              size);       // Size of trace area, including this

private:                            // Bitwise copy is prohibited
   Trace(const Trace&);             // Disallowed copy constructor
Trace&
   operator=(const Trace&);         // Disallowed assignment operator

//----------------------------------------------------------------------------
// Trace::Methods
//----------------------------------------------------------------------------
public:
Record*                             // -> Trace record
   allocate(                        // Allocate a trace record
     Size              size);       // Size of trace record

void
   dump( void ) const;              // Dump the trace table
}; // class Trace

#endif // TRACE_H_INCLUDED
