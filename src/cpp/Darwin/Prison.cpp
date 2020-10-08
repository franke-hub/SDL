//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Prison.cpp
//
// Purpose-
//       Sample Darwin group: Prisoner's dilemma.
//       (Scientic American, July, 1992)
//
// Last change date-
//       2020/10/04
//
//----------------------------------------------------------------------------
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/define.h>
#include <com/Interval.h>
#include <com/Random.h>
#include <com/syslib.h>

#include "Prison.h"
#include "Prisoner.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "Prison  " // Source file name

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM                        // If defined, hard-core debug mode
#undef  HCDM                        // If defined, hard-core debug mode
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prison::Prison
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Prison::Prison(                  // Constructor
     unsigned int    elements)      // The number of group elements
:  DarwinPlex(elements)
,  minGeneration(0)
,  maxGeneration(ULONG_MAX)
,  completionReason(NotComplete)
,  checkRank(FALSE)
,  checkRule(FALSE)
,  checkChange(FALSE)
,  checkMutate(FALSE)
,  someNormal(FALSE)
,  stopOnCount(FALSE)
,  stopOnTimer(FALSE)
{
#ifdef HCDM
   debugf("Prison(%p)::Prison()\n", this);
#endif

   for(int i=0; i<256; i++)
     historyArray[i]= 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prison::~Prison
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Prison::~Prison( void )          // Constructor
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prison::isComplete
//
// Purpose-
//       Is evaluation complete?
//
//----------------------------------------------------------------------------
int                                 // TRUE if evaluation is complete
   Prison::isComplete( void )       // Evaluate and sort the group
{
   Prisoner*           ptrBase;     // -> Prisoner[0]
   Prisoner*           ptrPris;     // -> Prisoner
   DarwinUnit*         ptrUnit;     // -> DarwinUnit
   int                 anyChange;   // TRUE if any units are changed
   int                 allMutate;   // TRUE if all units are mutants

   unsigned int        r;
   int                 i;

   if( (generation & 0x03ff) != 0 )
     return FALSE;

   if( generation >= minGeneration )
   {
     // Look for differences in rank
     if( checkRank )
     {
       anyChange= FALSE;
       ptrBase= (Prisoner*)(getUnit(0)->castConcrete());
       r= ptrBase->evaluation;
       for(i=1; i<Prisoner::PrisonerCount/2; i++)
       {
         ptrPris= (Prisoner*)(getUnit(i)->castConcrete());
         if( r != ptrPris->evaluation )
         {
           anyChange= TRUE;
           break;
         }
       }
       if( !anyChange )
       {
         completionReason= AllSameRank;
         return TRUE;
       }
     }

     // Look for differences in rule
     if( checkRule )
     {
       anyChange= FALSE;
       ptrBase= (Prisoner*)(getUnit(0)->castConcrete());
       for(i=1; i<Prisoner::PrisonerCount/2; i++)
       {
         ptrPris= (Prisoner*)(getUnit(i)->castConcrete());
         if( memcmp(ptrBase->rule, ptrPris->rule, sizeof(ptrBase->rule)) != 0 )
         {
           anyChange= TRUE;
           break;
         }
       }
       if( !anyChange )
       {
         completionReason= AllSameRule;
         return TRUE;
       }
     }

     // Look for progress
     if( checkChange )
     {
       anyChange= FALSE;
       for(i=0; i<Prisoner::PrisonerCount/2; i++)
       {
         if( getUnit(i)->changed )
         {
           anyChange= TRUE;
           break;
         }
       }
       if( !anyChange )
       {
         completionReason= NoNewUnits;
         return TRUE;
       }
     }

     // See if all elements are mutants
     if( checkMutate )
     {
       allMutate= TRUE;
       for(i=0; i<Prisoner::PrisonerCount/2; i++)
       {
         ptrUnit= getUnit(i);
         if( !ptrUnit->mutated && !ptrUnit->evolMutate )
         {
           allMutate= FALSE;
           break;
         }
       }
       if( allMutate )
       {
         if( someNormal )
         {
           completionReason= AllMutants;
           return TRUE;
         }

         return FALSE;
       }

       someNormal= TRUE;              // Some normal units found
     }
   }

   // Evolution continues
   for(i=0; i<256; i++)
     historyArray[i]= 0;

   return FALSE;                    // Default action
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prison::evolveContinuous
//
// Purpose-
//       Evolve the group
//
//----------------------------------------------------------------------------
Prison::EvolveRc                    // Return code
   Prison::evolveContinuous( void ) // Evolve the group
{
   Generation          generation;  // (local) Generation
   Interval            interval;    // (local) Interval
   double              deltaTime;   // Elapsed time

   if( !stopOnCount )
     evolveCount= ULONG_MAX;

   for(unsigned i=0; i<used; i++)
   {
     unit[i]->changed= FALSE;
     unit[i]->mutated= FALSE;
     unit[i]->evolChange= FALSE;
     unit[i]->evolMutate= FALSE;
   }

   evaluate();
   for(generation=0; generation<evolveCount; generation++)
   {
     generate();
     evaluate();
     if( isComplete() )
       return EvolveComplete;

     if( (getGeneration() & 0x03FF) == 0 && getGeneration() != 0 )
     {
       if( stopOnTimer )
       {
         deltaTime= interval.stop();
         if( deltaTime > evolveTimer )
           return EvolveTimeout;
       }

       // Reset for next interval
       for(unsigned i=0; i<used; i++)
       {
         unit[i]->changed= FALSE;
         unit[i]->mutated= FALSE;
         unit[i]->evolChange= FALSE;
         unit[i]->evolMutate= FALSE;
       }
     }
   }

   if( stopOnCount )
     return EvolveLoopout;

   return EvolveInfinite;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prison::resetEvolveTimer
//
// Purpose-
//       Reset the evolveContinuous() time out
//
//----------------------------------------------------------------------------
void
   Prison::resetEvolveTimer( void ) // Reset evoveContinuous() time out
{
   stopOnTimer= FALSE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prison::resetEvolveCount
//
// Purpose-
//       Reset the evolveContinuous() loop out
//
//----------------------------------------------------------------------------
void
   Prison::resetEvolveCount( void ) // Reset evoveContinuous() loop out
{
   stopOnCount= FALSE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prison::setEvolveTimer
//
// Purpose-
//       Set evolveContinuous() timeout
//
//----------------------------------------------------------------------------
void
   Prison::setEvolveTimer(          // Set evolveContinuous() timeout
     double            timeOut)     // Timeout
{
   stopOnTimer= TRUE;
   evolveTimer= timeOut;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prison::setEvolveCount
//
// Purpose-
//       Set evolveContinuous() loopout
//
//----------------------------------------------------------------------------
void
   Prison::setEvolveCount(          // Set evolveContinuous() loopout
     Generation        loopOut)     // Loopout
{
   stopOnCount= TRUE;
   evolveCount= loopOut;
}

