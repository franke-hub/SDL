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
//       RdClient.cpp
//
// Purpose-
//       The RdServer's client.
//
// Last change date-
//       2026/07/27
//
// Usage-
//       RdClient <-options> <server_host<:server_port> <client_path>>
//
// Options-
//       --help Generate usage message and exit.
//       --hcdm  Generate usage message and exit.
//       --verbose{=n} Verbosity, higher is more verbose.
//
//       -E (erase)
//          Remove client target if it does not exist locally.
//          (This deletes targets which have been removed from the
//          server source tree.)
//
//       -K (keep)
//          Don't make any changes. (Dry run)
//
//       -O (older)
//          Update client target even if it is older than the source.
//          (This updates targets even though they are newer in the client
//          file tree than the server file tree.)
//
//       -Q (quiet)
//          Do not write informative messages.
//
//       -U (unsafe)
//          Ignore CWD directory name match verification.
//          (Overrides directory match safety feature.)
//
//       -V (verify)
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
//       Used in conjunction with RdServer for file distribution.
//
//       If a "!const" file exists, an attempt to update it terminates this
//       client process.
//
//----------------------------------------------------------------------------
#include <cstdio>                   // For fprintf

#include "ClientThread.h"           // For ClientThread
#include "IoCommon.h"               // For I/O common objects and subroutines

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // Generic enum

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static string          host_name= Socket::gethostname(); // Default, this host
static string          path_name= "."; // Default, current directory

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
                  "File transfer client\n"
                  "\n"
                  "rdclient {options} "
                  "{server_host{:server_port} {initial_path}}\n"
                  "\n"
                  "Options:\n"
                  "  --help\tPrint this message and exit.\n"
                  "  --hcdm\tEnable Hard Core Debug Mode.\n"
                  "  --verbose{=n}\tVerbosity, default 1.\n"
                  "\n"
                  "  -E (Erase)\tRemoves client files that do not "
                        "exist in the server.\n"
                  "  -K (Keep)\tDon't make any changes (Dry run)\n"
                  "  -O (Older)\tAllow older source file updates\n"
                  "  -Q (Quiet)\tSuppresses informative messages.\n"
                  "  -U (Unsafe)\tSkip current directory name verification\n"
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
   int argi= 0;                     // Flat parameter index

   for(int i= 1; i < argc; ++i) {
     const char* argp= argv[i];
     if( strcmp(argp, "--help") == 0 ) {
       opt_help= true;
     } else if( strcmp("--hcdm", argp) == 0 ) {
       opt_hcdm= true;
     } else if( strcmp(argp, "--verbose") == 0 ) {
       opt_verbose= 1;
     } else if( memcmp(argp, "--verbose=", 10) == 0 ) {
       opt_verbose= atoi(argp+10);
     } else if( argp[0] == '-' ) {
       for(size_t j= 1; j < strlen(argp); ++j) {
         switch( argp[j] ) {
           case 'E':
           case 'e':
             opt_erase= true;
             break;

           case 'K':
           case 'k':
             opt_keep= true;
             break;

           case 'O':
           case 'o':
             opt_older= true;
             break;

           case 'Q':
           case 'q':
             opt_quiet= true;
             break;

           case 'U':
           case 'u':
             opt_unsafe= true;
             break;

           case 'V':
           case 'v':
             opt_verify= true;
             break;

           default:
             opt_help= true;
             fprintf(stderr, "Invalid parameter '-%c'\n", argp[j]);
             break;
         }
       }
     } else {
       switch( argi ) {
         case 0: {{{{
           host_name= argp;
           size_t X= host_name.find(":");
           if( X != string::npos ) {
             port= std::stoi(host_name.substr(X+1));
             host_name= host_name.substr(0, X);
           }

           argi= 1;
           break;
           }}}}

         case 1: {{{{
           path_name= argp;
           argi= 2;
           break;
           }}}}

         default:
           opt_help= true;
           fprintf(stderr, "Invalid parameter '%s'\n", argp);
       }
     }
   }

   if( opt_help )
     info();
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
   // Socket::sockaddr_u sockaddr;
   // socklen_t          socksize;
   string nps= host_name;
          nps += ":" + std::to_string(port);

   Socket* socket= new Socket();
   if( socket == nullptr )
     throwf("%4d Unable to create socket\n", __LINE__);

   // int rc= socket->nameToAddr(nps, &sockaddr, &socksize, AF_INET);
   // if( addr == 0 )
   //   throwf("%4d Invalid host name(%s) %d:%s",
   //          __LINE__, s2c(nps), errno, strerror(errno));

   int
   rc= socket->open(AF_INET, SOCK_STREAM, PF_UNSPEC);
   if( rc )
     throwf("%4d %d= socket->open(AF_INET, SOCK_STREAM, PF_UNSPEC)\n"
           , __LINE__, rc);

   rc= socket->connect(nps);
   if( rc != 0 ) {
     fprintf(stderr, "%d= connect(%s) %d:%s\n", rc, s2c(nps)
                   , errno, strerror(errno));
     exit(EXIT_FAILURE);
   }

   //-------------------------------------------------------------------------
   // Create and directly invoke the client worker pseudo-Thread
   //-------------------------------------------------------------------------
   ClientThread thread(socket, path_name);
   thread.run();
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
   set_app_name("RdClient");        // Set the application name
   parm(argc, argv);                // Parameter analysis
   rdinit();                        // Initialize message services

   if( hcdm_verbose() ) {
     printf("--hcdm: %s\n",    opt_hcdm ? "true" : "false");
     printf("--verbose: %d\n", opt_verbose);

     printf("\n");
     printf("-E: %s\n", opt_erase  ? "true" : "false");
     printf("-K: %s\n", opt_keep   ? "true" : "false");
     printf("-O: %s\n", opt_older  ? "true" : "false");
     printf("-Q: %s\n", opt_quiet  ? "true" : "false");
     printf("-U: %s\n", opt_unsafe ? "true" : "false");
     printf("-V: %s\n", opt_verify ? "true" : "false");
   }

   try {
     client();                      // Operate the client
   } catch( const char* X ) {
     msgerr("RdClient exception(%s)\n", X);
   } catch(...) {
     msgerr("RdClient exception(%s)\n", "...");
   }

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   rdterm();

   if( HCDM ) printf("%4d RdClient::main() COMPLETE\n", __LINE__);
   return(0);
}
