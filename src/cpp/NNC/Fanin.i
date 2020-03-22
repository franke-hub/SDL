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
//       Fanin.i
//
// Purpose-
//       Fanin inline functions.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef FANIN_I_INCLUDED
#define FANIN_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       Fanin::index
//
// Purpose-
//       Compute Vaddr of Fanin[x]
//
//----------------------------------------------------------------------------
NN::Vaddr                           // Resultant offset
   Fanin::index(                    // Compute Vaddr(Fanin[index])
     NN::Vaddr         base,        // Base Fanin address
     unsigned          index)       // Element index
{
   return base + index*sizeof(Fanin);
}

#endif // FANIN_I_INCLUDED
