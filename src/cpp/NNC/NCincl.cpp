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
//       NCincl.cpp
//
// Purpose-
//       Neural Net Compiler: Include file
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <com/Debug.h>
#include <com/define.h>
#include <com/syslib.h>

#include "NC_com.h"
#include "NC_ifd.h"
#include "NC_sys.h"

#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NCINCL  " // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __BRINGUP__           FALSE // Bringup diagnostics?

//----------------------------------------------------------------------------
//
// Subroutine-
//       missingEnd
//
// Purpose-
//       Handle missing end statements.
//
//----------------------------------------------------------------------------
static void
   missingEnd(                      // Handle missing end statements
     NC_ifd*           ptrifd)      // Pointer to input descriptor
{
   NC_BeGroupSymbol* ptrbeg;        // The begin group descriptor

   for(;;)                          // Handle missing end statements
   {
     ptrbeg= NC_COM.begroup;        // Get the active element
     if (ptrbeg == NULL)            // If this is the last one
       return;                      // Return, function complete
     if (ptrbeg->source != ptrifd)  // If the begin is not for the active file
       return;                      // Return, function complete

     sprintf(NC_COM.word0, "%s:%d",
             ptrifd->filenm, ptrbeg->lineno);

     NCmess(NC_msg::ID_EndMissing, 1, NC_COM.word0);

     NC_COM.grpstak.remq();         // Remove the next element
     NC_COM.begroup= (NC_BeGroupSymbol*)NC_COM.grpstak.getHead();
     free(ptrbeg);                  // Release the prior element
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NCINCL
//
// Purpose-
//       Include a source file.
//
// Notes-
//       This routine is invoked recursively.
//
//----------------------------------------------------------------------------
extern void
   ncincl(                          // Include a source file
     const char*       filenm)      // Pointer to source file name
{
   NC_BeGroupSymbol*   ptrbeg;      // Pointer to begin descriptor
   NC_ifd*             ptrifd;      // Pointer to input descriptor

   int                 rc;          // Called routine return code

   //-------------------------------------------------------------------------
   // Open the source file
   //-------------------------------------------------------------------------
   ptrifd= NC_opn(filenm);          // Open the input file
   if (ptrifd == NULL)              // If file cannot be opened
     return;                        // Exit, function complete

   //-------------------------------------------------------------------------
   // Parse the source file
   //-------------------------------------------------------------------------
   NC_COM.dummyDebug->ifd= ptrifd;
   for(;;)                          // Parse the source file
   {
     NC_COM.debug= NC_COM.dummyDebug;
     NC_COM.debug->lineNumber= ptrifd->lineno;
     NC_COM.debug->column= ptrifd->column;

     rc= ncload(ptrifd);            // Load the next statement
     if (rc == EOF)                 // If end-of-file
       break;                       // Exit, function complete

     if (NC_COM.sw_listing)
       printf("         %s\n", NC_COM.stmtbuff);

     //-----------------------------------------------------------------------
     // Parse the statement
     //-----------------------------------------------------------------------
     ncstmt(&NC_COM.stmtbuff[0]);   // Parse the statement
   }

   //-------------------------------------------------------------------------
   // Make sure all opened begin blocks have been closed
   //-------------------------------------------------------------------------
   ptrbeg= NC_COM.begroup;          // Get the active element
   if (ptrbeg != NULL)              // If an active element exists
   {
     if (ptrbeg->source == ptrifd)  // If an end is missing
       missingEnd(ptrifd);          // Handle missing end
   }

   //-------------------------------------------------------------------------
   // Close the source file
   //-------------------------------------------------------------------------
   NC_cls(ptrifd);                  // Close the source file
}

