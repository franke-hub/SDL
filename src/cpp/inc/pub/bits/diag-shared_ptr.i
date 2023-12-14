//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       pub/bits/diag-shared_ptr.i
//
// Purpose-
//       Diagnostic activation control
//
// Last change date-
//       2023/12/04
//
// Implementation notes-
//       This file is useful to debug failure to delete shared_ptr objects.
//       Include it to (temporarily) to debug this problem or to simply
//       display shared_ptr usage.
//
//       For each object that contains shared_ptr objects, add
//         INS_DEBUG_OBJ(name) // In constructor
//         REM_DEBUG_OBJ(name) // In destructor (name ignored)
//
//       When a constructor isn't used, add
//         INS_DEBUG_MAP(name, that) // When created
//         REM_DEBUG_MAP(name, that) // When destroyed
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_BITS_DIAG_SHARED_PTR_I_INCLUDED
#define _LIBPUB_BITS_DIAG_SHARED_PTR_I_INCLUDED

#ifdef USE_DEBUG_PTR //- - - - - - - - - - - - - - - - - - - - - - - - - - - -
#include "pub/diag-shared_ptr.h"    // Include *before* redefines

#  define make_shared pub_diag::make_debug
#  define shared_ptr  pub_diag::debug_ptr
#  define weak_ptr    pub_diag::dweak_ptr

#  define INS_DEBUG_OBJ(x) { std::pub_diag::Debug_ptr::insert(this, x); }
#  define REM_DEBUG_OBJ(x) { std::pub_diag::Debug_ptr::remove(this); }
#  define INS_DEBUG_MAP(x, that) { std::pub_diag::Debug_ptr::insert(that, x); }
#  define REM_DEBUG_MAP(x, that) { std::pub_diag::Debug_ptr::remove(that); }

#else // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#  define INS_DEBUG_OBJ(x)
#  define REM_DEBUG_OBJ(x)
#  define INS_DEBUG_MAP(x, that)
#  define REM_DEBUG_MAP(x, that)
#endif //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#endif // _LIBPUB_BITS_DIAG_SHARED_PTR_I_INCLUDED
