//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Verify.h
//
// Purpose-
//       Verification error counter, for test cases.
//
// Last change date-
//       2007/01/01
//
// Macros-
//       verify() : Like standard assert macro, but counts error (no abort)
//       verify_info() : Displays "<FileName> line: " heading
//       verify_exit() : Displays completion message with error count.
//       verify_here() : Displays "<FileName> line: here\" message.
//       error_count() : Returns the current error count.
//
// Usage-
//       debugf() should be used rather than printf()
//
//       For error reporting:
//         (macro) verify( truth_function ) or
//         VerifyEC::_verify_(verificand, __FILE__, __LINE__, fmt, ...)
//
//       Optionally, for messages:
//         (macro) verify_info(); debugf("format\n", ...), or
//
//         // THESE MACROS ARE NOT AVAILABLE FOR MICROSOFT 6.0 COMPILER
//         (macro) verifyf("format\n", args), use
//         (macro) verifyf("%s", "message\n") if no arguments
//
//       On completion:
//         (macro) verify_exit() or
//         return VerifyEC::exit(__FILE__)
//
//----------------------------------------------------------------------------
#ifndef VERIFY_H_INCLUDED
#define VERIFY_H_INCLUDED

#include <stdarg.h>
#include <string.h>

#ifndef DEBUG_H_INCLUDED
#include <com/Debug.h>
#endif

#ifndef DEFINE_H_INCLUDED
#include "define.h"
#endif

#define verify(t) VerifyEC::_verify_((t), __FILE__, __LINE__, #t)

#if 0
  // We cannot use verifyf because our Microsoft compiler is back level
  #define verifyf(format, ...) \
  VerifyEC::message(__FILE__, __LINE__, format, __VA_ARGS__)
#endif

#define verify_exit() return VerifyEC::exit(__FILE__)
#define verify_file(s) return VerifyEC::copyFile(__FILE__, s, sizeof(s))
#define verify_here() VerifyEC::message(__FILE__, __LINE__, "here\n")
#define verify_info() VerifyEC::heading(__FILE__, __LINE__)
#define error_count() VerifyEC::getCount()
#define error_found() VerifyEC::get()->increment()

//----------------------------------------------------------------------------
//
// Class-
//       VerifyEC
//
// Purpose-
//       Error counter.
//
//----------------------------------------------------------------------------
class VerifyEC {                    // Verify Error counter
//----------------------------------------------------------------------------
// VerifyEC::Static attributes
//----------------------------------------------------------------------------
protected:
static VerifyEC*       singleton;   // The VerifyEC

//----------------------------------------------------------------------------
// VerifyEC::Object attributes
//----------------------------------------------------------------------------
protected:
unsigned long          count;       // The error count

//----------------------------------------------------------------------------
// VerifyEC::Constructors
//----------------------------------------------------------------------------
public:
   ~VerifyEC( void ) {}             // Destructor

protected:
   VerifyEC( void )                 // Default Constructor
:  count(0) {}

//----------------------------------------------------------------------------
//
// Internal static method-
//       copyFile
//
// Purpose-
//       Remove any prefix path and suffix type from the file name
//       e.g. "S/Verify.h" becomes "Verify"
//
//----------------------------------------------------------------------------
public:
static inline const char*           // First non-path character
   copyFile(                        // Copy the file name
     const char*       source,      // Source file name
     char*             target,      // Target file name
     int               length)      // Target file name length
{
   const char*         resultant;   // First non-path character
   int                 foundExt;    // TRUE iff extention found

   int                 i;

   if( strlen(source) >= length )
   {
     memcpy(target, source, length-1);
     target[length-1]= '\0';
   }
   else
     strcpy(target, source);

   foundExt= FALSE;
   resultant= target;
   for(i= strlen(target)-1; i>=0; i--)
   {
     if( target[i] == '/' )
     {
       resultant= &target[i+1];
       break;
     }

     if( target[i] == '.' && !foundExt )
     {
       foundExt= TRUE;
       target[i]= '\0';
     }
   }

   return resultant;
}

//----------------------------------------------------------------------------
//
// Internal static method-
//       get
//
// Purpose-
//       Returns a pointer to the singleton VerifyEC object
//
//----------------------------------------------------------------------------
public:
static inline VerifyEC*             // The (only) VerifyEC
   get( void )                      // Get the VerifyEC
{
   if( singleton == NULL )
     singleton= new VerifyEC();

   return singleton;
}

//----------------------------------------------------------------------------
//
// Internal object method-
//       retrieve
//
// Purpose-
//       Get the error count from the object
//
//----------------------------------------------------------------------------
public:
inline unsigned long                // The error count
   retrieve( void ) const           // Get error count
{
   return count;
}

//----------------------------------------------------------------------------
//
// Static method-
//       exit
//
// Purpose-
//       Target for verify_exit()
//       Reports error count statistics. RESETS the error count.
//
//----------------------------------------------------------------------------
public:
static inline int                   // Return code: 0 if no errors
   exit(                            // Exit the VerifyEC
     const char*       file= "Error counting")// Source file name
{
   VerifyEC*           ec= get();   // Get the VerifyEC object
   unsigned long       count= ec->retrieve(); // Get the error count

   int                 rc;          // Module return code
   char                string[2048];// For extracting file name

   // Reset the count
   ec->reset();

   // Display the completion message
   debugf("%s complete, ", copyFile(file, string, sizeof(string)));

   if( count == 0 )
     debugf("NO ");
   else
     debugf("%lu ", count);
   debugf("Error%s\n", count == 1 ? "" : "s");

   // Set the return code
   rc= 0;
   if( count > 0 )
     rc= 1;

   return rc;
}

//----------------------------------------------------------------------------
//
// Static method-
//       getCount
//
// Purpose-
//       Target for error_count()
//       Returns the current error count
//
//----------------------------------------------------------------------------
public:
static inline int                   // The error count
   getCount( void )                 // Get error count
{
   return get()->retrieve();
}

//----------------------------------------------------------------------------
//
// Static method-
//       heading
//
// Purpose-
//       Target for verify_info()
//       Display the "heading" information: i.e. "<FileName> line: "
//
//----------------------------------------------------------------------------
public:
static inline void
   heading(                         // Write standard heading
     const char*       file,        // Source file name
     int               line)        // Line number
{
   char                string[2048];// For extracting file name

   // Write <file> <line>: text
   debugf("%s %4d: ", copyFile(file, string, sizeof(string)), line);
}

//----------------------------------------------------------------------------
//
// Object method-
//       increment
//
// Purpose-
//       Increment the error count in the object
//
//----------------------------------------------------------------------------
public:
inline void
   increment(                       // Increment the error count
     unsigned long     errors= 1)   // By this amount
{
   count += errors;
}

//----------------------------------------------------------------------------
//
// Static method-
//       message
//
// Purpose-
//       Target for verifyf()
//       Display a message without modifying the error count.
//
//----------------------------------------------------------------------------
public:
static inline void
   vmessage(                        // Display a message plus heading
     const char*       file,        // Source file name
     int               line,        // Line number
     const char*       fmt,         // Message
     va_list           argptr)      // Argument list
   _ATTRIBUTE_PRINTF(3,0)
{
   heading(file, line);
   vdebugf(fmt, argptr);
}

static inline void
   message(                         // Display a message plus heading
     const char*       file,        // Source file name
     int               line,        // Line number
     const char*       fmt,         // Message
                       ...)         // Argument list
   _ATTRIBUTE_PRINTF(3,4)
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vmessage(file, line, fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Object method-
//       reset
//
// Purpose-
//       Reset the error count in the object (to zero.)
//
//----------------------------------------------------------------------------
public:
inline void
   reset( void )                    // Reset the error count
{
   count= 0;
}

//----------------------------------------------------------------------------
//
// Static method-
//       _verify_
//
// Purpose-
//       Target for verify()
//
//----------------------------------------------------------------------------
public:
static inline int                   // Returns verificand
   _verify_(                        // Verify TRUE
     int               verificand,  // Should be TRUE
     const char*       file,        // Source file name
     int               line,        // Line number
     const char*       fmt,         // Error message
                       ...)         // Argument list
   _ATTRIBUTE_PRINTF(4,5)
{
   va_list             argptr;      // Argument list pointer

   if( !verificand )
   {
     // Increment the error count
     get()->increment();

     // Write the error message
     message(file, line, "Verify error: ");

     va_start(argptr, fmt);         // Initialize va_ functions
     vdebugf(fmt, argptr);
     va_end(argptr);                // Close va_ functions

     debugf("\n");
   }

   return verificand;
}

//----------------------------------------------------------------------------
//
// Static method-
//       _verify_
//
// Purpose-
//       Target for verify("error message")
//
//----------------------------------------------------------------------------
public:
static inline int                   // Returns FALSE
   _verify_(                        // Verify TRUE
     const char*       verificand,  // Should be TRUE
     const char*       file,        // Source file name
     int               line,        // Line number
     const char*       fmt,         // Error message
                       ...)         // Argument list
{
   // Increment the error count
   get()->increment();

   // Write the error message
   message(file, line, "Verify error: %s\n", verificand);

   return FALSE;
}
}; // class VerifyEC

#endif // VERIFY_H_INCLUDED
