//----------------------------------------------------------------------------
//
//       Copyright (c) 2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       copy.cpp
//
// Purpose-
//       Copy files
//
// Last change date-
//       2025/03/03
//
//----------------------------------------------------------------------------
#include <new>                      // For std::bad_alloc
#include <cstdio>                   // For fprintf, stderr, stdout, ...
#include <cstdlib>                  // For exit, malloc, size_t, ...

#include <sys/stat.h>               // For stat, struct stat

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // generic enum

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter fault exit.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Parameter fault exit
{
   fprintf(stderr, "copy filename ... (to stdout)\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "filename ...\t(The input file names)\n");
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   int error= false;                // Error encountered indicator
   const char* inp_name= nullptr;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for(int argi=1; argi<argc; argi++ ) { // Analyze variable controls
     char* argp= argv[argi];        // Address the parameter

     if( *argp == '-' ) {           // If this parameter is in switch format
       error= true;
       fprintf(stderr, "Invalid parameter '%s'\n", argv[argi]);
     } else {                       // If filename parameter
       if( inp_name == nullptr )
         inp_name= argp;
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( inp_name == nullptr ) {
     error= true;
     fprintf(stderr, "No filenames specified\n");
   }

   if( error )                      // If error encountered
     info();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       copy
//
// Purpose-
//       Copy a file
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   copy(                            // Copy
     const char*       inp_name)    // This file
{
   //-------------------------------------------------------------------------
   // Get file state
   //-------------------------------------------------------------------------
   struct stat st;                  // The file information
   int rc= stat(inp_name, &st);     // Get file information
   if( rc ) {                       // If not found
     fprintf(stderr, "File(%s) ", inp_name);
     perror("stat");
     return 1;
   }

   //-------------------------------------------------------------------------
   // Open the file
   //-------------------------------------------------------------------------
   FILE* file= fopen(inp_name, "rb");      // Open the input file
   if( file == nullptr ) {
     fprintf(stderr, "File(%s) ", inp_name);
     perror("open failure");
     return 2;
   }

   //-------------------------------------------------------------------------
   // Allocate the input buffer
   //-------------------------------------------------------------------------
   size_t inp_size= st.st_size;     // Input buffer size
   char* inp_buff= (char*)malloc(inp_size); // Allocate the buffer
   if( inp_buff == nullptr )
     throw std::bad_alloc();

   //-------------------------------------------------------------------------
   // Read the file
   //-------------------------------------------------------------------------
   ssize_t L= fread(inp_buff, 1, inp_size, file);
   if( L != (ssize_t)inp_size ) {   // If read failure
     fprintf(stderr, "File(%s) ", inp_name);
     perror("read failure");
   }

   //-------------------------------------------------------------------------
   // Close the input file
   //-------------------------------------------------------------------------
   rc= fclose(file);                // Close the file
   if( rc != 0 ) {
     fprintf(stderr, "File(%s) ", inp_name);
     perror("close failure");
     rc= 1;
   }

   if( rc ) {
     free(inp_buff);                // Free the input buffer
     return rc;
   }

   //-------------------------------------------------------------------------
   // Write the file
   //-------------------------------------------------------------------------
   L= fwrite(inp_buff, 1, inp_size, stdout);
   if( L != (ssize_t)inp_size ) {   // If write failure
     fprintf(stderr, "STDOUT ");
     perror("write failure");
     rc= 1;
   }
   fflush(stdout);
   free(inp_buff);                  // Free the input buffer

   return rc;
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
   int                 rc= 0;       // Return code and resultant

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Copy the files
   //-------------------------------------------------------------------------
   for(int argi=1; argi<argc; argi++ ) { // Copy the files
     rc= copy(argv[argi]);          // Copy a file
     if( rc )                       // Exit on failure
       break;
   }
   fclose(stdout);

   return rc;
}
