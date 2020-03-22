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
//       Stock.h
//
// Purpose-
//       Define the Stock objects.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef STOCK_H_INCLUDED
#define STOCK_H_INCLUDED

#ifndef TYPES_H_INCLUDED
#include "Types.h"
#endif

#include "com/DarwinPlex.h"
#include "com/DarwinUnit.h"

#include "Fanin.h"
#include "Neuron.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define MINPRICEVALUE           0.0
#define MAXPRICEVALUE      100000.0
#define MINVOLUMEVALUE          0.0
#define MAXVOLUMEVALUE 2000000000.0

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define DIM_USED                179 // Number of inputs in use
#define DIM_INP                 200 // Number of inputs
#define DIM_L3                  200 // L3 Array size
#define DIM_L2                 2000 // L2 Array size
#define DIM_L1                  200 // L1 Array size
#define DIM_OUT                  20 // Number of outputs

#define DIM_HIST              20000 // Number of history data points
#define DIM_HIST_LOW            000 // Number of history data points
#define DIM_UNIT                 32 // Number of Units
#define DIM_FILE                  8 // Number of Generations to save on disk

//----------------------------------------------------------------------------
//
// Struct-
//       Global
//
// Purpose-
//       Miscellaneous global values
//
//----------------------------------------------------------------------------
struct Global                       // Globals
{
   unsigned          savedSeed;     // The last saved seed
   unsigned          seedControl;   // Seed control
   unsigned          traceControl;  // TraceMode control
   unsigned          revalControl;  // Forced Re-evaluation control

   double            changeProb;    // Change probability
   unsigned          transferFee;   // The transfer fee
   double            interestRate;  // (Yearly) interest rate
   double            dailyInterest; // (Daily) interest rate
   unsigned          initialBalance;// Opening balance
   unsigned          minimumBalance;// Minimum balance
}; // struct Global

//----------------------------------------------------------------------------
// Global data areas
//----------------------------------------------------------------------------
extern Global        global;        // Globals

extern unsigned      histIndex0;    // First used data point
extern unsigned      histIndexN;    // Last  used data point
extern unsigned      histIndexU;    // Number of index points to use
extern int           histJulian[DIM_HIST+1]; // Julian date on day
extern double        histPrice[DIM_HIST];  // NYSE composite price on day
extern double        histVolume[DIM_HIST]; // NYSE composite volume on day

extern NeuronValue*  inpArrayN;     // The input value array
extern Neuron*       l3ArrayN;      // The L3 array
extern Neuron*       l2ArrayN;      // The L2 array
extern Neuron*       l1ArrayN;      // The L1 array
extern Neuron*       outArrayN;     // The output array

extern Fanin**       l3ArrayF;      // The  L3 -> INP mesh
extern Fanin**       l2ArrayF;      // The  L2 ->  L3 mesh
extern Fanin**       l1ArrayF;      // The  L1 ->  L2 mesh
extern Fanin**       outArrayF;     // The OUT ->  L1 mesh

#define FANIN_COUNT ( (DIM_L3*DIM_INP) + (DIM_L2*DIM_L3) + \
                      (DIM_L1*DIM_L2) + (DIM_OUT*DIM_L1) )
#define NEURON_COUNT ( DIM_INP + DIM_L3 + DIM_L2 + DIM_L1 + DIM_OUT )
#define RULE_SIZE sizeof(unsigned short[FANIN_COUNT])

//----------------------------------------------------------------------------
//
// Class-
//       Unit
//
// Purpose-
//       Stock Unit.
//
//----------------------------------------------------------------------------
class Unit : public DarwinUnit {    // Unit
//----------------------------------------------------------------------------
// Unit::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Unit( void );                   // Destructor
   Unit( void );                    // Constructor

private:                            // Bitwise copy is prohibited
   Unit(const Unit&);               // Disallowed copy constructor
   Unit& operator=(const Unit&);    // Disallowed assignment operator

//----------------------------------------------------------------------------
// Unit::Virtual methods
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
     const DarwinUnit*   father,    // Father object
     const DarwinUnit*   mother);   // Mother object

virtual void
   mutate( void );                  // Mutate the rule

//----------------------------------------------------------------------------
// Unit::Methods
//----------------------------------------------------------------------------
public:
void
   loadFaninArray( void );          // Load the Fanin array

static void
   loadInputArray(                  // Load the Input array
     unsigned        x);            // For this day

void
   random( void );                  // Load a random rule

void
   showRule( void) const;           // Show the rule

//----------------------------------------------------------------------------
// Unit::Static attributes
//----------------------------------------------------------------------------
public:
static unsigned      minIndex;      // The minimum history index

//----------------------------------------------------------------------------
// Unit::Attributes
//----------------------------------------------------------------------------
public:
   unsigned long     cash;          // Cash valuation
   unsigned long     stock;         // Stock valuation
   unsigned long     lastTransfer;  // Last transfer date
   unsigned long     fee;           // Fees paid
   Value             outs[DIM_OUT]; // Output evaluation array

   unsigned short*   rule;          // The rule
}; // class Unit

//----------------------------------------------------------------------------
//
// Class-
//       Plex
//
// Purpose-
//       Stock Plex (group).
//
//----------------------------------------------------------------------------
class Plex : public DarwinPlex {    // Plex
//----------------------------------------------------------------------------
// Plex::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Plex( void );                   // Destructor
   Plex(                            // Constructor
     unsigned int    elements);     // The number of Units

private:                            // Bitwise copy is prohibited
   Plex(const Plex&);               // Disallowed copy constructor
   Plex& operator=(const Plex&);    // Disallowed assignment operator

//----------------------------------------------------------------------------
// Plex::Virtual methods
//----------------------------------------------------------------------------
public:
virtual void
   evaluate( void );                // Evaluate the group

//----------------------------------------------------------------------------
// Plex::Methods
//----------------------------------------------------------------------------
public:
void
   backup( void );                  // Save the Units

void
   debugDump( void ) const;         // Debugging dump

void
   oldFormat(                       // Restore old format
     int             handle);       // File handle

void
   newFormat(                       // Restore new format
     int             handle);       // File handle

void
   restore( void );                 // Restore the Units

//----------------------------------------------------------------------------
// Plex::Attributes
//----------------------------------------------------------------------------
public:
   unsigned long     outGeneration;   // File identifer to write
   unsigned long     minGeneration;   // Minimum generation count
   unsigned long     maxGeneration;   // Maximum generation count

   unsigned          checkChange : 1; // Check for no changed units
   unsigned          checkMutate : 1; // Check for all mutates
   unsigned          checkRank   : 1; // Check for all same rank
   unsigned          checkRule   : 1; // Check for all same rule
   unsigned          someNormal  : 1; // Have non-mutated units been found?
}; // class Plex

#endif // STOCK_H_INCLUDED
