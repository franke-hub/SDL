//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2021 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       utility.cpp
//
// Purpose-
//       Implement utility namespace methods.
//
// Last change date-
//       2021/05/23
//
//----------------------------------------------------------------------------
#include <mutex>                    // For std::lock_guard
#include <sstream>                  // For std::stringstream
#include <string>                   // For std::string
#include <thread>                   // For std::thread

#include <ctype.h>                  // For isspace
#include <errno.h>                  // For errno, EINVAL, ERANGE, ...
#include <inttypes.h>               // For PRIx64
#include <limits.h>                 // For INT_MIN, INT_MAX, ...
#include <stdarg.h>                 // For va_list, ...
#include <stdio.h>                  // For FILE definition
#include <string.h>                 // For memcpy, ...
#include <time.h>                   // For clock_gettime, ...

#include <pub/Debug.h>              // For Debug object (see dump())
#include "pub/utility.h"            // Function definitions

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Namespace-
//       utility
//
// Purpose-
//       Implement pub/utility functions.
//
//----------------------------------------------------------------------------
namespace utility {
//----------------------------------------------------------------------------
// Volatile data (For avoiding compiler optimizations)
//----------------------------------------------------------------------------
volatile int           data= 0;     // For any use
volatile int           unit= 1;     // By convention, always 1
volatile int           zero= 0;     // By convention, always 0
int nop( void ) { return 0; }       // Returns zero. Don't tell the compiler!

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::atoi
//
// Purpose-
//       Convert ascii string to integer.
//
// Implementation notes-
//       errno= EINVAL; // Indicates invalid value detected.
//       errno= ERANGE; // Indicates invalid range detected.
//
//----------------------------------------------------------------------------
int                                 // Resultant value
   atoi(                            // Convert ASCII to int
     const char*       inp)         // Input string
{
   inp= skip_space(inp);
   long result= atol(inp);
   if( inp[0] == '0' && (inp[1] == 'x' || inp[1] == 'X') ) {
     if( result & 0xffffffff00000000L )
       errno= ERANGE;
   } else if( result < long(INT_MIN) || result > long(INT_MAX) ) {
       if( result != (long(INT_MAX) + 1) || *inp != '-' )
         errno= ERANGE;
   }

   return int(result);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::atol
//
// Purpose-
//       Convert ascii string to long.
//
// Implementation notes-
//       errno= EINVAL; // Indicates invalid value detected.
//       errno= ERANGE; // Indicates invalid range detected.
//
//----------------------------------------------------------------------------
long                                // Resultant value
   atol(                            // Convert ASCII to long
     const char*       inp)         // Input string
{
   long                result= 0;   // Resultant

   inp= skip_space(inp);
   if( *inp == '0' && (inp[1] == 'x' || inp[1] == 'X') )
     result= atox(inp);
   else {
     bool minus= false;             // True if leading '-'

     if( *inp == '-' ) {            // If leading '-'
       minus= true;
       inp++;
     } else if( *inp == '+' ) {     // If leading '+'
       inp++;
     }

     if( *inp == '\0' )             // If empty string
       errno= EINVAL;
     while( *inp != '\0' && !isspace(*inp) ) {
       if( *inp < '0' || *inp > '9') {
         errno= EINVAL;
         break;
       }

       result *= 10;
       result += *inp - '0';
       if( result < 0 && (minus == false || result != LONG_MIN) ) { // If overflow
         errno= ERANGE;
         break;
       }

       inp++;
     }

     if( minus )
       result= -result;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::atoui
//       utility::atoul
//
// Purpose-
//       Unsigned versions of atoi, atol.
//
//----------------------------------------------------------------------------
unsigned                            // Resultant value
   atoui(                           // Convert ASCII to unsigned integer
     const char*       inp)         // Input string
{
   inp= skip_space(inp);
   long result= atoul(inp);
   if( inp[0] == '0' && (inp[1] == 'x' || inp[1] == 'X') ) {
     if( result & 0xffffffff00000000L )
       errno= ERANGE;
   } else if( result > long(UINT_MAX) ) {
       errno= ERANGE;
   }

   return int(result);
}

unsigned long                       // Resultant value
   atoul(                           // Convert ASCII to unsigned long
     const char*       inp)         // Input string
{
   long                result= 0;   // Resultant

   inp= skip_space(inp);
   if( *inp == '0' && (inp[1] == 'x' || inp[1] == 'X') )
     result= atox(inp);
   else {
     if( *inp == '+' )              // If leading '+'
       inp++;

     if( *inp == '\0' )             // If empty string
       errno= EINVAL;
     while( *inp != '\0' && !isspace(*inp) ) {
       if( *inp < '0' || *inp > '9') {
         errno= EINVAL;
         break;
       }

       result *= 10;
       result += *inp - '0';
       if( result < 0 ) {           // If overflow
         errno= ERANGE;
         break;
       }

       inp++;
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::atox
//
// Purpose-
//       Convert hexidecimal string to long.
//
// Implementation notes-
//       errno= EINVAL; // Indicates invalid value detected.
//       errno= ERANGE; // Indicates invalid range detected.
//
//----------------------------------------------------------------------------
long                                // Resultant value
   atox(                            // Convert ASCII to hexidecimal
     const char*       inp)         // Input string
{
   unsigned long       result= 0;   // Resultant

   inp= skip_space(inp);            // Skip leading blanks
   if( *inp == '0' && (inp[1] == 'x' || inp[1] == 'X') )
     inp += 2;

   if( *inp == '\0' )               // If empty string
     errno= EINVAL;
   while( *inp != '\0' && !isspace(*inp) ) {
     if( result & 0xf000000000000000L ) // If overflow will occur
       errno= ERANGE;
     result <<= 4;

     if( *inp >= '0' && *inp <= '9')
       result += *inp - '0';
     else if( *inp >= 'a' && *inp <= 'f')
       result += (*inp - 'a') + 10;
     else if( *inp >= 'A' && *inp <= 'F')
       result += (*inp - 'A') + 10;
     else {
       errno= EINVAL;
       return result;
     }

     inp++;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::clock
//
// Purpose-
//       Return the number of nanoseconds since the epoch start
//
//----------------------------------------------------------------------------
uint64_t                            // The nanoseconds since epoch start
   clock( void )                    // Get nanoseconds since epoch start
{
   struct timespec     time;        // UTC time base

   clock_gettime(CLOCK_REALTIME, &time); // Get UTC time base
   uint64_t nano= time.tv_sec * 1000000000;
   nano += time.tv_nsec;

   return nano;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::dump
//
// Purpose-
//       Dump formatter.
//
//----------------------------------------------------------------------------
void                                // Dump formatter
   dump(                            // Dump formatter
     FILE*             file,        // Output FILE
     const void*       addrp,       // Input data address
     size_t            size,        // Input data size
     const void*       addrv)       // Virtual data address (= addrp)
{
enum FSM                            // Finite State machine
{  FSM_FIRST                        // First file line
,  FSM_UNDUP                        // Not repeating line
,  FSM_INDUP                        // Repeating line
};

   auto position = [](int inp) { return (inp/4) + (inp*2); };

   intptr_t            oldAddr;     // Repeat address origin
   const char*         paddr= (const char*)addrp; // Physical address
   intptr_t            vaddr= (intptr_t)addrv;    // Virtual address

   int                 fsm= FSM_FIRST; // Current state

   char                newData[20]; // The current line string
   char                oldData[20]; // The repeated line string
   char                output[128]; // Format work area

   //-----------------------------------------------------------------------
   // Initialize for the first line
   //-----------------------------------------------------------------------
   oldAddr= vaddr;
   int offset= vaddr & 15;
   size_t length= 16 - offset;
   if( length > size )
     length= size;

   if( length > 0 )
     memcpy(newData+offset, paddr, length);

   vaddr &= (-16);
   size += offset;

   //-------------------------------------------------------------------------
   // Format lines
   //-------------------------------------------------------------------------
   while( size > 0 ) {              // Format lines
     // At this point:
     //   1) The remaining size is >= 16, or
     //   2) This is the last line to be dumped.
     switch(fsm) {                  // Process by state
       case FSM_UNDUP:              // If UNDUP state
         if( size <= 16 )           // If this is the last line
           break;                   // Do not go into INDUP state

         if( memcmp(newData, oldData, 16) == 0 ) // If duplicate line
           fsm= FSM_INDUP;          // Go into duplicate state
         break;

       case FSM_INDUP:              // If INDUP state
         if( size < 16 || memcmp(newData, oldData, 16) != 0 ) {
           fsm= FSM_UNDUP;
           if( sizeof(void*) > 4 )
             fprintf(file, "%.16" PRIX64 "  to %.16" PRIX64
                           ", lines duplicated\n",
                           int64_t(oldAddr), int64_t(vaddr-1));
           else
             fprintf(file, "%.8lX  to %.8lX, lines duplicated\n",
                           long(oldAddr), long(vaddr-1));
         }

         break;

       case FSM_FIRST:              // If FIRST file line
       default:                     // (No other case exists)
         break;                     // Cannot go into UNDUP state
     }

     switch(fsm) {                  // Process by state
       case FSM_FIRST:              // If FIRST file line
       case FSM_UNDUP:              // If UNDUP state
         if( size == 0 )
           break;

         memcpy(oldData, newData, 16); // Save the string for compare

         for(int i=0; i<16; i++)
           sprintf(output+position(i), "%.2X ", newData[i]&0x00ff);

         for(int i=0; i<16; i++) {
           if( !isprint(newData[i]) )
             newData[i]= '.';
         }

         offset= oldAddr & 15;
         for(int i=0; i<offset; i++) { // Handle leading unused
           newData[i]= '~';
           output[position(i)]= '~';
           output[position(i)+1]= '~';
         }

         for(int i=size; i<16; i++) { // Handle trailing unused
           newData[i]= '~';
           output[position(i)]= '~';
           output[position(i)+1]= '~';
         }

         if( sizeof(void*) > 4 )
           fprintf(file, "%.16" PRIX64 "  %s |%.16s|\n",
                         (int64_t)vaddr, output, newData);
         else
           fprintf(file, "%.8lX  %s |%.16s|\n",
                         (long)vaddr, output, newData);

         oldAddr= vaddr;            // Save duplicate origin (zero offset)
         if( offset == 0 )
           fsm= FSM_UNDUP;

         break;

       case FSM_INDUP:              // If INDUP state
       default:                     // (No other case exists)
         break;
     }

     // Set for next line
     if( size < 16 )                // If the line was not full
       break;                       // It must have been end of dump

     paddr += 16-offset;
     vaddr += 16;
     size  -= 16;
     offset = 0;

     if( size >= 16 )
       memcpy(newData, paddr, 16);
     else if( size > 0 )
       memcpy(newData, paddr, size);
   }

   if( fsm == FSM_INDUP ) {         // If duplicating lines
     if( sizeof(void*) > 4 )
       fprintf(file, "%.16" PRIX64 "  to %.16" PRIX64 ", lines duplicated\n",
                     int64_t(oldAddr), int64_t(vaddr-1));
     else
       fprintf(file, "%.8lX  to %.8lX, lines duplicated\n",
                     long(oldAddr), long(vaddr-1));
   }
}

void                                // Dump formatter
   dump(                            // Dump formatter
     FILE*             file,        // Output FILE
     const void*       addrp,       // Input data address
     size_t            size)        // Input data size
{  dump(file, addrp, size, addrp); }

void                                // Dump formatter
   dump(                            // Dump formatter
     const void*       addrp,       // Input data address
     size_t            size)        // Input data size
{
   Debug* debug= Debug::get();      // Get Debug*
   std::lock_guard<decltype(*debug)> lock(*debug);

   FILE* file= debug->get_FILE();
   dump(file, addrp, size, addrp);
   debug->flush();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::find_space
//
// Purpose-
//       Find next whitespace (or '\0') character.
//
//----------------------------------------------------------------------------
char*                               // Next whitespace or '\0' character
   find_space(                      // Find next whitespace character
     const char*       inp)         // Input string
{
   while( *inp != '\0' && !isspace(*inp) )
     inp++;

   return const_cast<char*>(inp);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::skip_space
//
// Purpose-
//       Find next non-whitespace character, including '\0'
//
//----------------------------------------------------------------------------
char*                               // Next non-whitespace character
   skip_space(                      // Find next non-whitespace character
     const char*       inp)         // Input string
{
   while( isspace(*inp) )
     inp++;

   return const_cast<char*>(inp);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::strcasecmp
//       utility::strncasecmp
//
// Purpose-
//       String compare, ignoring case.
//
// Implementation note-
//       ::strcasecmp and ::strncasecmp are not standard library functions
//
//----------------------------------------------------------------------------
int                                 // Resultant <0,=0,>0
   strcasecmp(                      // String insensitive compare
     const char*       L_,          // Left hand side
     const char*       R_)          // Right hand side
{
   const unsigned char* L= (const unsigned char*)L_;
   const unsigned char* R= (const unsigned char*)R_;
   while( true ) {
     int diff= toupper(*L) - toupper(*R);
     if( diff != 0 )
       return diff;
     if( *L == '\0' )
       break;

     L++;
     R++;
   }

   return 0;
}

int                                 // Resultant <0,=0,>0
   strncasecmp(                     // String insensitive compare
     const char*       L_,          // Left hand side
     const char*       R_,          // Right hand side
     size_t            size)        // Maximum comparison length
{
   const unsigned char* L= (const unsigned char*)L_;
   const unsigned char* R= (const unsigned char*)R_;
   while( size != 0 ) {
     int diff= toupper(*L) - toupper(*R);
     if( diff != 0 )
       return diff;
     if( *L == '\0' )
       break;

     L++;
     R++;
     size--;
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::wildchar::strcmp
//       utility::wildchar::strcasecmp
//
// Purpose-
//       Wildcard string compare.
//
//----------------------------------------------------------------------------
namespace wildchar {
int                                 // Resultant 0, !0
   strcmp(                          // Wildchar string compare
     const char*       L_,          // Left hand side (May contain wildchars)
     const char*       R_)          // Right hand side
{
   const unsigned char* W= (const unsigned char*)L_;
   const unsigned char* R= (const unsigned char*)R_;
   while( true ) {
     int diff= *W - *R;
     if( diff != 0 ) {
       if( *W == '*' ) {
         while( *W == '*' )
           W++;
         if( *W == '\0' )
           return 0;
         while( *R != '\0' ) {
           diff= strcmp((char*)W, (char*)R);
           if( diff == 0 )
             break;
           R++;
         }
         return diff;
       } else if( *W == '?' ) {
         if( *R == '\0' )
           return diff;
       } else
         return diff;
     } else if( *W == '\0' )
       break;

     W++;
     R++;
   }

   return 0;
}

int                                 // Resultant 0, !0
   strcasecmp(                      // String insensitive compare
     const char*       L_,          // Left hand side (May contain wildchars)
     const char*       R_)          // Right hand side
{
   const unsigned char* W= (const unsigned char*)L_;
   const unsigned char* R= (const unsigned char*)R_;
   while( true ) {
     int diff= toupper(*W) - toupper(*R);
     if( diff != 0 ) {
       if( *W == '*' ) {
         while( *W == '*' )
           W++;
         if( *W == '\0' )
           return 0;
         while( *R != '\0' ) {
           diff= strcasecmp((char*)W, (char*)R);
           if( diff == 0 )
             break;
           R++;
         }
         return diff;
       } else if( *W == '?' ) {
         if( *R == '\0' )
           return diff;
       } else
         return diff;
     } else if( *W == '\0' )
       break;

     W++;
     R++;
   }

   return 0;
}
}  // namespace wildchar

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::to_string
//
// Purpose-
//       Create string from printf format arguments
//
//----------------------------------------------------------------------------
std::string                         // Resultant
   to_string(                       // Create string from printf arguments
     const char*       fmt,         // Format string
                       ...)         // PRINTF arguments
{
   va_list args;
   va_start(args, fmt);
     std::string result= to_stringv(fmt, args);
   va_end(args);

   return result;
}

std::string                         // Resultant
   to_stringv(                      // Create string from printf arguments
     const char*       fmt,         // Format string
     va_list           args)        // PRINTF arguments
{
   std::string         result;      // Resultant

   char autobuff[512];              // Working buffer
   char* buffer= autobuff;          // -> Buffer

   va_list copy;
   va_copy(copy, args);
   unsigned L= vsnprintf(buffer, sizeof(autobuff), fmt, copy);
   va_end(copy);

   if( L < sizeof(autobuff) )       // If the normal case
     result= std::string(buffer);
   else {
     buffer= new char[L+1];
     vsnprintf(buffer, L+1, fmt, args);
     result= std::string(buffer);
     delete [] buffer;
   }

   return result;
}

std::string                         // Resultant
   to_string(                       // Create string from std::thread::id
     const std::thread::id& id)     // The std::thread::id
{
   std::stringstream ss;
   ss << id;
   return ss.str();
}

std::string                         // Resultant string
   to_string(                       // Get id string
     volatile const std::thread::id& id) // For this id
{  return to_string(const_cast<const std::thread::id&>(id)); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::visify
//
// Purpose-
//       Change control characters in string to their C++ equivalents.
//
// Implementation notes-
//       That is, '\n' converted to "\\n" (or '\\', 'n'), etc
//
//----------------------------------------------------------------------------
std::string                         // The visual representation
   visify(                          // Get visual representation of
     const std::string&inp)         // This string
{
   std::stringstream   out;         // The output stream

   int M= inp.length();
   for(int i= 0; i<M; i++) {
     char C= inp[i];
     switch(C) {
       case '\a':
         out << "\\a";
         break;

       case '\b':
         out << "\\b";
         break;

       case '\f':
         out << "\\f";
         break;

       case '\n':
         out << "\\n";
         break;

       case '\r':
         out << "\\r";
         break;

       case '\t':
         out << "\\t";
         break;

       case '\v':
         out << "\\v";
         break;

       case '\"':
         out << "\\\"";
         break;

       case '\'':
         out << "\\\'";
         break;

       case '\?':
         out << "\\\?";
         break;

       case '\\':
         out << "\\\\";
         break;

       default:
         out << C;
         break;
     }
   }

   return out.str();
}
}  // namespace utility
}  // namespace _PUB_NAMESPACE
