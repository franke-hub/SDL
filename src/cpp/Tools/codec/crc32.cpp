//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       crc32.cpp
//
// Purpose-
//       Compute CRC32 value for a file.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <com/AutoPointer.h>
#include <com/Reader.h>
#include <com/CRC32.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define BUFF_SIZE             32768 // Allocated line size

//----------------------------------------------------------------------------
//
// Subroutine-
//       crc32
//
// Purpose-
//       Display CRC32 for file.
//
//----------------------------------------------------------------------------
static void
   crc32(                           // Display CRC32
     char*             name)        // File name
{
   FileReader          inp;         // Source file (stdin)

   int rc= inp.open(name);
   if( rc != 0 )
   {
     fprintf(stderr, "File(%s) ", name);
     perror("Open failure");
     return;
   }

   AutoPointer ptr(BUFF_SIZE);
   char* inpLine= (char*)ptr.get();

   // Find and parse start delimiter
   CRC32 sum;
   for(;;)
   {
     int size= inp.read(inpLine, BUFF_SIZE);
     if( size == 0 )              // End of file or error
       break;

     sum.accumulate((unsigned char*)inpLine, size);
   }

   inp.close();
   printf("0x%.8lX %s\n", (long)sum.getValue(), name);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
int                                 // Main return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   for(int i=1; i<argc; i++)
   {
     crc32(argv[i]);
   }

   //-------------------------------------------------------------------------
   // Return
   //-------------------------------------------------------------------------
   return 0;
}

