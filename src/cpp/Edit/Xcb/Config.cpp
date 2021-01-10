//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2021 Frank Eskesen.
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
//       2021/01/10
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <fcntl.h>                  // For open, O_*, ...
#include <limits.h>                 // For INT_MAX, INT_MIN, ...
#include <stdarg.h>                 // For va_* functions
#include <stdio.h>                  // For fprintf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcpy, strerror, ...
#include <unistd.h>                 // For close, ...
#include <sys/mman.h>               // For mmap, ...
#include <sys/signal.h>             // For signal, ...
#include <sys/stat.h>               // For stat

#include <pub/Debug.h>              // For pub::Debug, namespace pub::debugging
#include <pub/Fileman.h>            // For namespace pub::fileman
#include <pub/Parser.h>             // For pub::Parser
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
,  TRACE_SIZE= pub::Trace::TABLE_SIZE_MIN * 4 // Trace table size
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

// Constants
static const key_t     SHM_TOKEN= key_t(0x81a2b47c9bbc2dFE);
// static const unsigned char[16] uuid= // (Currently unused)
//                     { 0xe7, 0x43, 0xe3, 0xac, 0x68, 0x16, 0x48, 0x78
//                     , 0x81, 0xa2, 0xb4, 0x7c, 0x9b, 0xbc, 0x2d, 0x37};

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
// Debugging controls- From command line -------------------------------------
const char*            config::opt_test= nullptr;  // Bringup test?
int                    config::opt_hcdm= false;    // Hard Core Debug Mode?
int                    config::opt_verbose= false; // Debug verbosity

// Screen colors ----- From configuration file -------------------------------
uint32_t               config::mark_bg=    0x00C0F0FF; // Text BG (marked)
uint32_t               config::mark_fg=    0x00000000; // Text FG (marked)

uint32_t               config::text_bg=    0x00FFFFF0; // Text BG (default)
uint32_t               config::text_fg=    0x00000000; // Text FG (default)

uint32_t               config::change_bg=  0x00F08080; // Status BG (modified)
uint32_t               config::change_fg=  0x00000000; // Status FG (modified)

uint32_t               config::status_bg=  0x0080F080; // Status BG (pristine)
uint32_t               config::status_fg=  0x00000000; // Status FG (pristine)

uint32_t               config::command_bg= 0x0000FFFF; // Command BG
uint32_t               config::command_fg= 0x00000000; // Command FG

uint32_t               config::message_bg= 0x00FFFF00; // Message BG
uint32_t               config::message_fg= 0x00900000; // Message FG

// XCB objects ------- Initialized at startup (Font configured) --------------
xcb::Device*           config::device= nullptr; // The root Device
xcb::Window*           config::window= nullptr; // A TEST Window TODO: BRINGUP
xcb::Font*             config::font= nullptr; // The Font object

// (Internal) -------- Initialized at startup --------------------------------
std::string            config::AUTO; // AUTOSAVE directory (~/.config/...)
std::string            config::HOME; // HOME directory (getenv("HOME"))

// (Internal) -------- Global event signals ----------------------------------
pub::signals::Signal<const char*> config::checkSignal; // The CheckEvent signal
pub::signals::Signal<const char*> config::debugSignal; // The DebugEvent signal
pub::signals::Signal<const int> config::signalSignal; // The SignalEvent signal

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
static void make_dir(std::string path);
static void make_file(std::string name, const char* data);

static void parser(std::string);    // Option parser
static int  init( void );           // Initialize
static void term( void );           // Terminate

//----------------------------------------------------------------------------
// Default Edit.conf
//----------------------------------------------------------------------------
static const char*     Edit_conf=
   "[Program]\n"
   "Http=http://eske-systems.com\n"
   "Exec=View ; Edit in read-only mode\n"
   "Exec=Edit ; Edit in read-write mode\n"
   "Purpose=Graphic text editor\n"
   "Version=0.0\n"
   "\n"
   "[Options]\n"
   ";; See sample: ~/src/cpp/Edit/Xcb/.SAMPLE/Edit.conf\n"
   ;

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

   // If required, create "$HOME/.config/uuid/" + UUID + "/Edit.conf"
   std::string S= HOME + "/.config";
   make_dir(S);
   S += "/uuid";
   make_dir(S);
   S += std::string("/") + UUID;
   make_dir(S);
   AUTO= S;

   S += std::string("/Edit.conf");
   make_file(S, Edit_conf);

   // Allocate XCB objects
   device= new xcb::Device();       // The screen/connection device
   font= new xcb::Font(device);     // The Font object

   // Parse the configuration file
   parser(S);

   // Set AUTOSAVE subdirectory
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

   // Delete XCB objects
   delete font;
// delete device;

   term();
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
     const char*       unit,        // Trace sub-identifier
     void*             _one,        // Word one
     void*             _two)        // Word two
{
   uintptr_t one= uintptr_t(_one);
   uintptr_t two= uintptr_t(_two);

   typedef ::pub::Trace::Record Record;
   Record* record= (Record*)trace();
   if( record ) {
     memcpy(&record->unit, unit, 4);

     for(unsigned i= 8; i>0; i--) {
       record->value[0 + i - 1]= char(one);
       record->value[8 + i - 1]= char(two);
       one >>= 8;
       two >>= 8;
     }
     record->trace(ident);
   }
}

void
   Config::trace(                   // Trace undo/redo operation
     const char*       ident,       // Trace identifier (.UDO, .RDO)
     const char*       unit,        // Trace sub-identifer (file,init,mark)
     EdRedo*           redo,        // The UNDO/REDO
     EdFile*           file,        // The UNDO/REDO file
     EdLine*           line)        // The UNDO/REDO cursor line
{
   typedef pub::Trace::Record Record;
   Record* record= (Record*)trace(32);
   if( record ) {
     memset(record->value, 0, sizeof(record->value) + 32);
     memcpy(&record->unit, unit, 4);

     uintptr_t V0= uintptr_t(file);
     uintptr_t V1= uintptr_t(line);
     uintptr_t R0= uintptr_t(redo->head_insert);
     uintptr_t R1= uintptr_t(redo->tail_insert);
     uintptr_t R2= uintptr_t(redo->head_remove);
     uintptr_t R3= uintptr_t(redo->tail_remove);

     for(unsigned i= 8; i>0; i--) {
       record->value[ 0 + i - 1]= char(V0);
       record->value[ 8 + i - 1]= char(V1);
       ((char*)(record->value))[16 + i - 1]= char(R0);
       ((char*)(record->value))[24 + i - 1]= char(R1);
       ((char*)(record->value))[32 + i - 1]= char(R2);
       ((char*)(record->value))[40 + i - 1]= char(R3);

       V0 >>= 8;
       V1 >>= 8;
       R0 >>= 8;
       R1 >>= 8;
       R2 >>= 8;
       R3 >>= 8;
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
       Config::failure("Cannot create %s", path.c_str());
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
       Config::failure("Cannot create %s",  name.c_str());

     size_t L0= strlen(data);
     size_t L1= fwrite(data, 1, L0, f);
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
     fprintf(stderr, "sig_handler(%d) recursion(%d)\n", id, recursion);
     fflush(stderr);
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
       config::device->operational= false; // Unconditional immediate exit
//     exit(EXIT_FAILURE);          // Unconditional immediate exit
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
   // Initialize signal handling
   sys1_handler= signal(SIGINT,  sig_handler);
   sys2_handler= signal(SIGSEGV, sig_handler);
   usr1_handler= signal(SIGUSR1, sig_handler);
   usr2_handler= signal(SIGUSR2, sig_handler);

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
   debugf("Editor PID(%4d) VID: %s %s\n", getpid(), __DATE__, __TIME__);

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

//----------------------------------------------------------------------------
//
// Subroutine-
//       parser
//
// Purpose-
//       Configuration parser
//
//----------------------------------------------------------------------------
struct Color_item {
const char*            name;        // The color name
uint32_t*              addr;        // The color value
};

struct Color_item      color_list[]= // The color name/value list
{ {"mark.bg",    &mark_bg}
, {"mark.fg",    &mark_fg}
, {"text.bg",    &text_bg}
, {"text.fg",    &text_fg}
, {"change.bg",  &change_bg}
, {"change.fg",  &change_fg}
, {"status.bg",  &status_bg}
, {"status.fg",  &status_fg}
, {"command.bg", &command_bg}
, {"command.fg", &command_fg}
, {"message.bg", &message_bg}
, {"message.fg", &message_fg}
, {nullptr,      nullptr}           // End of list
};

static void
   parse_error(                     // Handle parse error
     std::string       file,        // The parse file name
     const char*       fmt,         // The PRINTF format string
                       ...)         // PRINTF argruments
   _ATTRIBUTE_PRINTF(2, 3);

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

static int                          // The numeric value
   parse_number(                    // Parse numeric value
     const char*&      value,       // (IN/OUT) The current value string
     int               max= INT_MAX) // Naximum value
{
   while( *value == ' ' )           // Remove leading blanks
     ++value;

   if( *value < '0' || *value > '9' ) {
     value= nullptr;
     return -1;
   }

   int V= 0;                        // The value
   for(;;) {                        // Decode value
     if( *value == ' ' || *value == ',' || *value == '\0' )
       break;
     if( *value < '0' || *value > '9' ) {
       value= nullptr;
       return -1;
     }

     V *= 10;
     V += *value - '0';
     if( V > max ) {
       value= nullptr;
       return -1;
     }

     value++;
   }

   while( *value == ' ' )           // Remove trailing blanks
     ++value;

   return V;
}

static void
   parse_color(                     // Parse color option
     std::string       file,        // The parse file name
     const Color_item& item,        // The item name descriptor
     const char*       VALUE)       // The item value string
{
   const char* value= VALUE;        // Working value

   uint32_t color= 0;
   for(int rgb= 0; rgb < 3; rgb++) {
     int cc= parse_number(value, 255); // Get color component
     if( value == nullptr ) {
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

static bool            font_valid= false; // Font parameter present and valid?

static void
   parser(                          // Configuration parser
     std::string       name)        // The parser file name
{  if( opt_hcdm )                   // (Don't use debugf here)
     fprintf(stderr, "Config::parser(%s)\n", name.c_str());

   pub::Parser parser(name);        // The parser object
   for(const char* sect= parser.get_next(nullptr); sect;
       sect= parser.get_next(sect)) {
     if( strcmp(sect, "Options") != 0 ) {
       if( sect[0] != '\0' && strcmp(sect, "Program") != 0 )
         parse_error(name, "Unknown section [%s]\n", sect);
     } else {                       // Section [Options]
       for(const char* parm= parser.get_next(sect, nullptr); parm;
           parm= parser.get_next(sect, parm)) {
         std::string value= parser.get_value(sect,parm);
         Color_item* color= nullptr;
         for(int i= 0; color_list[i].name; i++) {
           if( strcmp(parm, color_list[i].name) == 0 ) {
             color= color_list + i;
             break;
           }
         }
         if( color ) {
           parse_color(name, *color, value.c_str());
           continue;
         }

         if( strcmp(parm, "font") == 0 ) { // Font parameter?
           int rc= font->open(value.c_str()); // Set the font
           if( rc == 0 )
             font_valid= true;
           continue;
         }

         if( editor::set_option(parm, parser.get_value(sect,parm)) < 0 )
           parse_error(name, "Unknown option '%s'\n", parm);
       }
     }
   }
   if( !font_valid )                // If valid font not present
     font->open();                  // Use default font
}

//============================================================================
// DEBUGGING EXTENSION: configCheck, configDebug, and configSignal listaners
//============================================================================
#include "Config.patch"             // Default config signal listeners
