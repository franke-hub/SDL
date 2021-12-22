//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Verify.cpp
//
// Purpose-
//       Verify some C++ features.
//
// Last change date-
//       2020/07/15
//
// Tests-
//       test_opts   Displays options: --throw={Exception,exception,...}
//         Displays options. If --throw option specified, tests exception
//         handling for pub::Excepion, std::exception, and const char*.
//
//       test_0001   Verifies automatic constructor/destructor invocations.
//
//----------------------------------------------------------------------------
#include <ctype.h>                  // For isprint()
#include <getopt.h>                 // For getopt()
#include <stdarg.h>                 // For va_list
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp

#include <pub/Debug.h>              // For debugging
#include <pub/Exception.h>          // For Exception

using namespace pub::debugging;     // Expose debugging subroutines

//----------------------------------------------------------------------------
// Contants for parameterization
//----------------------------------------------------------------------------
#define USE_ANON_CON_DESTRUCTOR true  // Test anonymous con/destructor?

#ifndef HCDM
#define HCDM false                  // Hard Core Debug Mode?
#endif

#include <pub/ifmacro.h>

#include "Verify.h"                 // Define the Verify classes
#include "Verify.i"                 // Define the Verify static constants

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_help= false; // --help (or error)
static int             opt_hcdm= false; // --hcdm (Hard Core Debug Mode)
static int             opt_index;   // Option index

static const char*     opt_throw= nullptr; // --throw
static int             opt_verbose= -1; // --verbose

static const char*     OSTR= ":";   // The getopt_long optstring parameter

static struct option   OPTS[]=      // Options
{  {"help",    no_argument,       &opt_help,    true}
,  {"hcdm",    no_argument,       &opt_hcdm,    true}

,  {"throw",   required_argument, nullptr,      0}
,  {"verbose", optional_argument, &opt_verbose, true}
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX
{  OPT_HELP
,  OPT_HCDM

,  OPT_THROW
,  OPT_VERBOSE
};

//----------------------------------------------------------------------------
//
// Section-
//       test_anon
//
// Purpose-
//       Test anonymous global constructor/destructor.
//
// Implementation note-
//       Anonymity for static structures isn't needed.
//       (No external name is generated, so there's no name conflict.)
//
//----------------------------------------------------------------------------
#if USE_ANON_CON_DESTRUCTOR

static struct {                     // An anonymous class
  struct N {                        // Wrapping a named class
    N() { printf("Anon constructor\n"); } // Do something when constructed
    ~N() { printf("Anon destructor\n"); } // Do something when destroyed
  } named;                          // Instantiate in anonymous class
} globalAnon;                       // Construct/Destruct anonymously

static struct N {                   // Verify anonymity (struct N usable)
  N() { printf("Name constructor\n"); } // Do something when constructed
  ~N() { printf("Name destructor\n"); } // Do something when destroyed
} globalName;                       // Construct/Destruct named

#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_case
//
// Purpose-
//       Sample test.
//
//----------------------------------------------------------------------------
static inline int                   // Error count
   test_case( void )                // Testcase
{
   int error_count= 0;

   debugf("\n%4d test_case\n", __LINE__);
   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_opts
//
// Purpose-
//       Test options.
//
//----------------------------------------------------------------------------
static inline int                   // Error count
   test_opts(                       // Test options
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int error_count= 0;

   debugf("\n%4d test_opts\n", __LINE__);

   printf("--hcdm(%d) --throw(%s) --verbose(%d)\n"
          , opt_hcdm, opt_throw, opt_verbose);
   printf("optind(%d) argc(%d)\n", optind, argc);
   if( opt_verbose > 0 )
     optind= 0;
   for(int i= optind; i<argc; i++)
     printf("[%2d] '%s'\n", i, argv[i]);

   if( opt_throw ) {
     std::string debug(opt_throw);
     if( debug == "Exception" )
       throw pub::Exception("throw == Exception");
     if( debug == "exception" )
       throw std::bad_exception();
     throw (char*)opt_throw;
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_0001
//
// Purpose-
//       Verify destructors called.
//
//----------------------------------------------------------------------------
static inline int                   // Error count
   test_0001( void )                // Verify destructors called
{
   int error_count= 0;

   debugf("\n%4d test_0001\n", __LINE__);
   {{{{
     Things things;
     error_count += VERIFY( Thing1::counter[Thing1::IX_OBJS] == 1 );
     error_count += VERIFY( Thing1::counter[Thing1::IX_NEWS] == 1 );
     error_count += VERIFY( Thing1::counter[Thing1::IX_OLDS] == 0 );

     error_count += VERIFY( Thing2::counter[Thing2::IX_OBJS] == 1 );
     error_count += VERIFY( Thing2::counter[Thing2::IX_NEWS] == 1 );
     error_count += VERIFY( Thing1::counter[Thing2::IX_OLDS] == 0 );
   }}}}

   error_count += VERIFY( Thing1::counter[Thing1::IX_OBJS] == 0 );
   error_count += VERIFY( Thing1::counter[Thing1::IX_NEWS] == 1 );
   error_count += VERIFY( Thing1::counter[Thing1::IX_OLDS] == 1 );

   error_count += VERIFY( Thing2::counter[Thing2::IX_OBJS] == 0 );
   error_count += VERIFY( Thing2::counter[Thing2::IX_NEWS] == 1 );
   error_count += VERIFY( Thing1::counter[Thing2::IX_OLDS] == 1 );

   if( opt_verbose > 5 ) {
     unsigned* I= Thing1::counter;
     debugf("Thing1: %d,%d,%d,%d\n", I[0], I[1], I[2], I[3]);
     I= Thing2::counter;
     debugf("Thing2: %d,%d,%d,%d\n", I[0], I[1], I[2], I[3]);
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter description.
//
//----------------------------------------------------------------------------
static void
   info( void)                      // Parameter description
{
   fprintf(stderr, "Verify [options] parameter...\n"
                   "Options:\n"
                   "  --help\tThis help message\n"
                   "  --hcdm\tHard Core Debug Mode\n"

                   "  --throw\t{arg} Throw exception\n"
                   "  --verbose\t{n} Set verbosity\n"
          );

// exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis. (getopt example)
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 C;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   opterr= 0;                       // Do not write error messages

   while( (C= getopt_long(argc, argv, OSTR, OPTS, &opt_index)) != -1 ) {
     if( opt_hcdm ) {
       if( C )
         printf("%4d {%c,%d} argv[%d] %s=%s\n", __LINE__, C, C
                , optind-1, argv[optind-1], optarg);
       else
         printf("%4d {%c,%d} OPTS[%d] %s=%s\n", __LINE__, C, C
                , opt_index, OPTS[opt_index].name, optarg);
     }

     switch( C )
     {
       case 0:
         {{{{
         switch( opt_index )
         {
           case OPT_HELP:           // These options handled by getopt
           case OPT_HCDM:
             break;

           case OPT_THROW:
             opt_throw= optarg;
             break;

           case OPT_VERBOSE:
             if( optarg )
               opt_verbose= atoi(optarg);
             else
               opt_verbose= 0;
             break;

           default:
             break;
         }
         }}}}
         break;

       case ':':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "%4d Option requires an argument '%s'.\n", __LINE__,
                           argv[optind-1]);
         else
           fprintf(stderr, "%4d Option requires an argument '-%c'.\n", __LINE__,
                           optopt);
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "%4d Unknown option '%s'.\n", __LINE__,
                           argv[optind-1]);
         else if( isprint(optopt) )
           fprintf(stderr, "%4d Unknown option '-%c'.\n", __LINE__, optopt);
         else
           fprintf(stderr, "%4d Unknown option character '0x%x'.\n", __LINE__,
                           optopt);
         break;

       default:
         fprintf(stderr, "%4d ShoudNotOccur ('%c',0x%x).\n", __LINE__, C, C);
         break;
     }
   }

   if( opt_help )
     info();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Mainline code
   //-------------------------------------------------------------------------
   parm(argc, argv);                // Argument analysis

   //-------------------------------------------------------------------------
   // Run the tests
   //-------------------------------------------------------------------------
   int error_count= 0;              // Error counter

   try {
     error_count += test_opts(argc, argv); // Test options
     error_count += test_0001();    // Test Things (Constructor/Destructor)
   } catch( pub::Exception& X ) {
     error_count++;
     debugf("\n%4d Exception: %s\n", __LINE__, ((std::string)X).c_str());
   } catch( std::exception& X ) {
     error_count++;
     debugf("\n%4d std::exception: %s\n", __LINE__, X.what());
   } catch( const char* X ) {
     error_count++;
     debugf("\n%4d catch((const char*)%s)\n", __LINE__, X);
   } catch( ... ) {
     error_count++;
     debugf("\n%4d catch(...)\n", __LINE__);
   }

   debugf("\n");
   if( error_count == 0 )
     debugf("NO errors\n");
   else if( error_count == 1 )
     debugf("1 error encountered\n");
   else
     debugf("%d errors encountered\n", error_count);

   return EXIT_SUCCESS;
}
