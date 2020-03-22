//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DbTime.cpp
//
// Purpose-
//       Implement DbTime object methods
//
// Last change date-
//       2018/01/01 DB4/5 compatibility
//
// Implementation notes-
//       TODO: NOT CODED YET. (All methods scaffolded)
//
//----------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Barrier.h>
#include <com/Debug.h>
#include <com/Julian.h>

#include "DbInfo.h"
#include "DbTime.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define BRINGUP_INDEX 0x10fe02fe03fe04feLL

//----------------------------------------------------------------------------
//
// Method-
//       DbTime::~DbTime
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DbTime::~DbTime( void )          // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       DbTime::DbTime
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DbTime::DbTime( void )           // Constructor
:  DbBase()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       DbTime::insert
//
// Purpose-
//       Insert item into database.
//
//----------------------------------------------------------------------------
uint64_t                            // The index (0 if error)
   DbTime::insert(                  // Insert
     uint64_t          value,       // Expiration time (Julian second)
     DbInfo*           assoc,       // Associated information
     DbTxn*            parent)      // Parent transaction
{
   return BRINGUP_INDEX;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbTime::locate
//
// Purpose-
//       Locate database item.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 if OK)
   DbTime::locate(                  // Locate
     uint64_t          index,       // This index
     DbInfo*           assoc,       // (OUT) Associated DbInfo (May be NULL)
     uint64_t*         value)       // (OUT) Expiration time (Julian second)
{
   int result= (-1);
   if( index == BRINGUP_INDEX )
   {
     *value= (int64_t)(Julian::current() + 120.0);
     result= 0;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbTime::nextIndex
//
// Purpose-
//       Retrieve next data base item, by index.
//
//----------------------------------------------------------------------------
uint64_t                            // Result (0 if error/missing/last)
   DbTime::nextIndex(               // Get next index
     uint64_t          index)       // After this index
{
   uint64_t result= 0;
   if( index == 0 )
     result= BRINGUP_INDEX;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbTime::nextValue
//
// Purpose-
//       Retrieve next data base item, by value.
//
//----------------------------------------------------------------------------
uint64_t                            // Result (0 if error/missing/last)
   DbTime::nextValue(               // Get next value index
     uint64_t          index,       // After this index
     DbInfo*           assoc,       // (OUT) Associated DbInfo (May be NULL)
     uint64_t*         value)       // (OUT) value (Julian second)
{
   uint64_t result= 0;
   if( index == 0 )
   {
     result= BRINGUP_INDEX;
     *value= (int64_t)(Julian::current() + 120.0);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbTime::remove
//
// Purpose-
//       Remove data base item.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 if OK)
   DbTime::remove(                  // Remove
     uint64_t          index)       // This index
{
   int result= (-1);
   if( index == BRINGUP_INDEX )
     result= 0;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbTime::replace
//
// Purpose-
//       Replace database item.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 if OK)
   DbTime::replace(                 // Locate
     uint64_t          index,       // This index
     DbInfo*           assoc,       // (OUT) Associated DbInfo (May be NULL)
     uint64_t*         value)       // (OUT) Expiration time (Julian second)
{
   int result= (-1);
   if( index == BRINGUP_INDEX )
     result= 0;

   return result;
}

