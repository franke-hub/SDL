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
//       RdCommon.cpp
//
// Purpose-
//       Common routines used by RdClient and RdServer.
//
// Last change date-
//       2026/07/12
//
// Environment variables-
//       LOG_HCDM=n    Hard Core Debug Mode verbosity
//       LOG_SCDM=n    Soft Core Debug Mode verbosity
//       LOG_IODM=n    In/Output Debug Mode maximum length
//       LOG_FILE=name Log file name (Client.log/Server.log)
//
//----------------------------------------------------------------------------
#include <mutex>                    // For std::mutex, std::lock_guard
#include <cerrno>                   // For errno
#include <cstdarg>                  // For va_* functions
#include <cstdio>                   // For fprintf, ...
#include <cstring>                  // For strcmp, memcmp, ...

#include <dirent.h>                 // For directory management
#include <endian.h>                 // For be16toh, be32toh, ...

#include <sys/signal.h>             // For signal, ...
#include <sys/stat.h>               // For stat, struct stat, ...

#include <pub/Latch.h>              // For pub::RecursiveLatch
#include <pub/List.h>               // For pub::List<>
#include <pub/Signals.h>            // For pub::signals::Signal

#include "RdCommon.h"               // For common objects and subroutines

using PUB::List;
using PUB::RecursiveLatch;
using PUB::signals::Signal;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  USE_SIGNAL= true                 // Use system signal handler?
}; // Generic enum

static const char*     LOG_FILENAME= "debug.log"; // The default log filename

//----------------------------------------------------------------------------
// Global data areas
//----------------------------------------------------------------------------
FILE*                  stdlog= nullptr; // Log file handle

int                    env_hcdm= 0; // Hard Core Debug Mode (LOG_HCDM)
int                    env_scdm= 0; // Soft Core Debug Mode (LOG_SCDM)
unsigned               env_iodm= 0; // In/Output Debug Size (LOG_IODM)

int                    opt_help= false; // --help option specified
int                    opt_hcdm= 0;     // --hcdm (Hard Core Debug Mode)
int                    opt_verbose= VERBOSE; // Verbosity, higher is more

int                    opt_erase= false; // Erase remote target if it does
                                    // not exist locally
int                    opt_older= false; // Update remote target even if
                                    // source is newer
int                    opt_keep= false;  // Keep mode (Don't make/allow change)
int                    opt_quiet= false; // Quiet mode
int                    opt_unsafe= false; // Unsafe mode
int                    opt_verify= false; // Verify mode

int                    port= SERVER_PORT; // The server port

//----------------------------------------------------------------------------
// Signal handlers
//----------------------------------------------------------------------------
typedef void (*sig_handler_t)(int);
static sig_handler_t   sys1_handler= nullptr; // System SIGINT  signal handler
static sig_handler_t   sys2_handler= nullptr; // System SIGSEGV signal handler
static sig_handler_t   usr1_handler= nullptr; // System SIGUSR1 signal handler
static sig_handler_t   usr2_handler= nullptr; // System SIGUSR2 signal handler

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static RecursiveLatch  mutex;       // Recursive mutex
static Signal*         the_check_signal= nullptr; // The singleton check signal
static Signal*         the_final_signal= nullptr; // The singleton final signal

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_check_signal
//       handle_check_signal
//       raise_check_signal
//
// Purpose-
//       Get the (singleton) check Signal
//       Define a check Signal handler
//       Raise check Signal
//
//----------------------------------------------------------------------------
Signal*
   get_check_signal( void )         // Get the debugging check Signal
{  lock_guard<decltype(mutex)> lock(mutex);

   if( the_check_signal == nullptr )
     the_check_signal= new Signal();

   return the_check_signal;
}

pub::signals::Connector             // The signal Connector
   handle_check_signal(             // Create check Signal Connector
     const pub::signals::Signal::Function&
                       function)    // The check Signal handler function
{  return get_check_signal()->connect(function); }

void
   raise_check_signal (             // Create check Signal
     const char*       info)        // Signal source information
{
   CheckEvent event;
   event.info= info;
   get_check_signal()->emit(event);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_final_signal
//       handle_final_signal
//       raise_final_signal
//
// Purpose-
//       Get the (singleton) final Signal
//       Define a final Signal handler
//       Raise final Signal
//
//----------------------------------------------------------------------------
Signal*
   get_final_signal( void )         // Get the termination final Signal
{  lock_guard<decltype(mutex)> lock(mutex);

   if( the_final_signal == nullptr )
     the_final_signal= new Signal();

   return the_final_signal;
}

pub::signals::Connector             // The signal Connector
   handle_final_signal(             // Create final Signal Connector
     const pub::signals::Signal::Function&
                       function)    // The final Signal handler function
{  return get_final_signal()->connect(function); }

void
   raise_final_signal (             // Create final Signal
     const char*       info)        // Signal source information
{
   FinalEvent event;
   event.info= info;
   get_final_signal()->emit(event);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       host_to_peer
//       peer_to_host
//
// Purpose-
//       Convert HOST (any endian) to PEER (big endian) format.
//       Convert PEER (big endian) to HOST (any endian) format.
//
//----------------------------------------------------------------------------
PEER16_t                            // PEER format
   host_to_peer(                    // Convert HOST format to PEER format
     HOST16_t          host16)      // HOST format short value
{  return htobe16(host16); }

PEER32_t                            // PEER format
   host_to_peer(                    // Convert HOST format to PEER format
     HOST32_t          host32)      // HOST format int value
{  return htobe32(host32); }

PEER64_t                            // PEER format
   host_to_peer(                    // Convert HOST format to PEER format
     HOST64_t          host64)      // HOST format 64 bit value
{  return htobe64(host64); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HOST16_t                            // HOST format
   peer_to_host(                    // Convert PEER format to HOST format
     PEER16_t          peer16)      // PEER format short value
{  return be16toh(peer16); }

HOST32_t                            // HOST format
   peer_to_host(                    // Convert PEER format to HOST format
     PEER32_t          peer32)      // PEER format int value
{  return be32toh(peer32); }

HOST64_t                            // HOST format
   peer_to_host(                    // Convert PEER format to HOST format
     PEER64_t          peer64)      // PEER format 64 bit value
{  return be64toh(peer64); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       msgdump
//
// Purpose-
//       Debugging dump to msglog.
//
//----------------------------------------------------------------------------
void
   msgdump(                         // Debugging dump to log
     const void*       addr,        // Dump address
     unsigned long     size)        // Dump length
{
   if( stdlog == nullptr )
     return;

   lock_guard<decltype(mutex)> lock(mutex); // Single thread mode

   dump(stdlog, addr, size);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       msgerr
//
// Purpose-
//       Write an error message to stderr and stdlog.
//
//----------------------------------------------------------------------------
ATTRIB_PRINTF(1, 2)
void
   msgerr(                          // Write an error message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   lock_guard<decltype(mutex)> lock(mutex); // Single thread mode

   va_list argptr;                  // Argument list pointer
   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stderr, fmt, argptr);   // Print the message on stderr
   va_end(argptr);
   fflush(stderr);

   if( stdlog ) {                   // If stdlog active
     va_list logptr;                // Argument list pointer
     va_start(logptr, fmt);         // Initialize va_ functions
     vfprintf(stdlog, fmt, logptr);
     va_end(logptr);                // Close va_ functions
     fflush(stdlog);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       msgioerr
//
// Purpose-
//       Write an I/O error message to stderr and stdlog.
//
// Implementation notes-
//       Preserves errno.
//
//----------------------------------------------------------------------------
ATTRIB_PRINTF(1, 2)
void
   msgioerr(                        // Write an I/O error message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   int ERRNO= errno;                // Preserve errno

   lock_guard<decltype(mutex)> lock(mutex); // Single thread mode
   char string[512];                // Message assembly area
   va_list argptr;                  // Argument list pointer
   va_start(argptr, fmt);           // Initialize va_ functions
   vsprintf(string, fmt, argptr);   // Print the message
   va_end(argptr);                  // Close va_ functions

   char prefix[32];                 // Date/time prefix
   prefix[0]='\0';                  // Empty prefix
   if( true ) {                     // Date/time included?
     time_t tod= time(nullptr);
     strcpy(prefix, ctime(&tod));
     if( prefix[strlen(prefix) - 1] == '\n' )
       prefix[strlen(prefix) - 1]= ' ';
   }

   fprintf(stderr, "%s%s %d:%s\n", prefix, string, ERRNO, strerror(ERRNO));
   fflush(stderr);

   if( stdlog ) {                   // If stdlog active
     fprintf(stdlog, "%s%s %d:%s\n", prefix, string, ERRNO, strerror(ERRNO));
     fflush(stdlog);
   }

   errno= ERRNO;                    // Restore errno
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       msglog
//
// Purpose-
//       Write message to stdlog.
//
//----------------------------------------------------------------------------
ATTRIB_PRINTF(1, 2)
void
   msglog(                          // Write message to stdlog
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   if( stdlog ) {
     lock_guard<decltype(mutex)> lock(mutex); // Single thread mode

     va_list argptr;                // Argument list pointer
     va_start(argptr, fmt);         // Initialize va_ functions
     vfprintf(stdlog, fmt, argptr); // Print the message
     va_end(argptr);                // Close va_ functions
     fflush(stdlog);                // Flush stdlog
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       msgout
//
// Purpose-
//       Write message to stdout and stdlog
//
//----------------------------------------------------------------------------
ATTRIB_PRINTF(1, 2)
void
   msgout(                          // Write message to stdout and stdlog
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   lock_guard<decltype(mutex)> lock(mutex); // Single thread mode

   va_list argptr;                  // Argument list pointer
   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stdout, fmt, argptr);   // Print the message on stdout
   va_end(argptr);
   fflush(stdout);                  // Flush stdout

   if( stdlog ) {                   // Write all messages to stdlog
     va_list logptr;                // Argument list pointer
     va_start(logptr, fmt);         // Initialize va_ functions
     vfprintf(stdlog, fmt, logptr);
     va_end(logptr);                // Close va_ functions
     fflush(stdlog);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       rd_signal_user
//
// Purpose-
//       Handle SIGUSR1/SIGUSR2 signal.
//
//----------------------------------------------------------------------------
static void
   rd_signal_user(                  // Display status
     const char*       info)        // Signal type
{
   Trace::trace(".SIG", __LINE__, info);
   Debug::Mode mode= debug_get_mode();
   debug_set_mode(Debug::MODE_INTENSIVE);

   raise_check_signal(info);

   debug_set_mode(mode);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       rd_signal_term
//
// Purpose-
//       Handle termination signal.
//
//----------------------------------------------------------------------------
static void
   rd_signal_term(                  // Display status
     const char*       info)        // Signal type
{
   Trace::trace(".SIG", __LINE__, info);
   Debug::Mode mode= debug_get_mode();
   debug_set_mode(Debug::MODE_INTENSIVE);

   raise_final_signal(info);

   debug_set_mode(mode);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       rd_signal
//
// Purpose-
//       Handle signals.
//
//----------------------------------------------------------------------------
static void
   rd_signal(                       // Handle signals
     int               id)          // The signal identifier
{
   static int recursion= 0;         // Signal recursion depth
   if( recursion ) {                // If signal recursion
     msgerr("sig_handler(%d) recursion\n", id);
     exit(EXIT_FAILURE);
   }

   // Handle signal
   recursion++;                     // Disallow recursion
   const char* text= "SIG????";
   if( id == SIGINT ) text= "Interrupt";
   else if( id == SIGSEGV ) text= "SIGSEGV";
   else if( id == SIGUSR1 ) text= "SIGUSR1";
   else if( id == SIGUSR2 ) text= "SIGUSR2";

   fprintf(stderr, "Signal(%d) %s\n", id, text);
   switch(id) {                     // Handle the signal
     case SIGINT:                   // Handle Ctrl-C
       rdterm();                    // (Terminate)
       exit(1);
       break;

     case SIGUSR1:
     case SIGUSR2:
       Trace::trace(".SIG", __LINE__, text);
       rd_signal_user(text);        // Handle SIGUSR1/SIGUSR2
       break;

     // Any other (handled) signal terminate all threads
     default:
//   case SIGSEGV:                  // (Program fault)
       Trace::trace(".BUG", __LINE__, text);
       Trace::stop();

       debug_set_mode(Debug::MODE_INTENSIVE);
       debug_backtrace();           // Attempt diagnosis (recursion aborts)
       debugf("..terminated..\n");
       rd_signal_term(text);        // Handle termination signal
       rdterm();
       debugf("..EXIT_FAILURE..\n");
       exit(EXIT_FAILURE);
       break;
   }

   recursion--;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       rdinit
//
// Purpose-
//       Initialize.
//
//----------------------------------------------------------------------------
void
   rdinit( void )                   // Initialize
{
   if( USE_SIGNAL ) {
     sys1_handler= signal(SIGINT,  rd_signal);
     sys2_handler= signal(SIGSEGV, rd_signal);
     usr1_handler= signal(SIGUSR1, rd_signal);
     usr2_handler= signal(SIGUSR2, rd_signal);
   }

   // Extract log controls
   const char* file_name= nullptr;

   const char* envout= getenv("LOG_HCDM");
   if( envout ) {
     env_hcdm= atol(envout);
     file_name= LOG_FILENAME;
   }

   envout= getenv("LOG_IODM");
   if( envout ) {
     env_iodm= atol(envout);
     file_name= LOG_FILENAME;
   }

   envout= getenv("LOG_SCDM");
   if( envout ) {
     env_scdm= atol(envout);
     file_name= LOG_FILENAME;
   }

   envout= getenv("LOG_FILE");
   if( envout )
     file_name= envout;

   if( file_name ) {
     stdlog= fopen(file_name, "w");
     if( stdlog == nullptr )
       msgioerr("LogFile(%s): Open failure", file_name);

     debug_set_mode(Debug::MODE_INTENSIVE); // (For debugf, not LOG_FILENAME)
     msgout("Environment options:\n");
     msgout("LOG_HCDM: %d\n", env_hcdm);
     msgout("LOG_IODM: %d\n", env_iodm);
     msgout("LOG_SCDM: %d\n", env_scdm);
     msgout("LOG_FILE: %s\n", file_name);
     msgout("\n");
   }

   string os_name= "Undefined";
   #if defined(_OS_CYGWIN)
     os_name= "CYGWIN";
   #elif defined(_OS_BSD)
     os_name= "BSD";
   #endif
   msglog("rdinit() %s %s %s\n", s2c(os_name), __DATE__, __TIME__);

   if( BRINGUP_MODE )
     msgout("BRINGUP MODE\n");

   if( env_hcdm > 8 ) {
     msglog("\n");
     msglog("%10zd = sizeof(PeerDesc.file_size)\n", sizeof(Peer_size_t));
     msglog("%10zd = sizeof(PeerDesc.file_info)\n", sizeof(Peer_info_t));
     msglog("%10zd = sizeof(PeerDesc.file_time)\n", sizeof(Peer_time_t));
     msglog("%10zd = sizeof(PeerDesc.file_ksum)\n", sizeof(Peer_ksum_t));
     msglog("%10zd = sizeof(PeerDesc)\n", sizeof(PeerDesc));
     msglog("%10zd = sizeof(PeerName)\n", sizeof(PeerName));
     msglog("%10zd = sizeof(PeerPair)\n", sizeof(PeerPair));
     msglog("%10zd = sizeof(PeerPath)\n", sizeof(PeerPath));
     msglog("%10zd = sizeof(PeerRequest)\n",  sizeof(PeerRequest));
     msglog("%10zd = sizeof(PeerResponse)\n", sizeof(PeerResponse));
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       rdterm
//
// Purpose-
//       Terminate.
//
//----------------------------------------------------------------------------
void
   rdterm( void )                   // Terminate
{
   msglog("rdterm()\n");

   //-------------------------------------------------------------------------
   // Terminate signal handling
   //-------------------------------------------------------------------------
   delete the_check_signal;         // Remove the signal handler
   the_check_signal= nullptr;

   if( sys1_handler ) signal(SIGINT,  sys1_handler);
   if( sys2_handler ) signal(SIGSEGV, sys2_handler);
   if( usr1_handler ) signal(SIGUSR1, usr1_handler);
   if( usr2_handler ) signal(SIGUSR2, usr2_handler);
   sys1_handler= sys2_handler= usr1_handler= usr2_handler= nullptr;

   //-------------------------------------------------------------------------
   // Close logging
   //-------------------------------------------------------------------------
   fflush(stdout);
   fflush(stderr);
   fflush(stdlog);
   fflush(nullptr);
   if( stdlog ) {
     fclose(stdlog);
     stdlog= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       set_log_name
//
// Purpose-
//       Set the default log filename
//
//----------------------------------------------------------------------------
extern void
   set_log_name(                    // Set the default log filename to
     const char*       name)        // This (constant) file name
{  LOG_FILENAME= name; }
