//----------------------------------------------------------------------------
//
//       Copyright (C) 2017 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       History.cpp
//
// Purpose-
//       History implementation.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#include "Define.h"
#include "History.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Subroutine-
//       initIntArray
//
// Purpose-
//       Initialize an integer array.
//
//----------------------------------------------------------------------------
static int*                         // The resultant array
   initIntArray(                    // Initialize an integer array
     int               count)       // Of this many elements
{
   int*                result= (int*)malloc(count*sizeof(int));

   int                 i;

   if( result == NULL )
     throw "NoStorageException";

   for(i= 0; i<count; i++)
     result[i]= 0;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerHistory::~PokerHistory
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   PokerHistory::~PokerHistory( void ) // Destructor
{
   #ifdef HCDM
     printf("PokerHistory(%p)::~PokerHistory()\n", this);
   #endif

   free(checkCount);
   free(callCount);
   free(callAmount);
   free(raiseCount);
   free(raiseAmount);
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerHistory::PokerHistory
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   PokerHistory::PokerHistory( void ) // Default constructor
:  count(DEFAULT_SIZE)
,  used(0)
,  checkCount(NULL)
,  callCount(NULL)
,  callAmount(NULL)
,  raiseCount(NULL)
{
   #ifdef HCDM
     printf("PokerHistory(%p)::PokerHistory()\n", this);
   #endif

   checkCount= initIntArray(count);
   callCount= initIntArray(count);
   callAmount= initIntArray(count);
   raiseCount= initIntArray(count);
   raiseAmount= initIntArray(count);
}

   PokerHistory::PokerHistory(      // Constructor
     int               count)       // Number of array elements
:  count(count)
,  used(0)
,  checkCount(NULL)
,  callCount(NULL)
,  callAmount(NULL)
,  raiseCount(NULL)
{
   #ifdef HCDM
     printf("PokerHistory(%p)::PokerHistory(%d)\n", this, count);
   #endif

   checkCount= initIntArray(count);
   callCount= initIntArray(count);
   callAmount= initIntArray(count);
   raiseCount= initIntArray(count);
   raiseAmount= initIntArray(count);
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerPlayerHistory::~PokerPlayerHistory
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   PokerPlayerHistory::~PokerPlayerHistory( void ) // Destructor
{
   #ifdef HCDM
     printf("PokerPlayerHistory(%p)::~PokerPlayerHistory()\n", this);
   #endif

   free(result);
   free(rating);
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerPlayerHistory::PokerPlayerHistory
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   PokerPlayerHistory::PokerPlayerHistory( void ) // Default constructor
:  PokerHistory()
,  result(NULL)
,  rating(NULL)
{
   #ifdef HCDM
     printf("PokerPlayerHistory(%p)::PokerPlayerHistory()\n", this);
   #endif

   result= (PokerResult*)malloc(count*sizeof(PokerResult));
   rating= (double*)malloc(count*sizeof(double));
   if( result == NULL || rating == NULL )
     throw "NoStorageException";
}

   PokerPlayerHistory::PokerPlayerHistory( // Constructor
     int               count)       // Number of array elements
:  PokerHistory(count)
,  result(NULL)
,  rating(NULL)
{
   #ifdef HCDM
     printf("PokerPlayerHistory(%p)::PokerPlayerHistory(%d)\n", this, count);
   #endif

   result= (PokerResult*)malloc(count*sizeof(PokerResult));
   rating= (double*)malloc(count*sizeof(double));
   if( result == NULL || rating == NULL )
     throw "NoStorageException";
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerHistory::create
//
// Purpose-
//       Create a new element
//
//----------------------------------------------------------------------------
void
   PokerHistory::create( void )     // Create a new element
{
   int                 i;

   if( used == count )
   {
     used--;
     for(i= 0; i<used; i++)
     {
       checkCount[i]= checkCount[i+1];
       callCount[i]= callCount[i+1];
       callAmount[i]= callAmount[i+1];
       raiseCount[i]= raiseCount[i+1];
       raiseAmount[i]= raiseAmount[i+1];
     }
   }

   checkCount[used]= 0;
   callCount[used]= 0;
   callAmount[used]= 0;
   raiseCount[used]= 0;
   raiseAmount[used]= 0;
   used++;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerPlayerHistory::create
//
// Purpose-
//       Create a new element
//
//----------------------------------------------------------------------------
void
   PokerPlayerHistory::create( void ) // Create a new element
{
   int                 i;

   PokerHistory::create();

   if( used == count )
   {
     used--;
     for(i= 0; i<used; i++)
     {
       result[i]= result[i+1];
       rating[i]= rating[i+1];
     }
   }

   result[used]= RESULT_FOLD;
   rating[used]= 0.0;
   used++;
}

