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
//       2020/07/15
//
// Usage notes-
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

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_help= false; // --help (or error)
static int             opt_hcdm= false; // Hard Core Debug Mode
static int             opt_index;   // Option index

static int             opt_a= 0;    // -a
static int             opt_b= 0;    // -b
static const char*     opt_c= nullptr; // -c
static const char*     opt_debug= "none"; // --debug
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
{  {"help",    no_argument,       &opt_help,    true} // --help
,  {"hcdm",    no_argument,       &opt_hcdm,    true} // --hcdm

,  {"debug",   required_argument, nullptr,      0} // --debug <string>
,  {"opterr",  required_argument, nullptr,      0} // --opterr <on || off>
,  {"verbose", optional_argument, &opt_verbose, 0} // --verbose {optional}
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX                      // Must match OPTS[]
{  OPT_HELP
,  OPT_HCDM

,  OPT_DEBUG
,  OPT_ERROR
,  OPT_VERBOSE
,  OPT_SIZE
};

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
   init(                            // Initialize
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
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
//----------------------------------------------------------------------------
static void
   term( void )                     // Terminate
{
// Placeholder
}

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
//       to_integer
//
// Purpose-
//       Convert string to integer, handling error cases
//
// Implementation note-
//       Leading or trailing blanks are NOT allowed.
//
//----------------------------------------------------------------------------
static int                          // The integer value
   to_integer(                      // Extract and verify integer value
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
   int value= to_integer(optarg);
   if( errno ) {
     opt_help= true;
     if( errno == ERANGE )
       fprintf(stderr, "--%s, range error: '%s'\n", OPTS[opt_index].name, optarg);
     else
       fprintf(stderr, "--%s, format error: '%s'\n", OPTS[opt_index].name, optarg);
   }

   return value;
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
static int                          // Return code (Always 1)
   info( void)                      // Parameter description
{
   fprintf(stderr, "%s <options> parameter ...\n"
                   "Options:\n"
                   "  --help\tThis help message\n"
                   "  --hcdm\tHard Core Debug Mode\n"

                   "  -a,-b\t\tSwitches\n"
                   "  -c\t\tSwitch requiring an argument\n"
                   "  --debug\targument\n"
                   "  --opterr\t{on|off}\n"
                   "  --verbose\t{=n} Verbosity, default 0\n"
                   , __FILE__
          );

   return 1;
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
static int                          // Return code (0 if OK)
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
           case OPT_HELP:           // These options handled by getopt
           case OPT_HCDM:
             break;

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
             debug_opt(__LINE__);

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

   // Return sequence
   int rc= 0;
   if( opt_help )
     rc= info();
   return rc;
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
   // Initialize
   //-------------------------------------------------------------------------
   int rc= parm(argc, argv);        // Argument analysis
   if( rc ) return rc;              // Return if invalid

   rc= init(argc, argv);            // Initialize
   if( rc ) return rc;              // Return if invalid

   printf("%s: %s %s\n", __FILE__, __DATE__, __TIME__); // Compile time message

   //-------------------------------------------------------------------------
   // Mainline code: Display option values
   //-------------------------------------------------------------------------
   printf("\n");
   printf("--debug(%s)\n", opt_debug);
   printf("--hcdm(%d)\n", opt_hcdm);
   printf("__verbose(%d)\n", opt_verbose);

   printf("-a(%d) -b(%d) -c(%s)\n", opt_a, opt_b, opt_c);

   printf("opterr(%d) optind(%d) argc(%d)\n", opterr, optind, argc);
   if( opt_verbose > 0 )
     optind= 0;
   for(int i= optind; i<argc; i++)
     printf("[%2d] '%s'\n", i, argv[i]);

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   term();                          // Termination cleanup

   return rc;
}
