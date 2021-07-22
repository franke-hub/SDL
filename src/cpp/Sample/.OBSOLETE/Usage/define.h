//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       define.h
//
// Purpose-
//       Standard C++ defines
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef DEFINE_H_INCLUDED
#define DEFINE_H_INCLUDED

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

#endif // DEFINE_H_INCLUDED
