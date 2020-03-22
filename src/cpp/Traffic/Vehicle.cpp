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
//       Vehicle.cpp
//
// Purpose-
//       Vehicle object implementation.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <com/Debug.h>

#include "Vehicle.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard-Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
static const double    SECONDS_PER_HOUR= 3600; // Number of seconds per hour

//----------------------------------------------------------------------------
//
// Method-
//       Vehicle::~Vehicle
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Vehicle::~Vehicle( void )        // Destructor
{
   IFHCDM(debugf("Vehicle(%p)::~Vehicle()\n", this);)
}

//----------------------------------------------------------------------------
//
// Method-
//       Vehicle::Vehicle
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Vehicle::Vehicle(                // Constructor
     double            exit,        // Exit mile marker
     double            lane,        // Lane
     double            pos,         // Position
     double            vel)         // Velocity
:  List<Vehicle>::Link()
,  passed(0)
,  passes(0)
,  laneChanges(0)
,  deltaLane(0.0)
,  deltaPos(0.0)
,  deltaVel(0.0)
{
   IFHCDM(debugf("Vehicle(%p)::Vehicle() %2.0f %8.2f\n", this, lane, pos);)

   this->exit= exit;
   this->lane= lane;
   this->pos=  pos;
   this->vel=  vel;
}

//----------------------------------------------------------------------------
//
// Method-
//       Vehicle::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Vehicle::debug(                  // Debugging display
     const char*       text)        // Associated text
{
   debugf("Vehicle(%p)::debug(%s)\n", this, text);
   debugf("..%8d passed\n", passed);
   debugf("..%8d passes\n", passes);
   debugf("..%8d laneChanges\n", laneChanges);
   debugf("..%8.1f exit\n", exit);
   debugf("..%8.1f lane\n", lane);
   debugf("..%8.1f pos\n",  pos);
   debugf("..%8.1f vel\n",  vel);
   debugf("..%8.6f deltaLane\n", deltaLane);
   debugf("..%8.6f deltaPos\n",  deltaPos);
   debugf("..%8.6f deltaVel\n",  deltaVel);
}

//----------------------------------------------------------------------------
//
// Method-
//       Vehicle::isPastExit
//
// Purpose-
//       Has Vehicle past its exit?
//
//----------------------------------------------------------------------------
int                                 // TRUE iff past exit
   Vehicle::isPastExit( void ) const// Has vehicle past its exit?
{
   if( vel >= 0.0 )
   {
     if( pos > exit )
       return TRUE;
   }
   else
   {
     if( pos < exit )
       return TRUE;
   }

   return FALSE;
}

//----------------------------------------------------------------------------
//
// Method-
//       Vehicle::interval_prepare
//
// Purpose-
//       Prepare for interval
//
//----------------------------------------------------------------------------
void
   Vehicle::interval_prepare(       // Prepare for interval
     double            interval)    // (The interval)
{
   // LIMITED FUNCTIONALITY
   deltaPos= (vel / SECONDS_PER_HOUR) * interval;
   pos += deltaPos;
}

//----------------------------------------------------------------------------
//
// Method-
//       Vehicle::interval_update
//
// Purpose-
//       Update for interval
//
//----------------------------------------------------------------------------
void
   Vehicle::interval_update( void ) // Update for interval
{
   // LIMITED FUNCTIONALITY
}

//----------------------------------------------------------------------------
//
// Method-
//       Vehicle::pass
//
// Purpose-
//       Update for interval
//
//----------------------------------------------------------------------------
void
   Vehicle::pass(                   // Pass
     Vehicle*          that)        // This Vehicle
{
// debugf("Vehicle(%p)::pass(%p) %4.1f %4.1f %8.4f %8.4f\n", this, that,
//        this->getVelocity(), that->getVelocity(),
//        this->getPosition(), that->getPosition());

   this->passes++;
   that->passed++;
}

