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
//       Prisoner.h
//
// Purpose-
//       Sample DarwinUnit: Prisoner's dilemma.
//       (Scientic American, July, 1992)
//
// Last change date-
//       2007/01/01
//
// Notes-
//       (Scientic American, July, 1992)
//       The prisoner's dilemma is:
//         A pair of prisoners have each been granted a chance to reduce
//         their sentence.  If both prisoners cooperate, both receive a
//         reduction.  If both defect, neither receives a reduction.
//         If one cooperates and one defects, the defector receives a
//         greater reduction.
//
//
//                  |     /B    |   /B
//           PLAYER | Cooperate | Defect
//        ----------|-----------|-------
//        Cooperate |    3/3    |  5/0
//        -----A/---|-----------|-------
//           Defect |    0/5    |  0/0
//        ----------|-----------|-------
//
//----------------------------------------------------------------------------
#ifndef PRISONER_H_INCLUDED
#define PRISONER_H_INCLUDED

#include "com/DarwinUnit.h"
#include <com/Random.h>

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Prison;

//----------------------------------------------------------------------------
//
// Class-
//       Prisoner
//
// Purpose-
//       Prisoner's dilemma unit.
//
//----------------------------------------------------------------------------
class Prisoner : public DarwinUnit {// Prisoner
//----------------------------------------------------------------------------
// Prisoner::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum                                // Generic enumeration
{
   PrisonerCount=                64 // The number of prisoners
};

enum Choice                         // Choice: Defect or Cooperate
{
   Cooperate=                     0,// Cooperate
   Defect=                        1 // Defect
};

//----------------------------------------------------------------------------
// Prisoner::Constructors
//----------------------------------------------------------------------------
public:
   virtual ~Prisoner( void );       // Destructor
   Prisoner( void );                // Constructor

private:                            // Bitwise copy is prohibited
   Prisoner(const Prisoner&);       // Disallowed copy constructor
   Prisoner& operator=(const Prisoner&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// Prisoner::Virtual methods
//----------------------------------------------------------------------------
public:
virtual void*                       // The concrete class
   castConcrete( void ) const;      // Cast to concrete class

virtual const char*                 // -> ClassName
   className( void ) const;         // Get the UNIQUE class name

virtual Evaluation                  // Evaluation
   evaluate( void );                // Evaluate the Rule

virtual void
   evolve(                          // Evolve the rule
     const DarwinUnit* father,      // Father object
     const DarwinUnit* mother);     // Mother object

virtual void
   mutate( void );                  // Mutate the rule

//----------------------------------------------------------------------------
// Prisoner::Methods
//----------------------------------------------------------------------------
public:
Choice                              // The choice
   choose(                          // Cooperate or defect?
     Prisoner*         other);      // The other prisoner

static unsigned int                 // Resultant state
   history(                         // Compute resultant state
     unsigned char     current,     // Current state
     Choice            his,         // His choice
     Choice            our);        // Our choice

void
   history(                         // Update the history
     Prisoner*         other,       // The other prisoner
     Choice            his,         // His choice
     Choice            our);        // Our choice

void
   setPrison(                       // Put the Prisoner in a Prison
     Prison*           prison);     // -> Prison

void
   showRule( void) const;           // Show the rule

//----------------------------------------------------------------------------
// Prisoner::Attributes
//----------------------------------------------------------------------------
public:
   unsigned int        cellNumber;  // Our (permanent) cell number
   Prison*             ptrPrison;   // The prison to which we are assigned

   char                rule[32];    // The rule
   unsigned char       historyArray[PrisonerCount]; // The history array

   static char         newUnitsForget; // TRUE if new units forget
   static char         forgetNewUnits; // TRUE if forget new units
}; // class Prisoner

#endif  // PRISONER_H_INCLUDED
