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
//       Prison.h
//
// Purpose-
//       DarwinPlex Prison, contains DarwinUnit Prisoner elements.
//       (Scientic American, July, 1992)
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef PRISON_H_INCLUDED
#define PRISON_H_INCLUDED

#include "com/DarwinPlex.h"
#include "Prisoner.h"

//----------------------------------------------------------------------------
//
// Class-
//       Prison
//
// Purpose-
//       Prisoner container.
//
//----------------------------------------------------------------------------
class Prison : public DarwinPlex {  // Prison
//----------------------------------------------------------------------------
// Prison::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum CompletionReason               // Completion reason
{
   NotComplete,                     // No completion reason
   AllSameRank,                     // All units have the same rank
   AllSameRule,                     // All units have the same rule
   AllMutants,                      // All new units are mutants
   NoNewUnits,                      // No new units
};

enum EvolveRc                       // Return code from evolve()
{
   EvolveComplete,                  // Evaluation complete
   EvolveLoopout,                   // Loopout (count exceeded)
   EvolveTimeout,                   // Timeout ( time exceeded)
   EvolveInfinite                   // Infinite loop (must be LAST)
}; // enum EvolveRc

//----------------------------------------------------------------------------
// Prison::Constructors
//----------------------------------------------------------------------------
public:
   virtual ~Prison( void );         // Destructor

   Prison(                          // Constructor
     unsigned int      elements);   // The number of group elements

private:                            // Bitwise copy is prohibited
   Prison(const Prison&);           // Disallowed copy constructor
   Prison& operator=(const Prison&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// Prison::Accessor methods
//----------------------------------------------------------------------------
public:
int
   isComplete( void );              // TRUE if evaluation is complete

void
   resetEvolveTimer( void );        // Reset evolveContinuous() time out

void
   resetEvolveCount( void );        // Reset evolveContinuous() loop out

void
   setEvolveTimer(                  // Set evolveContinuous() time out
     double            timeOut);    // Timeout (seconds)

void
   setEvolveCount(                  // Set evolveContinuous() loop out
     Generation        loopOut);    // Loopout

//----------------------------------------------------------------------------
// Prison::Methods
//----------------------------------------------------------------------------
public:
EvolveRc                            // Return code
   evolveContinuous( void );        // Evolve the group

//----------------------------------------------------------------------------
// Prison::Attributes
//----------------------------------------------------------------------------
public:
   double              probTest;    // Evaluation probability
   unsigned long       historyArray[256]; // The number of times a history array
                                    // index was used during the last
                                    // evaluation interval

   unsigned long       minGeneration; // Minimum number of generations
   unsigned long       maxGeneration; // Maximum number of generations

   CompletionReason    completionReason; // Completion reason
   unsigned            checkRank   : 1; // Check for all same rank
   unsigned            checkRule   : 1; // Check for all same rule
   unsigned            checkChange : 1; // Check for no changed units
   unsigned            checkMutate : 1; // Check for all mutates
   unsigned            someNormal  : 1; // Have non-mutated units been found?

   // evolveContinuous() controls
   unsigned            stopOnCount :1; // Stop on count
   unsigned            stopOnTimer :1; // Stop on time

   Generation          evolveCount; // Stop loopout value
   double              evolveTimer; // Stop timeout value
}; // class Prison

#endif // PRISON_H_INCLUDED
