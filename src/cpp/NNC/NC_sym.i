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
//       NC_sym.i
//
// Purpose-
//       NC_sym inline functions.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NC_SYM_I_INCLUDED
#define NC_SYM_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       ::operator<<(ostream&, const NC_sym&)
//
// Purpose-
//       Write the list.
//
//----------------------------------------------------------------------------
inline ostream&
   operator<<(
     ostream&          os,
     const NC_sym&     it)
{
   return it.toStream(os);
}

#endif // NC_SYM_I_INCLUDED
