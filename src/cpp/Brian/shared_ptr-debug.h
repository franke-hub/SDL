//----------------------------------------------------------------------------
//
//       Copyright (c) 2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       shared_ptr-debug.h
//
// Purpose-
//       Shared_ptr debugging control.
//
// Last change date-
//       2025/01/20
//
//----------------------------------------------------------------------------
#ifndef _SHARED_PTR_DEBUG_H_INCLUDED
#define _SHARED_PTR_DEBUG_H_INCLUDED

// When defined, USE_DEBUG_PTR activates shared_ptr debugging diagnostics.
// (Swap the next two lines to select or remove its definition.)
#undef  USE_DEBUG_PTR               // (Sans shared_ptr debugging)
#define USE_DEBUG_PTR               // (With shared_ptr debugging)
#include "pub/bits/diag-shared_ptr.i"

#endif // _SHARED_PTR_DEBUG_H_INCLUDED
