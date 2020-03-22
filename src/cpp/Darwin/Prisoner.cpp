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
//       Prisoner.cpp
//
// Purpose-
//       Sample Darwin object: Prisoner's dilemma.
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
#include <com/Random.h>
#include <com/syslib.h>

#include "Prison.h"
#include "Prisoner.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "Prisoner" // Source file name

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef TRACEUNIT                   // If defined, unit trace unit number
#undef  TRACEUNIT                   // If defined, unit trace unit number
#endif

#ifndef HCDM                        // If defined, hard-core debug mode
#undef  HCDM                        // If defined, hard-core debug mode
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char      className[]= "Prisoner::DarwinUnit"; // The class name

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
char                   Prisoner::forgetNewUnits= 0;
char                   Prisoner::newUnitsForget= 0;

//----------------------------------------------------------------------------
// External references
//----------------------------------------------------------------------------
static Random&         RNG= Random::standard; // Our random number generator

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prisoner::Prisoner
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Prisoner::Prisoner( void )       // Constructor
:  DarwinUnit()
,  ptrPrison(NULL)
{
   int               i;

#ifdef HCDM
   debugf("Prisoner(%p)::Prisoner()\n", this);
#endif

   for(i=0; i<sizeof(rule); i++)
     rule[i]= RNG.get();

   for(i=0; i<PrisonerCount; i++)
     historyArray[i]= RNG.get();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prisoner::~Prisoner
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Prisoner::~Prisoner( void )      // Destructor
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prisoner::castConcrete
//
// Purpose-
//       Get this concrete class.
//
//----------------------------------------------------------------------------
void*                               // The concrete class
   Prisoner::castConcrete( void ) const // Cast to concrete class
{
   return (void*)this;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prisoner::className
//
// Purpose-
//       Get the UNIQUE class name.
//
//----------------------------------------------------------------------------
const char*                         // -> ClassName
   Prisoner::className( void ) const// Get the UNIQUE class name
{
   return ::className;              // Return the class name
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prisoner::choose
//
// Function-
//       Select the next play.
//
//----------------------------------------------------------------------------
Prisoner::Choice                    // The choice
   Prisoner::choose(                // Cooperate or defect?
     Prisoner*         other)       // The other prisoner
{
   assert(other != NULL);
   assert(other->cellNumber < PrisonerCount);

   unsigned int        const otherCellNumber= other->cellNumber;
   unsigned int        const historyIndex= historyArray[otherCellNumber];

   this->ptrPrison->historyArray[historyIndex]++;

   return Choice(Bit::get(rule, historyIndex));
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prisoner::evaluate
//
// Function-
//       Evaluate the current Rule.
//
//----------------------------------------------------------------------------
Prisoner::Evaluation                // Evaluation
   Prisoner::evaluate( void )       // Evaluate the Rule
{
   Evaluation          resultant= 0;// Resultant

   Prison*             ptrPrison;   // -> Prison
   Prisoner*           ptrPrisoner; // -> Prisoner

   Choice              ourChoice;   // Our choice
   Choice              hisChoice;   // His (the other) choice

   unsigned int        used;        // Number of actual prisoners

#ifdef TRACEUNIT
   char                temp[512];
   int                 ourOld;
   int                 ourNew;
#endif

   int                 i;

   ptrPrison= this->ptrPrison;
   assert(ptrPrison != NULL);

   // Evaluate
   used= ptrPrison->getUsed();
   for(i=0; i<used; i++)
   {
     ptrPrisoner= (Prisoner*)((ptrPrison->getUnit(i))->castConcrete());
     if( this != ptrPrisoner && RNG.isTrue(ptrPrison->probTest) )
     {
       hisChoice= ptrPrisoner->choose(this);
       ourChoice= choose(ptrPrisoner);
       if( hisChoice == Cooperate )
       {
         if( ourChoice == Cooperate )
           resultant += 3;
         else
           resultant += 5;
       }

       // Update our history
#ifdef TRACEUNIT
       ourOld= historyArray[ptrPrisoner->cellNumber];
#endif

       history(ptrPrisoner, hisChoice, ourChoice);

#ifdef TRACEUNIT
       ourNew= historyArray[ptrPrisoner->cellNumber];
       if( cellNumber == TRACEUNIT )
       {
         tracef("CHOICE [%2d] %d%d [%.2X]=>[%.2X]\n",
                ptrPrisoner->cellNumber, hisChoice, ourChoice, ourOld, ourNew);
       }
#endif
     }
   }

#ifdef TRACEUNIT
   // Traceing
   if( cellNumber == TRACEUNIT )
   {
     tracef("RESULT %4d ", resultant);
     tracef("%s\n", DarwinUnit::toString(temp, sizeof(rule), rule) );
     tracef("\n\n");
   }
#endif

   return resultant;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prisoner::evolve
//
// Function-
//       Evolve the rule.
//
//----------------------------------------------------------------------------
void
   Prisoner::evolve(                // Evolve the rule
     const DarwinUnit* inpFather,   // Father object
     const DarwinUnit* inpMother)   // Mother object
{
   Prison*             ptrPrison;
   Prisoner*           ptrPrisoner;

   Prisoner*           father= (Prisoner*)(inpFather->castConcrete());
   Prisoner*           mother= (Prisoner*)(inpMother->castConcrete());

   int                 i;

   ptrPrison= this->ptrPrison;
   assert( ptrPrison != NULL );

   DarwinUnit::evolve(sizeof(rule), rule, father->rule, mother->rule);

#ifdef TRACEUNIT
   char                temp[512];

   if( cellNumber == TRACEUNIT )
   {
     tracef("EVOLVE   F: %s\n",
            DarwinUnit::toString(temp, sizeof(rule), father->rule) );
     tracef("EVOLVE   M: %s\n",
            DarwinUnit::toString(temp, sizeof(rule), mother->rule) );
     tracef("EVOLVE   R: %s\n",
            DarwinUnit::toString(temp, sizeof(rule), rule) );
     tracef("\n");
   }
#endif

   if( forgetNewUnits )
   {
     for(i= 0; i<PrisonerCount; i++)
     {
       // Our history array is no longer valid
       historyArray[i]= RNG.get();
     }
   }

   if( newUnitsForget )
   {
     for(i= 0; i<PrisonerCount; i++)
     {
       // His history array is no longer valid
       ptrPrisoner= (Prisoner*)((ptrPrison->getUnit(i))->castConcrete());
       ptrPrisoner->historyArray[cellNumber]= RNG.get();
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prisoner::history
//
// Function-
//       Update the history array.
//
// Notes-
//       Evaluation rule bits:
//
//       We have a 256 bit (32 byte) rule.
//       We keep a history of 8 plays: 4 by the opponent and 4 by us.
//       This history selects the next play.
//
//----------------------------------------------------------------------------
unsigned int                        // Resultant state
   Prisoner::history(               // Update the history
     unsigned char     current,     // Current state
     Choice            his,         // His choice
     Choice            our)         // Our choice
{
   unsigned int        resultant;   // The resultant history entry

   resultant= current;              // The current history entry
   resultant <<= 1;                 // Make room for choices
   resultant  &= 0x00EE;            // Turn off the choice bits
   resultant  |= (his << 4) | our;  // Turn on the selections

   return resultant;                // Return the update
}

void
   Prisoner::history(               // Update the history
     Prisoner*         other,       // The other prisoner
     Choice            his,         // His choice
     Choice            our)         // Our choice
{
   unsigned int        otherIndex;  // The other Prisoner's history index

   assert(other != NULL);
   otherIndex= other->cellNumber;
   assert(otherIndex < PrisonerCount);

   // Update the history
   historyArray[otherIndex]= history( historyArray[otherIndex], his, our );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prisoner::mutate
//
// Function-
//       Mutate the rule.
//
//----------------------------------------------------------------------------
void
   Prisoner::mutate( void )         // Mutate the rule
{
   DarwinUnit::mutate(sizeof(rule), rule);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prisoner::setPrison
//
// Function-
//       Put the Prisoner in a Prison
//
//----------------------------------------------------------------------------
void
   Prisoner::setPrison(             // Put the Prisoner in a Prison
     Prison*           prison)      // -> Prison
{
   assert(ptrPrison == NULL);
   cellNumber= prison->setUnit(this);
   assert(cellNumber < PrisonerCount);
   ptrPrison= prison;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Prisoner::showRule
//
// Function-
//       Show the rule
//
//----------------------------------------------------------------------------
void
   Prisoner::showRule( void ) const // Show the rule
{
   char                temp[512];

   debugf("%s\n", DarwinUnit::toString(temp, sizeof(rule), rule) );
}

