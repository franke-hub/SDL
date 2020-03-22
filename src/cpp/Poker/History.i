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
//       History.i
//
// Purpose-
//       History inline methods.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef HISTORY_I_INCLUDED
#define HISTORY_I_INCLUDED

//----------------------------------------------------------------------------
// PokerHistory::Accessor methods
//----------------------------------------------------------------------------
void                                // Exception iff error
   PokerHistory::isValid(           // Verify index
     int               ago) const   // The associated index
{
   if( ago >= used )                // If invalid index
     throw "PokerHistory::isValidException";
}

int                                 // The check count
   PokerHistory::getCheckCount(     // Get check count
     int               ago) const   // Associated index
{
   isValid(ago);

   return checkCount[used-ago-1];
}

int                                 // The call amount
   PokerHistory::getCallAmount(     // Get call amount
     int               ago) const   // Associated index
{
   isValid(ago);

   return callAmount[used-ago-1];
}

int                                 // The call count
   PokerHistory::getCallCount(      // Get call count
     int               ago) const   // Associated index
{
   isValid(ago);

   return callCount[used-ago-1];
}

int                                 // The raise amount
   PokerHistory::getRaiseAmount(    // Get raise amount
     int               ago) const   // Associated index
{
   isValid(ago);

   return raiseAmount[used-ago-1];
}

int                                 // The raise count
   PokerHistory::getRaiseCount(     // Get raise count
     int               ago) const   // Associated index
{
   isValid(ago);

   return raiseCount[used-ago-1];
}

int                                 // The number of valid elements
   PokerHistory::getSize( void ) const // Get number of valid elements
{
   return used;
}

double                              // The Rating
   PokerPlayerHistory::getRating(   // Get Rating
     int               ago) const   // Associated index
{
   isValid(ago);

   return rating[used-ago-1];
}

PokerResult                         // The PokerResult
   PokerPlayerHistory::getResult(   // Get PokerResult
     int               ago) const   // Associated index
{
   isValid(ago);

   return result[used-ago-1];
}

void
   PokerPlayerHistory::setResult(   // Set PokerResult
     PokerResult       result,      // The PokerResult
     double            rating)      // The Rating
{
   isValid(0);

   this->result[used-1]= result;
   this->rating[used-1]= rating;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerHistory::call
//
// Purpose-
//       Update the call history.
//
//----------------------------------------------------------------------------
void
   PokerHistory::call(              // Update call History
     int               amount)      // The call amount
{
   isValid(0);

   callCount[used-1]++;
   callAmount[used-1] += amount;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerHistory::check
//
// Purpose-
//       Update the check history.
//
//----------------------------------------------------------------------------
void
   PokerHistory::check( void )      // Update check history
{
   isValid(0);

   checkCount[used-1]++;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerHistory::raise
//
// Purpose-
//       Update the raise history.
//
//----------------------------------------------------------------------------
void
   PokerHistory::raise(             // Update raise history
     int               amount)      // The raise amount
{
   isValid(0);

   raiseCount[used-1]++;
   raiseAmount[used-1] += amount;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerHistory::reset
//
// Purpose-
//       Reset (clear) the history.
//
//----------------------------------------------------------------------------
void
   PokerHistory::reset( void )      // Reset the History
{
   used= 0;
}

#endif // HISTORY_I_INCLUDED
