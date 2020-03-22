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
//       Verify.cpp
//
// Purpose-
//       Verify some C++ features.
//
// Last change date-
//       2020/01/25
//
// Tests-
//       test_opts   Verify options: --debug={Exception,exception,...}
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
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <pub/ifmacro.h>

#include "Verify.h"                 // Define the Verify classes
#include "Verify.i"                 // Define the Verify static constants

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static const char*     opt_debug= "none"; // --debug
static int             opt_help= false; // --help (or error)
static int             opt_index;   // Option index
static int             opt_verbose= -1; // --verbose

static struct option   OPTS[]=      // Options
{  {"help",    no_argument,       &opt_help,    true}

,  {"debug",   required_argument, nullptr,      0}
,  {"verbose", optional_argument, &opt_verbose, true}
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX
{  OPT_DEBUG= 1
,  OPT_VERBOSE= 2
};

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
   printf("opterr(%d)\n",  optind);
   printf("optind(%d)\n",  optind);

   printf("debug(%s)\n",   opt_debug);
   printf("verbose(%d)\n", opt_verbose);

   if( opt_verbose > 2 )
     optind= 0;
   for(int i= optind; i<argc; i++)
     printf("[%2d] '%s'\n", i, argv[i]);

   std::string debug(opt_debug);
   if( debug == "Exception" )
     throw pub::Exception("debug == Exception");
   if( debug == "exception" )
     throw std::bad_exception();
   if( debug == "..." )
     throw "Broken code";

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
                   "  --debug\t{arg} Set debugging arguument\n"
                   "  --verbose\t{n} Set debugging verbosity\n"
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

   while( (C= getopt_long(argc, argv, "", OPTS, &opt_index)) != -1 )
     switch( C )
     {
       case 0:
         {{{{

         IFHCDM(
           printf("%4d Option %d %s=%s\n", __LINE__,
                  opt_index, OPTS[opt_index].name, optarg);
         )
         switch( opt_index )
         {
           case OPT_DEBUG:
             opt_debug= optarg;
             break;

           case OPT_VERBOSE:
             if( optarg )
               opt_verbose= atoi(optarg);
             else
               opt_verbose= 1;
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
     error_count += test_0001();    // Test Things
   } catch( pub::Exception& X ) {
     error_count++;
     debugf("\n%4d Exception: %s\n", __LINE__, ((std::string)X).c_str());
   } catch( std::exception& X ) {
     error_count++;
     debugf("\n%4d std::exception: %s\n", __LINE__, X.what());
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
