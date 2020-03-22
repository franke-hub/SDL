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
//       ExtendedTexasStrategy.cpp
//
// Purpose-
//       Extend TexasStrategy, used for testing.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <ctype.h>
#include <errno.h>

#include "Define.h"
#include "Player.h"
#include "Table.h"
#include "Utility.h"

#include "Strategy.h"
#include "ExtendedTexasStrategy.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define UNTESTED 0                  // Logic is untested
#define _INTEST_ 1                  // Logic is undergoing test

#define ACCEPTED 1                  // Logic is accepted
#define REJECTED 0                  // Logic is rejected

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#ifdef __DEBUG__
   #define ATLINE() printf("{%4d} ", __LINE__)
#else
   #define ATLINE() (void)0
#endif

//----------------------------------------------------------------------------
//
// Method-
//       ExtendedTexasStrategy::~ExtendedTexasStrategy
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   ExtendedTexasStrategy::~ExtendedTexasStrategy( void ) // Destructor
{
   #ifdef HCDM
     printf("ExtendedTexasStrategy(%p)::~ExtendedTexasStrategy()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       ExtendedTexasStrategy::ExtendedTexasStrategy
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   ExtendedTexasStrategy::ExtendedTexasStrategy( // Constructor
     PokerPlayer*      player,      // The PokerPlayer
     PokerTable*       table)       // The PokerTable
:  TexasStrategy(player, table, MODEL_DEFAULT)
{
   #ifdef HCDM
     printf("ExtendedTexasStrategy(%p)::ExtendedTexasStrategy()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       ExtendedTexasStrategy::bet
//
// Purpose-
//       Make a bet.
//
//----------------------------------------------------------------------------
int                                 // Actual bet
   ExtendedTexasStrategy::bet(      // Make a bet
     int               amount)      // Minimum to call
{
   #ifdef HCDM
     printf("ExtendedTexasStrategy(%p)::bet(%d)\n", this, amount);
   #endif

   return TexasStrategy::bet(amount);
}

//----------------------------------------------------------------------------
//
// Method-
//       ExtendedTexasStrategy::rate
//
// Purpose-
//       Rate the hand.
//
//----------------------------------------------------------------------------
void
   ExtendedTexasStrategy::rate( void ) // Rate the hand
{
   #ifdef HCDM
     printf("%4d ExtendedTexasStrategy::rate()\n", __LINE__);
   #endif

   //-------------------------------------------------------------------------
   // Rate the hand
   //-------------------------------------------------------------------------
   TexasStrategy::rate();
}

//----------------------------------------------------------------------------
//
// Method-
//       ExtendedTexasStrategy::reset
//
// Purpose-
//       Reset the ExtendedTexasStrategy.
//
//----------------------------------------------------------------------------
void
   ExtendedTexasStrategy::reset( void ) // Reset the ExtendedTexasStrategy
{
   #ifdef HCDM
     printf("ExtendedTexasStrategy(%p)::reset()\n", this);
   #endif

   TexasStrategy::reset();
}

