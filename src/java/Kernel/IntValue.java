//----------------------------------------------------------------------------
//
//       Copyright (C) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       IntValue.java
//
// Purpose-
//       (Settable) Integer value.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       IntValue
//
// Purpose-
//       Container for an integer.
//
//----------------------------------------------------------------------------
public class IntValue
{
//----------------------------------------------------------------------------
// IntValue.attributes
//----------------------------------------------------------------------------
   int                 value= 0;    // The return value

//----------------------------------------------------------------------------
// IntValue.methods
//----------------------------------------------------------------------------
int                                 // The value
   get( )                           // Get value
{
   return value;
}

void
   set(                             // Set value
     int               value)       // New value
{
   this.value= value;
}
} // class IntValue

