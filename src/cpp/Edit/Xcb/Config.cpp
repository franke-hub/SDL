//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2024 Frank Eskesen.
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
//       2024/08/23
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <ctype.h>                  // For isspace
#include <errno.h>                  // For errno
#include <fcntl.h>                  // For open, O_*, ...
#include <limits.h>                 // For INT_MAX, INT_MIN, ...
#include <stdarg.h>                 // For va_* functions
#include <stdio.h>                  // For fprintf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcpy, strerror, ...
#include <unistd.h>                 // For close, ...
#include <arpa/inet.h>              // For htons
#include <sys/mman.h>               // For mmap, ...
#include <sys/signal.h>             // For signal, ...
#include <sys/stat.h>               // For stat

#include <pub/config.h>             // For ATTRIB_PRINTF macro
#include <pub/Debug.h>              // For pub::Debug, namespace pub::debugging
#include <pub/Fileman.h>            // For namespace pub::fileman
#include <pub/Parser.h>             // For pub::Parser
#include <pub/Signals.h>            // For pub::signals
#include <pub/Trace.h>              // For pub::Trace

#include "Active.h"                 // For Active
#include "Config.h"                 // For Config (Implementation class)
#include "EdData.h"                 // For EdData::debug
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile::debug
#include "EdHist.h"                 // For EdFile::debug
#include "EdMark.h"                 // For EdMark::debug
#include "EdOpts.h"                 // For EdOpts
#include "EdOuts.h"                 // For -D IODM recompile signal
#include "EdUnit.h"                 // For EdUnit
#include "EdView.h"                 // For EdView::debug

using namespace config;             // For implementation
using namespace pub::debugging;     // For pub::debugging
using pub::Debug;                   // For pub::Debug
using pub::Trace;                   // For pub::Trace

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
static void sig_handler(int);       // The signal handler

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?

,  PROT_RW= (PROT_READ | PROT_WRITE)
,  DIR_MODE= (S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH)
,  TRACE_SIZE= 0x00100000           // Trace table size (1,048,576)
}; // Compilation controls

//----------------------------------------------------------------------------
// Internal structures
//----------------------------------------------------------------------------
struct Option {                     // Value name, address pair
const char*            name;        // The value name
uint32_t*              addr;        // The value value
}; // struct Option

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static pub::Debug*     debug= nullptr; // Our Debug object
static std::string     debug_path;  // The debugging output path name
static void*           trace_table= nullptr; // The internal trace area

// Signal handlers
typedef void           (*sig_handler_t)(int);
static sig_handler_t   sys1_handler= nullptr; // System SIGINT  signal handler
static sig_handler_t   sys2_handler= nullptr; // System SIGSEGV signal handler
static sig_handler_t   usr1_handler= nullptr; // System SIGUSR1 signal handler
static sig_handler_t   usr2_handler= nullptr; // System SIGUSR2 signal handler

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
// Debugging controls- From command line -------------------------------------
int                    config::opt_hcdm= false; // Hard Core Debug Mode?
int                    config::opt_verbose= 0;  // Verbosity, larger == more

// Debugging controls- From compile-time definition (-D IODM ) ---------------
int                    config::opt_iodm= false; // I/O Debug Mode?

// Screen colors ----- From configuration file -------------------------------
uint32_t               config::mark_bg=    0x00C0F0FF; // Text BG (marked)
uint32_t               config::mark_fg=    0x00000000; // Text FG (marked)

uint32_t               config::text_bg=    0x00FFFFF0; // Text BG (default)
uint32_t               config::text_fg=    0x00000000; // Text FG (default)

uint32_t               config::change_bg=  0x00F08080; // Status BG (modified)
uint32_t               config::change_fg=  0x00000000; // Status FG (modified)

uint32_t               config::status_bg=  0x0080F080; // Status BG (pristine)
uint32_t               config::status_fg=  0x00000000; // Status FG (pristine)

uint32_t               config::message_bg= 0x00FFFF00; // Message BG
uint32_t               config::message_fg= 0x00900000; // Message FG

// Screen controls --- From configuration file -------------------------------
geometry_t             config::geom= {0, 0, 80, 50}; // The screen geometry

// Bringup controls -- From configuration file or set command ----------------
uint32_t               config::USE_MOUSE_HIDE= true; // Use mouse hide logic?

// (Internal) -------- Initialized at startup --------------------------------
std::string            config::AUTO; // AUTOSAVE directory
std::string            config::HOME; // HOME directory (getenv("HOME"))

// (Internal) -------- Global event signals ----------------------------------
// Implementation note: Static signals *MUST BE* initialized on access
static pub::signals::Signal<const char*>*
                       the_check_signal= nullptr;

pub::signals::Signal<const char*>*  // The RAII check_signal (pointer)
   config::check_signal(void)
{
   static pub::Latch latch;
   std::lock_guard<pub::Latch> lock(latch);
   if( the_check_signal == nullptr )
     the_check_signal= new pub::signals::Signal<const char*>();

   return the_check_signal;
}

namespace {
static struct cleanup {             // On termination, delete the_check_signal
   ~cleanup( void ) { delete the_check_signal; the_check_signal= nullptr; }
} static_cleanup;
} // (Anonymous namespace)

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
{  if( opt_hcdm ) printf("Config::init\n"); // (debug not initialized)

   //-------------------------------------------------------------------------
   // Initialize signal handling
   sys1_handler= signal(SIGINT,  sig_handler);
   sys2_handler= signal(SIGSEGV, sig_handler);
   usr1_handler= signal(SIGUSR1, sig_handler);
   usr2_handler= signal(SIGUSR2, sig_handler);

   //-------------------------------------------------------------------------
   // Register term as an atexit handler
// atexit(&term);                   // (Optional, not really needed)

   //-------------------------------------------------------------------------
   // Initialize/activate debugging trace (with options)
   std::string S= debug_path + "/debug.out";
   debug= new pub::Debug(S.c_str());
   debug->set_head(pub::Debug::HEAD_TIME);
   pub::Debug::set(debug);

   bool opt_intense= opt_hcdm;
   #ifdef HCDM
     opt_hcdm= true;
     opt_intense= true;
   #endif

   #ifdef IODM
     opt_iodm= true;
     opt_intense= true;
   #endif

   if( opt_intense )                // If Intensive Debug Mode
     debug->set_mode(pub::Debug::MODE_INTENSIVE);

   if( opt_hcdm )                   // If Hard Core Debug Mode
     traceh("Editor PID(%4d) VID: %s %s\n", getpid(), __DATE__, __TIME__);

   //-------------------------------------------------------------------------
   // Create memory-mapped trace file
   S= debug_path + "/trace.mem";
   int fd= open(S.c_str(), O_RDWR|O_CREAT, S_IRUSR|S_IWUSR| S_IRGRP| S_IROTH);
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

   pub::Trace::table= pub::Trace::make(trace_table, TRACE_SIZE);
   close(fd);                       // Descriptor not needed once mapped

   //-------------------------------------------------------------------------
   // Create the keyboard, screen, and mouse handler
   editor::unit= EdOpts::initialize();

   return 0;
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
{  if( opt_hcdm ) traceh("Config::term\n");

   // Delete the Unit
   if( editor::unit ) {
     EdOpts::terminate(editor::unit);
     editor::unit= nullptr;
   }

   //-------------------------------------------------------------------------
   // Terminate debugging. Note: pub::Debug global termination can handle this
   if( false && !opt_hcdm ) {
     pub::Debug::set(nullptr);      // Remove Debug object
     delete debug;                  // and delete it
     debug= nullptr;

     opt_hcdm= false;               // Prevent Config::errorf tracing
   }

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
     pub::Trace::table= nullptr;
     munmap(trace_table, TRACE_SIZE);
     trace_table= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       config_verrorf
//
// Purpose-
//       Display error message
//
//----------------------------------------------------------------------------
static void
   config_verrorf(                  // Debug write to stderr
     const char*       fmt,         // The PRINTF format string
     va_list           arginp)      // Argument list pointer
{
   va_list             argptr;      // Argument list pointer

   va_copy(argptr, arginp);         // Initialize va_ functions
   vfprintf(stderr, fmt, argptr);   // Write to stderr
   va_end(argptr);                  // Close va_ functions

   if( config::opt_hcdm ) {         // If Hard Core Debug Mode
     va_copy(argptr, arginp);
     pub::debugging::vtraceh(fmt, argptr);
     va_end(argptr);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       make_dir
//       make_file
//
// Purpose-
//       Insure directory exists
//       Insure file exists
//
//----------------------------------------------------------------------------
static void make_dir(std::string path) // Insure directory exists
{
   struct stat info;
   int rc= stat(path.c_str(), &info);
   if( rc != 0 ) {
     rc= mkdir(path.c_str(), DIR_MODE);
     if( rc )
       Config::failure("Cannot create %s", path.c_str());
   }
}

static void make_file(std::string name, std::string data) // Insure file exists
{
   struct stat info;
   int rc= stat(name.c_str(), &info);
   if( rc != 0 ) {
     FILE* f= fopen(name.c_str(), "wb"); // Open the file
     if( f == nullptr )             // If open failure
       Config::failure("Cannot create %s",  name.c_str());

     size_t L0= data.size();
     size_t L1= fwrite(data.c_str(), 1, L0, f);
     rc= fclose(f);
     if( L0 != L1 || rc )
       Config::failure("Write failure: %s", name.c_str());
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
     fprintf(stderr, "sig_handler(%d) recursion\n", id);
     fflush(stderr);
     exit(EXIT_FAILURE);
   }

   // Handle signal
   recursion++;                     // Disallow recursion
   const char* text= "<<Unexpected>>";
   if( id == SIGINT ) text= "SIGINT";
   else if( id == SIGSEGV ) text= "SIGSEGV";
   else if( id == SIGUSR1 ) text= "SIGUSR1";
   else if( id == SIGUSR2 ) text= "SIGUSR2";
   Config::errorf("sig_handler(%d) %s\n", id, text);

   switch(id) {                     // Handle the signal
     case SIGINT:                   // (Console CTRL-C)
       Trace::trace(".BUG", __LINE__, text);
       debug_set_mode(Debug::MODE_INTENSIVE);
       term();                      // Termination cleanup, then
       exit(EXIT_FAILURE);          // Unconditional immediate exit
       break;

     case SIGSEGV:                  // (Program fault)
       Trace::trace(".BUG", __LINE__, text);
       debug_set_mode(Debug::MODE_INTENSIVE);
       EdOpts::at_exit();           // Abnormal termination
       debug_backtrace();           // Attempt diagnosis (recursion aborts)
       Config::debug("SIGSEGV");
       debugf("..terminated..\n");
       exit(EXIT_FAILURE);
       break;

     default:                       // (SIGUSR1 || SIGUSR2)
       Trace::trace(".SIG", __LINE__, text);
       break;                       // (No configured action)
   }

   recursion--;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       strtoi
//
// Purpose-
//       Integer version of strtol
//
//----------------------------------------------------------------------------
static int                          // Resultant value
   strtoi(                          // Ascii string to integer
     const char*       head,        // First character
     char**            tail,        // (OUTPUT) Last character
     int               base= 0)     // Radix
{
   long R= strtol(head, tail, base); // Resultant
   if( R < INT_MIN || R > INT_MAX ) { // If range error
     errno= ERANGE;                 // Indicate range error
     *tail= (char*)head;
     R= 0;
   }

   return int(R);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parser
//
// Purpose-
//       Configuration parser
//
//----------------------------------------------------------------------------
struct Option          bool_list[]= // Boolean parameter list
{ {"locate.prior",     &editor::locate_back}
, {"locate.mixed",     &editor::locate_case}
, {"locate.wrap",      &editor::locate_wrap}
, {"USE_MOUSE_HIDE",   &USE_MOUSE_HIDE}
, {nullptr,            nullptr}     // End of list
};

struct Option          color_list[]= // The color parameter list
{ {"mark.bg",          &mark_bg}
, {"mark.fg",          &mark_fg}
, {"text.bg",          &text_bg}
, {"text.fg",          &text_fg}
, {"change.bg",        &change_bg}
, {"change.fg",        &change_fg}
, {"status.bg",        &status_bg}
, {"status.fg",        &status_fg}
, {"message.bg",       &message_bg}
, {"message.fg",       &message_fg}
, {nullptr,            nullptr}      // End of list
};

ATTRIB_PRINTF(2, 3)
static void
   parse_error(                     // Handle parse error
     std::string       file,        // The parse file name
     const char*       fmt,         // The PRINTF format string
                       ...);        // PRINTF argruments

ATTRIB_PRINTF(2, 3)
static void
   parse_error(                     // Handle parse error
     std::string       file,        // The parse file name
     const char*       fmt,         // The PRINTF format string
                       ...)         // PRINTF argruments
{
   static int error_count= 0;       // Error counter

   if( error_count == 0 )
     fprintf(stderr, "Config File(%s)\n", file.c_str());
   error_count++;

   va_list             argptr;      // Argument list pointer
   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stderr, fmt, argptr);   // Write to stderr
   va_end(argptr);                  // Close va_ functions
}

static void                         // {-1, 0, +1} [error, false, true]
   parse_bool(                      // Parse boolean value
     std::string       file,        // The parse file name
     const Option&     item,        // The item option descriptor
     const char*       VALUE)       // The item value string
{
   int32_t value= -1;               // Default, invalid value

   if( strcasecmp(VALUE, "true") == 0 )
     value= true;
   else if( strcasecmp(VALUE, "false") == 0 )
     value= false;

   else if( strcasecmp(VALUE, "1") == 0 )
     value= true;
   else if( strcasecmp(VALUE, "0") == 0 )
     value= false;

   else if( strcasecmp(VALUE, "on") == 0 )
     value= true;
   else if( strcasecmp(VALUE, "off") == 0 )
     value= false;

   else if( strcasecmp(VALUE, "yes") == 0 )
     value= true;
   else if( strcasecmp(VALUE, "no") == 0 )
     value= false;

   if( value >= 0 )                 // If the value is valid
     *item.addr= (uint32_t)value;   // (true or false)
   else
     parse_error(file, "Name(%s) '%s' invalid\n", item.name, VALUE);
}

static int                          // The integer value
   parse_int(                       // Parse integer value
     const char*&      value)       // (IN/OUT) The current value string
{
   char* tail;                      // Ending character
   int V= strtoi(value, &tail);     // Get value
   if( value == tail ) {            // If invalid
     value= nullptr;
     return -1;
   }

   value= tail;
   while( isspace(*value) )         // Remove trailing white space
     ++value;

   return V;
}

static void
   parse_color(                     // Parse color option
     std::string       file,        // The parse file name
     const Option&     item,        // The item option descriptor
     const char*       VALUE)       // The item value string
{
   const char* value= VALUE;        // Working value

   uint32_t color= 0;
   for(int rgb= 0; rgb < 3; rgb++) {
     int cc= parse_int(value);      // Get color component
     if( value == nullptr || cc < 0 || cc > 255 ) {
       parse_error(file, "Name(%s) '%s' invalid\n", item.name, VALUE);
       return;
     }

     color <<= 8;
     color  += cc;

     if( *value == ',' && rgb < 2 )
       value++;
   }

   if( *value != '\0' ) {
     parse_error(file, "Name(%s) '%s' invalid\n", item.name, VALUE);
     return;
   }

   *item.addr= color;
}

static void
   parser(                          // Configuration parser
     std::string       file_name)   // The parser file name
{  if( opt_hcdm )                   // (Don't use debugf here)
     printf("Config::parser(%s)\n", file_name.c_str());

   static bool font_valid= false;   // Font parameter present and valid?

   pub::Parser parser(file_name);   // The parser object
   for(const char* sect= parser.get_next(nullptr); // Parse sections
       sect;
       sect= parser.get_next(sect)) {
     if( strcmp(sect, "Options") == 0 ) { // Section [Options]
       for(const char* name= parser.get_next(sect, nullptr); name;
           name= parser.get_next(sect, name)) {
         const char* value= parser.get_value(sect,name);
         Option* option= nullptr;
         for(int i= 0; color_list[i].name; i++) {
           if( strcmp(name, color_list[i].name) == 0 ) {
             option= color_list + i;
             break;
           }
         }
         if( option ) {
           parse_color(file_name, *option, value);
           continue;
         }

         option= nullptr;
         for(int i= 0; bool_list[i].name; i++) {
           if( strcmp(name, bool_list[i].name) == 0 ) {
             option= bool_list + i;
             break;
           }
         }
         if( option ) {
           parse_bool(file_name, *option, value);
           continue;
         }

         if( strcmp(name, "font") == 0 ) { // Font parameter?
           int rc= editor::unit->set_font(value); // Set the font
           if( rc == 0 )
             font_valid= true;
           continue;
         }

         if( strcmp(name, "geometry") == 0 ) {
           if( value && *value == '\0' ) // If geometry=
             continue;              // Ignore, omitted
           geometry_t geom= {0, 0, 0, 0}; // The screen geometry
           const char* VALUE= value; // (For use in error message)
           geom.width= uint32_t(parse_int(value)); // Number of rows
           if( value && *value == 'x' ) {
             value++;
             geom.height= uint32_t(parse_int(value)); // Number of columns
             if( value && (*value == '+' || *value == '-') )
               geom.x= int32_t(parse_int(value)); // X position
             if( value && (*value == '+' || *value == '-') )
               geom.y= int32_t(parse_int(value)); // Y position
           } else                   // (Possibly x without y)
             value= VALUE;
           if( value && *value == '\0' ) { // If valid geometry
             config::geom= geom;
             editor::unit->set_geom(geom);
           } else {
             parse_error(file_name, "geometry(%s) invalid\n", VALUE);
           }
           continue;
         }

         parse_error(file_name, "Invalid option name: '%s=%s'\n", name, value);
       }
     } else if( strcmp(sect, "Program") == 0 ) { // Section [Program]
       // [Program] elements are for user information only (and ignored here.)
     } else if( *sect != '\0' ) {
       parse_error(file_name, "Unknown section [%s]\n", sect);
     }
   }
   if( !font_valid ) {              // If valid font not present
     int rc= editor::unit->set_font(); // Use default font
     if( rc )                       // If default font is unusable
       Config::failure("Default font invalid");
   }
}

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
{  if( opt_hcdm ) printf("Config::Config\n"); // (Don't use debugf)

   using namespace config;

   // Get EdOpts static variables
   std::string EDITOR= EdOpts::EDITOR();
   std::string DEFAULT_CONFIG= EdOpts::DEFAULT_CONFIG();

   // Initialize HOME, AUTO, and debug_path
   const char* env= getenv("HOME"); // Get HOME directory
   if( env == nullptr )
     Config::failure("No HOME directory");
   HOME= env;

   // If required, create "$HOME/.local/state/`EDITOR`"
   std::string S= HOME + "/.local";
   make_dir(S);
   S += "/state";
   make_dir(S);
   S += "/";
   S += EDITOR;
   make_dir(S);
   AUTO= debug_path= S;

   // Override AUTOSAVE directory, if required
   env= getenv("AUTOSAVE");         // Get AUTOSAVE directory override
   if( env )
     AUTO= env;

   // Look for any *AUTOSAVE* file in AUTOSAVE subdirectory
   pub::fileman::Path path(AUTO);
   pub::fileman::File* file= path.list.get_head();
   while( file ) {
     if( file->name.find(AUTOFILE) == 0 )
       Config::failure("File exists: %s/%s", AUTO.c_str(), file->name.c_str());

     file= file->get_next();
   }

   // Locate, possibly creating "$HOME/.local/config/`EDITOR`/Edit.conf"
   S= HOME + "/.local";
// make_dir(S);
   S += "/config";
   make_dir(S);
   S += "/";
   S += EDITOR;
   make_dir(S);
   S += std::string("/Edit.conf");
   make_file(S, DEFAULT_CONFIG);

   // Initialize signal handlers, debugging, and editor::unit
   // NOTE: No pub::debugging method should be called before init() invocation
   if( init() )
     Config::failure("Initialization failed");

   // Parse the configuration file
   parser(S);
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
{  if( opt_hcdm ) traceh("Config::~Config\n");

   term();
}

//----------------------------------------------------------------------------
//
// Method-
//       Config::check
//
// Purpose-
//       Raise check_signal (Run debugging consistency checks)
//
// Implementation notes-
//       Listener: EdFile.hpp
//
//----------------------------------------------------------------------------
void
   Config::check(                   // Debugging consistency check
     const char*       info)        // Informational text
{  check_signal()->signal(info); }

//----------------------------------------------------------------------------
//
// Method-
//       Config::debug
//
// Purpose-
//       Debugging displays
//
//----------------------------------------------------------------------------
void
   Config::debug(                   // Debugging displays
     const char*       info)        // Informational text
{
   static int recursion= 0;         // Recursion count (to avoid recursion)

   if( info == nullptr )
     info= "";

   debug_flush();
   traceh("\n============================================================\n");
   traceh("Config::debug(%s) recursion(%s)\n", info
         , recursion ? "true" : "false");
   if( recursion ) {
     debug_flush();
     return;
   }

   ++recursion;

   Editor::debug(info);
   traceh("\n");
   editor::mark->debug(info);
   traceh("\n");
   editor::file->debug("lines");
   traceh("\n");
   editor::unit->debug(info);
   traceh("\n");
   editor::data->debug(info);
   traceh("\n");
   editor::hist->debug(info);
   traceh("============================================================\n\n");
   debug_flush();

   --recursion;
}

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
   config_verrorf(fmt, argptr);     // Write to stderr, trace
   va_end(argptr);                  // Close va_ functions
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
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   config_verrorf(fmt, argptr);     // Write to stderr, trace
   va_end(argptr);                  // Close va_ functions
   errorf("\n");

   exit(EXIT_FAILURE);
}
