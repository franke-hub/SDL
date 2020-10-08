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
//       RdClient.cpp
//
// Purpose-
//       The (multi-threaded) client.
//
// Last change date-
//       2020/10/03
//
// Usage-
//       RdClient <-options> <server_host<:server_port> <client_path>>
//
// Options-
//       -E (erase)
//          Remove client target if it does not exist locally.
//          (This deletes targets which have been removed from the
//          server source tree.)
//
//       -O (older)
//          Update client target even if it is older than the source.
//          (This updates targets even though they are newer in the client
//          file tree than the server file tree.)
//
//       -U (unsafe)
//          Ignore CWD directory name match verification.
//          (Overrides directory match safety feature.)
//
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
//       Used in conjunction with RdServer for file distribution.
//
//       If a "!const" file exists, an attempt to update it terminates this
//       client process.
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
#include <com/Unconditional.h>

#include "RdCommon.h"
#include "ClientThread.h"

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
// Internal data areas
//----------------------------------------------------------------------------
static char*           hostName= NULL; // Server Host name
static const char*     pathName= NULL; // Client initial path

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
   fprintf(stderr,"RdClient <-options> "
                  "<server_host<:server_port> <client_path>>\n");
   fprintf(stderr,"\n");
   fprintf(stderr,"File transfer client\n");
   fprintf(stderr,"\n");
   fprintf(stderr,"-Options:\n");

   fprintf(stderr,"\n");
   fprintf(stderr,"-E (erase) Removes client files if they do not "
                  "exist in the server tree.\n");

   fprintf(stderr,"\n");
   fprintf(stderr,"-O (older) Updates target files from source files "
                  "even when a source is older\n");

   fprintf(stderr,"\n");
   fprintf(stderr,"-U (unsafe) Ignore CWD directory name match verification\n");

   fprintf(stderr,"\n");
   fprintf(stderr,"-V (verify) Use checksum difference verification.\n");

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
   int                 i, j, k;     // General index variables

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
   k= 0;                            // No flat parameters found
   error= FALSE;                    // Default, no error found
   for(j=1; j<argc; j++)            // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       if( strcmp("-help", argv[j]) == 0 )
         error= TRUE;

       else                         // Switch list
       {
         for(i=1; argv[j][i] != '\0'; i++) // Examine the switch list
         {
           switch(argv[j][i])       // Examine the switch
           {
             case 'E':              // -E (erase)
               sw_erase= TRUE;
               break;

             case 'O':              // -O (older)
               sw_older= TRUE;
               break;

             case 'U':              // -U (unsafe)
               sw_unsafe= TRUE;
               break;

             case 'V':              // -V (verify)
               sw_verify= TRUE;
               break;

             case 'q':              // -q (quiet)
               sw_quiet= TRUE;
               break;

             default:               // If invalid switch
               error= TRUE;
               msgout("Invalid switch '%c'\n", (int)argv[j][i]);
               break;
           }
         }
       }
       continue;
     }

     //-----------------------------------------------------------------------
     // Process flat parameter
     //-----------------------------------------------------------------------
     if( k == 0 )                   // If server_host:port parameter
     {
       k++;
       hostName= must_strdup(argv[j]); // Copy the parameter
       char* C= strchr(hostName, ':'); // Locate port delimiter
       if( C != NULL)               // If port specified
       {
         *C= '\0';                  // Terminate the hostName portion
         C++;                       // Skip over the delimiter
         port= atol(C);             // Set the port number
       }
     }

     else if( k == 1 )              // If initial path parameter
     {
       k++;
       pathName= argv[j];
     }

     else                           // If too many flat parameters
     {
       error= TRUE;
       msgout("Invalid parameter '%s'\n", argv[j]);
     }
   }

   //-------------------------------------------------------------------------
   // Validate the parameters
   //-------------------------------------------------------------------------
   if( hostName == NULL )
     hostName= must_strdup(Socket::getName());

   if( pathName == NULL )
     pathName= ".";

   if( error )                      // If an error was detected
   {
     info();                        // Tell how this works
     exit(2);                       // And exit, function aborted
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       client
//
// Function-
//       Operate the client
//
//----------------------------------------------------------------------------
static void
   client( void )                   // Operate the client
{
   //-------------------------------------------------------------------------
   // Connect to the Server
   //-------------------------------------------------------------------------
   Socket* socket= new Socket(Socket::ST_STREAM);
   HOST32 addr= socket->nameToAddr(hostName);
   if( addr == 0 )
     throwf("%4d Invalid host name(%s) %s",
            __LINE__, hostName, socket->getSocketEI());

   int rc= socket->connect(addr, port); // Connect
   if( rc != 0 )
     throwf("%4d %d= connect(%s:%d) %s", __LINE__,
            rc, hostName, port, socket->getSocketEI());

   //-------------------------------------------------------------------------
   // Set transfer size -- optimization attempt (has no noticable effect)
   //-------------------------------------------------------------------------
   socket->setSocketSO(Socket::SO_RCVBUF, 8192);

   //-------------------------------------------------------------------------
   // Create the client worker Thread
   //-------------------------------------------------------------------------
   ClientThread* thread= new ClientThread(socket, pathName);
   thread->start();

   //-------------------------------------------------------------------------
   // Wait for completion
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
   // Run the client
   //-------------------------------------------------------------------------
   IFHCDM(
     Debug::set(new Debug("/tmp/Client.out"));
     debugSetIntensiveMode();

     debugf("%4d RdClient::main() TEST VERSION\n", __LINE__);
   )
   rdinit();                        // Initialize message services

   try {
     parm(argc, argv);              // Parameter analysis

     client();                      // Operate the client
   } catch( const char* X ) {
     fprintf(stderr, "RdClient exception(%s)\n", X);
   } catch(...) {
     fprintf(stderr, "RdClient exception(%s)\n", "...");
   }

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   if( hostName != NULL )           // If hostName storage allocated
     free(hostName);                // Release it

   rdterm();

   IFHCDM( debugf("%4d RdClient::main() COMPLETE\n", __LINE__); )
   return(0);
}

