//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Ecode.cpp
//
// Purpose-
//       Ecode object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <com/define.h>
#include <com/Debug.h>

#include "com/Ecode.h"

//----------------------------------------------------------------------------
//
// Method-
//       Ecode::~Ecode
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Ecode::~Ecode( void )            // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Ecode::Ecode
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   Ecode::Ecode( void )             // Default constructor
:  ecode(EC_0)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Ecode::getEcode
//
// Purpose-
//       Extract the last encountered error number.
//
//----------------------------------------------------------------------------
int                                 // The last error code
   Ecode::getEcode( void ) const    // Extract the error code
{
   return ecode;
}

//----------------------------------------------------------------------------
//
// Method-
//       Ecode::setEcode
//
// Purpose-
//       Set the error number.
//
//----------------------------------------------------------------------------
int                                 // The last error code
   Ecode::setEcode(                 // Set the error code
     int               value)       // To this value
{
   ecode= value;
   return value;
}

