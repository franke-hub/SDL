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
//       History.h
//
// Purpose-
//       History description.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef HISTORY_H_INCLUDED
#define HISTORY_H_INCLUDED

#ifndef RESULT_H_INCLUDED
#include "Result.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       PokerHistory
//
// Purpose-
//       PokerHistory object descriptor.
//
//----------------------------------------------------------------------------
class PokerHistory                  // PokerHistory
{
//----------------------------------------------------------------------------
// PokerHistory::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum                                // Generic enum
{  DEFAULT_SIZE= 100                // The default PokerHistory table size
}; // enum

//----------------------------------------------------------------------------
// PokerHistory::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~PokerHistory( void );           // Destructor
   PokerHistory( void );            // Default constructor
   PokerHistory(                    // Constructor
     int               count);      // Number of array elements

private:                            // Bitwise copy is prohibited
   PokerHistory(const PokerHistory&); // Disallowed copy constructor
   PokerHistory& operator=(const PokerHistory&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// PokerHistory::Accessor methods
//----------------------------------------------------------------------------
public:
inline void                         // Exception iff error
   isValid(                         // Verify PokerHistory index
     int               ago= 0) const; // Verify this index

inline int                          // The check count
   getCheckCount(                   // Get check count
     int               ago= 0) const; // This many ago

inline int                          // The call amount
   getCallAmount(                   // Get call amount
     int               ago= 0) const; // This many ago

inline int                          // The call count
   getCallCount(                    // Get call count
     int               ago= 0) const; // This many ago

inline int                          // The raise amount
   getRaiseAmount(                  // Get raise amount
     int               ago= 0) const; // This many ago

inline int                          // The raise count
   getRaiseCount(                   // Get raise count
     int               ago= 0) const; // This many ago

inline int                          // The PokerHistory table size
   getSize( void ) const;           // Get PokerHistory table size

//----------------------------------------------------------------------------
// PokerHistory::Methods
//----------------------------------------------------------------------------
public:
virtual void
   create( void );                  // Create a new PokerHistory element

inline void
   call(                            // Update the call count
     int               amount);     // The amount of the call

inline void
   check( void );                   // Update the check count

inline void
   raise(                           // Update the raise count
     int               amount);     // The amount of the raise

inline void
   reset( void );                   // Reset the History

//----------------------------------------------------------------------------
// PokerHistory::Attributes
//----------------------------------------------------------------------------
protected:
   int                 count;       // Number of array elements
   int                 used;        // Number of array elements used

   int*                checkCount;  // Number of checks

   int*                callCount;   // Number of calls
   int*                callAmount;  // Total value of the calls

   int*                raiseCount;  // Number of raises
   int*                raiseAmount; // Total value of the raise
}; // class PokerHistory

//----------------------------------------------------------------------------
//
// Class-
//       PokerPlayerHistory
//
// Purpose-
//       PokerPlayerHistory object descriptor.
//
//----------------------------------------------------------------------------
class PokerPlayerHistory : public PokerHistory // PokerPlayerHistory
{
//----------------------------------------------------------------------------
// PokerPlayerHistory::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~PokerPlayerHistory( void );     // Destructor
   PokerPlayerHistory( void );      // Default constructor
   PokerPlayerHistory(              // Constructor
     int               count);      // Number of array elements

private:                            // Bitwise copy is prohibited
   PokerPlayerHistory(const PokerPlayerHistory&); // Disallowed copy constructor
   PokerPlayerHistory& operator=(const PokerPlayerHistory&); // Disallowed operator=

//----------------------------------------------------------------------------
// PokerPlayerHistory::Accessor methods
//----------------------------------------------------------------------------
public:
inline double                       // The Rating
   getRating(                       // Get Rating
     int               ago= 0) const; // This many ago

inline PokerResult                  // The PokerResult
   getResult(                       // Get PokerResult
     int               ago= 0) const; // This many ago

inline void
   setResult(                       // Set the result of the hand
     PokerResult       result,      // The result
     double            rating);     // The rating for the result

//----------------------------------------------------------------------------
// PokerPlayerHistory::Methods
//----------------------------------------------------------------------------
public:
virtual void
   create( void );                  // Create a new history element

//----------------------------------------------------------------------------
// PokerPlayerHistory::Attributes
//----------------------------------------------------------------------------
protected:
   PokerResult*        result;      // Hand resultant
   double*             rating;      // Hand rating (-1.0 iff unknown)
}; // class PokerPlayerHistory

#include "History.i"

#endif // HISTORY_H_INCLUDED
