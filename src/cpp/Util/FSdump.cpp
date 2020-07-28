//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       FSdump
//
// Purpose-
//       File system dump utility
//
// Last change date-
//       2020/07/28
//
// Options-
//       filespec = The file to be dumped
//       origin   = The dump origin [default 0]
//       length   = The dump length [default remainder of file]
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include <com/Debug.h>
#include <com/nativeio.h>
#include <com/syslib.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define BUFSIZE               16000 // I/O buffer (maximum size)

//----------------------------------------------------------------------------
// General work areas
//----------------------------------------------------------------------------
static char            buffer[BUFSIZE]; // I/O buffer

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Function-
//       Mainline code.
//
//----------------------------------------------------------------------------
int
   main(                            // FSDUMP utility
      int              argc,       // Argument count
      char*            argv[])     // Argument array
{
   char*               inpfile;     // Input file name
   int                 inpnum;      // Input file handle
   size_t              inporg;      // Dump origin
   size_t              inplen;      // Dump length

   int                 chain;       // Current chain state
   unsigned long       lenbuf;      // Sizeof(buffer) this operation

   //-------------------------------------------------------------------------
   // Argument anlaysis
   //-------------------------------------------------------------------------
   if (argc < 2)                    // If no filename present
   {
     printf("FSDUMP filespec origin length\n");
     printf("filespec: the file name to be dumped\n");
     printf("origin:   the dump origin within the file\n");
     printf("length:   the dump length\n");
     return 1;
   }

   inpfile= argv[1];                // Set filename pointer

   struct stat info;
   int rc= stat(inpfile, &info);    // Get file length
   if( rc ) {                       // If error
     fprintf(stderr, "File(%s): %s\n", inpfile, strerror(errno));
     return 2;
   }

   inporg= 0;                       // Set default offset
   inplen= info.st_size;            // Set default length

   if (argc > 2)                    // If length present
     inporg= atol(argv[2]);         // Set dump origin
   if (argc > 3)                    // If length present
     inplen= atol(argv[3]);         // Set dump length

   printf("Filename: '%s'[%zd:%zd]\n", inpfile, inporg, inplen);
   printf("\n");

   //-------------------------------------------------------------------------
   // File initialization
   //-------------------------------------------------------------------------
   inpnum= open(inpfile,            // Open the input file
                O_RDONLY|O_BINARY,
                0);

   if (inpnum < 0)                  // If we cannot open the input file
   {
     printf ("Error, cannot open input file '%s'\n",inpfile);
     return 1;
   }

   lseek(inpnum, inporg, SEEK_SET); // Position the file

   //-------------------------------------------------------------------------
   // Read the input file
   //-------------------------------------------------------------------------
   chain= Debug::ChainFirst;        // Indicate first in chain
   while(inplen > 0)                // Read the input file
   {
     lenbuf=  read(inpnum, buffer, BUFSIZE); // Read a buffer image
     if( lenbuf > inplen )
       lenbuf= inplen;

     if (lenbuf == 0)               // If zero length buffer
       break;                       // Exit, end of file

     snapv(buffer, lenbuf, (char*)inporg, Debug::Chain(chain));
     chain= Debug::ChainMiddle;     // Indicate middle chain

     inporg += lenbuf;
     inplen -= lenbuf;
   }

   if( chain != Debug::ChainFirst ) // If some data dumped
   {
     chain |= Debug::ChainLast;     // Indicate last in chain
     snapv(buffer, 0, (char*)inporg, Debug::Chain(chain));
   }
   else
     printf("%.8zX  (No data)\n", inporg);

   return 0;
}

