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
//       NC_com.cpp
//
// Purpose-
//       Neural Net Compiler - NC Common object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <com/define.h>

#include "NC_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NC_com  " // Source file, for debugging

//----------------------------------------------------------------------------
// Global data areas
//----------------------------------------------------------------------------
NC_com*                NC_com::nc_com= NULL; // -> Global object

//----------------------------------------------------------------------------
//
// Method-
//       NC_com::NC_com
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_com::NC_com( void )           // Constructor
:  message()
,  pass1()
,  pass2()
,  passN()
,  ist(sizeof(NC_SizeofSymbol))
,  xst(sizeof(NC_SizeofSymbol))
,  srcstak()
,  srclist()
,  objlist()
,  grpstak()
{
   nc_com= this;
}

