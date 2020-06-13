//----------------------------------------------------------------------------
//
//       Copyright (C) 2018-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Getopt.cpp
//
// Purpose-
//       Sample program: How to use getopt_long
//
// Last change date-
//       2020/06/13
//
// Implementation notes-
//       getopt_long does not print an invalid argument error message when
//       ':' is the first character of the optstring parameter.
//
//----------------------------------------------------------------------------
#include <ctype.h>                  // For isprint()
#include <errno.h>                  // For errno
#include <getopt.h>                 // For getopt_long()
#include <limits.h>                 // For INT_MAX, INT_MIN
#include <stdarg.h>                 // For va_list
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp

#include <pub/utility.h>            // For pub::utility::atoi

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_a= 0;    // -a
static int             opt_b= 0;    // -b
static const char*     opt_c= nullptr; // -c
static const char*     opt_debug= "none"; // --debug
static int             opt_help= false; // --help (or error)
static int             opt_index;   // Option index
static int             opt_verbose= -1; // --brief or --verbose

static const char*     OSTR= ":abc:"; // The getopt_long optstring parameter
                                    // Notes:
                                    // 1st character ':'
                                    //    Missing argument returns ':', not '?'
                                    //    (Invalid options always return '?')
                                    // a  (Switch -a)
                                    // b  (Switch -b)
                                    // c: (Argument -c) (Argument required)
static struct option   OPTS[]=      // The getopt_long longopts parameter
{  {"help",    no_argument,       &opt_help,    true} // --help (no argument)

,  {"debug",   required_argument, nullptr,      0} // --debug argument
,  {"opterr",  required_argument, nullptr,      0} // --opterr argument
,  {"verbose", optional_argument, &opt_verbose, true} // --verbose {optional}
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX                      // Must match OPTS[]
{  OPT_DEBUG= 1
,  OPT_ERROR= 2
,  OPT_VERBOSE= 3
,  OPT_SIZE= 4
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug_opt
//
// Purpose-
//       Display getopt debugging information.
//
//----------------------------------------------------------------------------
static inline void
   debug_opt(                       // Display getopt debugging information
     int               line = 0)    // Caller's line number
{
   const char* opt_name= "<<null>>";
   if( opt_index < OPT_SIZE )
     opt_name= OPTS[opt_index].name;
   else
     opt_name= "<<INVALID>>";
   fprintf(stderr, "%4d index(%d:%s) arg(%s) err(%d) ind(%d) opt(%c)\n",
                   line, opt_index, opt_name, optarg, opterr, optind, optopt);
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
   fprintf(stderr, "Getopt <options> parameter ...\n"
                   "Options:\n"
                   "  -a,-b\t\tSwitches\n"
                   "  -c\t\tSwitch requiring an argument\n"
                   "  --help\tThis help message\n"
                   "  --debug\targument\n"
                   "  --opterr\t{on|off}\n"
                   "  --verbose\t{=n} Verbosity, default 1\n"
          );

// exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm_int
//
// Purpose-
//       Convert parameter to integer, handling error cases
//
// Implementation note-
//       optarg: The argument string
//       opt_index: The argument index
//
//----------------------------------------------------------------------------
static int                          // The integer value
   parm_int( void )                 // Extract and verify integer value
{
   errno= 0;
   int value= pub::utility::atoi(optarg);
   if( errno ) {
     opt_help= true;
     if( errno == ERANGE )
       fprintf(stderr, "--%s, range error: '%s'\n", OPTS[opt_index].name, optarg);
     else if( *optarg == '\0' )
       fprintf(stderr, "--%s, no value specified\n", OPTS[opt_index].name);
     else
       fprintf(stderr, "--%s, format error: '%s'\n", OPTS[opt_index].name, optarg);
   }

   return value;
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
   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   int C;                           // The option character
   while( (C= getopt_long(argc, argv, OSTR, OPTS, &opt_index)) != -1 )
   {
     switch( C )
     {
       case 0:
       {{{{
         if( opt_verbose > 1 )
           debug_opt(__LINE__);

         switch( opt_index )
         {
           case OPT_DEBUG:
             opt_debug= optarg;
             break;

           case OPT_ERROR:
             if( strcmp(optarg, "on") == 0 )
               opterr= true;
             else if( strcmp(optarg, "off") == 0 )
               opterr= false;
             else
             {
               opt_help= true;
               fprintf(stderr, "%4d --opterr must be on or off\n", __LINE__);
             }
             break;

           case OPT_VERBOSE:
             if( optarg )
               opt_verbose= parm_int();
             break;

           default:
             fprintf(stderr, "%4d Unexpected opt_index(%d)\n", __LINE__,
                             opt_index);
             break;
         }
         break;
       }}}}

       case 'a':
         opt_a= 1;
         break;

       case 'b':
         opt_b= 1;
         break;

       case 'c':
         opt_c= optarg;
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
                           (optopt & 0x00ff));
         break;

       default:
         fprintf(stderr, "%4d ShouldNotOccur ('%c',0x%x).\n", __LINE__,
                         C, (C & 0x00ff));
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
   // Display option values
   //-------------------------------------------------------------------------
   printf("\n");
   printf("argc(%d)\n", argc);
   printf("debug(%s)\n", opt_debug);
   printf("opterr(%d)\n", opterr);
   printf("optind(%d)\n", optind);
   printf("verbose(%d)\n", opt_verbose);
   if( opt_verbose > 2 )
     debug_opt(__LINE__);

   printf("-a(%d) -b(%d) -c(%s)\n", opt_a, opt_b, opt_c);

   if( opt_verbose > 1 )
     optind= 0;
   for(int i= optind; i<argc; i++)
     printf("[%2d] '%s'\n", i, argv[i]);

   return EXIT_SUCCESS;
}
