//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Wrapper.cpp
//
// Purpose-
//       Implement Wrapper.h generic program wrapper.
//
// Last change date-
//       2023/04/29
//
//----------------------------------------------------------------------------
#include <mutex>                    // For std::lock_guard

#include <ctype.h>                  // For isprint()
#include <errno.h>                  // For errno
#include <fcntl.h>                  // For open, O_*, ...
#include <getopt.h>                 // For getopt_long()
#include <limits.h>                 // For INT_MAX, INT_MIN
#include <stdarg.h>                 // For va_list
#include <stdio.h>                  // For fprintf, vfprintf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strerror
#include <unistd.h>                 // For close, ...
#include <sys/mman.h>               // For mmap, ...

#include <pub/Debug.h>              // For namespace `debugging`
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Trace.h>              // For pub::Trace
#include <pub/utility.h>            // For pub::utility::to_string

#include "pub/Wrapper.h"            // For class Wrapper, implemented

using namespace pub;                // For pub:: classes
using std::string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, greater is more verbose
}; // enum

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
int                    opt_hcdm= HCDM; // Hard Core Debug Mode
int                    opt_verbose= VERBOSE; // Verbosity

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_help= false; // --help (or error)

static const char*     ostr= ":";   // The getopt_long optstring parameter
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"help",    no_argument,       &opt_help,    true}
,  {"hcdm",    no_argument,       &opt_hcdm,    true}
,  {"verbose", optional_argument, &opt_verbose,    1}
,  {0, 0, 0, 0}                     // (End of Wrapper internal option list)
};

enum OPT_INDEX                      // Must match opts[]
{  OPT_HELP
,  OPT_HCDM
,  OPT_VERBOSE
,  OPT_SIZE
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugf
//
// Purpose-
//       Write error message
//
//----------------------------------------------------------------------------
_LIBPUB_PRINTF(1, 2)
static void
   debugf(                          // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
                       ...);        // The PRINTF argument list
static void
   debugf(                          // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
{
   va_list argptr;
   va_start(argptr, fmt);

   if( Debug::show() )
     debugging::vdebugf(fmt, argptr);
   else
     vfprintf(stderr, fmt, argptr);

   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       format_opt
//
// Purpose-
//       Generate debugging display line for option
//
//----------------------------------------------------------------------------
static string                       // Output string
   debug_opt(                       // Debugging display
     option*           opt)         // For this option
{
   const char* type= "INVALID_argument";
   if( opt->has_arg == no_argument )
     type= "no_argument";
   else if( opt->has_arg == required_argument )
     type= "required_argument";
   else if( opt->has_arg == optional_argument )
     type= "optional_argument";

   return utility::to_string("%-10s %-18s 0x%.10lX %8d", opt->name, type
                            , (intptr_t)opt->flag, opt->val);
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::~Wrapper
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Wrapper::~Wrapper()
{
   if( OPTS != opts ) {
     free(OPTS);
     OPTS= nullptr;
   }
   if( OSTR != ostr ) {
     free(OSTR);
     OSTR= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::Wrapper
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Wrapper::Wrapper(                // Default/option list constructor
     option*           O,           // The option list
     const char*       S)           // The option string
:  info_f([]() {})
,  init_f([](int, char**) { return 0; })
,  main_f([](int, char**) { return 0; })
,  parm_f([](string, const char*) { return 0; })
,  term_f([]() {})
,  program()
{
   OSTR= const_cast<char*>(ostr);   // Default, internal option string
   OPTS= opts;                      // Default, internal option table
   OPNO= OPT_SIZE;                  // Default, internal option table size

   if( O ) {                        // If an option list was specified
     size_t N= 0;
     for(; O[N].name; ++N)          // Count the options
       ;

     OPNO= N + OPT_SIZE + 1;        // The total option count
     OPTS= (struct option*)malloc(OPNO * sizeof(struct option));
     if( OPTS == nullptr )
       throw std::bad_alloc();

     for(size_t i= 0; i<OPT_SIZE; ++i)
       OPTS[i]= opts[i];

     for(size_t i= OPT_SIZE; i<OPNO; ++i) {
       OPTS[i]= O[i - OPT_SIZE];

       if( OPTS[i].flag == nullptr && OPTS[i].val != 0 ) {
         string S= debug_opt(&OPTS[i]);
         debugf("Configuration error for option %s:\n%s\n"
                "When the 3rd field, flag, is zero (nullptr) "
                "the 4th field, val, should be zero.\n"
               , OPTS[i].name, S.c_str());

//       opt_help= 2;               // (Correctable) configuration error
         OPTS[i].val= 0;            // Correct the error
       }
     }
   }

   if( S ) {                        // If an option string was specified
     size_t len= strlen(S);         // The string length
     if( *S != ':' )
       ++len;

     OSTR= (char*)malloc(len + 1);  // (Room for trailing '\0'
     if( OSTR == nullptr )
       throw std::bad_alloc();

     *OSTR= '\0';
     if( *S != ':' )
       strcpy(OSTR, ":");
     strcat(OSTR, S);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Wrapper::debug(                  // Debugging display
     const char*       info) const  // Caller information
{
   debugf("Wrapper(%p)::debug(%s)\n", this, info);
   debugf("..optarg(%s) opterr(%d) optind(%d) optopt(%d)\n"
         , optarg, opterr, optind, optopt);
   debugf("..opt_index(%d) opt_hcdm(%d) opt_verbose(%d)\n"
         , opt_index, opt_hcdm, opt_verbose);
   debugf("..OPNO(%zd) OPTS(%p) OSTR(%s)\n", OPNO, OPTS, OSTR);
   for(size_t i= 0; i<OPNO; ++i) {
     option* opt= &OPTS[i];
     string S= debug_opt(opt);
     debugf("[%2zd] %s\n", i, S.c_str());
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::atoi
//
// Purpose-
//       Convert string to integer, *always* setting errno.
//
// Implementation note-
//       Leading or trailing blanks are NOT allowed.
//
//----------------------------------------------------------------------------
int                                 // The integer value
   Wrapper::atoi(                   // Extract and verify integer value
     const char*       inp)         // From this string
{
   errno= 0;
   char* strend;                    // Ending character
   long value= strtol(inp, &strend, 0);
   if( strend == inp || *inp == ' ' || *strend != '\0' )
     errno= EINVAL;
   else if( value < INT_MIN || value > INT_MAX )
     errno= ERANGE;

   return value;
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::info
//
// Purpose-
//       Informational exit.
//
//----------------------------------------------------------------------------
[[noreturn]]
void
   Wrapper::info( void ) const
{
   if( opt_help > 1 )
     debugf("\n\n");
   debugf("%s <options> ...\n"
          "Options:\n"
          "  --help\tThis help message\n"
          "  --hcdm\tHard Core Debug Mode\n"
          "  --verbose\t{=n} Verbosity, default 1\n"
          , program.c_str());

   info_f();
   exit(opt_help > 1 ? 1 : 0);
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::init
//
// Purpose-
//       Initialize
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Wrapper::init(                   // Initialize
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // User extension
   return init_f(argc, argv);
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::init_debug
//
// Purpose-
//       Initialize debugging output file
//
//----------------------------------------------------------------------------
Debug*                              // The (initialized) debug file
   Wrapper::init_debug(             // Initialize debugging output file
     const char*       file,        // The debug file name
     const char*       mode,        // The debug file mode
     int               head)        // Heading options
{
   Debug* debug= new Debug(file);
   Debug::set(debug);

   if( head )
     debug->set_head(head);
   if( mode )
     debug->set_file_mode(mode);
   if( opt_hcdm )
     debug->set_mode(Debug::MODE_INTENSIVE);

   return debug;
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::init_trace
//
// Purpose-
//       Initialize memory mapped trace file
//
//----------------------------------------------------------------------------
void*                               // The (initialized) trace file
   Wrapper::init_trace(             // Initialize memory mapped trace file
     const char*       file,        // The trace file name
     int               size)        // The trace file size
{
   if( size > Trace::TABLE_SIZE_MAX )
     size= Trace::TABLE_SIZE_MAX;
   else if( size < Trace::TABLE_SIZE_MIN )
     size= Trace::TABLE_SIZE_MIN;

   int mode= O_RDWR | O_CREAT;
   int perm= S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
   int fd= open(file, mode, perm);
   if( fd < 0 ) {
     debugf("%4d open(%s) %s\n", __LINE__, file, strerror(errno));
     return nullptr;
   }

   int rc= ftruncate(fd, size);     // (Truncate/expand to size)
   if( rc ) {
     debugf("%4d ftruncate(%s,%.8x) %s\n", __LINE__
           , file, size, strerror(errno));
     close(fd);
     return nullptr;
   }

   mode= PROT_READ | PROT_WRITE;
   void* table= mmap(nullptr, size, mode, MAP_SHARED, fd, 0);
   if( table == MAP_FAILED ) {    // If no can do
     debugf("%4d mmap(%s,%.8x) %s\n", __LINE__, file, size, strerror(errno));
     close(fd);
     return nullptr;
   }

   Trace::table= Trace::make(table, size);
   close(fd);                     // Descriptor not needed once mapped

   Trace::trace(".INI", 0, "TRACE STARTED") ;
   return table;
}

//----------------------------------------------------------------------------
//
// Method-
//       option1
//
// Purpose-
//       Get char option index
//
//----------------------------------------------------------------------------
ssize_t                             // The switch option index, or -1
   Wrapper::option1(                // Get switch option index
     int               opt)         // The switch character
{
   opt &= 0x000000FF;               // Character mask
   for(ssize_t i= 0; OSTR[i]; ++i) {
     if( opt == (OSTR[i] & 0x000000FF) )
       return i;
   }

   return -1;                       // Not found
}

//----------------------------------------------------------------------------
//
// Method-
//       option2
//
// Purpose-
//       Get long option index
//
//----------------------------------------------------------------------------
option*                             // The option descriptor, or nullptr
   Wrapper::option2(                // Get option descriptor
     const char*       name)        // The option name
{
#if 0 // Currently unused and protected
   if( name[0] == '-' && name[1] == '-' )
     name += 2;
   const char* X= strchr(name, '=');
   string S;
   if( X ) {
     S= string(name, X-name);
     name= S.c_str();
   }

   for(option* opt= OPTS; opt->name; ++opt) {
     if( strcmp(name, opt->name) == 0 ) // If name match
       return opt;
   }
#else
   (void)name;
#endif

   return nullptr;                  // Not found
}

//----------------------------------------------------------------------------
//
// Method-
//       parm
//
// Purpose-
//       Parameter analysis, exits if --help specified or error detected.
//
//----------------------------------------------------------------------------
void
   Wrapper::parm(                   // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   int C;                           // The option character
   opt_index= 0;
   while( (C= getopt_long(argc, argv, OSTR, OPTS, &opt_index)) != -1 ) {
     switch( C ) {
       case 0: {{{{
         switch( opt_index ) {
           case OPT_HELP:           // These options handled by getopt_long
           case OPT_HCDM:
             break;

           case OPT_VERBOSE:
             if( optarg )
               opt_verbose= ptoi(optarg, OPTS[opt_index].name);
             break;

           default: {{{{
             if( size_t(opt_index) >= OPNO ) {
               opt_help= 2;
               debugf("%4d Unexpected opt_index(%d)\n", __LINE__, opt_index);
               break;
             }
             option* opt= OPTS + opt_index;
             if( opt->has_arg != no_argument ) {
               if( parm_f(opt->name, optarg) )
                 opt_help= 2;
             }
             break;
           }}}}
         }
         break;
       }}}}

       case ':':
         opt_help= 2;
         if( optopt == 0 ) {        // If long option
           if( strchr(argv[optind-1], '=') )
             debugf("Option '%s' no argument allowed.\n", argv[optind-1]);
           else
             debugf("Option '%s' requires an argument.\n", argv[optind-1]);
         } else {                   // If char option
           debugf("Option '-%c' requires an argument.\n", optopt);
         }
         break;

       case '?':
         opt_help= 2;
         if( optopt == 0 )
           debugf("Unknown option '%s'.\n", argv[optind-1]);
         else if( isprint(optopt) )
           debugf("Unknown option '-%c'.\n", optopt);
         else
           debugf("Unknown option character '0x.2%x'.\n", (optopt & 0x00ff));
         break;

       default:                     // Handle an option character
         ssize_t x= option1(C);
         if( x >= 0 ) {
           char buff[3]= {'-', (char)C, '\0'};
           string S= buff;
           if( OSTR[x+1] == ':' ) { // If switch has an argument
             if( OSTR[x+2] == ':' || optarg ) { // If optional argument found
               if( parm_f(S, optarg) )
                 opt_help= 2;
               break;
             }
             debugf("Option '%s' requires an argument\n", buff);
             opt_help= 2;
           } else {                 // If switch has no argument
             if( parm_f(S, nullptr) )
               opt_help= 2;
           }
           break;
         }

         opt_help= 2;
         debugf("%4d %s Should not occur ('%c',%#.2X)\n"
               , __LINE__, __FILE__, C, (C & 0x00ff));
         break;
     }

     opt_index= -1;
   }

   if( opt_help )
     info();
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::ptoi
//
// Purpose-
//       Convert parameter to integer, handling error cases
//
// Implementation note-
//       optarg: The argument string
//       opt_index: The argument index
//
//----------------------------------------------------------------------------
int                                 // The integer value
   Wrapper::ptoi(const char* V, const char* N) // Extract/verify parameter
{
   int value= atoi(V);
   if( errno ) {
     opt_help= 2;
     if( N == nullptr )
       N= "parameter";

     if( errno == ERANGE )
       debugf("--%s, range error: '%s'\n", N, V);
     else if( *optarg == '\0' )
       debugf("--%s, no value specified\n", N);
     else
       debugf("--%s, format error: '%s'\n", N, V);
   }

   return value;
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::report_errors
//
// Purpose-
//       Display the error count
//
//----------------------------------------------------------------------------
void
   Wrapper::report_errors(int error_count) // Display the error count
{
   if( error_count == 0 )
     debugf("NO errors detected\n");
   else if( error_count == 1 )
     debugf(" 1 error detected\n");
   else
     debugf("%2d errors detected\n", error_count);
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::run
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
int                                 // Return code
   Wrapper::run(                    // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int rc;                          // Return code

   try {
     //-----------------------------------------------------------------------
     // Initialize
     //-----------------------------------------------------------------------
     if( argc > 0 )
       program= argv[0];            // Use the caller's program name
     parm(argc, argv);              // (Exits if parameter error or --help)
     rc= init(argc, argv);

     //-----------------------------------------------------------------------
     // Run the main function
     //-----------------------------------------------------------------------
     if( rc == 0 )
       rc= main_f(argc, argv);
   } catch(Exception& x) {
     debugf("Exception(%s)\n", x.what());
     rc= 2;
   } catch(std::exception& x) {
     debugf("std::exception(%s)\n", x.what());
     rc= 2;
   } catch(const char* x) {
     debugf("const char*(%s) exception\n", x);
     rc= 2;
   } catch(...) {
     debugf("Exception ...\n");
     rc= 2;
   }

   //-------------------------------------------------------------------------
   // Termination cleanup and exit
   //-------------------------------------------------------------------------
   term();
   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::term
//
// Purpose-
//       Termination cleanup
//
//----------------------------------------------------------------------------
void
   Wrapper::term( void )                      // Terminate
{
   //-------------------------------------------------------------------------
   // User termination extension
   term_f();
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::term_debug
//
// Purpose-
//       Close the debugging trace file
//
//----------------------------------------------------------------------------
void
   Wrapper::term_debug(              // Terminate debugging
     Debug*            debug)        // Output from init_debug
{
   // If we're still the active debugging trace file, deactivate it
   {{{ std::lock_guard<decltype(*Debug::get())> lock(*Debug::get());
     if( debug == Debug::show() )
       Debug::set(nullptr);
   }}}

   delete debug;                      // Delete (close) the debugging file
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::term_trace
//
// Purpose-
//       Terminate internal trace
//
//----------------------------------------------------------------------------
void
   Wrapper::term_trace(              // Terminate internal trace
     void*             table,        // Output from init_trace
     int               size)         // The trace table size
{
   if( size > Trace::TABLE_SIZE_MAX )
     size= Trace::TABLE_SIZE_MAX;
   else if( size < Trace::TABLE_SIZE_MIN )
     size= Trace::TABLE_SIZE_MIN;

   if( table ) {
     Trace::table= nullptr;
     munmap(table, size);
   }
}
} // namespace _LIBPUB_NAMESPACE
