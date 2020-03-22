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
//       Vehicle.h
//
// Purpose-
//       Vehicle object descriptor.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#ifndef VEHICLE_H_INCLUDED
#define VEHICLE_H_INCLUDED

#include <com/List.h>

//----------------------------------------------------------------------------
//
// Class-
//       Vehicle
//
// Purpose-
//       Vehicle object descriptor.
//
//----------------------------------------------------------------------------
class Vehicle : public List<Vehicle>::Link { // Vehicle object descriptor
//----------------------------------------------------------------------------
// Vehicle::Attributes
//----------------------------------------------------------------------------
protected:
int                    passed;      // Number of vehicles passed
int                    passes;      // Number of times passed
int                    laneChanges; // Number of lane changes

double                 exit;        // Exit mile marker
double                 lane;        // Lane (zero based)
double                 pos;         // Position (mile marker)
double                 vel;         // Velocity (mph)

double                 deltaLane;   // Lane change velocity (lanes/second)
double                 deltaPos;    // Position change
double                 deltaVel;    // Acceleration

//----------------------------------------------------------------------------
// Vehicle::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Vehicle( void );                // Destructor
   Vehicle(                         // Initializing constructor
     double            exit,        // Exit mile marker
     double            lane,        // Lane
     double            pos,         // Position
     double            vel);        // Velocity

private:                            // Bitwise copy is prohibited
   Vehicle(const Vehicle&);         // Disallowed copy constructor
   Vehicle& operator=(const Vehicle&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// Vehicle::Accessor methods
//----------------------------------------------------------------------------
public:
inline double
   getExit( void ) const            // Extract exit mile marker
{
   return exit;
}

inline double
   getLane( void ) const            // Extract lane
{
   return lane;
}

inline int
   getLaneChanges( void ) const     // Extract lane change counter
{
   return laneChanges;
}

inline int
   getPassed( void ) const          // Extract passed counter
{
   return passed;
}

inline int
   getPasses( void ) const          // Extract passes counter
{
   return passes;
}

inline double
   getPosition( void ) const        // Extract position
{
   return pos;
}

inline double
   getVelocity( void ) const        // Extract velocity
{
   return vel;
}

int                                 // TRUE iff past exit
   isPastExit( void ) const;        // Should Vehicle exit?

void
   setLane(                         // Change lane
     double            delta);      // Range +1 .. -1

void
   setVelocity(                     // Change velocity
     double            delta);      // Range +1 .. -1

//----------------------------------------------------------------------------
// Vehicle::Methods
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

virtual void
   pass(                             // Pass
     Vehicle*          vehicle);     // This Vehicle
}; // class Vehicle

#endif // VEHICLE_H_INCLUDED
