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
//       NCfile.cpp
//
// Purpose-
//       NeuralNet Compiler - file control
//
// Last change date-
//       2007/01/01
//
// Entry points-
//       NC_OPN     Open source file
//       NC_CLS     Close source file
//       NC_RD      Read source file
//
//----------------------------------------------------------------------------
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#ifdef _OS_CYGWIN
#include <io.h>
#endif

#include <com/Debug.h>
#include <com/define.h>
#include <com/syslib.h>

#include "NC_com.h"
#include "NC_sys.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NCFILE  " // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define NC_INPBUFF            16384 // Sizeof(input buffer)

//----------------------------------------------------------------------------
//
// Subroutine-
//       NC_OPN
//
// Purpose-
//       Open source file.
//
//----------------------------------------------------------------------------
extern NC_ifd*
   NC_opn(                          // Open source file
     const char*       filenm)      // Pointer to source file name
{
   NC_ifd*             ptrifd;      // Pointer to source file descriptor

   int                 fh;          // Working file handle

   //-------------------------------------------------------------------------
   // Open the source file
   //-------------------------------------------------------------------------
   fh= open(filenm,                 // Open the file
            O_RDONLY,               // (in read-only mode)
            S_IREAD);               // Permission to read
   if (fh < 0)                      // If open failed
   {
     NCmess(NC_msg::ID_IOROpen, 1, filenm); // Write error message
     return(NULL);                  // Return, open failed
   }

   //-------------------------------------------------------------------------
   // Allocate a source file descriptor
   //-------------------------------------------------------------------------
   ptrifd= (NC_ifd*)malloc(sizeof(NC_ifd)); // Allocate a desciptor entry
   if (ptrifd == NULL)              // If storage shortage
   {
     NCmess(NC_msg::ID_IORStorage, 1, filenm); // Error message
     close(fh);                     // Might as well close it
     return(NULL);                  // Return, open failed
   }
   memset(ptrifd, 0, sizeof(NC_ifd)); // Clear the block

   ptrifd->buffer= (unsigned char*)malloc(NC_INPBUFF);// Allocate the buffer
   if (ptrifd->buffer == NULL)      // If storage shortage
   {
     NCmess(NC_msg::ID_IORStorage, 1, filenm); // Error message
     free(ptrifd);                  // Might as well release it
     close(fh);                     // Might as well close it
     return(NULL);                  // Return, open failed
   }

   //-------------------------------------------------------------------------
   // Initialize the source file descriptor
   //-------------------------------------------------------------------------
   strcpy(ptrifd->filenm, filenm);  // Set the file name

   ptrifd->fh= fh;                  // Set file handle
   ptrifd->lineno= 1;               // Set initial position in file
   ptrifd->column= 1;

   //-------------------------------------------------------------------------
   // Add the file to the source lists
   //-------------------------------------------------------------------------
   NC_COM.srcfile= ptrifd;          // Set the source descriptor
   NC_COM.srclist.lifo(&ptrifd->srclink);// Add this file to the list of
                                    // source files
   NC_COM.srcstak.lifo(&ptrifd->actlink);// This source file is now active
   return(ptrifd);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NC_CLS
//
// Purpose-
//       Close source file.
//
//----------------------------------------------------------------------------
extern void
   NC_cls(                          // Close source file
     NC_ifd*           inpifd)      // Pointer to source descriptor
{
   SHSL_List<void>::Link*
                       ptrlink;     // The source descriptor's link

   //-------------------------------------------------------------------------
   // Verify closure of active file
   //-------------------------------------------------------------------------
   if (&inpifd->actlink != NC_COM.srcstak.getHead()) // If we are not closing
                                    // the active file
   {
     NCmess(NC_msg::ID_BugFileLine, 2, __SOURCE__, "0001");// This is not good
     return;
   }

   //-------------------------------------------------------------------------
   // Close the source file
   //-------------------------------------------------------------------------
   close(inpifd->fh);               // Close the file

   free(inpifd->buffer);            // Release the associated buffer
   inpifd->buffer= NULL;
   inpifd->buffsz= 0;
   inpifd->buffix= 0;

   //-------------------------------------------------------------------------
   // Remove the file from the active list
   //-------------------------------------------------------------------------
   NC_COM.srcstak.remq();           // This source file is now inactive

   ptrlink= (SHSL_List<void>::Link*)NC_COM.srcstak.getHead(); // Current tail link
   NC_COM.srcfile= NULL;            // Default, no active file
   if (ptrlink != NULL)             // If active source files exist
     NC_COM.srcfile= NC_ifd::fromActlink(ptrlink);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NC_RD
//
// Purpose-
//       Read from source file.
//
//----------------------------------------------------------------------------
extern int
   NC_rd(                           // Read from the source file
     NC_ifd*           inpifd)      // Pointer to source file descriptor
{
   unsigned char       c;           // Return character
   int                 l;           // Actual length read

   //-------------------------------------------------------------------------
   // Read a valid character
   //-------------------------------------------------------------------------
   for(;;)                          // Read a valid character
   {
     //-----------------------------------------------------------------------
     // Make sure that there is some data in the buffer
     //-----------------------------------------------------------------------
     while (inpifd->buffix >= inpifd->buffsz) // If a physical read is
                                    // required
     {
       l= read(inpifd->fh, inpifd->buffer, NC_INPBUFF);// Read
       if (l <= 0)                  // If read error or EOF
       {
         if (l == 0)                // If normal end of file
           return(EOF);             // Just return end of file

         NCmess(NC_msg::ID_IORFault, 1, inpifd->filenm);
         return(EOF);               // Indicate end of file
       }

       inpifd->buffsz= l;
       inpifd->buffix= 0;
     }

     //-----------------------------------------------------------------------
     // Retrieve the next character
     //-----------------------------------------------------------------------
     c= inpifd->buffer[inpifd->buffix];// Retrieve the next character
     inpifd->buffix++;              // Update the buffer position

     //-----------------------------------------------------------------------
     // Update the file position
     //-----------------------------------------------------------------------
     if (c == '\r')                 // If line feed
     {
       inpifd->column= 0;           // Reset the column number
       continue;                    // And otherwise ignore it
     }
     else if (c == '\n')            // If carriage return
     {
       inpifd->lineno++;            // Update the line number
       inpifd->column= 0;           // Reset the column number
     }
     else                           // If any other character
       inpifd->column++;            // Update the column number

     break;                         // Accept the character
     }

   //-------------------------------------------------------------------------
   // Return the next character
   //-------------------------------------------------------------------------
   return(c);                       // Return the next character
}

