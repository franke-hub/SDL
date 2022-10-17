//----------------------------------------------------------------------------
//
//       Copyright (c) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       dev/bits/devconfig.h
//
// Purpose-
//       Configuration control macros.
//
// Last change date-
//       2022/10/16
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HTTP_DEVCONFIG_H_INCLUDED
#define _LIBPUB_HTTP_DEVCONFIG_H_INCLUDED

#if true                            // Debugging mode?
#define USE_DEBUG_PTR               // (Swap lines to disable/enable)
#undef  USE_DEBUG_PTR               // (Swap lines to enable/disable)

#include "pub/bits/Diagnostic.i"
#endif

#endif // _LIBPUB_HTTP_DEVCONFIG_H_INCLUDED
