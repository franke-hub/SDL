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
//       Sample program: Macro usage, including using enums for #defines
//
// Last change date-
//       2020/10/04
//
// Usage notes-
//       You need to look at the listing to verify that compiler optimization
//       removes functions which can never be called.
//
// Implementation notes-
//       Includes some trivial lambda function syntax demonstrations.
//
// Implementation notes-
//       The gcc compiler optimizes better than expected. It even keeps track
//       of variables which have known values. All static const char* strings
//       on unreachable paths are not instantiated.
//
//----------------------------------------------------------------------------
#include <stdio.h>                  // For printf, fprintf

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
// This value is used by the preprocessor. Therefore, the #define is REQUIRED.
#define USE_DEFINE_INSIDE_MACRO 3   // See test_macros: If 0, compile fails.

enum // Compilation controls (modifiable)
{  HCDM= true                       // Hard Core Debug Mode
}; // Compilation controls (modifiable)

enum // Local controls (DO NOT MODIFY)
{  FIDM= false                      // False If Debug Mode
,  TIDM= true                       // True If Debug Mode
}; // local controls

// These might be in an include file
namespace option {                  // Substitutes for #defines
enum // "External" controls (DO NOT MODIFY)
{  USE_DONT= false                  // Instead of #ifdef  false ... #endif
,  USE_TRUE= true                   // Instead of #ifdef  true  ... #endif
,  USE_WHAT= 0xFEEDBEEF             // (This enum is never used)
,  USE_VALUE= false                 // Instead of #undef  VALUE
,  THE_VALUE= 732                   // Instead of #define VALUE 732
}; // "External" controls

// This string is referenced during execution. (NO compile-time concatenation)
static const char*     ISUSED= "IS VISIBLE"; // #define ISUSED "IS VISIBLE"

// This string is never referenced during execution.
static const char*     STRING= "INVISIBLE"; // #define STRING "INVISIBLE"
} // namespace option

//----------------------------------------------------------------------------
// Macro IFVERBOSE, see test_macros (Test #define, #if .. #endif inside macro)
//----------------------------------------------------------------------------
#define IFVERBOSE(n, stmt) if( opt_verbose >= n ) { stmt }

//----------------------------------------------------------------------------
// should_not_occur: (Something that should not occur did)
//----------------------------------------------------------------------------
static inline int                   // Always 1, for convenience
   should_not_occur(                // An unexpected situation happened
     int               line)        // At this line number
{  printf("%4d Should_not_occur\n", line); return 1; }

//----------------------------------------------------------------------------
// this_should_work: (Something that should occur did)
//----------------------------------------------------------------------------
static inline int                   // Always -1, for convenience
   this_should_work(                // The expected happened
     int               line)        // At this line number
{  if( HCDM ) printf("%4d Expected\n", line); return -1; }

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
   init(int, char**)                // Initialize, for example
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
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
   if( option::USE_DONT ) {         // We know this is false
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
//       empty_null
//
// Purpose-
//       This does nothing and has no parameters. See if it's instantiated.
//
// Implementation note-
//       (This might be a placeholder)
//
//----------------------------------------------------------------------------
static void
   empty_null( void )               // Empty subroutine, no return value
{  }                                // Placeholder

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
{
   (void)line;                      // Unused parameter
   return 44;
}

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
{  (void)line; }                    // Placeholder, parameter ignored

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
//       test_macros
//
// Purpose-
//       Test preprocessor macro expansions
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors encountered
   test_macros( void )              // Test macro expansions
{
   int                 error_count= 0; // Number of errors encountered

   printf("\ntest_macros(%d)\n",  USE_DEFINE_INSIDE_MACRO);

   // This just tests normal usage
   int opt_verbose= 0;              // (We're just testing here)
   IFVERBOSE(1,                     // 'Tis false
     error_count += should_not_occur(__LINE__);
   )

   // Can #if be used inside of a macro? YES!
   IFVERBOSE(0,                     // 'Tis true
     #if false                      // This seems to work OK
       error_count += should_not_occur(__LINE__);
     #else
       opt_verbose= 1;
     #endif
   )
   if( opt_verbose != 1 )           // Verifies #if properly handled
     error_count += should_not_occur(__LINE__);

#if USE_DEFINE_INSIDE_MACRO == 0    // --COMPILE FAILS: TF undefined----------
//============================================================================
// Here we #define inside of the macro. When we #undef inside the macro the
// compile fails, since #define exapands AFTER the wrapping macro.

   IFVERBOSE(1,                     // 'Tis true
     #define TF should_not_occur(__LINE__); // Compile failure expected
     TF;
     #undef  TF
   )

   should_not_occur(__LINE__);      // Compile failure expected

#elif USE_DEFINE_INSIDE_MACRO == 1  // ---COMPILES: leaves TF #defined -------
//============================================================================
// Here we #define inside of the macro. We can't #undef inside the macro since
// the #define expansion occurs AFTER the wrapping macro.

   error_count++;
   IFVERBOSE(1,                     // 'Tis true
     #define TF this_should_work(__LINE__);
     error_count += TF;
//// #undef  TF // This is the difference
   )
   if( error_count != 0 )           // Verify correct operation
     error_count += should_not_occur(__LINE__);

#elif USE_DEFINE_INSIDE_MACRO == 2  // Compiles: better practice -------------
//============================================================================
// Here we #define outside of the macro. This has the advantage in that TF
// is opaque, and the __LINE__ parameter is as invoked.

   #define TF this_should_work(__LINE__); // (Temporary #define)

   error_count++;
   IFVERBOSE(1,                     // 'Tis true
     error_count += TF;
   )
   if( error_count != 0 )           // Verify correct operation
     error_count += should_not_occur(__LINE__);

   #undef  TF // Don't leave #defines hanging. TF might easily be a subroutine

#else                               // Compiles: better practice -------------
//============================================================================
// Here we use a lambda function rather than a #define. This has the advantage
// in that there is nothing to #undef, but it requires function syntax.
// This replacement applies only where the TF defines a single function.

   error_count++;
   IFVERBOSE(1,                     // 'Tis true
     if( true ) {                   // Either option works. Take your pick.
       auto TF = [] (int line) { return this_should_work(line); };
       error_count += TF(__LINE__); // This line number
     } else {
       auto TF = [] () { return this_should_work(__LINE__); };
       error_count += TF();         // Note that we need the () here. Also
                                    // the line number is where TF's defined
     }
   )
   if( error_count != 0 )           // Verify correct operation
     error_count += should_not_occur(__LINE__);
#endif

   //-------------------------------------------------------------------------
   // The GCC compiler tracks known values extremely well. It's smart enough
   // to figure out that error_count must now be zero, even though its value
   // changes throughout this subroutine. This author is impressed.
   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_optimize
//
// Purpose-
//       Test compiler optimizations (Sample main replacement)
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors encountered
   test_optimize(                   // Empty subroutine, no return value
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 rc= 0;       // Return code

   // This let's us know we're here
   printf("\ntest_optimize: %s %s\n", __DATE__, __TIME__);

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

   //-------------------------------------------------------------------------
   // Mainline code: Display option values
   //-------------------------------------------------------------------------
   printf("\n%4d Optimization test sequence begins\n", __LINE__);
   if( FIDM ) { fprintf(stderr, "%d FIDM\n", __LINE__); }
   else printf("%4d Not false path taken\n", __LINE__);
   doit();                          // Does nothing

   if( TIDM ) { printf("%d TIDM\n", __LINE__); }
   else fprintf(stderr, "%4d Not true path taken\n", __LINE__);
   printf("%d= empty_line()\n", empty_line()); // Prints 31

   if( TIDM ) { printf("%d TIDM\n", __LINE__); }
   printf("%d= empty_line(L)\n", empty_line(__LINE__)); // Prints __LINE__

   if( TIDM ) { printf("%d TIDM\n", __LINE__); }
   printf("%d= empty_parm\n", empty_parm(__LINE__)); // Prints 44

   if( TIDM ) { printf("%d TIDM\n", __LINE__); }
   empty_void(__LINE__);            // Does nothing (Ignores parameter)

   if( TIDM ) { printf("%d TIDM\n", __LINE__); }
   empty_null();                    // Does nothing (No body, no parameter)

   if( FIDM ) { fprintf(stderr, "%d FIDM\n", __LINE__); }
   printf("The visible string: '%s'\n", option::ISUSED);

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   if( TIDM ) { printf("%d TIDM\n", __LINE__); }
   if( option::USE_DONT ) {         // We know this is false
     const char* CC= term();
     fprintf(stderr, "%s= term, but it's never called\n", CC);
   }

   if( rc == 5 ) {                  // It is. Let's see if the compiler knows
     printf("%d == 27, or so I'm told\n\n", 27); // It does. How cool is that?
     rc= 0;
   }

   printf("You need to look at the listing\n"); // Is this one or two puts?
   printf("Compiling with OPTIMIZE=-O3 makes this quicker\n"); // (It's two)

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
   int                 error_count= 0; // Number of errors encountered

   error_count += test_macros();    // Test macro usage
   error_count += test_optimize(argc, argv); // Test optimization

   if( error_count ) {              // (The GCC compiler knows it's zero!)
     printf("%d Error%s occurred\n", error_count, error_count == 1 ? "" : "s");
     error_count= 1;
   }

   return error_count;              // (The GCC compiler knows it's zero!)
}
