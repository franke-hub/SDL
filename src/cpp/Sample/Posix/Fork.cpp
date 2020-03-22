//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Fork.cpp
//
// Purpose-
//       Test the operation of fork().
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static int            commonIdent;  // Child process identifier

//----------------------------------------------------------------------------
//
// Subroutine-
//       kiddo
//
// Purpose-
//       Run the child process.
//
//----------------------------------------------------------------------------
static int                          // Process return code
   kiddo(                           // Child process
     int             processIdent)  // Process identifier
{
   int               copiedIdent;
   int               returncd;

   copiedIdent= commonIdent;
   returncd= 0;
   if( copiedIdent != processIdent )
   {
     fprintf(stderr, "%s %d: static variable error\n"
                     "expected(%d) got(%d)\n",
                     __FILE__, __LINE__,
                     processIdent, copiedIdent);
     returncd= 1;
   }

   return returncd;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   int               status;        // Child process status
   int               child;         // ID of child process
   int               id[100];       // Saved child ids
   int               processCount;  // Number of processes
   int               processIdent;  // Process identifier
   int               success;       // Number of successful children

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   processCount= 1;                 // Default, one process
   if( argc > 1 )                   // If process count specified
     processCount= atol(argv[1]);
   if( processCount > 100 )
     processCount= 100;

   //-------------------------------------------------------------------------
   // Start the child processes
   //-------------------------------------------------------------------------
   for(processIdent= 0; processIdent<processCount; processIdent++)
   {
      commonIdent= processIdent;
      child= fork();
      if( child < 0 )
      {
        fprintf(stderr,"fork failed\n");
        exit(1);
      }
      if( child == 0 )              // If this is the child
      {
        return kiddo(processIdent);
      }
      id[processIdent]= child;
   }

   //-------------------------------------------------------------------------
   // Wait for each child to complete
   //-------------------------------------------------------------------------
   success= 0;
   for(processIdent= 0; processIdent<processCount; processIdent++)
   {
      waitpid(id[processIdent], &status, 0);
      if( WEXITSTATUS(status) != 0 )
      {
        fprintf(stderr, "[%3d] Failed, status(0x%.8X)= ",
                        processIdent, status);
        fprintf(stderr, "STOP(%d), ", WSTOPSIG(status));
        fprintf(stderr, "EXIT(%d), ", WEXITSTATUS(status));
        fprintf(stderr, "TERM(%d)\n", WTERMSIG(status));
      }
      else
        success++;
   }
   fprintf(stderr, "%d of %d successful\n", success, processCount);

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   return 0;
}

