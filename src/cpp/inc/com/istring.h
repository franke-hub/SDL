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
/*       istring.h                                                          */
/*                                                                          */
/* Purpose-                                                                 */
/*       String extensions, ignoring case.                                  */
/*                                                                          */
/* Last change date-                                                        */
/*       2014/01/01                                                         */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifndef ISTRING_H_INCLUDED
#define ISTRING_H_INCLUDED

#include <ostream>                  // For operator <<
#include <string>                   // For char_traits<char> base class
#include <string.h>                 // Include case sensitive methods

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
     const char*       string1,     /* Source string                        */
     const char*       string2,     /* Compare string                       */
     size_t            length);     /* Length                               */
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Subroutine-                                                              */
/*       strichr                                                            */
/*                                                                          */
/* Purpose-                                                                 */
/*       strchr, ignoring case.                                             */
/*                                                                          */
/*--------------------------------------------------------------------------*/
char*                               /* Resultant                            */
   strichr(                         /* Search for character, ignoring case  */
     const char*       string,      /* Source string                        */
     int               match);      /* Match character                      */

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Subroutine-                                                              */
/*       stricmp                                                            */
/*                                                                          */
/* Purpose-                                                                 */
/*       strcmp, ignoring case.                                             */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifndef _OS_WIN
int                                 /* Resultant                            */
   stricmp(                         /* String compare, ignoring case        */
     const char*       string1,     /* Source string                        */
     const char*       string2);    /* Compare string                       */
#endif

#if defined(_OS_LINUX)
  #define stricmp strcasecmp        /* Compare strings, ignoring case       */
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
     const char*       string,      /* Source string                        */
     const char*       substr);     /* Substring                            */

#ifdef __cplusplus
} // extern "C" (begin class istring) =======================================

//----------------------------------------------------------------------------
//
// Struct-
//       ichar_traits
//
// Purpose-
//       Define the case-insensitive case comparison functions.
//
//----------------------------------------------------------------------------
struct ichar_traits : public std::char_traits<char> {
static bool eq(char L, char R) { return toupper(L) == toupper(R); }
static bool ne(char L, char R) { return toupper(L) != toupper(R); }
static bool lt(char L, char R) { return toupper(L) <  toupper(R); }

static int
   compare(                      // Case-insensitive compare
     const char*       L,
     const char*       R,
     size_t            n)
{  return memicmp(L, R, n); }

static const char*               // Case-insensitive find
   find(
     const char*       s,
     int               n,
     char              a)
{
   char A= toupper(a);

   while( n-- > 0 && toupper(*s) != A )
     ++s;

   return s;
}
}; // struct ichar_traits

//----------------------------------------------------------------------------
//
// Class-
//       istring
//
// Purpose-
//       The case-insensitive comparison string.
//
//----------------------------------------------------------------------------
typedef std::basic_string<char,ichar_traits> istring;

//----------------------------------------------------------------------------
//
// Function-
//       ::operator<<(ostream&, const istring&)
//
// Purpose-
//       Define global operator (ostream << istring)
//
//----------------------------------------------------------------------------
inline std::ostream&                // The updated ostream
   operator<<(                      // OUTPUT operator
     std::ostream&     os,          // The ostream
     const istring&    object)      // The istring
{
   return (os << object.c_str());
}

#endif /* #ifdef __cplusplus */
#endif /* ISTRING_H_INCLUDED */
