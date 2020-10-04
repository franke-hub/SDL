//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Trace.cpp
//
// Purpose-
//       Trace object methods.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <com/define.h>
#include <com/Atomic.h>
#include <com/Debug.h>

#include "com/Trace.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define ALIGNMENT                32 // Default alignment

//----------------------------------------------------------------------------
//
// Method-
//       Trace::~Trace
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Trace::~Trace( void )            // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Trace::Trace
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Trace::Trace( void )             // Constructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Trace::Trace
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Trace::Trace(                    // Constructor
     Size              size)        // Size of trace area
:  next(0)
,  top(0)
,  bot(0)
,  size(size)
{
   uintptr_t           offset;      // Working offset
   uintptr_t           temp;        // Working value

   if( size < MinimumSize )         // If size is too small
     throw "ParameterValueException";

   offset= sizeof(Trace);           // Offset
   size -= sizeof(Trace);           // (Decrements size)
   temp= (uintptr_t)this;           // Trace area
   temp &= (ALIGNMENT - 1);         // Rounding factor
   if( temp > 0 )                   // If rounding required
   {
     temp= ALIGNMENT - temp;        // Number of wasted bytes
     offset += temp;
     size -= temp;
   }

   size &= ~(ALIGNMENT - 1);        // Exact number of records
   next= offset;
   top=  offset;
   bot=  offset + size;

   wrap[0]= 0;
   wrap[1]= 0;
   wrap[2]= 0;
   wrap[3]= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Trace::allocate
//
// Purpose-
//       Allocate a trace record.
//
//----------------------------------------------------------------------------
Trace::Record*                      // -> Trace record
   Trace::allocate(                 // Allocate a trace record
     Size              size)        // Size of trace record
{
   uint32_t*           oldRecord;   // -> Record
   Record*             ptrRecord;   // -> Record
   int                 wrapped;     // TRUE if wrapped

   uint32_t            oldV;        // Old value
   uint32_t            newV;        // New value

   int                 cc;
   int                 i;

   size += ALIGNMENT - 1;
   size &= ~(ALIGNMENT - 1);
   if ( size > this->size )
     return NULL;

   do
   {
     wrapped= FALSE;
     oldV= next;
     newV= oldV + size;
     ptrRecord= (Record*)((char*)this + oldV);
     if ( newV > bot )
     {
       wrapped= TRUE;
       ptrRecord= (Record*)((char*)this + top);
       newV= top + size;
     }

     cc= csw((ATOMIC32*)&next, oldV, newV);
   } while( cc != 0 );

   if( wrapped != 0 )
   {
     if( oldV < bot )
     {
       oldRecord= (uint32_t*)((char*)this + oldV);
       memset(oldRecord, 0, 32);
       *oldRecord= *((uint32_t*)(".END"));
     }

     for(i=3; i>=0; i--)
     {
       do
       {
         oldV= wrap[i];
         newV= oldV + 1;
         cc= csw((ATOMIC32*)&wrap[i], oldV, newV);
       } while( cc != 0 );

       if( newV != 0 )
         break;
     }
   }

   return ptrRecord;
}

//----------------------------------------------------------------------------
//
// Method-
//       Trace::dump
//
// Purpose-
//       Dump the trace table.
//
//----------------------------------------------------------------------------
void
   Trace::dump( void ) const        // Dump the trace table
{
   tracef("Trace(%p)::dump\n", this);
   tracef("..top(%8x) next(%.8x) bot(%.8x) size(%.8x)\n", top, next, bot, size);
   tracef("..wrao %.8x %.8x %.8x %.8x\n", wrap[0], wrap[1], wrap[2], wrap[3]);

   ::dumpv(this, size, nullptr);
}

