//----------------------------------------------------------------------------
//
//       Copyright (c) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Driver.h
//
// Purpose-
//       Driver object descriptor.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#ifndef DRIVER_H_INCLUDED
#define DRIVER_H_INCLUDED

#include <com/Link.h>

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Action;                       // Action item

//----------------------------------------------------------------------------
//
// Class-
//       Driver
//
// Purpose-
//       Driver object descriptor.
//
//----------------------------------------------------------------------------
class Driver {                      // Driver object descriptor
//----------------------------------------------------------------------------
// Driver::Attributes
//----------------------------------------------------------------------------
protected:
List<Action>           action;      // Action items

double                 exit;        // Exit mile marker
double                 vel;         // Desired (delta) velocity (mph)

//----------------------------------------------------------------------------
// Driver::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Driver( void );                 // Destructor
   Driver(                          // Initializing constructor
     double            exit,        // Exit mile marker
     double            vel);        // Velocity

private:                            // Bitwise copy is prohibited
   Driver(const Driver&);           // Disallowed copy constructor
   Driver& operator=(const Driver&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// Driver::Accessor methods
//----------------------------------------------------------------------------
public:
Action*
   getAction( void ) const;         // Get primary action item

inline double
   getExit( void ) const            // Extract exit mile marker
{
   return exit;
}

inline double
   getVelocity( void ) const        // Extract velocity
{
   return vel;
}

int                                 // TRUE iff past exit
   isPastExit( void ) const;        // Should Driver exit?

void
   setAction(                       // Set action item
     Action*           action);     // The action item

//----------------------------------------------------------------------------
// Driver::Methods
//----------------------------------------------------------------------------
public:
void
   debug(                            // Debugging display
     const char*       text= "");    // Associated text

virtual void
   interval_prepare(                 // Prepare for interval
     double            interval);    // Interval (in seconds)

virtual void
   interval_update( void );          // Update for interval
}; // class Driver

#endif // DRIVER_H_INCLUDED
