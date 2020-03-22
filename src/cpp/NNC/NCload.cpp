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
//       NCload.cpp
//
// Purpose-
//       Neural Net Compiler: Statement loader.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <fcntl.h>
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
#define __SOURCE__       "NCLOAD  " // Source file, for debugging

//----------------------------------------------------------------------------
//
// Subroutine-
//       SKIPB
//
// Purpose-
//       Skip blanks.
//
//----------------------------------------------------------------------------
static int
   skipb(                           // Skip blanks
     NC_ifd*           ptrifd)      // Pointer to file descriptor
{
   int                 c;           // Current character

   for(;;)                          // Skip blanks
   {
     c= NC_rd(ptrifd);              // Read the next character
     if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
       break;
   }

   return(c);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       overflow
//
// Purpose-
//       Handle statement overflow.
//
//----------------------------------------------------------------------------
static int
   overflow(                        // Statement buffer overflow
     NC_ifd*           ptrifd)      // Pointer to file descriptor
{
   int                 c;           // Current character

   NCmess(NC_msg::ID_SynStmtTooLong, 0);  // Write error message

   for (;;)                         // Read to end of statement
   {
     c= NC_rd(ptrifd);              // Read the next character
     if (c == ';')                  // If statement delimiter
       break;
     if (c == EOF)                  // If end of file
       break;
   }

   NC_COM.stmtbuff[0]= '\0';        // Replace with null statement
   return(1);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       outchar
//
// Purpose-
//       Add character to statement buffer.
//
//----------------------------------------------------------------------------
static int
   outchar(                         // Add character to buffer
     int               stmtix,      // Output buffer index
     int               c)           // Output character
{
   NC_ifd*             ptrifd;      // Pointer to file descriptor

   if (stmtix == 0)                 // If this is the first character
                                    // for this statement
   {
     ptrifd= NC_COM.srcfile;        // Address the active IFD
     NC_COM.lineno= ptrifd->lineno; // Save statement origin
     NC_COM.column= ptrifd->column;
   }

   NC_COM.stmtbuff[stmtix]= c;      // Accept the character
   stmtix++;                        // Next character position
   if (stmtix >= NC_COM.max_stmt)   // If statement too large
     return(EOF);

   return(stmtix);                  // Return next position
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NCLOAD
//
// Purpose-
//       Load the next statement.
//
//----------------------------------------------------------------------------
extern int
   ncload(                          // Load the next statement
     NC_ifd*           ptrifd)      // Pointer to file descriptor
{
   int                 c;           // Current character
   int                 p;           // Previous character
   int                 q;           // String quote character
   int                 stmtix;      // Statement buffer index

   //-------------------------------------------------------------------------
   // Set the statement position
   //-------------------------------------------------------------------------
   NC_COM.lineno= ptrifd->lineno;   // Save statement origin
   NC_COM.column= ptrifd->column;

   NC_COM.stmtbuff[0]= '\0';        // Default, null statement
   stmtix= 0;

   //-------------------------------------------------------------------------
   // Fill the statement buffer
   //-------------------------------------------------------------------------
   c= skipb(ptrifd);                // Skip blanks
   for(;;)                          // Fill the statement buffer
   {
     if (c == EOF)                  // If end of file
     {
       if (stmtix == 0)             // If no statement
         return (EOF);              // Exit, end of file
       break;                       // Exit, end of file
     }

     if (c == ' ')                  // If blank
     {
       if (stmtix == 0              // If the accumulator is empty
           ||NC_COM.stmtbuff[stmtix-1] == ' ') // or the previous
                                    // character is a blank
       {
         c= skipb(ptrifd);          // Read the next non-blank
         continue;                  // (Ignore the blank)
       }
     }

     else if (c == '\n')            // If newline
     {
       if (stmtix != 0              // If the accumulator isn't empty
           &&NC_COM.stmtbuff[0] == '#') // if this is a control stmt
         break;

       c= NC_rd(ptrifd);            // Read the next character
       continue;                    // (Ignore the newline)
     }

     else if (c == '\r')            // If carriage return
     {
       c= NC_rd(ptrifd);            // Read the next character
       continue;                    // (Ignore the carriage return)
     }

     else if (c == '/')             // If divide sign
     {
       c= NC_rd(ptrifd);            // Get next character
       if (c == '/')                // If comment to end of line
       {
         for (;;)                   // Read to end of line
         {
           c= NC_rd(ptrifd);        // Read the next character
           if (c == '\n'            // If end of line
               ||c == EOF)          // or end of file
             break;                 // (EOF will still be active)
         }

         if (NC_COM.stmtbuff[0] == '#') // If control statement
           break;                   // End of statement encountered

         c= NC_rd(ptrifd);          // Read the next character
         continue;                  // Ignore the newline
       }

       if (c == '*')                // If spanning comment
       {
         p= '\0';                   // Previous character not '*'
         for (;;)                   // Read to end of comment
         {
           c= NC_rd(ptrifd);        // Read the next character
           if (c == EOF)            // If end of file
             break;                 // (EOF will still be active)

           if (p == '*' && c == '/')// If ending delimiter
             break;                 // Exit, end of comment
           p= c;                    // Save previous character
         }
         c= NC_rd(ptrifd);          // Read the next character
         continue;                  // Ignore the comment
       }

       stmtix= outchar(stmtix, '/');// Accept the '/' character
       if (stmtix == EOF)           // If overflow
         return(overflow(ptrifd));  // Purge the buffer
     }

     //-----------------------------------------------------------------------
     // Accept the character
     //-----------------------------------------------------------------------
     stmtix= outchar(stmtix, c);    // Accept the character
     if (stmtix == EOF)             // If overflow
       return(overflow(ptrifd));    // Purge the buffer

     if (c == ';')                  // If statement delimiter
       break;

     if (c == '\\')                 // If backslash
     {
       c= NC_rd(ptrifd);            // Get the next character
       if (c == EOF)                // If end of file
       {
         NCmess(NC_msg::ID_SynGeneric, 0); // Write error message
         break;
       }
       if (c == '\n'                // If Carriage return
           ||c == '\r')             // or line feed
       {
         NCmess(NC_msg::ID_SynGeneric, 0); // Write error message
         break;
       }

       stmtix= outchar(stmtix, '\\'); // Accept the '\\' character
       if (stmtix == EOF)           // If overflow
         return(overflow(ptrifd));  // Purge the buffer

       stmtix= outchar(stmtix, c);  // Accept the character
       if (stmtix == EOF)           // If overflow
         return(overflow(ptrifd));  // Purge the buffer

       c= NC_rd(ptrifd);            // Get the next character
       continue;                    // Continue, using new character
     }

     if (c == '\''                  // If single quote
         ||c == '\"')               // or double quote
     {
       q= c;                        // Save the quote type
       for(;;)                      // Extract the string
       {
         c= NC_rd(ptrifd);          // Get the next character
         if (c == EOF)              // If end of file
         {
           NCmess(NC_msg::ID_SynStringEnd, 0); // Write error message
           break;                   // Break, EOF still active
         }
         if (c == '\n' || c == '\r')// If CR or LF
         {
           NCmess(NC_msg::ID_SynStringEnd, 0); // Write warning message
           if (NC_COM.stmtbuff[0] == '#') // if this is a control statement
           {
             NCmess(NC_msg::ID_SynGeneric, 0); // Write error message
             goto accept_statement;
           }
           continue;                // Ignore the character
         }

         stmtix= outchar(stmtix, c);// Accept the character
         if (stmtix == EOF)         // If overflow
           return(overflow(ptrifd));// Purge the buffer

         if (c == q)                // If ending delimiter
           break;                   // Exit, end of string
       }

       c= NC_rd(ptrifd);            // Get the next character
       continue;                    // Continue, using new character
     }

     c= NC_rd(ptrifd);              // Read the next character
   }

accept_statement:
   NC_COM.stmtbuff[stmtix]= '\0';   // Set ending delimiter
   return(stmtix);
}

