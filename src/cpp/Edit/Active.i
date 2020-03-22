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
//       Active.i
//
// Purpose-
//       Editor: Active inline methods.
//
// Last change date-
//       2016/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#ifndef ACTIVE_I_INCLUDED
#define ACTIVE_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       Active::getLine
//
// Purpose-
//       Extract the current Line
//
//----------------------------------------------------------------------------
EdLine*                             // The current Line
   Active::getLine( void ) const    // Get current Line
{
   return line;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::setLine
//
// Purpose-
//       Make another line current, but otherwise don't change it.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Active::setLine(                 // Replace the current Line
     EdRing*           edRing,      // Using this ring
     EdLine*           edLine)      // Using this line
{
   if( text != base || ring != edRing )
     return "Invalid state";

   line= edLine;
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::getRing
//
// Purpose-
//       Extract the current Ring
//
//----------------------------------------------------------------------------
EdRing*                             // The current Ring
   Active::getRing( void ) const    // Get current Ring
{
   return ring;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::getState
//
// Purpose-
//       Extract the current State
//
//----------------------------------------------------------------------------
Active::State                       // The current State
   Active::getState( void ) const   // Get the current State
{
   return State(state);
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::getText
//
// Purpose-
//       Extract the text
//
//----------------------------------------------------------------------------
const char*                         // The current text
   Active::getText( void ) const    // Get current text
{
   if( text == NULL )
     return "";

   return text;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::getUsed
//
// Purpose-
//       Extract the number of bytes used
//
//----------------------------------------------------------------------------
unsigned                            // The number of bytes used
   Active::getUsed( void ) const    // Get number of bytes used
{
   return textUsed;
}

#endif // ACTIVE_I_INCLUDED
