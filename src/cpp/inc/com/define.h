/*----------------------------------------------------------------------------
**
**       Copyright (C) 2007-2014 Frank Eskesen.
**
**       This file is free content, distributed under the Lesser GNU
**       General Public License, version 3.0.
**       (See accompanying file LICENSE.LGPL-3.0 or the original
**       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
**
**--------------------------------------------------------------------------*/
/*                                                                          */
/* Title-                                                                   */
/*       define.h                                                           */
/*                                                                          */
/* Purpose-                                                                 */
/*       Standard defines, C or C++.                                        */
/*                                                                          */
/* Last change date-                                                        */
/*       2014/01/01                                                         */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifndef DEFINE_H_INCLUDED
#define DEFINE_H_INCLUDED

/*--------------------------------------------------------------------------*/
/* Standard constants                                                       */
/*--------------------------------------------------------------------------*/
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

/*--------------------------------------------------------------------------*/
/* Force consistent NULL definition                                         */
/*--------------------------------------------------------------------------*/
#undef  NULL                        /* Delete the current definition        */
#if defined(__GNUC__)               /* GNU compiler has special definition  */
#define NULL __null
#elif defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void*)0)
#endif

/*--------------------------------------------------------------------------*/
/* Attributes (compiler dependent, no effect if not supported)              */
/*--------------------------------------------------------------------------*/
#ifdef __GNUC__
   #define _ATTRIBUTE_PRINTF(fmt_parm, arg_parm) \
               __attribute__ ((format (printf, fmt_parm, arg_parm)))

   #define _ATTRIBUTE_NORETURN __attribute__ ((noreturn))
#else
   #define _ATTRIBUTE_PRINTF(fmt_parm, arg_parm)
   #define _ATTRIBUTE_NORETURN
#endif

#endif /* DEFINE_H_INCLUDED */
