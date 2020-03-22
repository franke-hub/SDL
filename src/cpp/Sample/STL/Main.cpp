//----------------------------------------------------------------------------
//
//       Copyright (c) 2017 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Main.cpp
//
// Purpose-
//       Sample STL C++ program.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#include "Main.h"

#include "Nice.h"
#include "Nice.i"

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define INVOKE(name) {extern void name( void ); name();}

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
int                    __logLevel= LevelStd; // Logging Level
int                    bugLevel= 0; // Demonstrate bugs?
const char*            nameList[DIM]= // Name list
{  "00000"
,  "11111"
,  "22222"
,  "33333"
,  "44444"
,  "55555"
,  "alpha"
,  "bravo"
,  "charlie"
,  "delta"
,  "echo"
,  "foxtrot"
,  "golf"
,  "hotel"
,  "india"
,  "juliet"
,  "kilo"
,  "lima"
,  "mike"
,  "november"
,  "oscar"
,  "papa"
,  "quebec"
,  "romeo"
,  "sierra"
,  "tango"
,  "uniform"
,  "victor"
,  "whiskey"
,  "x-ray"
,  "yankee"
,  "zulu"
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       trash
//
// Purpose-
//       Insure that the call stack does not contain residual data.
//
//----------------------------------------------------------------------------
extern void
   trash( void )                    // Remove residual data from call stack
{
   wtlc(LevelAll, "Main::trash()\n");

   char                stack[65536];

   memset(stack, 0, sizeof(stack));
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Usage information
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Usage information
{
   fprintf(stderr,
     "Usage information: main {-D -DD ...}\n"
     "\n"
     "Test STL (Standard Template Library) with additional levels of\n"
     "verbosity as specified by the -D (debugging) parameter\n"
     );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testLevel
//
// Purpose-
//       Test logging level.
//
//----------------------------------------------------------------------------
static inline void
   testLevel( void )                // Test logging level
{
   Logger::log("testLevel\n");
   wtlc(LevelAll,    "LevelAll\n");
   wtlc(LevelInfo,   "LevelInfo\n");
   wtlc(LevelStd,    "LevelStd\n");
   wtlc(LevelError,  "LevelError\n");
   wtlc(LevelAbort,  "LevelAbort\n");
   wtlc(LevelIgnore, "LevelIgnore\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 argx;        // Argument index
   int                 swVerify;    // Verify parameters

   int                 i;

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   swVerify= FALSE;

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   for(argx= 1; argx<argc; argx++)  // Examine the parameter list
   {
     if( argv[argx][0] == '-' )     // If this is a switch list
     {
       if( strcmp("-help", argv[argx]) == 0
           || strcmp("--help", argv[argx]) == 0 )
         VerifyEC::get()->increment();

       else                         // Switch list
       {
         for(i=1; argv[argx][i] != '\0'; i++) // Examine the switch list
         {
           switch(argv[argx][i])    // Examine the switch
           {
             case 'B':              // -B (BugLevel)
             {{{{
               bugLevel++;
               break;
             }}}}

             case 'D':              // -D (Debugging)
             {{{{
               int logLevel= getLogLevel();
               if( logLevel > 0 )
                 setLogLevel(logLevel-1);
               break;
             }}}}

             case 'V':              // -V (verify)
               swVerify= TRUE;
               break;

             default:               // If invalid switch
               VerifyEC::get()->increment();
               debugf("Invalid switch '%c'\n", (int)argv[argx][i]);
               break;
           }
         }
       }
       continue;
     }

     //-----------------------------------------------------------------------
     // Process a flat (non-switch) parameter
     //-----------------------------------------------------------------------
     VerifyEC::get()->increment();
     debugf("Extra parameter '%s'\n", argv[argx]);
   }

   //-------------------------------------------------------------------------
   // Validate the parameters
   //-------------------------------------------------------------------------
   if( error_count() != 0 )         // If an error was detected
     info();                        // Tell how this works

   if( swVerify )
   {
     printf("%10d LogLevel\n", getLogLevel());
     printf("%10d bugLevel\n", bugLevel);
   }
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
   Logger::set(new Logger("1>"));   // Log to STDOUT
   setLogLevel(LevelError);         // Log errors

   parm(argc, argv);
   // testLevel();
   if( error_count() == 0 )
   {
     INVOKE(exemplar);
     INVOKE(test00);
     INVOKE(test01);

     INVOKE(testAsync);
     INVOKE(testString);
     INVOKE(testThread);
     INVOKE(testVector);
   }

   if( error_count() == 0 && bugLevel > 0 )
   {
     INVOKE(testPoorly);
   }

   verify_exit();
   Logger::set(NULL);
}

