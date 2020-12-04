//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdTemp.cpp
//
// Purpose-
//       Editor: (TEMPORARY) implementation place holder
//
// Last change date-
//       2020/12/01
//
// Implementation note-
//       Used to avoid circular references.
//
//----------------------------------------------------------------------------
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp
#include <unistd.h>                 // For close, ftruncate
#include <sys/stat.h>               // For stat
#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types

#include "EdMisc.h"                 // For EdHist

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= false               // Extra brinbup diagnostics?
}; // Compilation controls

//----------------------------------------------------------------------------
//
// Method-
//       EdMisc::draw
//
// Purpose-
//       Draw the Window
//
// Implementation note-
//       ANOMOLY: The draw ONLY visible when the debugging display occurs.
//                (Looks like a timing problem.)
//       PROBLEM: USER ERROR: Expose events ignored. (Now fixed.)
//
//----------------------------------------------------------------------------
void
   EdMisc::draw( void )             // Draw the Window
{
   xcb::PT_t X= xcb::PT_t(rect.width)  - 1;
   xcb::PT_t Y= xcb::PT_t(rect.height) - 1;
   xcb_point_t points[]=
       { {0, 0}
       , {0, Y}
       , {X, Y}
       , {X, 0}
       , {0, 0}
       , {X, Y}
       };

   ENQUEUE("xcb_poly_line", xcb_poly_line_checked(c
          , XCB_COORD_MODE_ORIGIN, widget_id, drawGC, 6, points));
   if( xcb::opt_hcdm || false ) {   // ???WHY IS THIS NEEDED???
     xcb::debugf("EdMisc::draw %u:[%d,%d]\n", drawGC, X, Y);
     for(int i= 0; i<6; i++)
       xcb::debugf("[%2d]: [%2d,%2d]\n", i, points[i].x, points[i].y);
   }

// (Attempts to fix problem without expose handler.)
// ::pub::Thread::sleep(0.001);     // Does this fix the problem?  NO!
// ::pub::Thread::sleep(0.010);     // Does this fix the problem? (sometimes)
// ::pub::Thread::sleep(0.020);     // Does this fix the problem? YES!

   flush();
}
