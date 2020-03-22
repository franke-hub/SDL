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
//       NC__con.cpp
//
// Purpose-
//       Neural Net Compiler: CONSTANT statement.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <com/Debug.h>
#include <com/define.h>
#include <com/syslib.h>

#include "NC_com.h"
#include "NC_sys.h"

#include "NN_com.h"
#include "NN_psv.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NC__CON " // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __BRINGUP__           FALSE // Bringup diagnostics?

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#include "NCdiag.h"                 // Diagnostic macros

//----------------------------------------------------------------------------
//
// Subroutine-
//       NC__CON
//
// Purpose-
//       Process CONSTANT statement.
//
//----------------------------------------------------------------------------
extern void
   nc__con(                         // Process CONSTANT statement
     const char*       inpbuf,      // Current buffer
     int               inpndx)      // Current buffer index
{
   //-------------------------------------------------------------------------
   // Convert the CONSTANT statement into NEURON[constant]
   //-------------------------------------------------------------------------
   strcpy(NC_COM.exprbuff, "NEURON[constant]");
   strcpy(NC_COM.exprbuff+16, inpbuf+inpndx);
   strcpy(NC_COM.stmtbuff, NC_COM.exprbuff);

   //-------------------------------------------------------------------------
   // Process the converted statement
   //-------------------------------------------------------------------------
   nc__neu(NC_COM.stmtbuff, 6);     // Process the statement
}

