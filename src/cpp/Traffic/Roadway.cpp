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
//       Roadway.cpp
//
// Purpose-
//       Roadway object implementation.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <com/Debug.h>

#include "Roadway.h"
#include "Vehicle.h"

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
static const double    EPSILON= 0.01; // Allowed error delta

//----------------------------------------------------------------------------
//
// Method-
//       Roadway::~Roadway
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Roadway::~Roadway( void )        // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Roadway::Roadway
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Roadway::Roadway(                // Constructor
     int               posEntryC,   // Number of entries
     int               posExitsC,   // Number of exits
     double            posLength,   // Roadway length
     const double*     posEntry,    // (Entry mile marker)
     const double*     posExits,    // (Exit mile marker)
     const double*     posLanes,    // (Mile marker, lane count)
     const double*     posLimit)    // (Mile marker, speed limit)
:  time(0.0)
,  removef(NULL)
,  pos()
{
   this->posEntryC= posEntryC;
   this->posExitsC= posExitsC;
   this->posLength= posLength;
   this->posEntry=  posEntry;
   this->posExits=  posExits;
   this->posLanes=  posLanes;
   this->posLimit=  posLimit;
}

//----------------------------------------------------------------------------
//
// Method-
//       Roadway::check
//
// Purpose-
//       Debugging check
//
//----------------------------------------------------------------------------
void
   Roadway::check( void )           // Debugging check
{
   Vehicle* vehicle= getVehicle();
   if( vehicle != NULL )
     vehicle= vehicle->getNext();

   int errorCount= 0;
   while( vehicle != NULL )
   {
     Vehicle* prev= vehicle->getPrev();
     if( (vehicle->getPosition() - prev->getPosition()) > EPSILON )
     {
       debugf("Roadway(%p)::check() %p %p\n", this, vehicle, prev);
       errorCount++;
     }

     vehicle= vehicle->getNext();
   }

   if( errorCount )
   {
     debug();
     throw "Roadway::check";
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Roadway::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Roadway::debug( void )           // Debugging display
{
   debugf("Roadway(%p)::debug()\n", this);

   int index= 0;
   Vehicle* vehicle= getVehicle();
   while( vehicle != NULL )
   {
     index++;
     debugf("[%3d] %p %2.2f %10.4f %4.2f\n", index, vehicle,
            vehicle->getLane(), vehicle->getPosition(), vehicle->getVelocity());

     vehicle= vehicle->getNext();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Roadway::getVehicle
//
// Purpose-
//       Get head Vehicle*
//
//----------------------------------------------------------------------------
Vehicle*
   Roadway::getVehicle( void ) const // Get head Vehicle
{
   return pos.getHead();
}

//----------------------------------------------------------------------------
//
// Method-
//       Roadway::insert
//
// Purpose-
//       Insert Vehicle onto Roadway
//
//----------------------------------------------------------------------------
void
   Roadway::insert(                 // Insert Vehicle
     Vehicle*          vehicle)     // (This Vehicle)
{
// debugf("%10.2f INSERTS %p %6.2f %4d %4d %4d\n", time, vehicle, vehicle->getVelocity(),
//        vehicle->getPasses(), vehicle->getPassed(), vehicle->getLaneChanges());

   if( vehicle->getPosition() == 0.0 )
   {
     pos.fifo(vehicle);
     return;
   }

   Vehicle* prev= NULL;
   Vehicle* next= pos.getHead();
   while( next != NULL && next->getPosition() > vehicle->getPosition() )
   {
     prev= next;
     next= next->getNext();
   }

   pos.insert(prev, vehicle, vehicle);
}

//----------------------------------------------------------------------------
//
// Method-
//       Roadway::remove
//
// Purpose-
//       Remove Vehicle from Roadway
//
//----------------------------------------------------------------------------
void
   Roadway::remove(                 // Remove Vehicle
     Vehicle*          vehicle)     // (This Vehicle)
{
   pos.remove(vehicle, vehicle);

   if( removef != NULL )
     removef(vehicle, this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Roadway::interval
//
// Purpose-
//       Process Roadway interval
//
//----------------------------------------------------------------------------
void
   Roadway::interval(               // Process Roadway interval
     double            deltaT)      // Of this length
{
   // Update each vehicle
   Vehicle* vehicle= getVehicle();
   while( vehicle != NULL )
   {
     vehicle->interval_prepare(deltaT);
     vehicle= vehicle->getNext();
   }

   // Account for passed Vehicles
   vehicle= getVehicle();
   while( vehicle != NULL )
   {
     vehicle->interval_update();

     Vehicle* prev= vehicle->getPrev();
     if( prev != NULL && vehicle->getPosition() > prev->getPosition() )
     {
       vehicle->pass(prev);

       pos.remove(vehicle, vehicle);
       pos.insert(prev->getPrev(), vehicle, vehicle);
     }

     Vehicle* next= vehicle->getNext();
     if( vehicle->isPastExit() )
     {
       remove(vehicle);
       delete vehicle;
     }

     vehicle= next;
   }

   check();                         // Debugging check
   time += deltaT;                  // Current time
}

