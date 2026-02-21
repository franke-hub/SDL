//----------------------------------------------------------------------------
//
//       Copyright (C) 2026 Frank Eskesen.
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
//       System.cpp
//
// Purpose-
//       System method implementations.
//
// Last change date-
//       2026/02/19
//
// Implementation note-
//       The system logfile is "$HOME/.local/log/syslog.out"
//
//----------------------------------------------------------------------------
#include <atomic>                   // For atomic_uint64_t
#include <string>                   // For std::string
#include <cstdlib>                  // For getenv
#include <ctime>                    // For time, localtime

#include <sys/stat.h>               // For stat

#include <pub/Debug.h>              // For debugging
#include <pub/Reporter.h>           // For pub::Reporter::report, ...
#include "pub/System.h"             // For pub::System, implemented
#include <pub/Thread.h>             // For pub::Thread::static_debug
#include <pub/Worker.h>             // For pub::WorkerPool::debug

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging methods
using std::string;                  // For convenience

namespace _LIBPUB_NAMESPACE::System {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
// Production mode settings: HCDM= false; VERBOSE= 0;
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  DIR_MODE= (S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH)
}; // generic enum

static constexpr const char* log_file_name= // The log file name
                       "$HOME/.local/log/syslog.out";

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static log_level_t     log_level= LL_DEFAULT; // The LOG level

//----------------------------------------------------------------------------
// Constant data areas
//----------------------------------------------------------------------------
const char* _month[]=
{  "Jan"
,  "Feb"
,  "Mar"
,  "Apr"
,  "May"
,  "Jun"
,  "Jul"
,  "Aug"
,  "Sep"
,  "Oct"
,  "Nov"
,  "Dec"
};

//----------------------------------------------------------------------------
// Global initialization/termination
//----------------------------------------------------------------------------
#if 0  // Not needed
static struct StaticGlobal {
   StaticGlobal( void )             // Static constructor
{  if( HCDM ) debugf("System::StaticGlobal!\n");

}

   ~StaticGlobal( void )            // Static destructor
{  if( HCDM ) debugf("System::StaticGlobal~\n");
}
}  static_global;
}  // Anonymous namespace
#endif // Not needed

//----------------------------------------------------------------------------
//
// Subroutine-
//       make_dir
//
// Purpose-
//       Insure directory exists
//
// Implementation notes-
//       Consider making this more general and exposing the interface.
//
//----------------------------------------------------------------------------
static inline bool                  // TRUE iff successful
   make_dir(string path)            // Insure directory exists
{
   struct stat info;
   int rc= stat(path.c_str(), &info);
   if( rc != 0 ) {
     rc= mkdir(path.c_str(), DIR_MODE);
     if( rc )
       return false;
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       System::Accessor methods
//
// Purpose-
//       Get/Set System controls
//
//       System::get_log_level      Get current log_level
//       System::set_log_level      Set current log_level
//
// Implementation notes-
//       The log_level is the minimum value that causes log messages to be
//       written to stdout.
//
//----------------------------------------------------------------------------
log_level_t                         // The current console log_level
   get_log_level( void )            // Get current console log_level
{  return log_level; }

void
   set_log_level(                   // Set console log_level
     log_level_t      level)        // To this level
{  if( level < LL_INFO ) level= LL_INFO; log_level= level; }

//----------------------------------------------------------------------------
//
// Method-
//       System::debug
//
// Purpose-
//       Debugging displays
//
//----------------------------------------------------------------------------
void
   debug(                           // Debugging display
     const char*       info)        // Caller information
{
   {{{{ // The Debug lock insures sequential debugf outputs
     std::lock_guard<Debug> debug(*Debug::get());

     debugf("System::debug(%s)\n", info);

     // Thread report
     debugf("\n");
     Thread::static_debug(info);

     // WorkerPool report
     debugf("\n");
     WorkerPool::debug(info);

     // Reporter report (If active Reporter)
     Reporter* reporter= Reporter::show();
     if( reporter ) {
       debugf("\nReporter.report()\n");
       reporter->report([](Reporter::Record& record) {
         string report= record.h_report();
         debugf("%s\n", report.c_str());
       }); // reporter->report
     }
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       System::Hardware accessor methods
//
// Purpose-
//       Get hardware information
//
//       System::get_LR   Get Link Register (Caller's return address)
//       System::get_SP   Get Stack Pointer
//       System::get_TSC  Get Time Stamp Counter
//
//----------------------------------------------------------------------------

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// get_LR
#if defined(__GNUC__)               // GNU compiler required
void*                               // The Link Register
   get_LR( void )                   // Get Link Register
{  return __builtin_extract_return_addr(__builtin_return_address(0)); }

#else // (Simulated) - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void*
   get_LR( void )
{  return nullptr; }
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// get_SP
#if  defined(__GNUC__)              // GNU compiler required
void*                               // The Stack Pointer
   get_SP( void )                   // Get Stack Pointer
{  return __builtin_frame_address(0); }

#else // (Simulated) - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void*
   get_SP( void )
{  return nullptr; }
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// get_TSC
#if defined(__GNUC__) && defined(_HW_X86) // GNU compiler, x86 required
uint64_t                            // The Time Stamp Counter
   get_TSC( void )                  // Get Time Stamp Counter
{
   uint64_t            result;      // Resultant

#ifdef __x86_64__                   // 64 bit assembler code
   asm volatile("\n"
"        rdtsc\n"                   // Read the time stamp counter
"        salq    $32, %%rdx\n"      // Position the upper resultant
"        orq     %%rdx, %%rax\n"    // Combine with lower resultant
       : "=a" (result)              // %0
       :                            // (No INP parameter)
       :
       );

   return result;
#else
   asm volatile("\n"                // 32 bit assembler code
"        rdtsc\n"                   // Read the time stamp counter
       : "=A" (result)              // %0
       :                            // (No INP parameter)
       :
       );
#endif

   return result;
}

#else // (Simulated) - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uint64_t
   get_TSC( void )
{  static std::atomic<std::uint64_t> tsc(0); return ++tsc; }
#endif

//----------------------------------------------------------------------------
//
// Method-
//       System::log
//
// Purpose-
//       Record an event in the system log.
//
// Implementation notes-
//       Additionally, if the current log_level is >= `level`, the message is
//       written to stdout.
//
//----------------------------------------------------------------------------
_LIBPUB_PRINTF(2, 3)
void
   log(                             // Record a system event
     log_level_t       level,       // The event level
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
{
   // The log file is opened and closed for each log invocation
   Debug* log_file= nullptr;
   bool   log_file_is_debug_out= false;

   {{{{
     // We use the Debug lock to serialize creation of the log_file directory,
     // and continue to hold that lock while writing to the log_file.
     // This also prevents us from interfering with debugging statements.
     std::lock_guard<Debug> lock(*Debug::get());

     // If necessary, create the log file subdirectory
     const char* env= getenv("HOME");
     if( env == nullptr ) {
       log_file_is_debug_out= true;
       log_file= Debug::get();
     } else {
       string path= env;
       path += "/.local/log";
       if( make_dir(path) ) {
         path += "/syslog.out";
         log_file= new Debug(path.c_str());
         if( log_file == nullptr ) {
           log_file_is_debug_out= true;
           log_file= Debug::get();
         } else {
           log_file->set_file_mode("ab");
         }
       } else {
         log_file_is_debug_out= true;
         log_file= Debug::get();
       }
     }

     // Write the date and time
     time_t gt= time(nullptr);
     struct tm* lt= localtime(&gt);

     if( level >= log_level )
       log_file->debugf("%3s %2d %4d %.2d:%.2d:%.2d "
                       , _month[lt->tm_mon], lt->tm_mday, lt->tm_year + 1900
                       , lt->tm_hour, lt->tm_min, lt->tm_sec);
     else
       log_file->tracef("%3s %2d %4d %.2d:%.2d:%.2d "
                       , _month[lt->tm_mon], lt->tm_mday, lt->tm_year + 1900
                       , lt->tm_hour, lt->tm_min, lt->tm_sec);

     // Write the log message
     va_list argptr;
     va_start(argptr, fmt);         // Initialize va_ functions
       if( level >= log_level )
         log_file->vdebugf(fmt, argptr);
       else
         log_file->vtracef(fmt, argptr);
     va_end(argptr);                // Close va_ functions

     // Cleanup
     log_file->flush();             // Insure log written
     if( log_file_is_debug_out ) {
       static bool once= true;
       if( once ) {
         once= false;
         debugf("Unable to create \"%s\"\n", log_file_name);
       }
     } else {
       delete log_file;
     }
   }}}}
}
} // namespace _LIBPUB_NAMESPACE::System
