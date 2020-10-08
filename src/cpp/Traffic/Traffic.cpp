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
//       Traffic.cpp
//
// Purpose-
//       Traffic simulation.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <com/Debug.h>
#include <com/Random.h>

#include "Roadway.h"
#include "Vehicle.h"

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
static const double    SEC_PER_HOUR= 3600.0; // Number of seconds/hour
static const double    VEH_PER_MILE= 16.0;   // Number of vehicles/mile
static const double    MILE_PER_VEH= 1.0/16.0; // Number of miles/vehicle
static const double    SIMULATION_TIME= 6000.0; // Simulated time (seconds)

//----------------------------------------------------------------------------
// Roadway controls
//----------------------------------------------------------------------------
#define ROADWAY_LANES   2.0         // Number of lanes
#define ROADWAY_LENGTH 10.0         // Roadway length
#define ROADWAY_LIMIT  65.0         // Speed limit

static const double    INTERVAL= (1.0/16.0); // Simulation interval
static const int       POS_ENTRY_COUNT= 1; // Number of entrances
static const int       POS_EXITS_COUNT= 1; // Number of exits
static const double    POS_ENTRY[]= {0.0};
static const double    POS_EXITS[]= {ROADWAY_LENGTH};

static const double    POS_LANES[]=
{             0.0, ROADWAY_LANES
,  ROADWAY_LENGTH, ROADWAY_LANES
};

static const double    POS_LIMIT[]=
{             0.0, ROADWAY_LIMIT
,  ROADWAY_LENGTH, ROADWAY_LIMIT
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       deltaV
//
// Purpose-
//       Calculate random velocity delta.
//
//----------------------------------------------------------------------------
static double                       // Random velocity delta
   deltaV( void )                   // Get velocity delta
{
   return double(Random::standard.get() % 25) - 5.0; // Range -5 .. +20
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       remove
//
// Purpose-
//       Handle Vehicle removal from Roadway
//
//----------------------------------------------------------------------------
static void
   remove(                          // Remove vehicle from Roadway
     Vehicle*          vehicle,     // This Vehicle removed
     Roadway*          roadway)     // From this Roadway
{
   debugf("%10.2f REMOVED %p %6.2f %4d %4d %4d\n", roadway->getTime(), vehicle,
          vehicle->getVelocity(),
          vehicle->getPasses(), vehicle->getPassed(), vehicle->getLaneChanges());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(int, char**)                // Mainline code
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{
   int                 interval= 0; // Simulation interval
   Vehicle*            vehicle;     // Working Vehicle*

   Roadway roadway(POS_ENTRY_COUNT,
                   POS_EXITS_COUNT,
                   ROADWAY_LENGTH,
                   POS_ENTRY,
                   POS_EXITS,
                   POS_LANES,
                   POS_LIMIT);      // Create the roadway
   roadway.setRemove(remove);       // Set remove function

   // Add vehicles to the Roadway
   double pos= 0.0;
   while( pos < ROADWAY_LENGTH )
   {
     double lane= double(Random::standard.get()%int(ROADWAY_LANES));
     vehicle= new Vehicle(ROADWAY_LENGTH,
                          lane,
                          pos,
                          ROADWAY_LIMIT + deltaV());
     roadway.insert(vehicle);
     pos += MILE_PER_VEH;
   }
   roadway.debug();

   if( FALSE )
   {
     roadway.interval(1.0);
     roadway.debug();
     return 0;
   }

   // Run the simulation
   double MPS = ROADWAY_LIMIT/SEC_PER_HOUR; // Miles/Second
   double MPI = MPS * INTERVAL;             // Miles/Interval
   int MAX_INTERVAL= int((ROADWAY_LENGTH+MPS)/MPI); // Interval count
   MAX_INTERVAL= 1000000;
   debugf("MAX_INTERVAL %d\n", MAX_INTERVAL);

   // Number of intervals between Vehicles
   // VPS = MPS * VEH/MILE
   // VPI = VPS * INTERVAL
   // IPV = 1/VPI
   double VPS = MPS * VEH_PER_MILE; // Vehicles/Second
   double VPI = VPS * INTERVAL;     // Vehicles/Interval
   int NEW_INTERVAL= int(1.0/VPI) + 1; // Intervals/Vehicle
   int insertion= NEW_INTERVAL;
   debugf("NEW_INTERVAL %d\n", NEW_INTERVAL);

   for(interval= 1; interval < MAX_INTERVAL; interval++)
   {
     try {
       roadway.interval(INTERVAL);
     } catch(const char* X) {
       debugf("Exception(%s)\n", X);
       break;
     } catch(...) {
       debugf("Exception(...)\n");
       break;
     }

     double time= roadway.getTime();
     if( time >= SIMULATION_TIME )
       break;

     int interval_time= int(time*1000.0);
     if( (interval_time % 100000) == 0 )
       debugf("%10.2f Time %8d\n", time, interval);

     insertion--;
     if( insertion < 0 )
     {
       insertion= NEW_INTERVAL;
       vehicle= new Vehicle(ROADWAY_LENGTH,
                            0.0,
                            0.0,
                            ROADWAY_LIMIT + deltaV());
       roadway.insert(vehicle);
     }
   }
   debugf("%10.2f DONE %8d\n", roadway.getTime(), interval);
   roadway.debug();

   return 0;
}

