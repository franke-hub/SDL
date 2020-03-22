//----------------------------------------------------------------------------
//
//       Copyright (C) 2002 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       common.h
//
// Purpose-
//       Describe functions missing on one platform or another.
//
// Last change date-
//       2002/07/01
//
//----------------------------------------------------------------------------
#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Subroutine-                                                              */
/*       memicmp                                                            */
/*                                                                          */
/* Purpose-                                                                 */
/*       memcmp, ignoring case.                                             */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifndef _OS_WIN
int                                 /* Resultant                            */
   memicmp(                         /* Memory compare, ignoring case        */
     const char*     string1,       /* Source string                        */
     const char*     string2,       /* Compare string                       */
     unsigned        length);       /* Length                               */
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Subroutine-                                                              */
/*       stristr                                                            */
/*                                                                          */
/* Purpose-                                                                 */
/*       strstr, ignoring case.                                             */
/*                                                                          */
/*--------------------------------------------------------------------------*/
char*                               /* Resultant                            */
   stristr(                         /* Search for substring, ignoring case  */
     const char*     string,        /* Source string                        */
     const char*     substr);       /* Substring                            */

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Subroutine-                                                              */
/*       strrev                                                             */
/*                                                                          */
/* Purpose-                                                                 */
/*       Reverse a string.                                                  */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifndef _OS_WIN
char*                               /* Resultant                            */
   strrev(                          /* Memory compare, ignoring case        */
     char*           string);       /* Source string                        */
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* COMMON_H_INCLUDED */
