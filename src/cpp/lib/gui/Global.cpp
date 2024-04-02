//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       gui/Global.cpp
//
// Purpose-
//       Instantiate externals.
//
// Last change date-
//       2024/03/31
//
// Implementation notes-
//       Only Window.cpp invokes xcbcheck and only xcbcheck invoked xcbdebug
//
//----------------------------------------------------------------------------
#include <pub/Debug.h>              // For Debug object
#include <pub/Trace.h>              // For Trace object

#include "gui/Global.h"             // Implementation class

using pub::Debug;                   // For Debug object
using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Global data areas
//----------------------------------------------------------------------------
int                    gui::opt_hcdm= false; // Hard Core Debug Mode?
const char*            gui::opt_test= nullptr; // Run bringup test?
int                    gui::opt_verbose= -1; // Verbosity, default NONE

namespace gui {
//----------------------------------------------------------------------------
//
// Subroutine-
//       gui::xcberror
//
// Purpose-
//       XCB error diagnostic display.
//
//----------------------------------------------------------------------------
void
   xcberror(                        // XCB error disagnostic display
     xcb_generic_error_t* E)        // The error response
{
   errorf("XCB error(%u) id(%u) op[%u,%u]\n", E->error_code
         , E->resource_id, E->major_code, E->minor_code);
}
}  // namespace gui
