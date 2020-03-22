//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Function.h
//
// Purpose-
//       Describes some "Function objects"
//
// Last change date-
//       2007/01/01
//
// Implementation notes-
//       "Function objects" contain the function call operator, i.e. operator()
//
//----------------------------------------------------------------------------
#ifndef FUNCTION_H_INCLUDED
#define FUNCTION_H_INCLUDED

//----------------------------------------------------------------------------
//
// Struct-
//       gtSTR
//
// Purpose-
//       Define greater than operator for C-strings.
//
//----------------------------------------------------------------------------
struct gtSTR
{
   bool operator()(const char* l, const char* r) const
   {
     return strcmp(l, r) > 0;
   }
};

//----------------------------------------------------------------------------
//
// Struct-
//       ltSTR
//
// Purpose-
//       Define less than operator for C-strings.
//
//----------------------------------------------------------------------------
struct ltSTR
{
   bool operator()(const char* l, const char* r) const
   {
     return strcmp(l, r) < 0;
   }
};

#endif // FUNCTION_H_INCLUDED
