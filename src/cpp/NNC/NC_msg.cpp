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
//       NC_msg.cpp
//
// Purpose-
//       Neural Net Compiler - Message writer.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <com/Debug.h>
#include <com/define.h>
#include <com/syslib.h>

#include "NC_com.h"
#include "NC_msg.h"
#include "NC_sys.h"
#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NC_MSG  " // Source file, for debugging

//----------------------------------------------------------------------------
//
// Method-
//       NC_msg::NC_msg
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NC_msg::NC_msg( void )           // Constructor
:  Message()
,  stopLevel(ML_Error)
,  highLevel(ML_Info)
,  showLevel(ML_Info)
,  infoCount(0)
,  warnCount(0)
,  errsCount(0)
,  sevsCount(0)
,  termCount(0)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_msg::message
//
// Purpose-
//       Write error message.
//
//----------------------------------------------------------------------------
void
   NC_msg::message(                 // Write error message
     MessageId         msgno,       // Message number
     int               argc,        // Argument count
                       ...)         // Argument array
{
   const Message::MessageLink*
                       ptrLink;     // -> MessageLink
   int                 C;           // Message type
   MessageLevel        L;           // Message level
   va_list             argv;        // Argument element

   //-------------------------------------------------------------------------
   // Format the message
   //-------------------------------------------------------------------------
   ptrLink= NC_COM.message.locate(msgno);
   C= 'X';
   if( ptrLink != NULL )
     C= *ptrLink->getText();

   switch (C)                       // Extract warning level
   {
     case 'I':                      // Informational
       L= ML_Info;
       infoCount++;
       break;

     case 'W':
       L= ML_Warn;
       warnCount++;
       break;

     case 'E':                      // Error
       L= ML_Error;
       errsCount++;
       break;

     case 'S':                      // Serious error
       L= ML_Severe;
       sevsCount++;
       break;

     default:                       // Terminating
       L= ML_Terminating;
       termCount++;
       break;
   }

   if( L > highLevel )              // If higher msglevel found
     highLevel= L;                  // Set new value

   if( L >= showLevel )             // If message to be displayed
   {
     printf("NC[%.4d] ", msgno);    // Display message header

     va_start(argv, argc);
     Message::message(msgno, argc, argv);
     va_end(argv);
   }

   if( (errsCount+sevsCount) > 32 ) // If too many errors
   {
     fprintf(stdout, "Too many errors encountered\n");
     L= ML_Terminating;
   }

   if( L >= ML_Terminating )        // If this is a terminating error
   {
     fprintf(stdout, "Compile aborted\n");
     exit(EXIT_FAILURE);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_msg::internalError
//
// Purpose-
//       Write compiler error message.
//
//----------------------------------------------------------------------------
void
   NC_msg::internalError(           // Write compiler error message
     const char*       fileName,    // File name
     int               lineNumber)  // Line number
{
   char                string[16];  // Line number string

   sprintf(string, "%d", lineNumber);

   message(ID_BugFileLine, 2, fileName, string);
}

