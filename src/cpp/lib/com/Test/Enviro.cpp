//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2018 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Enviro.cpp
//
// Purpose-
//       Display environmental control variables.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#define _FILE_OFFSET_BITS 64        // (Required for LINUX)
// #define _LARGEFILE_SOURCE 1
// #define _LARGEFILE64_SOURCE 1

#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include <exception>
#include <string>
using namespace std;

#include "com/define.h"
#include "com/Debug.h"
#include "com/ifmacro.h"
#include "com/inline.h"
#include "com/Random.h"
#include "com/Unconditional.h"

#ifdef _OS_BSD
  #include <curses.h>
#endif

#ifdef _OS_WIN
  #include <Windows.h>
  #if( _MSC_VER > 1200 )
    #define lstat64 _stat64         // No links in _OS_WIN
    #define stat64  _stat64         // stat64 == _stat64
  #else
    #define lstat64 _stat           // No links in _OS_WIN
    #define stat64  _stat           // stat64 == _stat
  #endif
#endif

#ifdef _OS_CYGWIN
#define stat64 stat                 // stat == stat64 in CYGWIN
#endif

#undef _ADDR64
#if defined(_WIN64) || defined(__x86_64__)
  #define _ADDR64
#elif !defined(_CC_MSC) && !defined(_CC_GCC)
  #error "_ADDR64 indeterminate"
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#include "com/ifmacro.h"            // Verify multiple inclusion

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define concat(x) "" #x ""
#define catcon(x) concat(x)

#define MACROF(x) \
   printf ("%8.5s %s(%s)\n",  "", #x, catcon(x))

#define VERIFY(x) verify(x, #x)

//----------------------------------------------------------------------------
//
// Class-
//       MyException
//
// Purpose-
//       Extend std::exception with message
//
//----------------------------------------------------------------------------
class MyException : public std::exception {
protected:
const char*            message;

public:
   MyException(                    // Constructor
     const char*       message= "MyException") // resultant message
:  message(message)
{
}

virtual const char*                // Resultant
   what( void ) const              // Return descriptor message
   throw()                         // NO EXCEPTIONS
{
   return message;
}
}; // class MyException

//----------------------------------------------------------------------------
//
// Class-
//       Random31
//
// Purpose-
//       Extend com/Random to return positive integer value
//
//----------------------------------------------------------------------------
class Random31 : public Random {
public:
inline int32_t                     // A positive integer
   get31( void )                   // Get a positive integer
{
   return int32_t(get() & 0x7fffffff);
}
}; // class Random31

//----------------------------------------------------------------------------
//
// Subroutine-
//       torf
//
// Purpose-
//       Returns " TRUE" or "FALSE" string
//
//----------------------------------------------------------------------------
static inline const char*           // " TRUE" or "FALSE"
   torf(                            // True or False test
     unsigned int    const variable)// The test variable
{
   if (variable)                    // If variable is true
     return (" TRUE");              // Indicate true

   else                             // If variable is false
     return ("FALSE");              // Indicate false
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verify
//
// Purpose-
//       Verify statement
//
//----------------------------------------------------------------------------
static inline int                   // Returns !cc
   verify(                          // Verify statement
     int             cc,            // Condition (Should be TRUE)
     const char*     stmt)          // Condition test
{
   if( cc )                         // If variable is true
     return 0;                      // That's what should happen

   printf("Verify error: %s\n", stmt); // Verification error
   return TRUE;                     // Indicate error
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       environment
//
// Purpose-
//       Verify compilation controls
//
//----------------------------------------------------------------------------
static inline int
   environment( void )              // Verify compilation controls
{
   const char*       chs;           // Pointer to character string
   int               result= 0;     // Resultant

   //-------------------------------------------------------------------------
   // Variables which may be defined
   //-------------------------------------------------------------------------
   printf("\n\n");
   printf("Definition variables:\n");
   printf("__LINE__(%d) __FILE__(%s)\n", __LINE__, __FILE__);
   printf("\n");

   chs= "NOT";
#ifdef _ADDR64
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "_ADDR64");

   chs= "NOT";
#ifdef _LONG_LONG
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "_LONG_LONG");

   chs= "NOT";
#ifdef LONGLONG_MIN
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "LONGLONG_MIN");

   chs= "NOT";
#ifdef LONG_LONG_MIN
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "LONG_LONG_MIN");

   chs= "NOT";
#ifdef _ALL_SOURCE
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "_ALL_SOURCE");

   chs= "NOT";
#ifdef _ANSI_C_SOURCE
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "_ANSI_C_SOURCE");

   chs= "NOT";
#ifdef __GNUC__
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "__GNUC__");

   chs= "NOT";
#ifdef __GNUG__
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "__GNUG__");

   chs= "NOT";
#ifdef _POSIX_SOURCE
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "_POSIX_SOURCE");

   chs= "NOT";
  #ifdef _WIN64
    chs= "IS";
  #endif
   printf("%8.5s defined(%s)\n", chs, "_WIN64");

   chs= "NOT";
#ifdef _XOPEN_SOURCE
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "_XOPEN_SOURCE");

   chs= "NOT";
#ifdef _X86_
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "_X86_");

   chs= "NOT";
#ifdef __x86_64__
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "__x86_64__");

   //-------------------------------------------------------------------------
   // Windows.h
   //-------------------------------------------------------------------------
   printf("\n");

   MACROF(_INTEGRAL_MAX_BITS);
   MACROF(_MSC_VER);
   MACROF(LONG);
   MACROF(WINADVAPI);
   MACROF(WINAPI);
   MACROF(WINVER);

   //-------------------------------------------------------------------------
   // GNU Compiler
   //-------------------------------------------------------------------------
   printf("\n");
   MACROF(__GNUC__);
   MACROF(__GNUG__);
   MACROF(__GCC_ATOMIC_BOOL_LOCK_FREE);
   MACROF(__GCC_ATOMIC_CHAR_LOCK_FREE);
   MACROF(__GCC_ATOMIC_SHORT_LOCK_FREE);
   MACROF(__GCC_ATOMIC_INT_LOCK_FREE);
   MACROF(__GCC_ATOMIC_LONG_LOCK_FREE);
   MACROF(__GCC_ATOMIC_LLONG_LOCK_FREE);
   MACROF(__GCC_ATOMIC_POINTER_LOCK_FREE);
   MACROF(__GNUC_STDC_INLINE__);

   // Temporary tests
   // MACROF(_ELIDABLE_INLINE);     "static __inline__"

   //-------------------------------------------------------------------------
   // BSD/Linux
   //-------------------------------------------------------------------------
   printf("\n");
   MACROF(__BSD_VISIBLE);
   MACROF(__cplusplus);
   MACROF(__FAVOR_BSD);
   MACROF(__KERNEL_STRICT_NAMES);
   MACROF(__LARGE64_FILES);
   MACROF(__MISC_VISIBLE);
   MACROF(__POSIX_VISIBLE);
   MACROF(__SSP_FORTIFY_LEVEL);
   MACROF(__USE_BSD);
   MACROF(__USE_FILE_OFFSET64);
   MACROF(__USE_GNU);
   MACROF(__USE_ISOC9X);
   MACROF(__USE_LARGEFILE);
   MACROF(__USE_LARGEFILE64);
   MACROF(__USE_MISC);
   MACROF(__USE_POSIX);
   MACROF(__USE_POSIX199309);
   MACROF(__USE_POSIX199506);
   MACROF(__USE_POSIX2);
   MACROF(__USE_REENTRANT);
   MACROF(__USE_SVID);
   MACROF(__USE_UNIX98);
   MACROF(__USE_XOPEN);
   MACROF(__USE_XOPEN_EXTENDED);
   MACROF(__XSI_VISIBLE);
   MACROF(_FILE_OFFSET_BITS);
   MACROF(_LARGEFILE_SOURCE);
   MACROF(_LARGEFILE64_SOURCE);
   MACROF(NULL);
   MACROF(lstat);
   MACROF(off_t);

   //-------------------------------------------------------------------------
   // Operating system controls
   //-------------------------------------------------------------------------
   printf("\n");

   chs= "NOT";
#ifdef _CC_GCC
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "_CC_GCC");

   chs= "NOT";
#ifdef _CC_MSC
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "_CC_MSC");

   chs= "NOT";
#ifdef _CC_XLC
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "_CC_XLC");

   chs= "NOT";
#ifdef _OS_BSD
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "_OS_BSD");

   chs= "NOT";
#ifdef _OS_CYGWIN
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "_OS_CYGWIN");

   chs= "NOT";
#ifdef _OS_DOS
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "_OS_DOS");

   chs= "NOT";
#ifdef _OS_LINUX
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "_OS_LINUX");

   chs= "NOT";
#ifdef _OS_WIN
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "_OS_WIN");

   //-------------------------------------------------------------------------
   // Hardware controls
   //-------------------------------------------------------------------------
   printf("\n");

   chs= "NOT";
#ifdef _HW_PPC
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "_HW_PPC");

   chs= "NOT";
#ifdef _HW_X86
   chs= "IS";
#endif
   printf("%8.5s defined(%s)\n", chs, "_HW_X86");

   //-------------------------------------------------------------------------
   // inline.h
   //-------------------------------------------------------------------------
   printf("\n"); // inline.h
   MACROF(INLINE);
   MACROF(INLINING);

   //-------------------------------------------------------------------------
   // Variables which must be defined
   //-------------------------------------------------------------------------
   struct stat s;
   struct stat64 s64;
   printf("\n\n");
   printf("Required variables:\n");
   printf("\n");
   printf("%8x INT_MAX\n", INT_MAX);
   printf("%8lx LONG_MAX\n", LONG_MAX);
   printf("%8d sizeof(off_t)\n", (int)sizeof(off_t));
   printf("%8d sizeof(long)\n", (int)sizeof(long));
   printf("%8d sizeof(size_t)\n", (int)sizeof(size_t));
   printf("%8d sizeof(struct stat.st_size)\n", (int)sizeof(s.st_size));
   printf("%8d sizeof(struct stat64.st_size)\n", (int)sizeof(s64.st_size));
   printf("%8d sizeof(void*)\n", (int)sizeof(void*));

   //-------------------------------------------------------------------------
   // Verify stdint.h
   //-------------------------------------------------------------------------
   result |= VERIFY(sizeof(  int8_t) == 1);
   result |= VERIFY(sizeof( uint8_t) == 1);
   result |= VERIFY(sizeof( int16_t) == 2);
   result |= VERIFY(sizeof(uint16_t) == 2);
   result |= VERIFY(sizeof( int32_t) == 4);
   result |= VERIFY(sizeof(uint32_t) == 4);
   result |= VERIFY(sizeof( int64_t) == 8);
   result |= VERIFY(sizeof(uint64_t) == 8);

   //-------------------------------------------------------------------------
   // Environment variables
   //-------------------------------------------------------------------------
   printf("\n\n");
   printf("Environment variables:\n");
   printf("\n");
   printf("HOME(%s)\n", getenv("HOME"));
   printf("HOST(%s)\n", getenv("HOST"));
   printf("JAVA_HOME(%s)\n", getenv("JAVA_HOME"));
   printf("TEMP(%s)\n", getenv("TEMP"));
   printf("USER(%s)\n", getenv("USER"));

   printf("\n");
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_stdlib_rand
//
// Purpose-
//       Test rand()
//
// Notes-
//       In _OS_WIN, RAND_MAX == 0x7fff and this test fails.
//       Use "com/Random.h" Random object.
//
//----------------------------------------------------------------------------
static inline int
   test_stdlib_rand( void )         // Test rand()
{
   printf("test_stdlib_rand()\n");

   enum                             // Generic enum
   {  ITERATIONS= 10000
   ,  BUFF_COUNT= 100
   }; // enum

   int                 item[BUFF_COUNT]; // RAND resultant array
   int                 i, j;

   srand( time(NULL) );             // Generate random seed
   for(i= 0; i<BUFF_COUNT; i++)
     item[i]= rand();

   for(i= 0; i<BUFF_COUNT; i++)
   {
     for(j= i+1; j<BUFF_COUNT; j++)
     {
       if( item[i] == item[j] )
       {
         fprintf(stderr, "%4d %d %d\n", __LINE__, item[i], item[j]);
         throw "Should Not Occur";
       }
     }
   }

   for(int iteration= 0; iteration<ITERATIONS; iteration++)
   {
     int value= rand();
     for(i= 0; i<BUFF_COUNT; i++)
     {
       if( item[i] == value )
       {
         fprintf(stderr, "%4d %d %d\n", __LINE__, item[i], value);
         throw "Should Not Occur";
       }
     }
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_stdlib
//
// Purpose-
//       Test stdlib.h function
//
//----------------------------------------------------------------------------
static inline int
   test_stdlib( void )              // Test stdlib.h
{
   int                 result= 0;   // Resultant

   try {
     result |= test_stdlib_rand();  // RAND function
   } catch(const char* x) {
     fprintf(stderr, "catch(const char*(%s))\n", x);
     result= 2;
   } catch(exception& x) {
     fprintf(stderr, "catch(exception.what(%s))\n", x.what());
     result= 2;
   } catch(...) {
     fprintf(stderr, "catch(...)\n");
     result= 2;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_types
//
// Purpose-
//       Test types
//
//----------------------------------------------------------------------------
static inline int
   test_types( void )               // Test types
{
   int                 some_int;    // An integer
   char                some_char;   // A character

   printf("test_types()\n");

   // Are characters unsigned by default?
   some_char= -1;
   some_int= some_char;
   // printf("verify(%d)\n", some_int);

   if( some_int < 0 ) {
     printf("#define CHAR_IS_SIGNED\n");
     printf("#undef  CHAR_IS_UNSIGNED\n");
   } else {
     printf("#undef  CHAR_IS_SIGNED\n");
     printf("#define CHAR_IS_UNSIGNED\n");
   }

   // Is right shift unsigned for signed values?
   some_int= -1;
   some_int= some_int >> 1;
   // printf("verify(%d)\n", some_int);

   if( some_int < 0 ) {
     printf("#define RIGHT_SHIFT_IS_SIGNED\n");
     printf("#undef  RIGHT_SHIFT_IS_UNSIGNED\n");
   } else {
     printf("#undef  RIGHT_SHIFT_IS_SIGNED\n");
     printf("#define RIGHT_SHIFT_IS_UNSIGNED\n");
   }

   // ncursec/curses.h values
   #ifdef _OS_BSD
     int can_change_color_= 0;
     int colors_= -1;
     int color_pairs_= -1;
     int has_colors_= 0;

     initscr();

     has_colors_= has_colors();
     if( has_colors_ ) {
       start_color();
       can_change_color_= can_change_color();
       colors_= COLORS;
       color_pairs_= COLOR_PAIRS;
     } else {
       debugf("COLORS not supported\n");
     }

     endwin();

     printf("\nncurses/curses.h variables:\n");
     printf("%5s= has_colors()\n", has_colors_ ? "true" : "false");
     printf("%5s= can_change_color()\n", can_change_color_ ? "true" : "false");
     printf("%5d= COLORS\n", colors_);
     printf("%5d= COLOR_PAIRS\n", color_pairs_);
   #endif

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       demo_std_exception_usage_error
//
// Purpose-
//       Demonstrate std::exception usage error
//
// Elaboration-
//       The catch statement should read: catch(exception& x)
//       Since it does not, the exception is COPIED into a std::exception
//       and that exception is the BASE std::exception
//
//----------------------------------------------------------------------------
static inline int
   demo_std_exception_usage_error( void )
{
   int                 result= 0;

   try {
     MyException up("oops");
     throw up;
   } catch(exception x) {
     if( strcmp(x.what(), "oops") != 0 )
     {
       result= 2;
       printf("WHAT(%s) HAPPENED?\n", x.what());
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_std_exception
//
// Purpose-
//       Test std::exception
//
//----------------------------------------------------------------------------
static inline int
   test_std_exception( void )       // Test std::exception
{
   try {
     MyException up("oops");
     throw up;
   } catch(exception& x) {
     if( strcmp(x.what(), "oops") != 0 )
     {
       printf("WHAT(%s) HAPPENED?\n", x.what());
       throw "Should Not Occur";
     }
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_std_string
//
// Purpose-
//       Test std::string for memory leaks
//
//----------------------------------------------------------------------------
static inline int
   test_std_string( void )          // Test std::string
{
   printf("test_std_string()...\n");

   enum                             // Generic enum
   {  ITERATIONS= 10000000
   ,  BUFF_SIZE=  2048
   ,  BUFF_COUNT= 200000
   }; // enum

   string**            object;      // The string object array
   char                data[32];    // The next data item
   Random31            rand;        // Random number generator

   char                pri[BUFF_SIZE]; // PRImary buffer
   char                sec[BUFF_SIZE]; // SEConcary buffer
   int                 priX= (-1);  // Primary buffer index
   int                 secX= (-1);  // Secondary buffer index

   int                 i;

   //--------------------------------------------------------------------------
   // Allocate the object array
   object= (string**)Unconditional::malloc(BUFF_COUNT * sizeof(string*));

   memset(pri, '*', BUFF_SIZE-1);
   pri[BUFF_SIZE-1]= '\0';

   for(i= 0; i<BUFF_COUNT; i++)
     object[i]= new string(pri);

   printf("test_std_string()... memory leak test\n");

   //--------------------------------------------------------------------------
   // String hard-core usage, test for memory leakage
   rand.randomize();
   for(int iteration= 0; iteration < ITERATIONS; iteration++)
   {
     int x= rand.get31() % BUFF_COUNT;
     if( x == priX )
     {
       if( strcmp(pri, object[x]->c_str()) != 0 )
         throw "Should Not Occur";
     }

     if( x == secX )
     {
       if( strcmp(sec, object[x]->c_str()) != 0 )
         throw "Should Not Occur";
     }

     if( object[x] == NULL )
     {
       if( priX < 0 )
       {
         priX= x;
         pri[0]= '\0';
       }

       else if( secX < 0 )
       {
         secX= x;
         sec[0]= '\0';
       }

       object[x]= new string("");
     }

     int length= object[x]->length();
     int L= sprintf(data, "%s%d", (length == 0) ? "" : ",", rand.get31());
     if( (length + L) >= BUFF_SIZE )
     {
       delete object[x];
       object[x]= NULL;

       if( x == priX )
         priX= (-1);
       else if( x == secX )
         secX= (-1);

       continue;
     }

     *object[x] += data;
     if( x == priX )
       strcat(pri, data);
     else if( x == secX )
       strcat(sec, data);
   }

   //--------------------------------------------------------------------------
   // Verify the PRI and SEC buffers
   if( priX > 0 && strcmp(pri, object[priX]->c_str()) != 0 )
     throw "Should Not Occur";

   if( secX > 0 && strcmp(sec, object[secX]->c_str()) != 0 )
     throw "Should Not Occur";

   IFSCDM(
     printf("[%d] '%s'\n", priX, pri);
     printf("[%d] '%s'\n", secX, sec);
   )

   //--------------------------------------------------------------------------
   // Release all allocated storage
   for(i= 0; i<BUFF_COUNT; i++)
     delete object[i];

   free(object);

   printf("...test_std_string()\n");
   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_dirty
//
// Purpose-
//       Quick and dirty test.
//
//----------------------------------------------------------------------------
static inline int
   test_dirty( void )               // Quick and dirty test
{
   unsigned int foo= 3;

   // Similar code fails in a build project
   return (::strncasecmp("alpha", "ALPHA", foo) != 0);
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
int                                 // Return code
   main(                            // Mainline entry
     int             argc,          // Parameter count
     char*           argv[])        // Parameter vector
{
   int               result= 0;     // Function resultant

   try {
     result |= environment();       // Test compliation environment
     result |= test_types();        // Test types

//   result |= test_stdlib();       // Test stdlib.h functions
//   result |= demo_std_exception_usage_error(); // Demo std::exception usage error
//   result |= test_std_exception();          // Test std::exception
//   result |= test_std_string();             // Test std::string
     result |= test_dirty();        // Quick and dirty test
   } catch(const char* x) {
     fprintf(stderr, "catch(const char*(%s))\n", x);
     result= 2;
   } catch(exception& x) {
     fprintf(stderr, "catch(exception.what(%s))\n", x.what());
     result= 2;
   } catch(...) {
     fprintf(stderr, "catch(...)\n");
     result= 2;
   }

   if( result == 0 )
     printf("\nNo errors detected\n");
   else
     printf("\nresult(%d)\n", result);

   return(result);
}

