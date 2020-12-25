//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Config.cpp
//
// Purpose-
//       Editor: Implement Config.h
//
// Last change date-
//       2020/12/25
//
//----------------------------------------------------------------------------
#include <fcntl.h>                  // For open, ...
#include <stdio.h>                  // For fprintf
#include <stdlib.h>                 // For various
#include <unistd.h>                 // For close, ...
#include <sys/mman.h>               // For mmap, ...
#include <sys/signal.h>             // For signal, ...
#include <sys/stat.h>               // For stat

#include <pub/Debug.h>              // For pub::Debug, namespace pub::debugging
#include <pub/Fileman.h>            // For namespace pub::Fileman
#include <pub/Signals.h>            // For pub::signals
#include <pub/Thread.h>             // For pub::Thread
#include <pub/Trace.h>              // For pub::Trace
#include <pub/utility.h>            // For pub::utility::to_stringv, ...

#include "Config.h"                 // For Config (Implementation class)
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile
#include "EdHist.h"                 // For EdFile TODO: REMOVE
#include "EdText.h"                 // For EdText

using namespace config;             // For implementation
using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  PROT_RW= (PROT_READ | PROT_WRITE)
,  DIR_MODE= (S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH)
,  TRACE_SIZE= pub::Trace::TABLE_SIZE_MIN
,  USE_BRINGUP= false               // Extra bringup diagnostics?
}; // Compilation controls

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static pub::Debug*     debug= nullptr; // Our Debug object
static void*           trace_table= nullptr; // The internal trace area

// Signal handlers
typedef void (*sig_handler_t)(int);
static sig_handler_t   sys1_handler= nullptr; // System SIGINT  signal handler
static sig_handler_t   sys2_handler= nullptr; // System SIGSEGV signal handler
static sig_handler_t   usr1_handler= nullptr; // System SIGUSR1 signal handler
static sig_handler_t   usr2_handler= nullptr; // System SIGUSR2 signal handler

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
// Debugging controls (initialized externally)
const char*            config::opt_test= nullptr;  // Bringup test?
int                    config::opt_hcdm= false;    // Hard Core Debug Mode?
int                    config::opt_verbose= false; // Debug verbosity

// Operational controls
int                    config::autowrap= false;
int                    config::ignore_case= true;
int                    config::search_mode= 0;

// One-time lookups
std::string            config::AUTO; // AUTOSAVE directory
std::string            config::HOME; // Home directory

// Signals
pub::signals::Signal<const char*> config::checkSignal; // The CheckEvent signal
pub::signals::Signal<const char*> config::debugSignal; // The DebugEvent signal
pub::signals::Signal<const int> config::signalSignal; // The SignalEvent signal

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
static void make_dir(std::string path);
static void make_file(std::string name, const char* data);

static int  init( void );           // Initialize
static void term( void );           // Terminate

//----------------------------------------------------------------------------
// Default .README.txt
//----------------------------------------------------------------------------
static const char*     README_text=
   "[Program]\n"
   "Http=http://eske-systems.com\n"
   "Exec=view\n"
   "Exec=edit\n"
   "Purpose=Graphic text editor\n"
   "Version=0.0\n"
   "\n"
   "[Options]\n"
   "## autosave_dir=~/.cache/uuid/e743e3ac-6816-4878-81a2-b47c9bbc2d37\n"
   "## ignore_case=true\n"
   ;

//----------------------------------------------------------------------------
//
// Method-
//       Config::Config
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Config::Config(int, char**)      // Constructor
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{  if( opt_hcdm ) fprintf(stderr, "Config::Config\n"); // (Don't use debugf)
   using namespace config;

   // Initialize HOME and AUTO
   const char* env= getenv("HOME"); // Get HOME directory
   if( env == nullptr )
     Config::failure("No HOME directory");
   HOME= env;

   // If required, create "$HOME/.config/uuid/" + UUID + "/.README.txt"
   std::string S= HOME + "/.config";
   make_dir(S);
   S += "/uuid";
   make_dir(S);
   S += std::string("/") + UUID;
   make_dir(S);
   AUTO= S;

   S += std::string("/.README.txt");
   make_file(S, README_text);

   // Parse the configuration file
   // TODO: NOT CODED YET

   // Set AUTOSAVE subdirectory
   env= getenv("AUTOSAVE");         // Get AUTOSAVE directory override
   if( env )
     AUTO= env;

   // Look for any *AUTOSAVE* file in AUTOSAVE subdirectory
   pub::Fileman::Path path(AUTO);
   pub::Fileman::File* file= path.list.get_head();
   while( file ) {
     if( file->name.find(AUTOFILE) == 0 )
       Config::failure(std::string("File exists: ") + AUTO + "/" + file->name);

     file= file->get_next();
   }

   // System related initialization
   if( init() )
     Config::failure("Initialization failed");
}

//----------------------------------------------------------------------------
//
// Method-
//       Config::~Config
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Config::~Config( void )          // Destructor
{  if( opt_hcdm ) fprintf(stderr, "Config::~Config\n"); // (Don't use debugf)

   term();
}

//----------------------------------------------------------------------------
//
// Method-
//       Config::alertf
//
// Purpose-
//       Debugging alert
//
//----------------------------------------------------------------------------
void
   Config::alertf(                  // Debugging alert
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   std::string S= pub::utility::to_stringv(fmt, argptr);
   va_end(argptr);                  // Close va_ functions

   debugf("%s\n", S.c_str());
   debug(S.c_str());
   Config::trace(".BUG", __LINE__, "Config.cpp");
   backtrace();

   editor::put_message(S.c_str(), EdMess::T_MESS);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Config::backtrace
//
// Purpose-
//       Display backtrace information. (CYGWIN version does nothing.)
//
//----------------------------------------------------------------------------
#define BOOST_STACKTRACE_LINK       // (Use static library)
#define BOOST_STACKTRACE_USE_BACKTRACE
#include <iostream>                 // For std::cerr
#include <boost/stacktrace.hpp>     // For boost::stacktrace::stacktrace()
void
   Config::backtrace( void )        // Display backtrace information
{  std::cerr << boost::stacktrace::stacktrace(); }

//----------------------------------------------------------------------------
//
// Method-
//       Config::check
//
// Purpose-
//       Raise checkSignal
//
//----------------------------------------------------------------------------
void
   Config::check(                   // Debugging consistency check
     const char*       info)        // Informational text
{  checkSignal.signal(info); }

//----------------------------------------------------------------------------
//
// Method-
//       Config::debug
//
// Purpose-
//       Debugging display (modified as needed)
//
//----------------------------------------------------------------------------
void
   Config::debug(                   // Debugging display
     const char*       info)        // Informational text
{  debugSignal.signal(info); }

//----------------------------------------------------------------------------
//
// Method-
//       Config::errorf
//
// Purpose-
//       Write to stderr, write to debug trace file iff opt_hcdm
//
//----------------------------------------------------------------------------
void
   Config::errorf(                  // Debug write to stderr
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stderr, fmt, argptr);   // Write to stderr
   va_end(argptr);                  // Close va_ functions

   if( opt_hcdm ) {                 // If Hard Core Debug Mode
     va_start(argptr, fmt);
     pub::debugging::vtraceh(fmt, argptr);
     va_end(argptr);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Config::failure
//
// Purpose-
//       Write error message and exit
//
//----------------------------------------------------------------------------
void
   Config::failure(                 // Write error message and exit
     std::string       mess)        // (The error message)
{  errorf("%s\n", mess.c_str()); exit(EXIT_FAILURE); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       Config::trace
//
// Purpose-
//       Trace utilities
//
//----------------------------------------------------------------------------
void*                               // The trace record (uninitialized)
   Config::trace(                   // Get trace record
     unsigned          size)        // Of this extra size
{
  typedef ::pub::Trace::Record Record;
  size += unsigned(sizeof(Record));
  Record* record= (Record*)::pub::Trace::storage_if(size);
  return record;
}

void
   Config::trace(                   // Simple trace event
     const char*       ident,       // Trace identifier
     uint32_t          code,        // Trace code
     const char*       info)        // Trace info (15 characters max)
{
   typedef ::pub::Trace::Record Record;
   Record* record= (Record*)trace();
   if( record ) {
     char* unit= (char*)&record->unit;
     unit[3]= char(code >>  0);
     unit[2]= char(code >>  8);
     unit[1]= char(code >> 16);
     unit[0]= char(code >> 24);

     memset(record->value, 0, sizeof(record->value));
     if( info )
       strcpy(record->value, info);
     record->trace(ident);
   }
}

void
   Config::trace(                   // Simple trace event
     const char*       ident,       // Trace identifier
     const char*       code,        // Trace sub-identifier
     void*             _one,        // Word one
     void*             _two)        // Word two
{
   uintptr_t one= uintptr_t(_one);
   uintptr_t two= uintptr_t(_two);

   typedef ::pub::Trace::Record Record;
   Record* record= (Record*)trace();
   if( record ) {
     memcpy(&record->unit, code, 4);

     for(unsigned i= 8; i>0; i--) {
       record->value[0 + i - 1]= char(one);
       record->value[8 + i - 1]= char(two);
       one >>= 8;
       two >>= 8;
     }
     record->trace(ident);
   }
}

//----------------------------------------------------------------------------
// Subroutine: make_dir, insure directory exists
//----------------------------------------------------------------------------
static void make_dir(std::string path) // Insure directory exists
{
   struct stat info;
   int rc= stat(path.c_str(), &info);
   if( rc != 0 ) {
     rc= mkdir(path.c_str(), DIR_MODE);
     if( rc )
       Config::failure(std::string("Cannot create ") + path);
   }
}

//----------------------------------------------------------------------------
// Subroutine: make_file, insure file exists
//----------------------------------------------------------------------------
static void make_file(std::string name, const char* data) // Insure file exists
{
   struct stat info;
   int rc= stat(name.c_str(), &info);
   if( rc != 0 ) {
     FILE* f= fopen(name.c_str(), "wb"); // Open the file
     if( f == nullptr )             // If open failure
       Config::failure(std::string("Cannot create ") + name);

     size_t L0= strlen(data);
     size_t L1= fwrite(data, 1, L0, f);
     rc= fclose(f);
     if( L0 != L1 || rc )
       Config::failure(std::string("Write failure: ") + name);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       sig_handler
//
// Purpose-
//       Handle signals.
//
//----------------------------------------------------------------------------
static void
   sig_handler(                     // Handle signals
     int               id)          // The signal identifier
{
   static int recursion= 0;         // Signal recursion depth
   if( recursion ) {                // If signal recursion
     fprintf(stderr, "sig_handler(%d) recursion(%d)\n", id, recursion);
     abort();
   }

   // Handle signal
   recursion++;                     // Disallow recursion
   const char* text= "<<Unexpected>>";
   if( id == SIGINT ) text= "SIGINT";
   else if( id == SIGSEGV ) text= "SIGSEGV";
   else if( id == SIGUSR1 ) text= "SIGUSR1";
   else if( id == SIGUSR2 ) text= "SIGUSR2";
   Config::errorf("\n\nsig_handler(%d) %s\n", id, text);

   switch(id) {                     // Handle the signal
     case SIGINT:                   // (Console CTRL-C)
       exit(EXIT_FAILURE);          // Unconditional immediate exit
       break;

     case SIGSEGV:
       Config::backtrace();         // Attempt diagnosis (recursion aborts)
       Config::debug("SIGSEGV");
       Config::errorf("..terminating..\n");
       exit(EXIT_FAILURE);          // Exit, no stacktrace
       break;

     default:
       signalSignal.signal(id);
       break;
   }

   recursion--;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   init( void )                     // Initialize
{  if( opt_hcdm ) fprintf(stderr, "Config::init\n"); // (Don't use debugf)

   //-------------------------------------------------------------------------
   // BRINGUP: Register term as an atexit handler
// atexit(&term);                   // TODO: DISABLE

   //-------------------------------------------------------------------------
   // Initialize/activate debugging trace (with options)
   std::string S= AUTO + "/debug.out";
   debug= new pub::Debug(S.c_str());
   debug->set_head(pub::Debug::HEAD_TIME);
   pub::Debug::set(debug);

// if( opt_hcdm )                   // If Hard Core Debug Mode TODO: REPAIR
     debug->set_mode(pub::Debug::MODE_INTENSIVE);

   //-------------------------------------------------------------------------
   // Create memory-mapped trace file
   S= AUTO + "/trace.out";
   int fd= open(S.c_str(), O_RDWR | O_CREAT, S_IRWXU);
   if( fd < 0 ) {
     Config::errorf("%4d open(%s) %s\n", __LINE__, S.c_str()
                   , strerror(errno));
     return 1;
   }

   int rc= ftruncate(fd, TRACE_SIZE); // (Expand to TRACE_SIZE)
   if( rc ) {
     Config::errorf("%4d ftruncate(%s,%.8x) %s\n", __LINE__
                   , S.c_str(), TRACE_SIZE, strerror(errno));
     return 1;
   }

   trace_table= mmap(nullptr, TRACE_SIZE, PROT_RW, MAP_SHARED, fd, 0);
   if( trace_table == MAP_FAILED ) { // If no can do
     Config::errorf("%4d mmap(%s,%.8x) %s\n", __LINE__
                   , S.c_str(), TRACE_SIZE, strerror(errno));
     trace_table= nullptr;
     return 1;
   }

   pub::Trace::trace= pub::Trace::make(trace_table, TRACE_SIZE);
   close(fd);                       // Descriptor not needed once mapped
   // TODO: REMOVE (BRINGUP)
   debugf("%4d Config::init backtrace test\n", __LINE__);
   Config::backtrace();
   debugf("%4d Config::init(%d) %s %s\n", __LINE__, getpid(), __DATE__, __TIME__);

   //-------------------------------------------------------------------------
   // Initialize signal handling
   sys1_handler= signal(SIGINT,  sig_handler);
   sys2_handler= signal(SIGSEGV, sig_handler);                // TODO: REPAIR
   debugf("%4d Config SIGSEGV handler enabled\n", __LINE__);  // TODO: REMOVE
   usr1_handler= signal(SIGUSR1, sig_handler);
   usr2_handler= signal(SIGUSR2, sig_handler);

   //-------------------------------------------------------------------------
   // Initialize locale
   setlocale(LC_NUMERIC, "");       // Allows printf("%'d\n", 123456789);
   printf("%4d Config::init(%'d) ?commas?\n", __LINE__, 123456789);

   return 0;                        // Placeholder
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       terminate
//
// Implementation note-
//       May be called multiple times
//
//----------------------------------------------------------------------------
static void
   term( void )                     // Terminate
{  if( opt_hcdm ) fprintf(stderr, "Config::term\n"); // (Don't use debugf)

   //-------------------------------------------------------------------------
   // Restore system signal handlers
   if( sys1_handler ) signal(SIGINT,  sys1_handler);
   if( sys2_handler ) signal(SIGSEGV, sys2_handler);
   if( usr1_handler ) signal(SIGUSR1, usr1_handler);
   if( usr2_handler ) signal(SIGUSR2, usr2_handler);
   sys1_handler= sys2_handler= usr1_handler= usr2_handler= nullptr;

   //-------------------------------------------------------------------------
   // Free the trace table (and disable tracing)
   if( trace_table ) {
     pub::Trace::trace= nullptr;
     munmap(trace_table, TRACE_SIZE);
     trace_table= nullptr;
   }

   //-------------------------------------------------------------------------
   // Terminate debugging
   pub::Debug::set(nullptr);        // Remove Debug object
   delete debug;                    // and delete it
   debug= nullptr;

   opt_hcdm= false;                 // Prevent Config::errorf tracing
}

//============================================================================
// DEBUGGING EXTENSION: configCheck, configDebug, and configSignal listaners
//============================================================================
#include "Config.patch"             // Default config signal listeners
