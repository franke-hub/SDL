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
//       DarwinPlex.cpp
//
// Purpose-
//       DarwinPlex methods.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>

#include <com/define.h>
#include <com/Debug.h>
#include <com/Interval.h>
#include <com/Random.h>

#include "com/DarwinUnit.h"
#include "com/DarwinPlex.h"
#if !(INLINING)
#include "com/DarwinPlex.i"
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM                        // If defined, hard-core debug mode
#undef  HCDM                        // If defined, hard-core debug mode
#endif

//----------------------------------------------------------------------------
// External references
//----------------------------------------------------------------------------
static Random&         RNG= Random::standard; // Our random number generator

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinPlex::~DarwinPlex
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DarwinPlex::~DarwinPlex( void )  // Destructor
{
   delete [] unit;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinPlex::DarwinPlex
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DarwinPlex::DarwinPlex(          // Constructor
     unsigned int      elements)    // The number of group elements
:  className(NULL)
,  count(elements)
,  used(0)
,  generation(0)
,  mutation(0)
,  probCull(0.5)
,  probMute(0.0)
{
   unsigned int        i;

   #ifdef HCDM
     debugf("DarwinPlex(%p)::DarwinPlex()\n", this);
   #endif

   unit= new DarwinUnit*[elements]; // Allocate the Unit array
   assert(unit != NULL);
   for(i=0; i<elements; i++)
     unit[i]= NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinPlex::evaluate
//
// Purpose-
//       Evaluate the group
//
//----------------------------------------------------------------------------
void
   DarwinPlex::evaluate( void )     // Evaluate and sort the group
{
   DarwinUnit*         ptrUnit;     // -> DarwinUnit

   unsigned int        i, j;

   #ifdef HCDM
     debugf("DarwinPlex(%p)::evaluate()\n", this);
   #endif

   // Evaluate the group
   for(i=0; i<used; i++)
   {
     if( !unit[i]->isValid )
       unit[i]->evaluation= unit[i]->evaluate();
   }

   // Sort the group
   for(i=0; i<used; i++)
   {
     for(j=i+1; j<used; j++)
     {
       if( unit[j]->evaluation >= unit[i]->evaluation )
       {
         ptrUnit= unit[i];
         unit[i]= unit[j];
         unit[j]= ptrUnit;
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinPlex::generate
//
// Purpose-
//       Create a new generation
//
//----------------------------------------------------------------------------
void
   DarwinPlex::generate( void )     // Create a new generation
{
   unsigned int        culls;       // Number of culls
   unsigned int        keeps;       // Number of keeps
   unsigned int        mom, pop;    // Mother, father element indexes

   unsigned int        i;

   #ifdef HCDM
     debugf("DarwinPlex(%p)::generate()\n", this);
   #endif

   generation++;                    // Increment the generation
   if( used == 0 )                  // If empty group
     return;                        // Nothing to generate

   // Cull the bottom elements
   culls= getCull();
   if( culls > 0 )                  // If cull required
   {
     #ifdef HCDM
       debugf(">>[%10lu] cull(%d) of (%d)\n", generation, culls, used);
     #endif

     keeps= used-culls;             // Number of units to keep
     if( keeps < 2 )
       keeps= 2;

     for(i= keeps; i<used; i++)
     {
       mom= RNG.get() % keeps;
       pop= RNG.get() % keeps;
       if( mom == pop )
       {
         mom--;
         if( pop == 0 )
           mom= keeps - 1;
       }

       unit[i]->evolve(unit[pop], unit[mom]);
       unit[i]->generation= generation;
       unit[i]->changed= 1;
       unit[i]->mutated= 0;
       unit[i]->isValid= 0;

       if( unit[pop]->changed || unit[mom]->changed )
         unit[i]->evolChange= 1;
       if( unit[pop]->mutated || unit[mom]->mutated )
         unit[i]->evolMutate= 1;

       // Mutate the new element
       if( RNG.isTrue(probMute) )
       {
         unit[i]->mutate();
         unit[i]->mutated= 1;
         mutation++;
       }
       #ifdef HCDM
         debugf(">>[%10lu] [%2d] <= [%2d]+[%2d] %s\n", generation, i, mom, pop,
                unit[i]->mutated ? "Mutate" : "" );
       #endif
     }
   }
}

