//----------------------------------------------------------------------------
//
//       Copyright (c) 2022-2025 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
// SPDX-License-Identifier: LGPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       dev/bits/devconfig.h
//
// Purpose-
//       Configuration control macros.
//
// Last change date-
//       2025/09/14 (Default deactivated)
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HTTP_BITS_DEVCONFIG_H_INCLUDED
#define _LIBPUB_HTTP_BITS_DEVCONFIG_H_INCLUDED

// When defined, USE_DEBUG_PTR activates shared_ptr debugging diagnostics.
// (Swap the next two lines to select or remove its definition.)
#define USE_DEBUG_PTR               // (Activate shared_ptr diagnostics)
#undef  USE_DEBUG_PTR               // (Deactivate shared_ptr diagnostics)
#include "pub/bits/diag-shared_ptr.i"

#endif // _LIBPUB_HTTP_BITS_DEVCONFIG_H_INCLUDED
