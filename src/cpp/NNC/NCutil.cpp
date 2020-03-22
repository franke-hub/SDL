//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       NCutil.cpp
//
// Purpose-
//       Neural Net Compiler: Utility functions.
//
// Last change date-
//       2007/01/01
//
// Entry points-
//       ncnextw    Extract next word from buffer
//       ncskipb    Skip over blanks
//       ncstring   Extract string from buffer
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <com/Debug.h>
#include <com/define.h>
#include <com/syslib.h>

#include "NC_com.h"
#include "NC_sys.h"
#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NCUTIL  " // Source file, for debugging

//----------------------------------------------------------------------------
//
// Subroutine-
//       ncnextw
//
// Purpose-
//       Extract next word from buffer.
//
// Returns-
//       Updated buffer index.
//
//----------------------------------------------------------------------------
extern int                          // Updated buffer index
   ncnextw(                         // Extract next word
     const char*       inpbuf,      // Current buffer
     int               inpndx,      // Current buffer index
     char*             waccum)      // Word accumulator, 256 bytes
{
   int                 c;           // Current character
   int                 stmtix;      // Current statement index
   int                 wordix;      // Current word accumulator ix

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   stmtix= ncskipb(inpbuf, inpndx); // Skip over blanks
   wordix= 0;                       // Initialize accumulator index

   //-------------------------------------------------------------------------
   // Extract the first character
   //-------------------------------------------------------------------------
   c= inpbuf[stmtix];               // Get the next character
   if( (! isascii(c)) || (! isalpha(c)) ) // If first character is not alphabetic
   {
     waccum[0]= '\0';               // Null accumulator word
     return(stmtix);
   }

   waccum[wordix++]= c;             // Set the first character

   //-------------------------------------------------------------------------
   // Extract the remainder of the word
   //-------------------------------------------------------------------------
   for(;;)                          // Extract the rest of the word
   {
     c= inpbuf[++stmtix];           // Get the next character
     if( (! isalnum(c) )            // If not alphanumeric
         &&c != '_')                // and not underscore
       break;                       // Break, delimiter found

     waccum[wordix++]= c;           // Save the character
     if( wordix == 256 )            // If size limit exceeded
     {
       waccum[255]= '\0';           // Limit the word size
       NCmess(NC_msg::ID_SynWordTooLong, 1, waccum); // Error message

       for(;;)                      // Extract the rest of the word
       {
         c= inpbuf[++stmtix];       // Get the next character
         if( (! isalnum(c) )        // If not alphanumeric
             &&c != '_')            // and not underscore
           break;                   // Break, delimiter found
       }

       return(stmtix);
     }
   }

   //-------------------------------------------------------------------------
   // Delimit the word
   //-------------------------------------------------------------------------
   waccum[wordix]= '\0';            // Set the word delimiter
   return(stmtix);                  // Stmtix points past the word
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ncskipb
//
// Purpose-
//       Skip over blanks.
//
// Returns-
//       Updated buffer index.
//
//----------------------------------------------------------------------------
extern int                          // Updated buffer index
   ncskipb(                         // Skip over blanks
     const char*       inpbuf,      // Current buffer
     int               stmtix)      // Current buffer index
{
   int                 c;           // Current character

   //-------------------------------------------------------------------------
   // Skip blanks
   //-------------------------------------------------------------------------
   for(;;)                          // Skip blanks
   {
     c= inpbuf[stmtix];             // Get the next character
     if( c != ' ' && c != '\t' )
       break;

     stmtix++;                      // Skip the blank
     break;
   }

   return(stmtix);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ncstring
//
// Purpose-
//       Extract a string.
//
// Return codes-
//       > 0        (Updated input statement index)
//                  (Statement index points past closing ']')
//       ERR_SYNTAX (Invalid syntax, including end-of-string)
//       ERR_LENGTH (Output field length exceeded)
//                  (NO error message is generated)
//
// Notes-
//       Strings begin with a '(' character and end with a ')'.
//       Quoted strings begin with '(', '\"' and end with '\"', ')'.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   ncstring(                        // Extract a string
     const char*       inpbuf,      // Input statement buffer
     int               inpndx,      // Input statement index
     char*             s,           // Output string buffer
     int               l)           // Output string length
{
   int                 i;           // General index variable
   int                 stmtix;      // Current statement index

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   stmtix= ncskipb(inpbuf, inpndx); // Skip over blanks
   if( inpbuf[stmtix] != '(' )      // If invalid delimiter
   {
     NCmess(NC_msg::ID_SynGeneric, 0); // Syntax error
     return(ERR);
   }

   stmtix++;                        // Skip over the '[' character
   stmtix= ncskipb(inpbuf, stmtix); // Skip over blanks
   i= 0;                            // Initialize output index

   //-------------------------------------------------------------------------
   // Process quoted string
   //-------------------------------------------------------------------------
   if( inpbuf[stmtix] == '\"' )     // If quoted string
   {
     stmtix++;                      // Skip over the quote

     while( inpbuf[stmtix] != '\"' )// Extract the string
     {
       if( i >= l )                 // If string too large
       {
         s[l-1]= '\0';              // Delimit the string
         return(ERR_LENGTH);        // String too large
       }

       if( inpbuf[stmtix] == '\0' ) // If end of string
       {
         NCmess(NC_msg::ID_SynGeneric, 0); // Syntax error
         return(ERR);
       }

       if( inpbuf[stmtix] == '\\' ) // If special delimiter
       {
         stmtix++;                  // Skip over the delimiter
         switch(inpbuf[stmtix])     // Special character
         {
           case '\0':               // End of string (bad)
             NCmess(NC_msg::ID_SynGeneric, 0); // Syntax error
             return(ERR);

           case 'a':                // Alarm
             s[i]= '\a';
             break;

           case 'n':                // New line
             s[i]= '\n';
             break;

           case 'r':                // Carriage return
             s[i]= '\r';
             break;

           case 't':                // Tab
             s[i]= '\r';
             break;

           case '\'':               // Embedded single quote
           case '\"':               // Embedded double quote
           default:                 // Standard character
             s[i]= inpbuf[stmtix];
             break;
         }
       }

       else                         // If standard character
         s[i]= inpbuf[stmtix];

       i++;                         // Next output character
       stmtix++;                    // Next input  character
     }

     s[i]= '\0';                    // Delimit the output string
     stmtix++;                      // Skip over the quote
     stmtix= ncskipb(inpbuf, stmtix); // Skip over any blanks
     if( inpbuf[stmtix] != ')' )    // If invalid sequence
     {
       NCmess(NC_msg::ID_SynGeneric, 0); // Syntax error
       return(ERR);
     }

     stmtix++;                      // Skip over the ']' character
     return(stmtix);                // Return, function complete
   }

   //-------------------------------------------------------------------------
   // Process unquoted string
   //-------------------------------------------------------------------------
   while( inpbuf[stmtix] != ')' )   // Extract the string
   {
     if( i >= l )                   // If string too large
     {
       s[l-1]= '\0';                // Delimit the string
       return(ERR_LENGTH);          // String too large
     }

     switch(inpbuf[stmtix])         // Special character scan
     {
       case '\0':                   // End of string (bad)
       case '\\':                   // Escape (backslash)
       case '\'':                   // Embedded single quote
       case '\"':                   // Embedded double quote
         NCmess(NC_msg::ID_SynGeneric, 0); // Syntax error
         return(ERR);

       default:                     // Standard character
         s[i]= inpbuf[stmtix];
         break;
     }

     i++;                           // Next output character
     stmtix++;                      // Next input  character
   }

   s[i]= '\0';                      // Delimit the output string
   stmtix++;                        // Skip over the ')' character
   return(stmtix);                  // Return, function complete
}

