//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2016 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdDraw.i
//
// Purpose-
//       Editor: EdDraw inline methods.
//
// Last change date-
//       2016/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#ifndef EDDRAW_I_INCLUDED
#define EDDRAW_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       EdDraw::getTerminal
//
// Purpose-
//       Extract the Terminal
//
//----------------------------------------------------------------------------
Terminal*                           // The Terminal
   EdDraw::getTerminal( void )      // Get the Terminal
{
   return terminal;
}

#endif // EDDRAW_I_INCLUDED
