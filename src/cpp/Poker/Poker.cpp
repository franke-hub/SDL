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
//       Poker.cpp
//
// Purpose-
//       Poker simulation.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#include <time.h>

#include "Poker.h"
#include "ExtendedTexasStrategy.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define USE_TEST       TRUE         // TRUE for test PokerStrategy
#define USE_YOU        FALSE        // TRUE for human input

#if TRUE
  #define RANDOMIZE 12345
#else
  #define RANDOMIZE time(NULL)
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#ifdef HCDM
  #define ATLINE() printf("%4d HCDM Poker.cpp\n", __LINE__)
#else
  #define ATLINE() (void)0
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static TexasTable      table;       // The featured Table
static Deck            deck;        // The current Deck
static int             count;       // The number of cards in the Deck

static PokerPlayer     p0("Zero ", 1000);
static PokerPlayer     p1("One  ", 1000);
static PokerPlayer     p2("Two  ", 1000);
static PokerPlayer     p3("Three", 1000);
static PokerPlayer     p4("Four ", 1000);
static PokerPlayer     p5("Five ", 1000);
static PokerPlayer     p6("Six  ", 1000);
static PokerPlayer     test("Test ", 1000);
static PokerPlayer     you("*YOU*", 1000);

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize
//
//----------------------------------------------------------------------------
static void
   init( void )                     // Initialize
{
   int                 i;

   srand(RANDOMIZE);                // RANDOMIZE

   ATLINE();
   count= deck.getCount();
   for(i= 0; i < count; i++)
     deck.shuffle();

   ATLINE();
   you.setStrategy(new HumanPokerStrategy(&you, &table));
   test.setStrategy(new ExtendedTexasStrategy(&test, &table));

   #if USE_YOU
     table.addPlayer(&you);
   #else
     table.addPlayer(&p0);
   #endif

   table.addPlayer(&p1);
   table.addPlayer(&p2);
   table.addPlayer(&p3);
   table.addPlayer(&p4);
   table.addPlayer(&p5);

   #if USE_TEST
     table.addPlayer(&test);
   #else
     table.addPlayer(&p6);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       game
//
// Purpose-
//       Play Poker.
//
//----------------------------------------------------------------------------
static void
   game( void )                     // Play Poker
{
   int                 playerCount; // Current player count

   ATLINE();
   playerCount= table.getPlayerCount();
   while( playerCount > 1 )
   {
     ATLINE();
     table.play();

     printf("\n");
     ATLINE();
     table.display();

     if( playerCount > table.getPlayerCount() )
     {
       playerCount= table.getPlayerCount();
       table.setBigBlind(table.getBigBlind() + 10);
       table.setSmallBlind(table.getSmallBlind() + 5);

       printf("\n\n");
       printf("**REMAINING PLAYERS**\n");
       table.debug();
     }

     printf("--DONE--\n\n");
     fflush(stdout);
     #if USE_YOU
       char string[256]; // Input data area
       gets(string);
     #endif
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
int                                 // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   try {
     ATLINE();
     init();

     ATLINE();
     game();
   } catch(const char* X) {
     printf("EXCEPTION(%s)\n", X);
   } catch(...) {
     printf("EXCEPTION(...)\n");
   }

   return 0;
}

