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
//       EdLine.i
//
// Purpose-
//       Editor: EdLine inline methods.
//
// Last change date-
//       2016/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#ifndef EDLINE_I_INCLUDED
#define EDLINE_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       EdLine::getSize
//
// Purpose-
//       Return the text string length.
//
//----------------------------------------------------------------------------
unsigned                            // Length of text string
   EdLine::getSize( void ) const    // Get text string length
{
   if( text == NULL )
     return 0;

   return strlen(text);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdLine::getText
//
// Purpose-
//       Return the text string.
//
//----------------------------------------------------------------------------
const char*                         // -> Associated text string
   EdLine::getText( void ) const    // Get text string
{
   if( text == NULL )
     return "";

   return (char*)text;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdLine::setText
//
// Purpose-
//       Update the text string.
//
//----------------------------------------------------------------------------
void
   EdLine::setText(                 // Set text string
     char*             text)        // New text string
{
   this->text= text;
}

#endif // EDLINE_I_INCLUDED
