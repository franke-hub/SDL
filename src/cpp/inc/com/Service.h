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
//       Service.h
//
// Purpose-
//       Service controls.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef SERVICE_H_INCLUDED
#define SERVICE_H_INCLUDED

#include <stdint.h>

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define SERVICE_INFO(word) Service::debug(__LINE__, __FILE__, (unsigned)word)

//----------------------------------------------------------------------------
//
// Class-
//       Service
//
// Purpose-
//       Container
//
//----------------------------------------------------------------------------
class Service {
public:
//----------------------------------------------------------------------------
//
// Struct-
//       Service::Global
//
// Purpose-
//       Define the global area.
//
//----------------------------------------------------------------------------
struct Global {
//----------------------------------------------------------------------------
// Service::Global::Attributres
//----------------------------------------------------------------------------
char                   ident[8];    // "*GLOBAL" Identifier
uint32_t               vword;       // Validation word
uint32_t               latch;       // Trace latch
uint32_t               traceOffset; // Trace area offset
uint32_t               traceLength; // Trace area length

//----------------------------------------------------------------------------
// Service::Global::Enumerations and typedefs
//----------------------------------------------------------------------------
enum
{  VALIDATOR=          0xfe010002   // Dataword validator
}; // enum

//----------------------------------------------------------------------------
// Service::Global::Methods
//----------------------------------------------------------------------------
inline unsigned                     // Length of this area
   getLength( void ) const;         // Return length of area
}; // struct Service::Global

//----------------------------------------------------------------------------
// Service::Static attributes
//----------------------------------------------------------------------------
private:
static Global*         global;      // -> Global area

//----------------------------------------------------------------------------
// Service::Attributes
//----------------------------------------------------------------------------
   // None defined

//----------------------------------------------------------------------------
//
// Struct-
//       Service::Record
//
// Purpose-
//       Define base trace record
//
//----------------------------------------------------------------------------
public:
struct Record {
uint32_t               rid;         // Record identifier
uint16_t               pid;         // Process identifier
uint16_t               tid;         // Thread identifier
uint64_t               tod;         // Timestamp
}; // struct Service::Record

//----------------------------------------------------------------------------
//
// Struct-
//       Service::DebugRecord
//
// Purpose-
//       Define file/line trace record.
//
//----------------------------------------------------------------------------
struct DebugRecord : public Record {
   char                file[8];     // Source file name
   uint32_t            line;        // File line number
   uint32_t            data;        // Associated data word
}; // struct Service::DebugRecord

//----------------------------------------------------------------------------
//
// Struct-
//       Service::TraceRecord
//
// Purpose-
//       Define standard trace record.
//
//----------------------------------------------------------------------------
struct TraceRecord : public Record {
   void*               data[4];     // Associated data words
}; // struct Service::TraceRecord

//----------------------------------------------------------------------------
// Service::Constructors
//----------------------------------------------------------------------------
public:
   ~Service( void );                // Destructor
   Service( void );                 // Contructor

//----------------------------------------------------------------------------
// Service::Methods
//----------------------------------------------------------------------------
public:
inline static unsigned              // Length(Global)
   getLength( void );               // Get Global length

inline static int                   // TRUE if active
   isActive( void );                // Is Service active?

inline static uint32_t              // Resultant word
   word(                            // Convert string to word
     const char*       string);     // String (length >=4)

static void
   debug(                           // Debugging trace
     unsigned          line,        // Line number
     const char*       file,        // File name
     unsigned          data);       // Associated data word

static Record*                      // -> Record
   getRecord(                       // Allocate a Record
     const char*       type,        // Of this type
     unsigned          length);     // Of this length

static void
   info(                            // Return information
     Global*           global);     // -> Return area

static void
   reset( void );                   // Reset Service

static void
   start( void );                   // Start Service
}; // class Service

#include "Service.i"

#endif // SERVICE_H_INCLUDED
