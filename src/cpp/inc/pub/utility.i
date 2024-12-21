//----------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       utility.i
//
// Purpose-
//       Utility inline functions.
//
// Last change date-
//       2024/12/20
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_UTILITY_I_INCLUDED
#define _LIBPUB_UTILITY_I_INCLUDED

#include <string>                   // For std::string
#include <endian.h>                 // For be64toh
#include <stdint.h>                 // For intptr_t
#include <string.h>                 // For strlen, strcpy, memcpy, ...

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::b2c      bool to C-string
//       pub::i2i      integer to intptr_t
//       pub::i2v      integer to void*
//       pub::s2c      std::string to C-string
//       pub::v2i      void* to intptr_t
//       pub::c2v      C-string to void*
//
// Purpose-
//       Convert boolean to C-string
//       Convert integer to intptr_t
//       Convert integer to void*
//       Convert std::string to C-string
//       Convert void* to intptr_t
//       Convert C-string to void*
//
// Implementation notes-
//       Except for the more complex c2v conversion, these routines are simple
//       shortcuts for static cast operations.
//
//----------------------------------------------------------------------------
static inline const char*           // "true" or "false"
   b2c(bool b)
{  return b ? "true" : "false"; }

//- - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - - - - -
static inline intptr_t
   i2i(intptr_t i)
{  return i; }

//- - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - - - - -
static inline void*
   i2v(intptr_t i)
{  return (void*)i; }

//- - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - - - - -
static inline const char*
   s2c(const std::string& S)
{  return S.c_str(); }

//- - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - - - - -
static inline intptr_t
   v2i(void* v)
{  return intptr_t(v); }

static inline intptr_t
   v2i(const void* v)
{  return v2i(const_cast<void*>(v)); }

//- - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - - - - -
static inline void*
   c2v(const char* _c)
{
   union {
     char      buffer[2*sizeof(intptr_t)];
     intptr_t  result[2];
   } u;

   u.result[0]= 0; u.result[1]= 0;
   if( strlen(_c) <= sizeof(intptr_t) )
     strcpy(u.buffer, _c);
   else
     memcpy(u.buffer, _c, sizeof(intptr_t));

   return i2v(be64toh(u.result[0]));
}
static_assert(sizeof(intptr_t) == 8, "c2v coding error: size checking needed");
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_UTILITY_I_INCLUDED
