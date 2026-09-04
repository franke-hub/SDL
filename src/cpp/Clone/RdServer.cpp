//----------------------------------------------------------------------------
//
//       Copyright (c) 2014-2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       RdServer.cpp
//
// Purpose-
//       The (multi-threaded) file server.
//
// Last change date-
//       2026/08/01
//
// Usage-
//       RdServer <-options>
//
// Options-
//       --help Generate usage message and exit.
//       --hcdm  Generate usage message and exit.
//       --verbose{=n} Verbosity, higher is more verbose.
//
//       -P (Port)
//          Use specified port number
//
//       -V (Verify)
//          Use checksum difference verification.
//          (Updates targets which have differing 64 bit checksums.)
//
// Environment variables-
//       LOG_HCDM=n    Hard Core Debug Mode verbosity
//       LOG_SCDM=n    Soft Core Debug Mode verbosity
//       LOG_IODM=n    In/Output Debug Mode size
//       LOG_FILE=name Log file name (debug.log)
//
// Implementation notes-
//       Used in conjunction with RdClient for file distribution.
//
//----------------------------------------------------------------------------
#include "IoCommon.h"               // For I/O common objects and subroutines
#include "ListenThread.h"           // The Listener thread
#include "ServerThread.h"           // For ServerThread

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= true                       // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // Generic enum

//----------------------------------------------------------------------------
//
// Subroutine-
//       hcdm
//       verbose
//       hcdm_verbose
//
// Purpose-
//       Is Hard Core Debug Mode active?
//       Is Verbosity greater than N?
//       Are hcdm() && verbose(N) both true?
//
//----------------------------------------------------------------------------
static inline bool                  // TRUE if Hard Core Debug Mode is active
   hcdm( void )                     // Is Hard Core Debug Mode active?
{  return HCDM || opt_hcdm; }

static inline bool                  // TRUE if Verbosity is greater than N
   verbose(                         // Is Verbosity greater than
     int               N= 0)        // This value?
{  return VERBOSE > N || opt_verbose > N; }

static inline bool                  // TRUE if hcdm && verbose(N)
   hcdm_verbose(                    // If hcdm() && verbose(N)
     int               N= 0)
{  return hcdm() && verbose(N); }

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
   fprintf(stderr,"\n"
                  "File transfer server\n"
                  "\n"
                  "rdserver {options}\n"
                  "\n"
                  "Options:\n"
                  "  --help\tPrint this message and exit.\n"
                  "  --hcdm\tEnable Hard Core Debug Mode.\n"
                  "  --verbose{=n}\tVerbosity, default 1.\n"
                  "\n"
                  "  -P number\tOverride the default port number.\n"
                  "  -V (Verify)\tUse checksum verification.\n"
                 );
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
   bool is_port= false;             // Last parameter -p?
   for(int i=1; i<argc; ++i) {      // Examine the parameter list
     const char* argp= argv[i];
     if( is_port ) {                // If port number parameter
       is_port= false;
       port= atol(argp);
       continue;
     }

     if( strcmp("--help", argp) == 0 ) {
       opt_help= true;
     } else if( strcmp("--hcdm", argp) == 0 ) {
       opt_hcdm= true;
     } else if( strcmp(argp, "--verbose") == 0 ) {
       opt_verbose= 1;
     } else if( memcmp(argp, "--verbose=", 10) == 0 ) {
       opt_verbose= atoi(argp+10);
     } else if( argp[0] == '-' ) {  // If this is a control parameter
       for(int j= 1; argp[j] != '\0' ; ++j) {
         switch( argp[j] ) {
           case 'p':                // Port number
           case 'P':
             if( argp[j+1] != '\0' ) { // if -pnumber format
               const char* numb= argp + j + 1; // The number, or garbage
               port= atol(numb);    // (Never used when it's invalid)
               if( strspn(numb, "0123456789") != strlen(numb) ) {
                 opt_help= true;    // Not all digits: invalid switch
                 msgout("Invalid switch '-%s'\n", argp + j);
               }

               j= strlen(argp) - 1; // Consume the rest of the switch
               break;
             }
             is_port= true;
             break;

           case 'v':                // Verify option
           case 'V':
             opt_verify= true;
             break;

           default:                 // If invalid switch
             opt_help= true;
             msgout("Invalid switch '%c'\n", (int)argp[j]);
             break;
         }

         if( is_port )
           break;
       }
     } else {                       // If non-switch parameter
       opt_help= true;
       msgout("Invalid parameter '%s'\n", argp);
     }
   }

   if( is_port ) {                  // If port parameter missing
     opt_help= true;
     msgout("Missing port number\n");
   }

   if( opt_help )
     info();
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
   thread->join();                  // Wait for Listener completion
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
   set_app_name("RdServer");        // Set the application name
   parm(argc, argv);                // Parameter analysis
   rdinit();                        // Initialize message services

   if( hcdm_verbose() ) {
     printf("--hcdm: %s\n",    opt_hcdm ? "true" : "false");
     printf("--verbose: %d\n", opt_verbose);

     printf("\n");
     printf("-V: %s\n", opt_verify ? "true" : "false");
   }

   //-------------------------------------------------------------------------
   // Run the server
   //-------------------------------------------------------------------------
   try {
     server();                      // Operate the server
   } catch(std::exception& X) {
     msgerr("RdServer exception(%s)\n", X.what());
   } catch( const char* X ) {
     msgerr("RdServer const char*(%s)\n", X);
   } catch(...) {
     msgerr("RdServer catch(%s)\n", "...");
   }

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   rdterm();

   if( HCDM ) debugf("RdServer::main() COMPLETE\n");
   return(0);
}
