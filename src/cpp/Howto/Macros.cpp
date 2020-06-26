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
//       Macros.cpp
//
// Purpose-
//       Sample program: How to substitute enums for #define controls
//
// Last change date-
//       2020/06/26
//
// Usage notes-
//       You need to look at the listing to verify that compiler optimization
//       removes functions which can never be called.
//
// Implementation notes-
//       The gcc compiler optimizes as expected, and even keeps track of
//       variables which have known values. Unused static const char* strings
//       are not instantiated.
//
//----------------------------------------------------------------------------
#include <stdio.h>                  // For printf, fprintf

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Local controls
{  FIDM= false                      // False If Debug Mode
,  TIDM= true                       // True If Debug Mode
}; // local controls

// These might be in an include file
namespace option {                  // Substitutes for #defines
enum // "External" controls
{  USE_DOIT= false                  // Instead of #ifdef  false ... #endif
,  USE_DONT= false                  // Instead of #ifdef  false ... #endif
,  USE_TEST= true                   // Instead of #ifdef  false ... #endif
,  USE_TRUE= true                   // Instead of #ifdef  true  ... #endif
,  USE_WHAT= false                  // (This enum is never used)
,  USE_VALUE= false                 // Instead of #undef  VALUE
,  THE_VALUE= 732                   // Instead of #define VALUE 732
}; // "External" controls

// This string is referenced during execution. (NO compile-time concatenation)
static const char*     ISUSED= "IS VISIBLE"; // #define ISUSED "IS VISIBLE"

// This string is never referenced during execution.
static const char*     STRING= "INVISIBLE"; // #define STRING "INVISIBLE"
} // namespace option

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Always called, but always returns 0.
//
// Implementation note-
//       A smart compiler should optimize this out. It always returns 0
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   init(                            // Initialize, for example
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int rc= 5;
   if( option::USE_VALUE )          // (We know it's false)
     rc= option::THE_VALUE;         // We return something

   return rc;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       Test if( option::USE_DONT ) { etc, etc }
//
// Implementation note-
//       A smart compiler should optimize this out. It's never called.
//
//----------------------------------------------------------------------------
static const char*
   term( void )                     // Terminate, for example
{
   return option::STRING;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       doit
//
// Purpose-
//       This is called, but does nothing. See if it's even instantiated.
//
// Implementation note-
//       Look at all these strings that are never used.
//
//----------------------------------------------------------------------------
static void
   doit(                            // Display getopt debugging information
     int               line = 0)    // Caller's line number
{
   if( option::USE_DOIT ) {         // We know this is false
      fprintf(stderr, "%4d HCDM (You were there)\n", line);
      fprintf(stderr, "%4d HCDM (No, you weren't)\n", line);
      fprintf(stderr, "%4d HCDM (You're not even here)\n", line);
      fprintf(stderr, "%4d HCDM STRING(%s)\n", line, option::STRING);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       empty_line
//
// Purpose-
//       The return value is known during compile. See if it's instantiated.
//
// Implementation note-
//       (This might be a placeholder)
//
//----------------------------------------------------------------------------
static int                          // Always returns line
   empty_line(                      // Empty subroutine, returns line
     int               line = 31)   // Caller's line number
{  return line; }                   // Placeholder

//----------------------------------------------------------------------------
//
// Subroutine-
//       empty_parm
//
// Purpose-
//       This returns a constant. See if it's instantiated.
//
//----------------------------------------------------------------------------
static int                          // Always returns 44
   empty_parm(                      // Empty subroutine, returns 44
     int               line)        // Caller's line number
{  return 44; }                     // Placeholder

//----------------------------------------------------------------------------
//
// Subroutine-
//       empty_void
//
// Purpose-
//       This does nothing. See if it's instantiated.
//
// Implementation note-
//       (This might be a placeholder)
//
//----------------------------------------------------------------------------
static void
   empty_void(                      // Empty subroutine, no return value
     int               line = 0)    // Caller's line number (ignored)
{  }                                // Placeholder

//----------------------------------------------------------------------------
//
// Subroutine-
//       never_called
//
// Purpose-
//       This is never called, so it's easy for the compiler to eliminate
//
// Implementation note-
//       Look at all these strings that are never used.
//
//----------------------------------------------------------------------------
static inline void
   never_called(                    // (Don't call me, I'll call you)
     int               line = 0)    // Caller's line number
{
   fprintf(stderr, "%4d HCDM (Hi there, Yogi bear)\n", line);
   fprintf(stderr, "%4d HCDM (What's up, doc?)\n", line);
   fprintf(stderr, "%4d HCDM (You're not even here)\n", line);
   fprintf(stderr, "%4d HCDM STRING(%s)\n", line, option::STRING);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test
//
// Purpose-
//       This does nothing and has no parameters. See if it's instantiated.
//
// Implementation note-
//       (This might be a placeholder)
//
//----------------------------------------------------------------------------
static void
   test( void )                     // Empty subroutine, no return value
{  }                                // Placeholder

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
   int                 rc= 0;       // Return code

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   if( FIDM ) { printf("%d FIDM\n", __LINE__); }
   if( option::USE_TRUE ) {
     rc= init(argc, argv);          // (Always returns 5)
     if( rc != 5 ) {                // (rc is always 5)
       fprintf(stderr, "%d= init()\n", rc);
       fprintf(stderr, "Init zero return code not compiled out\n");
       return rc;
     }
   }

   // This let's us know we're here
   printf("Macros.cpp: %s %s\n", __DATE__, __TIME__); // Compile time message

   //-------------------------------------------------------------------------
   // Mainline code: Display option values
   //-------------------------------------------------------------------------
   printf("\n");
   if( FIDM ) { fprintf(stderr, "%d FIDM\n", __LINE__); }
   else printf("%4d Not false path taken\n", __LINE__);
   doit();                          // Does nothing

   if( TIDM ) { printf("%d TIDM\n", __LINE__); } // (Currently 252)
   else fprintf(stderr, "%4d Not true path taken\n", __LINE__);
   printf("%d= empty_line()\n", empty_line()); // Prints 31

   if( TIDM ) { printf("%d TIDM\n", __LINE__); }
   printf("%d= empty_line(L)\n", empty_line(__LINE__)); // Prints __LINE__

   if( TIDM ) { printf("%d TIDM\n", __LINE__); }
   printf("%d= empty_parm\n", empty_parm(__LINE__)); // Prints 44

   if( TIDM ) { printf("%d TIDM\n", __LINE__); }
   empty_void(__LINE__);            // Does nothing (Ignores parameter)

   if( TIDM ) { printf("%d TIDM\n", __LINE__); }
   test();                          // Does nothing (No body, no parameter)

   if( FIDM ) { fprintf(stderr, "%d FIDM\n", __LINE__); }
   printf("The visible string '%s'\n", option::ISUSED);

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   if( TIDM ) { printf("%d TIDM\n", __LINE__); }
   if( option::USE_DONT ) {         // We know this is false
     const char* CC= term();
     fprintf(stderr, "%s= term, but it's never called\n", CC);
   }

   if( rc == 5 ) {                  // It is. Let's see if the compiler knows
     printf("%d= 27, or so I'm told\n\n", 27);
     rc= 0;
   }

   printf("You need to look at the listing\n"); // Is this one or two puts?
   printf("Compiling with OPTIMIZE=-O3 makes this quicker\n"); // (It's two)

   return rc;
}
