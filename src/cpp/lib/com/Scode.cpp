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
//       Scode.cpp
//
// Purpose-
//       Scode object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <com/define.h>
#include <com/Debug.h>

#include "com/Scode.h"

//----------------------------------------------------------------------------
//
// Method-
//       Scode::~Scode
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Scode::~Scode( void )            // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Scode::Scode
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   Scode::Scode( void )             // Default constructor
:  scode(SC_RESET)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Scode::getScode
//
// Purpose-
//       Extract the current status code.
//
//----------------------------------------------------------------------------
int                                 // The current status code
   Scode::getScode( void ) const    // Extract the status code
{
   return scode;
}

//----------------------------------------------------------------------------
//
// Method-
//       Scode::setScode
//
// Purpose-
//       Set the status code.
//
//----------------------------------------------------------------------------
int                                 // The last status code
   Scode::setScode(                 // Set the status code
     int               value)       // To this value
{
   scode= value;
   return value;
}

