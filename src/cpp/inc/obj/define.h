//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       define.h
//
// Purpose-
//       Standard obj library defines
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef OBJ_DEFINE_H_INCLUDED
#define OBJ_DEFINE_H_INCLUDED

//----------------------------------------------------------------------------
// Standard constants
//----------------------------------------------------------------------------
#define _OBJ_NAMESPACE obj            // Change requires library recompile

//----------------------------------------------------------------------------
// Attributes (compiler dependent, no effect if not supported)
//----------------------------------------------------------------------------
#ifdef __GNUC__
   #define _ATTRIBUTE_PRINTF(fmt_parm, arg_parm) \
               __attribute__ ((format (printf, fmt_parm, arg_parm)))

   #define _ATTRIBUTE_NORETURN __attribute__ ((noreturn))
#else
   #define _ATTRIBUTE_PRINTF(fmt_parm, arg_parm)
   #define _ATTRIBUTE_NORETURN
#endif

#endif // OBJ_DEFINE_H_INCLUDED
