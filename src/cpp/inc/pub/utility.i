//----------------------------------------------------------------------------
//
//       Copyright (c) 2024-2026 Frank Eskesen.
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
//       utility.i
//
// Purpose-
//       Utility inline functions.
//
// Last change date-
//       2026/01/12
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_UTILITY_I_INCLUDED
#define _LIBPUB_UTILITY_I_INCLUDED

#include <string>                   // For std::string
#include <cstdint>                  // For intptr_t
#include <cstdio>                   // For sprintf
#include <cstring>                  // For strlen, strcpy, memcpy, ...

#include <endian.h>                 // For be64toh

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
//       pub::v2s      void* to std::string
//       pub::c2v      C-string to void*
//
// Purpose-
//       Convert boolean to C-string
//       Convert integer to intptr_t
//       Convert integer to void*
//       Convert std::string to C-string
//       Convert void* to intptr_t
//       Convert void* to std::string
//       Convert C-string to void*
//
// Implementation notes-
//       The subroutines are simple cast operations. Their names are as short
//       as possible while still containing still containing some meaning.
//       They have been found useful in Trace::trace invocations, since
//       these calls have all void* or all intptr_t parameters but not all
//       data match these types. The b2c and s2c conversions are useful in
//       printf/debugf invocations, mainly to reduce typing.
//
//       Given bool b and string s, a printf statement could be:
//         printf("%s(%s)\n", s.c_str(), b ? "true" : "false"); // or
//         printf("%s(%s)\n", s2c(s), b2c(b)); // (With a lot less typing)
//         // (Of course, you'll need to import pub or pub::s2c and pub::b2c
//         // into your namespace so it's only useful if you use them a lot.)
//
//----------------------------------------------------------------------------

//- - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - - - - -
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
static inline std::string
   v2s(void* v)
{
   intptr_t i= intptr_t(v);

   char buffer[32];
   sprintf(buffer, "0x%.6zx'%.8zx", i >> 32, i & intptr_t(0x00'ffff'ffff));
   return std::string(buffer);
}

static inline std::string
   v2s(const void* v)
{  return v2s(const_cast<void*>(v)); }

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
