//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2021 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/pub/macro/bits/usrconfig.h
//
// Purpose-
//       Configuration controlled user macros.
//
// Last change date-
//       2022/04/07
//
//----------------------------------------------------------------------------
#ifndef _PUB_BITS_USRCONFIG_H_INCLUDED
#define _PUB_BITS_USRCONFIG_H_INCLUDED

#include <pub/bits/pubconfig.h>

//----------------------------------------------------------------------------
// Standard constants
//----------------------------------------------------------------------------
#define _PUB_NAMESPACE _LIBPUB_NAMESPACE // I.E. `pub`

//----------------------------------------------------------------------------
// Attributes
//----------------------------------------------------------------------------
#define ATTRIB_NORETURN         _LIBPUB_NORETURN
#define ATTRIB_PRINTF(fmt, arg) _LIBPUB_PRINTF(fmt, arg)
#endif // _PUB_BITS_USRCONFIG_H_INCLUDED
