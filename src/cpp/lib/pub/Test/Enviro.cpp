//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2020 Frank Eskesen.
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
//       2020/10/03
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

#include "pub/ifmacro.h"
#include "pub/Object.h"
using namespace _PUB_NAMESPACE;

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

#include "pub/ifmacro.h"            // Verify multiple inclusion

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
class MyException : public std::runtime_error {
   using std::runtime_error::runtime_error;
}; // class MyException

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
     return false;                  // That's what should happen

   printf("Verify error: %s\n", stmt); // Verification error
   return true;                     // Indicate error
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       demo_std_exception
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
   demo_std_exception( void )
{
#ifdef _CC_GCC                      // (This DEMOs the problem)
   #pragma GCC diagnostic ignored "-Wcatch-value"
#endif

   try {
     MyException up("oops");
     throw up;
   } catch(exception x) {
     if( strcmp(x.what(), "oops") != 0 )
     {
       printf("WHAT(%s) HAPPENED?\n", x.what());
       printf("WHAT(%s) HAPPENED? is what was sort of expected\n", "oops");
     }
   }

   printf("demo_std_exception (fault)\n");
   return 0;
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
   printf("test_std_exception, verifies success\n");

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
   MACROF(__cplusplus);
   MACROF(__FAVOR_BSD);
   MACROF(__KERNEL_STRICT_NAMES);
   MACROF(__LARGE64_FILES);
   MACROF(__POSIX_VISIBLE);
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
   printf("%8d sizeof(struct stat.st_size)\n", (int)sizeof(s.st_size));
   printf("%8d sizeof(struct stat64.st_size)\n", (int)sizeof(s64.st_size));
   printf("%8d sizeof(long)\n", (int)sizeof(long));
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

   //-------------------------------------------------------------------------
   // pub Library
   //-------------------------------------------------------------------------
   printf("\n");
   MACROF(_PUB_NAMESPACE);
#ifndef _OS_WIN
   MACROF(_ATTRIBUTE_NORETURN);
#endif

   printf("\n");
   return result;
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
   main(int, char**)                // Mainline entry
//   int             argc,          // Parameter count
//   char*           argv[])        // Parameter vector
{
   int               result= 0;     // Function resultant

   try {
     result |= environment();       // Test compliation environment

     result |= demo_std_exception(); // Demo std::exception usage error
     result |= test_std_exception(); // Test std::exception
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

   if( result != 0 )
     printf("result(%d)\n", result);

   return(result);
}
