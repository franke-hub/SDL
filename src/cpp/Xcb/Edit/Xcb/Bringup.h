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
//       Bringup.h
//
// Purpose-
//       Editor: Bringup: Debugging controls (imported from Xcb/Global.h)
//
// Last change date-
//       2020/09/06
//
// Implementation notes-
//       TODO: REMOVE: For BRINGUP
//
//----------------------------------------------------------------------------
#ifndef BRINGUP_H_INCLUDED
#define BRINGUP_H_INCLUDED

#include "Xcb/Global.h"             // For xcb::opt_* controls

using ::xcb::opt_hcdm;
using ::xcb::opt_test;
using ::xcb::opt_verbose;

using ::xcb::debugf;
using ::xcb::debugh;
using ::xcb::tracef;
using ::xcb::traceh;

#endif // BRINGUP_H_INCLUDED
