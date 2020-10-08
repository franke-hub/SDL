//----------------------------------------------------------------------------
//
//       Copyright (c) 2014-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       RdServer.cpp
//
// Purpose-
//       The (multi-threaded) file server.
//
// Last change date-
//       2020/10/03
//
// Usage-
//       RdServer <-options>
//
// Options-
//       -V (verify)
//          Use checksum difference verification.
//          (Updates targets which have differing 64 bit checksums.)
//
//       -q (quiet)
//          Do not write informative messages.
//
//       -help
//          Generate usage message and exit.
//
// Environment variables-
//       LOG_HCDM=n    Hard Core Debug Mode verbosity
//       LOG_SCDM=n    Soft Core Debug Mode verbosity
//       LOG_IODM=n    In/Output Debug Mode size
//       LOG_FILE=name Log file name (rdist.log)
//
// Implementation notes-
//       Used in conjunction with RdClient for file distribution.
//
//----------------------------------------------------------------------------
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <com/Debug.h>
#include <com/FileInfo.h>
#include <com/Software.h>

#include "RdCommon.h"
#include "ListenThread.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Function-
//       Display parameter information.
//
//----------------------------------------------------------------------------
static void
   info( void )
{
   fprintf(stderr,"\n");
   fprintf(stderr,"RdServer <-options>\n");
   fprintf(stderr,"\n");
   fprintf(stderr,"File transfer server\n");
   fprintf(stderr,"\n");
   fprintf(stderr,"-Options:\n");

   fprintf(stderr,"\n");
   fprintf(stderr,"-V (verify) Use checksum difference verification.\n");

   fprintf(stderr,"\n");
   fprintf(stderr,"-p port_number\n");
   fprintf(stderr,"   Override the default port number\n");

   fprintf(stderr,"\n");
   fprintf(stderr,"-q (quiet mode) "
                  "Suppresses informative messages.\n");

   fprintf(stderr,"\n");
   fprintf(stderr,"-help "
                  "Print this message and exit.\n");

   exit(2);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 error;       // Error switch
   int                 i, j;        // General index variables

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   sw_erase= FALSE;                 // Default switch settings
   sw_older= FALSE;
   sw_quiet= FALSE;
   sw_unsafe= FALSE;
   sw_verify= FALSE;

   port= SERVER_PORT;               // Default port number

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no error found
   for(j=1; j<argc; j++)            // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       if( strcmp("-help", argv[j]) == 0 )
         error= TRUE;

       else if( strcmp("-p", argv[j]) == 0 ) // If port number
       {
         j++;                       // Address the next parameter
         if( j >= argc              // If no next parameter
             ||argv[j][0] == '-' )  // or the next parameter is a switch, not a file name
         {
           error= TRUE;
           msgout("-p but port_number is missing\n");
         }
         else
           port= atol(argv[j]);
       }
       else                         // Switch list
       {
         for(i=1; argv[j][i] != '\0'; i++) // Examine the switch list
         {
           switch(argv[j][i])       // Examine the switch
           {
             case 'q':              // -q (quiet)
               sw_quiet= TRUE;
               break;

             case 'V':              // -V (verify)
               sw_verify= TRUE;
               break;

             default:               // If invalid switch
               error= TRUE;
               msgout("Invalid switch '%c'\n", (int)argv[j][i]);
               break;
           }
         }
       }
     }
     else                           // If non-switch parameter
     {
       error= TRUE;
       msgout("Invalid parameter '%s'\n", argv[j]);
     }
   }

   //-------------------------------------------------------------------------
   // Validate the parameters
   //-------------------------------------------------------------------------
   if( error )                      // If an error was detected
   {
     info();                        // Tell how this works
     exit(2);                       // And exit, function aborted
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       server
//
// Function-
//       Operate the server
//
//----------------------------------------------------------------------------
static void
   server( void )                   // Operate the server
{
   //-------------------------------------------------------------------------
   // Create the Listener Thread
   //-------------------------------------------------------------------------
   ListenThread* thread= new ListenThread(port);
   thread->start();

   //-------------------------------------------------------------------------
   // Wait for Listener completion
   //-------------------------------------------------------------------------
   thread->waiter();                // Wait for completion
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Function-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   IFHCDM(
     Debug::set(new Debug("/tmp/Server.out"));
     debugSetIntensiveMode();

     debugf("%4d RdServer::main() TEST VERSION\n", __LINE__);
   )
   rdinit();                        // Initialize message services
   parm(argc, argv);                // Parameter analysis

   //-------------------------------------------------------------------------
   // Run the server
   //-------------------------------------------------------------------------
   try {
     server();                      // Operate the server
   } catch( const char* X ) {
     fprintf(stderr, "RdServer exception(%s)\n", X);
   } catch(...) {
     fprintf(stderr, "RdServer exception(%s)\n", "...");
   }

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   rdterm();

   IFHCDM( debugf("%4d RdServer::main() COMPLETE\n", __LINE__); )
   return(0);
}

