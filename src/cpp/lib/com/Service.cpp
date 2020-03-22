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
//       Service.cpp
//
// Purpose-
//       Service object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Barrier.h>
#include <com/Debug.h>
#include <com/Exception.h>
#include <com/SharedMem.h>
#include <com/sysmac.h>
#include <com/Trace.h>
#include <com/Software.h>
#include <com/Hardware.h>

#include "com/Service.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define SERVICE_PAGE_COUNT      128 // Number of trace pages
#define SERVICE_PAGE_SIZE      4096 // Size of a page
#define MEMTOKEN         0xfe010320 // SharedMem segment token

//----------------------------------------------------------------------------
//
// Struct-
//       GlobalArea
//
// Purpose-
//       Define the global area.
//
//----------------------------------------------------------------------------
struct GlobalArea : public Service::Global
{
   char                padding[SERVICE_PAGE_SIZE-sizeof(Service::Global)];
   char                traceArea[SERVICE_PAGE_SIZE*SERVICE_PAGE_COUNT];
}; // struct GlobalArea

//----------------------------------------------------------------------------
//
// Struct-
//       LocalRecord
//
// Purpose-
//       Emergency trace area.
//
//----------------------------------------------------------------------------
struct LocalRecord : public Service::Record
{
   int                 padding[20]; // Padding
}; // struct LocalRecord

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static Barrier         barrier= BARRIER_INIT; // Thread latch
static unsigned        useCount= 0; // Reference counter
static SharedMem*      shared= NULL;// Shared memory accessor
static LocalRecord     localRecord; // Local trace record
static Trace*          trace= NULL; // Global trace pointer

//----------------------------------------------------------------------------
// Global data areas
//----------------------------------------------------------------------------
Service::Global*       Service::global= NULL;

//----------------------------------------------------------------------------
//
// Subroutine-
//       initglobal
//
// Purpose-
//       Initialize a Global.
//
//----------------------------------------------------------------------------
static Service::Global*             // Resultant
   initGlobal(                      // Initialize a Global
     Service::Global*  global,      // -> Global area
     unsigned          pages)       // Number of pages in trace area

{
   unsigned            traceOffset; // Trace offset
   unsigned            traceLength; // Trace length

   traceOffset= sizeof(Service::Global) + (SERVICE_PAGE_SIZE) - 1;
   traceOffset &= -(SERVICE_PAGE_SIZE);
   traceLength=  pages * (SERVICE_PAGE_SIZE);

   memset(global, 0, traceOffset + traceLength);

   strcpy(global->ident, "*GLOBAL");// Set the identifier
   global->traceOffset= traceOffset;// Set the trace offset
   global->traceLength= traceLength;// Set the trace length

   trace= (Trace*)((char*)global + traceOffset);
   new(trace) Trace(traceLength);

   global->vword= Service::Global::VALIDATOR;
   return global;
}

//----------------------------------------------------------------------------
//
// Method-
//       Service::~Service
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Service::~Service( void )        // Destructor
{
   #ifdef HCDM
     debugf("Service::~Service() useCount(%d)\n", useCount);
   #endif

   AutoBarrier lock(barrier);
   {
     useCount--;
     if( useCount == 0 )
     {
       trace=  NULL;
       global= NULL;
       if( shared != NULL )
       {
         delete shared;
         shared= NULL;
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Service::Service
//
// Purpose-
//       Contructor.
//
//----------------------------------------------------------------------------
   Service::Service( void )         // Contructor
{
   GlobalArea*         zero= NULL;

   #ifdef HCDM
     debugf("Service::Service() useCount(%d)\n", useCount);
   #endif

   AutoBarrier lock(barrier);
   {
     useCount++;

     if( global == NULL )
     {
       try {
         shared= new SharedMem(sizeof(GlobalArea),
                            SharedMem::getToken(MEMTOKEN),
                            SharedMem::Keep | SharedMem::Write);
         global= (Global*)shared->getAddress();
//////// trace= (Trace*)((char*)global + offsetof(GlobalArea, traceArea[0]));
         trace= (Trace*)((char*)global +
                        (size_t)&zero->traceArea[0]
                        );
         useCount++;

       } catch(...) {
         #ifdef HCDM
           debugf("Service not active\n");
         #endif
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Service::debug
//
// Purpose-
//       Format a DebugRecord
//
//----------------------------------------------------------------------------
void
   Service::debug(                  // Debugging trace
     unsigned          line,        // Line number
     const char*       file,        // File name
     unsigned          data)        // Associated data word
{
   DebugRecord*        record;

   #if defined(HCDM)
     record= (DebugRecord*)&localRecord;
     if( trace != NULL )
       record= (DebugRecord*)trace->allocate(sizeof(DebugRecord));

     memset(record, 0xff, sizeof(DebugRecord));
     record->rid= Service::word("0BUG");// Initialize the trace entry
     memcpy(record->file, file, sizeof(record->file));
     record->line= line;
     record->data= data;

     record->pid= Software::getPid();
     record->tid= Software::getTid();
     record->tod= Hardware::getTSC();
     record->rid= Service::word(".BUG");

   #else
     record= (DebugRecord*)getRecord(".BUG", sizeof(DebugRecord));
     memcpy(record->file, file, sizeof(record->file));
     record->line= line;
     record->data= data;
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Service::getRecord
//
// Purpose-
//       Allocate a Record.
//
//----------------------------------------------------------------------------
Service::Record*                    // -> Record
   Service::getRecord(              // Allocate a Record
     const char*       type,        // Of this type
     unsigned          length)      // Of this length
{
   Record*             result;      // Resultant

   result= &localRecord;
   if( trace != NULL )
     result= (Record*)trace->allocate(length);

   result->rid= Service::word(type);// Initialize the trace entry
   result->pid= Software::getPid();
   result->tid= Software::getTid();
   result->tod= Hardware::getTSC();
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Service::info
//
// Purpose-
//       Return a copy of QbrFile::Global
//
//----------------------------------------------------------------------------
void                                // Resultant
   Service::info(                   // Return information
     Global*           target)      // -> Return area
{
   unsigned            length= getLength();

   #ifdef HCDM
     debugf("Service::info(%p) %s\n", target, isActive() ? "ACTIVE" : "IDLE");
   #endif

   AutoBarrier lock(barrier);
   {
     if( isActive() )
     {
       memcpy(target, global, length); // (This pages in all pages)
       memcpy(target, global, length); // (Less likely to page fault)
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Service::reset
//
// Purpose-
//       Reset (terminate) Service
//
//----------------------------------------------------------------------------
void
   Service::reset( void )           // Reset Service
{
   Global*             global= Service::global;

   #ifdef HCDM
     debugf("Service::reset()\n");
   #endif

   AutoBarrier lock(barrier);
   {
     if( isActive() )
     {
       SERVICE_INFO(Service::word("TERM")); // Put entry in trace

       trace= NULL;                 // Terminate trace
       Service::global= NULL;       // Terminate Service
       SharedMem::detach(global);   // Detach the global area
       SharedMem::remove(SharedMem::getToken(MEMTOKEN));
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Service::start
//
// Purpose-
//       Start Service
//
//----------------------------------------------------------------------------
void
   Service::start( void )           // Start Service
{
   Global*             global;      // -> Global

   #ifdef HCDM
     debugf("Service::start()\n");
   #endif

   AutoBarrier lock(barrier);
   {
     if( !isActive() )
     {
       try {
         shared= new SharedMem(sizeof(GlobalArea),
                            SharedMem::getToken(MEMTOKEN),
                            SharedMem::Create | SharedMem::Exclusive |
                            SharedMem::Keep   | SharedMem::Write);
         global= (Global*)shared->getAddress();
         Service::global= initGlobal(global, SERVICE_PAGE_COUNT);
         useCount++;

       } catch(Exception e) {
         fprintf(stderr, "Failed(%s)\n", e.what());

       } catch(char* c) {
         fprintf(stderr, "Failed(%s)\n", c);

       } catch(...) {
         fprintf(stderr, "Failed\n");
       }

       SERVICE_INFO(Service::word("INIT")); // Put entry in trace
     }
     else
       fprintf(stderr, "Already active\n");
   }
}

