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
//       Xcb/Keysym.h
//
// Purpose-
//       Wrapper for X11/keysymdef.h
//
// Last change date-
//       2020/09/06
//
//----------------------------------------------------------------------------
#ifndef XCB_KEYSYM_H_INCLUDED
#define XCB_KEYSYM_H_INCLUDED

#define XK_XKB_KEYS                 // Include extension keys
#define XK_MISCELLANY               // Include miscellaneous keys
#include <X11/keysymdef.h>          // For Keysym definitions
#undef  XK_XKB_KEYS
#undef  XK_MISCELLANY

#endif // XCB_KEYSYM_H_INCLUDED
