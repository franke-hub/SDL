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
//       Roadway.h
//
// Purpose-
//       Roadway object descriptor.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#ifndef ROADWAY_H_INCLUDED
#define ROADWAY_H_INCLUDED

#include <com/List.h>

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Vehicle;                      // Vehicle object

//----------------------------------------------------------------------------
//
// Class-
//       Roadway
//
// Purpose-
//       Roadway object descriptor.
//
//----------------------------------------------------------------------------
class Roadway {                     // Roadway object descriptor
//----------------------------------------------------------------------------
// Roadway::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef void Removef(Vehicle*,Roadway*); // Remove function

//----------------------------------------------------------------------------
// Roadway::Attributes
//----------------------------------------------------------------------------
protected:
double                 time;        // Current time
Removef*               removef;     // Remove function

// Positive direction controls
List<Vehicle>          pos;         // The ordered list of vehicles
int                    posEntryC;   // Number of entries
int                    posExitsC;   // Number of exits
double                 posLength;   // Roadway length
const double*          posEntry;    // (Entry mile marker)
const double*          posExits;    // (Exit mile marker)
const double*          posLanes;    // (Mile marker, lane count)
const double*          posLimit;    // (Mile marker, speed limit)

//----------------------------------------------------------------------------
// Roadway::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Roadway( void );                // Destructor
   Roadway(                         // Constructor
     int               posEntryC,   // Number of entries
     int               posExitsC,   // Number of exits
     double            posLength,   // Roadway length
     const double*     posEntry,    // (Entry mile marker)
     const double*     posExits,    // (Exit mile marker)
     const double*     posLanes,    // (Mile marker, lane count)
     const double*     posLimit);   // (Mile marker, speed limit)

private:                            // Bitwise copy is prohibited
   Roadway(const Roadway&);         // Disallowed copy constructor
   Roadway& operator=(const Roadway&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// Roadway::Accessor methods
//----------------------------------------------------------------------------
public:
double
   getTime( void ) const             // Get current time
{
   return time;
}

void
   setRemove(                        // Set remove function
     Removef*          removef= NULL) // New remove function
{
   this->removef= removef;
}

//----------------------------------------------------------------------------
// Roadway::Methods
//----------------------------------------------------------------------------
public:
void
   check( void );                    // Debugging check

void
   debug( void );                    // Debugging display

//----------------------------------------------------------------------------
// Roadway::getVehicle()
//
// Get first Vehicle on the Roadway
//----------------------------------------------------------------------------
public:
virtual Vehicle*                     // The first Vehicle
   getVehicle( void ) const;         // Get first Vehicle on the Roadway

//----------------------------------------------------------------------------
// Roadway::insert()
//
// Add a vehicle to the roadway
//----------------------------------------------------------------------------
public:
virtual void
   insert(                           // Add a vehicle to the Roadway
     Vehicle*          vehicle);     // The vehicle to add

//----------------------------------------------------------------------------
// Roadway::remove()
//
// Remove a vehicle from the roadway
//----------------------------------------------------------------------------
public:
virtual void
   remove(                           // Remove a vehicle from the Roadway
     Vehicle*          vehicle);     // The vehicle to remove

//----------------------------------------------------------------------------
// Roadway::interval()
//
// Process a roadway interval.
//----------------------------------------------------------------------------
public:
virtual void
   interval(                        // Process interval
     double            interval);   // Interval (in seconds)
}; // class Roadway

#endif // ROADWAY_H_INCLUDED
