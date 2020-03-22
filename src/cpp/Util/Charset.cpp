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
//       Charset.c
//
// Purpose-
//       Print out character set.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

//----------------------------------------------------------------------------
// Local variables
//----------------------------------------------------------------------------
static int             sw_all = 0;  // Non-zero to print special characters

//----------------------------------------------------------------------------
//
// Subroutine-
//       CPRINT
//
// Purpose-
//       Print one character
//
//----------------------------------------------------------------------------
static void
   cprint(
     int               c)           // Character to print
{
#if 0
   if      (c == '\\')              // If backslash
     printf("\\\\");                // Alias backslash
   else if (c == '\?')              // If question mark
     printf("\\\?");                // Alias question mark
   else if (c == '\'')              // If single quote
     printf("\\\'");                // Alias single quote
   else if (c == '\"')              // If double quote
     printf("\\\"");                // Alias double quote
   else
#endif
#if 0
        if (c == '\a')              // If alert
     printf("\\a");                 // Alias alert
   else if (c == '\b')              // If backspace
     printf("\\b");                 // Alias backspace
   else if (c == '\f')              // If formfeed
     printf("\\f");                 // Alias formfeed
   else if (c == '\n')              // If newline
     printf("\\n");                 // Alias newline
   else if (c == '\r')              // If carriage return
     printf("\\r");                 // Alias carriage return
   else if (c == '\t')              // If horizontal tab
     printf("\\t");                 // Alias horizontal tab
   else if (c == '\v')              // If vertical tab
     printf("\\v");                 // Alias vertical tab
   else if (c <  ' ')               // If some other special character
#endif
   if( c < ' ' || c > '~' )         // If a special character
     printf(" ");                   // Replace with space
   else                             // If default character
     printf("%c", c );              // Print the character
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Handle parameters
//
//----------------------------------------------------------------------------
static inline void
   parm(                            // Handle parameters
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   for(int i= 1; i<argc; i++)
   {
     if( strcmp("-all", argv[i]) == 0 || strcmp("--all", argv[i]) == 0 )
        sw_all = 1;
     else if( strcmp("-none", argv[i]) == 0 || strcmp("--none", argv[i]) == 0 )
        sw_all = 0;
     else
        fprintf(stderr, "Usage: %s {--help | --all | --none}\n", argv[0]);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       specials
//
// Purpose-
//       Display special characters
//
//----------------------------------------------------------------------------
static inline void
   specials( void )                 // The special characters
{
   // Hex Char Name
   printf("0x00 -- NUL (Null)\n");
   printf("0x01 -- SOH (Start of heading)\n");
   printf("0x02 -- STX (Start text)\n");
   printf("0x03 -- ETX (End text)\n");
   printf("0x04 -- EOT (End of transmission)\n");
   printf("0x05 -- ENQ (Enquiry)\n");
   printf("0x06 -- ACK (Acknowledge)\n");
   printf("0x07 \\a BEL (Bell)\n");
   printf("0x08 \\b BS  (Backspace)\n");
   printf("0x09 \\t TAB (Horizontal tab)\n");
   printf("0x0a \\n NL  (New line, LF Line feed)\n");
   printf("0x0b \\v VT  (Vertical tab)\n");
   printf("0x0c \\f FF  (Form feed)\n");
   printf("0x0d \\r CR  (Carriage return)\n");
   printf("0x0e -- SO  (Shift out)\n");
   printf("0x0f -- SI  (Shift in)\n");
   printf("\n");
   printf("0x10 -- DLE (Data link escape)\n");
   printf("0x11 -- DC1 (Device control 1)\n");
   printf("0x12 -- DC2 (Device control 2)\n");
   printf("0x13 -- DC3 (Device control 3)\n");
   printf("0x14 -- DC4 (Device control 4)\n");
   printf("0x15 -- NAK (Negative acknowledge)\n");
   printf("0x16 -- SYN (Synchronous idle)\n");
   printf("0x17 -- ETB (End transmission block)\n");
   printf("0x18 -- CAN (Cancel)\n");
   printf("0x19 -- EM  (End of media)\n");
   printf("0x1a -- SUB (Substitute)\n");
   printf("0x1b -- ESC (Escape)\n");
   printf("0x1c -- FS  (Field separator)\n");
   printf("0x1d -- GS  (Group separator)\n");
   printf("0x1e -- RS  (Record separator)\n");
   printf("0x1f -- US  (Unit separator)\n");
   printf("\n");
   printf("0x7f -- DEL (Delete)\n");
}

//----------------------------------------------------------------------------
//
// Segment-
//       main
//
// Function-
//       Mainline code.
//
//----------------------------------------------------------------------------
int
   main(
     int               argc,        // Argument count
     char*             argv[])      // Argument array
  {                                 // Subroutine entry
   int                 i, j;        // General loop indexes
   int                 c;           // Character of interest

   parm(argc, argv);                // Handle parameters

   for (i=0; i<128; i+=16)          // Print out character array
   {
     for (j=0; j<16; j+=4)          // Print out hex value
     {
       c= i+j;                      // Current word value
       printf ("%.2x%.2x%.2x%.2x ",c ,c+1 ,c+2 ,c+3);
     }

     printf (" *");
     for (j=0; j<16; j++)           // Print out character value
     {
       c= i+j;                      // Current word value
       cprint(c);
     }
     printf ("*\n");
   }

   if( sw_all )                     // If requested
     specials();                    // Display special characters

   return 0;
}

