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
//       Unit.cpp
//
// Purpose-
//       Stock evaluation unit.
//
// Last change date-
//       2007/01/01
//
// Input array-
//       [  0]..[ 19] Composite index daily
//       [ 20]..[ 39] Composite index weekly
//       [ 40]..[ 99] Composite index monthly
//       [100]..[119] Composite volume daily
//       [120]..[139] inp[0+i] * inp[100+i]
//       [140]..[159] Feedbacks (out[0]..out[19])
//       [160]..[169] Constant 1 .. 1,000,000,000
//       [170]  Number of days until next open
//       [171]  inp[170]*out[0]
//       [172]  inp[170]*out[1]
//       [173]  inp[170]*dailyInterestRate
//       [174]  julianDay % 7   (day of week)
//       [175]  julianDay % 91  (day of quarter)
//       [176]  julianDay % 365 (day of year)
//       [177]  dailyInterestRate
//       [178]  transferFee
//       [179]..[199] Unused, Unattached
//
// Output array-
//       [ 0] Stock weight
//       [ 1] Cash  weight
//       [ 2] Inhibit trigger
//       [ 3] (Unused, feedback)
//          :
//       [19] (Unused, feedback)
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

#include "Fanin.h"
#include "Neuron.h"
#include "Stock.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "UNIT    " // Source file name

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM                        // If defined, hard-core debug mode
#undef  HCDM                        // If defined, hard-core debug mode
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define DAYS_PER_WEEK             5 // These are days in history file
#define DAYS_PER_MONTH           22
#define DAYS_PER_YEAR           264

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define X_DUO                   170 // Index: Number of days until open
#define X_DUO_F0                171 // Index: [X_DAY_COUNT]*out[0]
#define X_DUO_F1                172 // Index: [X_DAY_COUNT]*out[1]
#define X_DUO_F2                173 // Index: [X_DAY_COUNT]*dailyInterestRate
#define X_DOW                   174 // Index: Day of week
#define X_DOQ                   175 // Index: Day of quarter
#define X_DOY                   176 // Index: Day of year
#define X_DIR                   177 // Index: Daily interest rate
#define X_FEE                   178 // Index: Transfer fee

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
unsigned             Unit::minIndex= (60*DAYS_PER_MONTH)+1; // Lowest index

//----------------------------------------------------------------------------
// External references
//----------------------------------------------------------------------------
static Random&         RNG= Random::standard; // Our random number generator

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char    className[]= "Unit::DarwinUnit"; // The class name
static const double  scaleFactor= 5.0 / (double)0x00007fff; // Scaling factor

//----------------------------------------------------------------------------
// Inlines
//----------------------------------------------------------------------------
#include "Fanin.i"
#include "Neuron.i"

//----------------------------------------------------------------------------
//
// Method-
//       Unit::~Unit
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Unit::~Unit( void )              // Destructor
{
   #ifdef HCDM
     debugf("Unit(%p)::~Unit()\n", this);
   #endif

   delete [] rule;
}

//----------------------------------------------------------------------------
//
// Method-
//       Unit::Unit
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Unit::Unit( void )               // Constructor
:  DarwinUnit()
,  cash(0)
,  stock(0)
,  lastTransfer(0)
,  fee(0)
,  rule(new unsigned short[FANIN_COUNT])
{
   #ifdef HCDM
     debugf("Unit(%p)::Unit() rule(%p)\n", this, rule);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Unit::castConcrete
//
// Purpose-
//       Get this concrete class.
//
//----------------------------------------------------------------------------
void*                               // The concrete class
   Unit::castConcrete( void ) const // Cast to concrete class
{
   return (void*)this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Unit::className
//
// Purpose-
//       Get the UNIQUE class name.
//
//----------------------------------------------------------------------------
const char*                         // -> ClassName
   Unit::className( void ) const    // Get the UNIQUE class name
{
   return ::className;              // Return the class name
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::toDouble
//
// Function-
//       Convert unsigned short to float.
//
//----------------------------------------------------------------------------
static inline double                // Resultant
   toDouble(                        // Convert
     unsigned short  source)        // From unsigned short
{
   double            resultant;
   unsigned          temp;

   temp= source & 0x00007fff;       // Remove sign bit
   temp <<= 3;

   resultant= (double)temp*scaleFactor;
   if( source & 0x00008000 )
     resultant= (-resultant);

   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       Unit::loadFaninArray
//
// Function-
//       Load the Fanin array from the Rule.
//
//----------------------------------------------------------------------------
void
   Unit::loadFaninArray( void )     // Externalize the Rule
{
   int               i, j;
   int               x;

   #ifdef HCDM
     debugf("Unit(%p)::loadFaninArray()\n", this);
   #endif

   x= 0;
   for(i=0; i<DIM_L3; i++)
     for(j=0; j<DIM_INP; j++)
       l3ArrayF[i][j].weight= toDouble(rule[x++]);

   for(i=0; i<DIM_L2; i++)
     for(j=0; j<DIM_L3; j++)
       l2ArrayF[i][j].weight= toDouble(rule[x++]);

   for(i=0; i<DIM_L1; i++)
     for(j=0; j<DIM_L2; j++)
       l1ArrayF[i][j].weight= toDouble(rule[x++]);

   for(i=0; i<DIM_OUT; i++)
     for(j=0; j<DIM_L1; j++)
       outArrayF[i][j].weight= toDouble(rule[x++]);

   assert( x == FANIN_COUNT );

   #if 0
     int             k;

     debugf("L3ArrayF\n");
     for(i= 0; i<DIM_L3; i++)
     {
       k= 0;
       for(j= 0; j<DIM_INP; j++)
       {
         if( k == 4 )
         {
           k= 0;
           tracef("\n");
         }
         k++;
         tracef("[%4d][%4d] %6.3f ", i, j, l3ArrayF[i][j].weight);
       }
       tracef("\n");
     }

     debugf("L2ArrayF\n");
     for(i= 0; i<DIM_L2; i++)
     {
       k= 0;
       for(j= 0; j<DIM_L3; j++)
       {
         if( k == 4 )
         {
           k= 0;
           tracef("\n");
         }
         k++;
         tracef("[%4d][%4d] %6.3f ", i, j, l2ArrayF[i][j].weight);
       }
       tracef("\n");
     }

     debugf("L1ArrayF\n");
     for(i= 0; i<DIM_L1; i++)
     {
       k= 0;
       for(j= 0; j<DIM_L2; j++)
       {
         if( k == 4 )
         {
           k= 0;
           tracef("\n");
         }
         k++;
         tracef("[%4d][%4d] %6.3f ", i, j, l1ArrayF[i][j].weight);
       }
       tracef("\n");
     }

     debugf("L0ArrayF\n");
     for(i= 0; i<DIM_OUT; i++)
     {
       k= 0;
       for(j= 0; j<DIM_L1; j++)
       {
         if( k == 4 )
         {
           k= 0;
           tracef("\n");
         }
         k++;
         tracef("[%4d][%4d] %6.3f ", i, j, outArrayF[i][j].weight);
       }
       tracef("\n");
     }

     fprintf(stderr, "DEBUG: Dump fanin array\n");
     exit(EXIT_SUCCESS);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Unit::loadInputArray
//
// Function-
//       Load the Input array from the history file.
//
//----------------------------------------------------------------------------
void
   Unit::loadInputArray(            // Load the input array
     unsigned        x)             // For this day
{
   int               today;         // Today's julian date
   int               days;
   int               i;

   #ifdef HCDM
     debugf("Unit(*)::loadInputArray()\n");
   #endif

   for(i=0; i<20; i++)
   {
     inpArrayN[i+  0].setValue(histPrice[x-i-1]);
     inpArrayN[i+ 20].setValue(histPrice[x-i*DAYS_PER_WEEK-DAYS_PER_WEEK]);
     inpArrayN[i+100].setValue(histVolume[x-i-1]);
     inpArrayN[i+120].setValue(inpArrayN[i+0].getValue() *
                               inpArrayN[i+100].getValue());
     inpArrayN[i+140].setValue(outArrayN[i].getValue()); // Feedbacks
   }

   for(i=0; i<60; i++)
     inpArrayN[i+ 40].setValue(histPrice[x-i*DAYS_PER_MONTH-DAYS_PER_MONTH]);

   inpArrayN[160].setValue(1.0);
   inpArrayN[161].setValue(10.0);
   inpArrayN[162].setValue(100.0);
   inpArrayN[163].setValue(1000.0);
   inpArrayN[164].setValue(10000.0);
   inpArrayN[165].setValue(100000.0);
   inpArrayN[166].setValue(1000000.0);
   inpArrayN[167].setValue(10000000.0);
   inpArrayN[168].setValue(100000000.0);
   inpArrayN[169].setValue(1000000000.0);

   today= histJulian[x];
   days=  histJulian[x+1] - today;
   inpArrayN[X_DUO].setValue(days);
   inpArrayN[X_DUO_F0].setValue((double)days * outArrayN[0].getValue());
   inpArrayN[X_DUO_F1].setValue((double)days * outArrayN[1].getValue());
   inpArrayN[X_DUO_F2].setValue((double)days * global.dailyInterest);

   inpArrayN[X_DOW].setValue(today % 7);
   inpArrayN[X_DOQ].setValue(today % 91);
   inpArrayN[X_DOY].setValue(today % 365);
   inpArrayN[X_DIR].setValue(global.dailyInterest);
   inpArrayN[X_FEE].setValue(global.transferFee);

   assert( DIM_USED == (X_FEE+1) );

   #if 0
     for(i= 0; i<DIM_USED; i++)
     {
       debugf("[%3d] %f\n", i, inpArrayN[i].getValue());
     }
     exit(EXIT_SUCCESS);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::transfer
//
// Function-
//       Calculate the transfer (stock->cash) amount.
//
//----------------------------------------------------------------------------
static signed int                   // Transfer amount
   transfer(                        // Calculate the transfer amount
     unsigned        sValue,        // Stock value
     unsigned        cValue,        // Cash value
     double          sWeight,       // Stock weight
     double          cWeight)       // Cash weight
{
   signed            resultant;     // Transfer amount
   unsigned          newStock;      // New stock valuation
   double            weight= sWeight + cWeight;

   assert( cWeight >= 0.0 && sWeight >= 0.0 );
   assert( (signed)sValue >= 0 && (signed)cValue >= 0);

   // Calculate the new stock value
   if( weight == 0.0 )
     weight= 0.5;
   else
     weight= sWeight / weight;

   // The new stock value
   newStock= (unsigned)(weight * (double)(sValue + cValue));

   // Calculate the transfer amount (in whole dollars)
   resultant= newStock - sValue;
   resultant /= 100;
   resultant *= 100;

   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       Unit::evaluate
//
// Function-
//       Evaluate the current Rule.
//
//----------------------------------------------------------------------------
Unit::Evaluation                    // Evaluation
   Unit::evaluate( void )           // Evaluate the Rule
{
   int               isTrace;       // TRUE if tracing
   unsigned long     cash, stock, fee; // Valuations (in pennies)
   unsigned long     opening;       // Opening balance
   long              xferAmount;    // Transfer amount
   double            share;         // Number of shares
   double            today, prior;  // Valuations

   int               i;
   int               x;

   #ifdef HCDM
     debugf("Unit(%p)::evaluate() started\n", this);
   #endif

   //-------------------------------------------------------------------------
   // Load the Fanin array
   //-------------------------------------------------------------------------
   loadFaninArray();

   //-------------------------------------------------------------------------
   // Load the Initial outputs
   //-------------------------------------------------------------------------
   opening= global.initialBalance;  // Default opening balance
   isTrace= global.traceControl;    // Default trace control
   if( isValid )                    // If incremental evaluation
   {
     opening= evaluation;           // Start with prior balance
     isTrace= TRUE;                 // TRACING is active

     for(i= 0; i<DIM_OUT; i++)      // Load initial outputs
       outArrayN[i].setValue(outs[i]); // (Current values)
   }
   else
   {
     for(i= 0; i<DIM_OUT; i++)        // Load initial outputs
       outArrayN[i].setValue(0.5);    // (Default values)
   }

   //-------------------------------------------------------------------------
   // Evaluate the initial outputs
   //-------------------------------------------------------------------------
   x= histIndex0;                   // First history index
   loadInputArray(x);               // Load data for today
   inpArrayN[X_FEE].setValue(0.0);  // No transaction fee today

   Neuron::globalClock++;
   for(i=0; i<DIM_OUT; i++)
   {
     outArrayN[i].resolve();
   }

   //-------------------------------------------------------------------------
   // Make the initial transfer
   //-------------------------------------------------------------------------
   lastTransfer= histJulian[x];
   stock= transfer(0, opening,
                   outArrayN[0].getValue(),
                   outArrayN[1].getValue());
   cash= opening - stock;
   if( isTrace )
     debugf("[%5d] V(%12ld) S(%10ld) C(%10ld)\n",
            x, (stock+cash)/100, stock/100, cash/100);

   //-------------------------------------------------------------------------
   // Evaluate
   //-------------------------------------------------------------------------
   fee= 0;
   for(x++; x<histIndexN; x++)
   {
     //-----------------------------------------------------------------------
     // Evaluate yesterday's investment decision
     //-----------------------------------------------------------------------
     prior= histPrice[x-1];
     today= histPrice[x];
     cash += (unsigned long)
             ((double)cash * global.dailyInterest *
             (double)(histJulian[x] - histJulian[x-1]));
     share= (double)stock / prior;  // Prior number of shares
     stock= (unsigned long)(today * share); // Today's stock evaluation

     if( isTrace )
       debugf("[%5d] V(%12ld) S(%10ld) C(%10ld) %6.2f => %6.2f\n",
              x, (stock+cash)/100, stock/100, cash/100, prior, today);

     //-----------------------------------------------------------------------
     // Make today's investment decision
     //-----------------------------------------------------------------------
     loadInputArray(x);

     Neuron::globalClock++;
     for(i=0; i<DIM_OUT; i++)
     {
       outArrayN[i].resolve();
     }

     if( outArrayN[2].getValue() < 0.5 )
     {
       xferAmount= transfer(stock, cash,
                            outArrayN[0].getValue(),
                            outArrayN[1].getValue());
       if( xferAmount != 0 )
       {
         if( (stock+cash) < global.minimumBalance )
         {
           stock= 0;
           cash= 0;
           break;
         }

         if( isTrace )
           debugf("[%5d] V(%12ld) S(%10ld) C(%10ld) => T(%10ld)\n",
                  x, (stock+cash)/100, stock/100, cash/100, xferAmount/100);

         if( xferAmount > 0 )
           assert( cash >= xferAmount);
         else
           assert( stock >= -xferAmount);

         stock += xferAmount;
         cash  -= xferAmount;
         if( stock > cash )
           stock -= global.transferFee;
         else
           cash  -= global.transferFee;
         fee += global.transferFee;
         lastTransfer= histJulian[x];
         if( isTrace )
           debugf("[%5d] V(%12ld) S(%10ld) C(%10ld)\n",
                  x, (stock+cash)/100, stock/100, cash/100);
       }
     }
   }

   this->cash=  cash;
   this->stock= stock;
   this->fee=   fee;
   for(i=0; i<DIM_OUT; i++)
     outs[i]= outArrayN[i].getValue();

   if( isTrace )
   {
     showRule();
     debugFlush();
   }

   isValid= 1;

   #ifdef HCDM
     debugf("Unit(%p)::evaluate() complete\n", this);
   #endif

   return stock + cash;             // Resultant value
}

//----------------------------------------------------------------------------
//
// Method-
//       Unit::evolve
//
// Function-
//       Evolve the rule.
//
//----------------------------------------------------------------------------
void
   Unit::evolve(                    // Evolve the rule
     const DarwinUnit*
                     inpFather,     // Father object
     const DarwinUnit*
                     inpMother)     // Mother object
{
   Unit*             father= (Unit*)(inpFather->castConcrete());
   Unit*             mother= (Unit*)(inpMother->castConcrete());

   #ifdef HCDM
     debugf("Unit(%p)::evolve(%p,%p)\n", this, inpFather, inpMother);
   #endif

   DarwinUnit::evolve(RULE_SIZE, (char*)rule,
                      (char*)father->rule, (char*)mother->rule);
}

//----------------------------------------------------------------------------
//
// Method-
//       Unit::random
//
// Function-
//       Set the Rule to a random value
//
//----------------------------------------------------------------------------
void
   Unit::random( void )             // Random rule
{
   char*             R= (char*)rule;
   int               i;

   for(i=0; i<RULE_SIZE; i++)
     R[i]= RNG.get();
}

//----------------------------------------------------------------------------
//
// Method-
//       Unit::mutate
//
// Function-
//       Mutate the rule.
//
//----------------------------------------------------------------------------
void
   Unit::mutate( void )             // Mutate the rule
{
   int               i, m;

   #ifdef HCDM
     debugf("Unit(%p)::mutate()\n", this);
   #endif

   m= (int)(RULE_SIZE * global.changeProb);
   if( m > 0 )
     m= RNG.get() % m;

   for(i=0; i<m; i++)
     DarwinUnit::mutate(RULE_SIZE, (char*)rule);
}

//----------------------------------------------------------------------------
//
// Method-
//       Unit::showRule
//
// Function-
//       Show the rule
//
//----------------------------------------------------------------------------
void
   Unit::showRule( void ) const     // Show the rule
{
   debugf("== V(%12ld) S(%6.2f)%% C(%6.2f)%% Days(%5ld) F(%6ld)\n",
          (stock+cash)/100,
          (stock+cash == 0) ? 0.0 : stock*100.0/(stock+cash),
          (stock+cash == 0) ? 0.0 : cash*100.0/(stock+cash),
          histJulian[histIndexN-1]-lastTransfer,
          fee/100);
}

