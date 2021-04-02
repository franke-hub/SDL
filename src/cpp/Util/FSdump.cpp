//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2021 Frank Eskesen.
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
//       2021/04/01
//
// Options-
//       filespec = The file to be dumped
//       origin   = The dump origin [default 0]
//       length   = The dump length [default remainder of file]
//
//----------------------------------------------------------------------------
#include <errno.h>                  // For errno
#include <fcntl.h>                  // For open
#include <stdint.h>                 // For size_t
#include <stdio.h>                  // For printf, stdout
#include <string.h>                 // For strerror
#include <unistd.h>                 // For close
#include <sys/mman.h>               // For mmap
#include <sys/stat.h>               // For stat

#include <pub/utility.h>            // For pub::utility::dump

#ifndef O_BINARY
#define O_BINARY 0
#endif

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
   //-------------------------------------------------------------------------
   // Argument anlaysis
   //-------------------------------------------------------------------------
   if (argc < 2) {                  // If no filename present
     printf("FSDUMP filespec origin length\n");
     printf("filespec: the file name to be dumped\n");
     printf("origin:   the dump origin within the file\n");
     printf("length:   the dump length\n");
     return 1;
   }

   const char* inpfile= argv[1];    // Set filename pointer

   struct stat info;
   int rc= stat(inpfile, &info);    // Get file length
   if( rc ) {                       // If error
     fprintf(stderr, "File(%s): %s\n", inpfile, strerror(errno));
     return 2;
   }

   ssize_t inporg= 0;               // Set default offset
   ssize_t inplen= info.st_size;    // Set default length

   if (argc > 2)                    // If length present
     inporg= atol(argv[2]);         // Set dump origin
   if (argc > 3)                    // If length present
     inplen= atol(argv[3]);         // Set dump length
   if( (inporg + inplen) > info.st_size ) { // (Don't dump past end)
     inplen= info.st_size - inporg;
     if( inplen < 0 ) {
       printf("Origin > Length(%zd)\n", info.st_size);
       return 0;
     }
   }

   printf("Filename: '%s'[%zd:%zd]\n", inpfile, inporg, inplen);
   printf("\n");
   if( inplen == 0 ) {
     printf("(No data)\n");
     return 0;
   }

   //-------------------------------------------------------------------------
   // File initialization
   //-------------------------------------------------------------------------
   int fd= open(inpfile, O_RDONLY|O_BINARY, 0); // Open the input file
   if( fd < 0 ) {                   // If we cannot open the input file
     printf("Error, cannot open '%s'\n",inpfile);
     return 1;
   }

   //-------------------------------------------------------------------------
   // Dump the input file
   //-------------------------------------------------------------------------
   const void* buffer= mmap(nullptr, info.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
   if( buffer == MAP_FAILED )
     fprintf(stderr, "File(%s): mmap %s\n", inpfile, strerror(errno));
   else {
     pub::utility::dump(stdout, (const char*)buffer + inporg, inplen, (void*)inporg);
     munmap((void*)buffer, info.st_size);
   }

   close(fd);
   return 0;
}

