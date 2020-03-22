//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestPrisoner.cpp
//
// Purpose-
//       Sample Darwin group: Prisoner's dilemma.
//       (Scientic American, July, 1992)
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <com/Bit.h>
#include <com/Debug.h>
#include <com/define.h>
#include <com/Interval.h>
#include <com/params.h>
#include <com/Random.h>
#include <com/syslib.h>

#include "Prison.h"
#include "Prisoner.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "TestPris" // Source file name

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM                        // If defined, hard-core debug mode
#undef  HCDM                        // If defined, hard-core debug mode
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Interval        timer;       // Interval timer

static Prison          prison(Prisoner::PrisonerCount); // The prison
static Prisoner        prisoner[Prisoner::PrisonerCount]; // The prisoners

//----------------------------------------------------------------------------
// Constant data areas
//----------------------------------------------------------------------------
static char          hextab[]= "0123456789ABCDEF";

//----------------------------------------------------------------------------
//
// Subroutine-
//       byteToString
//
// Function-
//       Convert a byte to a bit string
//
//----------------------------------------------------------------------------
static char*                        // Resultant
   byteToBitstring(                 // Convert byte to Bitstring
     char*             resultant,   // Resultant string
     int               byte)        // The byte to convert
{
   int                 outx= 0;     // The resultant index
   int                 m;           // Bit mask
   int                 i;

   m= 0x80;
   for(i=0; i<8; i++)
   {
     if( (byte&m) == 0 )
       resultant[outx++]= '0';
     else
       resultant[outx++]= '1';

     m >>= 1;
   }

   resultant[outx]= '\0';

   return resultant;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugInfo
//
// Function-
//       Show the rules.
//
//----------------------------------------------------------------------------
extern void
   debugInfo( void )                // Debugging information
{
#if 0  // Debugging info
   int                 i;

#if 0  // Show the rule addresses
   for(i=0; i<8; i++)
   {
     debugf("[%.2d] rule(%p)\n", i, prisoner[i].rule);
   }
#endif // Show the rule addresses

   // The initial rule should be pseudo-random
   prisoner[0].showRule();

   // Rule[0]= 0's, Rule[1]= 1's
   for(i=0; i<32 /* sizeof(Prisoner::rule) */; i++)
   {
     prisoner[0].rule[i]= 0x00;
     prisoner[1].rule[i]= 0xff;
   }

   // All zeros + All ones
   debugf("\n");
   prisoner[2].evolve(&prisoner[0], &prisoner[0]);
   debugf("[00] ", 0); prisoner[2].showRule();
   prisoner[2].evolve(&prisoner[1], &prisoner[1]);
   debugf("[ff] ", 1); prisoner[2].showRule();

   // Random: pop(1's) mom(0's)
   debugf("\n");
   prisoner[2].evolve(&prisoner[0], &prisoner[1]);
   prisoner[3].evolve(&prisoner[0], &prisoner[1]);
   prisoner[4].evolve(&prisoner[0], &prisoner[1]);
   prisoner[5].evolve(&prisoner[0], &prisoner[1]);
   prisoner[6].evolve(&prisoner[0], &prisoner[1]);
   prisoner[7].evolve(&prisoner[0], &prisoner[1]);

   for(i=0; i<8; i++)
   {
     debugf("[%.2d] ", i); prisoner[i].showRule();
   }

   // Random: pop(0's) mom(1's)
   debugf("\n");
   prisoner[2].evolve(&prisoner[1], &prisoner[0]);
   prisoner[3].evolve(&prisoner[1], &prisoner[0]);
   prisoner[4].evolve(&prisoner[1], &prisoner[0]);
   prisoner[5].evolve(&prisoner[1], &prisoner[0]);
   prisoner[6].evolve(&prisoner[1], &prisoner[0]);
   prisoner[7].evolve(&prisoner[1], &prisoner[0]);

   for(i=0; i<8; i++)
   {
     debugf("[%.2d] ", i); prisoner[i].showRule();
   }

   // Mutations
   debugf("\n");
   debugf("Mutate:\n");
   for(i=0; i<8; i++)
   {
     prisoner[0].mutate();
     debugf("[%.2d] ", 0); prisoner[0].showRule();
   }

   debugf("\n");
   for(i=0; i<8; i++)
   {
     prisoner[1].mutate();
     debugf("[%.2d] ", 1); prisoner[1].showRule();
   }

   exit(EXIT_SUCCESS);
#endif // Debugging info
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       display
//
// Function-
//       Show the rules.
//
//----------------------------------------------------------------------------
static void
   display(                         // Show the rules
     Prison::EvolveRc  evolveRc)    // Reason for display
{
   char                string[128]; // Temporary
   Prisoner*           ptrPrisoner; // -> Prisoner
   Prisoner*           ptrHighRank; // -> Highest ranked Prisoner
   DarwinUnit*         ptrUnit;     // -> Unit

   const char*         ptrC;        // -> char
   int                 os, is;      // Zeros and ones
   int                 refs;        // Number of references

   int                 i, j, k;

   // Common information
   ptrC= "unknown";
   switch(evolveRc)
   {
     case Prison::EvolveComplete:
       ptrC= "Complete";
       switch(prison.completionReason)
       {
         case Prison::AllSameRank:
           ptrC= "Same Rank";
           break;

         case Prison::AllSameRule:
           ptrC= "Same Rule";
           break;

         case Prison::AllMutants:
           ptrC= "All Mutants";
           break;

         case Prison::NoNewUnits:
           ptrC= "No Changes";
           break;

         default:
           break;
       }
       break;

     case Prison::EvolveInfinite:
       ptrC= "Infinite";
       break;

     case Prison::EvolveLoopout:
       ptrC= "Loopout";
       break;

     case Prison::EvolveTimeout:
       ptrC= "Timeout";
       break;

     default:
       break;
   }

   debugf("%12s evolveReason()\n", ptrC);
   debugf("%12lu generations\n",   (long)prison.getGeneration() );
   debugf("%12lu mutations\n",     (long)prison.getMutation() );
   debugf("\n");

   // Show the rules
   for(j=0; j<Prisoner::PrisonerCount; j++)
   {
     ptrUnit= prison.getUnit(j);
     ptrPrisoner= (Prisoner*)(ptrUnit->castConcrete());
     assert(ptrUnit == ptrPrisoner);
     debugf("[%2d] ", j);

     if( ptrUnit->changed || ptrUnit->evolChange )
       debugf("C");
     else
       debugf("*");

     if( ptrUnit->mutated || ptrUnit->evolMutate )
       debugf("M");
     else
       debugf("*");

     debugf(" ");
     ptrPrisoner->showRule();
   }

   // Show the states
   for(j=0; j<Prisoner::PrisonerCount; j++)
   {
     for(i=0; i<Prisoner::PrisonerCount; i++)
     {
       ptrPrisoner= (Prisoner*)(prison.getUnit(i)->castConcrete());
       if( ptrPrisoner->cellNumber == j )
         break;
     }

     debugf("[%2d]=[%2d] ", ptrPrisoner->cellNumber, i);
     debugf("E(%4ld) H[", ptrPrisoner->evaluation);

     debugf("%.2X", ptrPrisoner->historyArray[0]);
     for(i=1; i<Prisoner::PrisonerCount; i++)
     {
       debugf(".%.2X", ptrPrisoner->historyArray[i]);
     }
     debugf("]\n");
   }

   // Show the history
   ptrHighRank= (Prisoner*)((prison.getUnit(0))->castConcrete());
   for(i=0; i<256; i++)
   {
     os= 0;
     is= 0;
     for(j=0; j<Prisoner::PrisonerCount/2; j++)
     {
       ptrPrisoner= (Prisoner*)((prison.getUnit(j))->castConcrete());
       if( Bit::get(ptrPrisoner->rule,i) == 0 )
         os++;
       else
         is++;
     }

     refs= 0;
     for(j=0; j<Prisoner::PrisonerCount; j++)
     {
       ptrPrisoner= (Prisoner*)((prison.getUnit(j))->castConcrete());
       for(k=0; k<Prisoner::PrisonerCount; k++)
       {
         if( ptrPrisoner->cellNumber != k
             && ptrPrisoner->historyArray[k] == i )
           refs++;
       }
     }

     debugf("[%.2x] (%4d,%10lu) [%s] ",
            i, refs, prison.historyArray[i], byteToBitstring(string, i));
     debugf("%c ", hextab[Bit::get(ptrHighRank->rule, i)]);

     if( is == 0 )
       debugf("0              ");
     else if( is < Prisoner::PrisonerCount/16 )
       debugf("0 (mostly)     ");
     else if( os == 0 )
       debugf("1              ");
     else if( os < Prisoner::PrisonerCount/16 )
       debugf("1 (mostly)     ");
     else
       debugf("* 0(%2d) 1(%2d)  ", os, is);


     debugf("00[%.2x] ", Prisoner::history(i,
                                           Prisoner::Choice(0),
                                           Prisoner::Choice(0)));

     debugf("01[%.2x] ", Prisoner::history(i,
                                           Prisoner::Choice(0),
                                           Prisoner::Choice(1)));

     debugf("10[%.2x] ", Prisoner::history(i,
                                           Prisoner::Choice(1),
                                           Prisoner::Choice(0)));

     debugf("11[%.2x] ", Prisoner::history(i,
                                           Prisoner::Choice(1),
                                           Prisoner::Choice(1)));

     debugf("\n");
   }

#if 0
   // Last born generation
   for(i=0; i<Prisoner::PrisonerCount; i++)
   {
     ptrPrisoner= (Prisoner*)((prison.getUnit(i))->castConcrete());
     debugf("[%2d] b(%5d)\n", i, ptrPrisoner->generation);
   }
#endif

   debugf("\n\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Display usage information.
//
//----------------------------------------------------------------------------
static void
   info(void)                       // Display usage information
{
   fprintf(stderr, "Parameters:\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "-cull:\n");
   fprintf(stderr, "  Culls (percentage per generation).\n");
   fprintf(stderr, "-mutate:\n");
   fprintf(stderr, "  Mutations (percentage).\n");
   fprintf(stderr, "-test:\n");
   fprintf(stderr, "  Tests (percentage per generation).\n");

   fprintf(stderr, "\n");
   fprintf(stderr, "-g:\n");
   fprintf(stderr, "  The number of generations to simulate.\n");
   fprintf(stderr, "-gmax:\n");
   fprintf(stderr, "  The maximum number of generations to simulate.\n");
   fprintf(stderr, "-gmin:\n");
   fprintf(stderr, "  The minimum number of generations to simulate.\n");

   fprintf(stderr, "\n");
   fprintf(stderr, "-checkChange\n");
   fprintf(stderr, "  Check for no change in rank.\n");
   fprintf(stderr, "-checkMutate\n");
   fprintf(stderr, "  Check for all units mutated.\n");
   fprintf(stderr, "-checkRank\n");
   fprintf(stderr, "  Check for all units same rank.\n");
   fprintf(stderr, "-checkRule\n");
   fprintf(stderr, "  Check for all units same rule.\n");

   fprintf(stderr, "\n");
   fprintf(stderr, "-forgetNew\n");
   fprintf(stderr, "  Forget new units.\n");
   fprintf(stderr, "-newForget\n");
   fprintf(stderr, "  New units forget.\n");

   fprintf(stderr, "\n");
   fprintf(stderr, "-verify\n");
   fprintf(stderr, "  Verify parameters.\n");

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Segment-
//       parm
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis routine
     int               const argc,  // Argument count
     char*             const argv[])// Argument array
{
   char*               argp;        // Argument pointer
   int                 argi;        // Argument index

   int                 error;       // Error encountered indicator
   int                 verify;      // Verification control

   double              temp;

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   error=  FALSE;                   // Default, no error
   verify= TRUE;                    // Default, verification

   prison.minGeneration= 10000;     // Minimum number of generations
   prison.maxGeneration= 10000;     // Maximum Number of generations

   prison.probCull= 0.250;
   prison.probMute= 0.001;
   prison.probTest= 0.250;

   prison.checkChange= FALSE;
   prison.checkMutate= FALSE;
   prison.checkRank=   TRUE;
   prison.checkRule=   FALSE;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   if (argc > 1 && *argv[1] == '?') // If query request
   {
     info();                        // Display options
     exit(EXIT_FAILURE);            // And exit, function complete
   }

   for (argi=1; argi<argc; argi++)  // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if (*argp == '-')              // If this parameter is in switch format
     {
       argp++;                      // Skip over the switch char

       if (swname("verify", argp))
         verify= swatob("verify", argp);

       else if (swname("cull:", argp))
       {
         temp= swatod("cull:", argp);
         prison.probCull= temp/100.0;
       }

       else if (swname("mutate:", argp))
       {
         temp= swatod("mutate:", argp);
         prison.probMute= temp/100.0;
       }

       else if (swname("test:", argp))
       {
         temp= swatod("test:", argp);
         prison.probTest= temp/100.0;
       }

       else if (swname("g:", argp))
       {
         prison.minGeneration= swatol("g:", argp);
         prison.maxGeneration= swatol("g:", argp);
       }

       else if (swname("gmax:", argp))
         prison.maxGeneration= swatol("gmax:", argp);

       else if (swname("gmin:", argp))
         prison.minGeneration= swatol("gmin:", argp);

       else if (swname("checkChange", argp))
         prison.checkChange= swatob("checkChange", argp);

       else if (swname("checkMutate", argp))
         prison.checkMutate= swatob("checkMutate", argp);

       else if (swname("checkRank", argp))
         prison.checkRank= swatob("checkRank", argp);

       else if (swname("checkRule", argp))
         prison.checkRule= swatob("checkRule", argp);

       else if (swname("forgetNew", argp))
         Prisoner::forgetNewUnits= swatob("forgetNew", argp);

       else if (swname("newForget", argp))
         Prisoner::newUnitsForget= swatob("newForget", argp);

       else                         // If invalid switch
       {
         error= TRUE;
         fprintf(stderr, "Invalid parameter '%s'\n",
                         argv[argi]);
       }
     }
     else                           // If not a switch parameter
     {
       {
         error= TRUE;
         fprintf(stderr, "Unknown parameter: '%s'\n", argp);
       }
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( error )                      // If error encountered
     info();

   if (verify)                      // If verification required
   {
     debugf("  %10lu -gmin\n", prison.minGeneration );
     debugf("  %10lu -gmax\n", prison.maxGeneration );
     debugf("%12.2f -cull   (percent)\n", prison.probCull*100.0 );
     debugf("%12.3f -mutate (percent)\n", prison.probMute*100.0 );
     debugf("%12.2f -test   (percent)\n", prison.probTest*100.0 );
     debugf("\n");
     debugf("  %10u -checkChange\n", prison.checkChange);
     debugf("  %10u -checkMutate\n", prison.checkMutate);
     debugf("  %10u -checkRank\n",   prison.checkRank);
     debugf("  %10u -checkRule\n",   prison.checkRule);
     debugf("\n");
     debugf("  %10u -forgetNew\n", Prisoner::forgetNewUnits);
     debugf("  %10u -newForget\n", Prisoner::newUnitsForget);
     debugf("\n\n");
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Function-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Environmental selection
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   Prison::EvolveRc    evolveRc;    // Prison::evolve() return code
   Interval            interval;

   int                 i;

   // Initialize
   parm(argc, argv);

   for(i=0; i<Prisoner::PrisonerCount; i++)
     prisoner[i].setPrison(&prison);

   debugInfo();

   // Timed run
   prison.setEvolveTimer(30.0);
   prison.setEvolveCount(prison.maxGeneration);

   interval.start();
   evolveRc= prison.evolveContinuous();
   interval.stop();

   debugf("%12.3f seconds\n", interval.toDouble());
   display(evolveRc);

   // Timing run (the interval continues)
   prison.setEvolveTimer(300.0);
   while( evolveRc == Prison::EvolveTimeout )
   {
     prison.setEvolveCount(prison.maxGeneration - prison.getGeneration());
     evolveRc= prison.evolveContinuous();
     interval.stop();

     debugf("%12.3f seconds\n", interval.toDouble());
     display(evolveRc);
   }

   // Return to caller
   return 0;
}

