//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       Base64Codec.cpp
//
// Purpose-
//       Instantiate Base64Codec Object.
//
// Last change date-
//       2007/01/01
//
/* Description, from RFC 2045:
RFC 2045                Internet Message Bodies            November 1996


                    Table 1: The Base64 Alphabet

     Value Encoding  Value Encoding  Value Encoding  Value Encoding
         0 A            17 R            34 i            51 z
         1 B            18 S            35 j            52 0
         2 C            19 T            36 k            53 1
         3 D            20 U            37 l            54 2
         4 E            21 V            38 m            55 3
         5 F            22 W            39 n            56 4
         6 G            23 X            40 o            57 5
         7 H            24 Y            41 p            58 6
         8 I            25 Z            42 q            59 7
         9 J            26 a            43 r            60 8
        10 K            27 b            44 s            61 9
        11 L            28 c            45 t            62 +
        12 M            29 d            46 u            63 /
        13 N            30 e            47 v
        14 O            31 f            48 w         (pad) =
        15 P            32 g            49 x
        16 Q            33 h            50 y

   The encoded output stream must be represented in lines of no more
   than 76 characters each.  All line breaks or other characters not
   found in Table 1 must be ignored by decoding software.  In base64
   data, characters other than those in Table 1, line breaks, and other
   white space probably indicate a transmission error, about which a
   warning message or even a message rejection might be appropriate
   under some circumstances.

   Special processing is performed if fewer than 24 bits are available
   at the end of the data being encoded.  A full encoding quantum is
   always completed at the end of a body.  When fewer than 24 input bits
   are available in an input group, zero bits are added (on the right)
   to form an integral number of 6-bit groups.  Padding at the end of
   the data is performed using the "=" character.  Since all base64
   input is an integral number of octets, only the following cases can
   arise: (1) the final quantum of encoding input is an integral
   multiple of 24 bits; here, the final unit of encoded output will be
   an integral multiple of 4 characters with no "=" padding, (2) the
   final quantum of encoding input is exactly 8 bits; here, the final
   unit of encoded output will be two characters followed by two "="
   padding characters, or (3) the final quantum of encoding input is
   exactly 16 bits; here, the final unit of encoded output will be three
   characters followed by one "=" padding character.

   Because it is used only for padding at the end of the data, the
   occurrence of any "=" characters may be taken as evidence that the
   end of the data has been reached (without truncation in transit).  No
   such assurance is possible, however, when the number of octets
   transmitted was a multiple of three and no "=" characters are
   present.

   Any characters outside of the base64 alphabet are to be ignored in
   base64-encoded data.

   Care must be taken to use the proper octets for line breaks if base64
   encoding is applied directly to text material that has not been
   converted to canonical form.  In particular, text line breaks must be
   converted into CRLF sequences prior to base64 encoding.  The
   important thing to note is that this may be done directly by the
   encoder rather than in a prior canonicalization step in some
   implementations.

   NOTE: There is no need to worry about quoting potential boundary
   delimiters within base64-encoded bodies within multipart entities
   because no hyphen characters are used in the base64 encoding.
*/
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Barrier.h>
#include <com/Reader.h>
#include <com/Writer.h>
#include "Base64Codec.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static char            ENCHAR[65]= "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz"
                                   "0123456789+/";
static int             ENCODE[64];


#define PAD_CHAR '='                // Pad character

static Barrier         barrier= BARRIER_INIT; // Barrier
static int             init= 0;       // Non-zero when initialized
static int             DECODE[256];   // Initialized using encode table
#define PAD_CERROR 0x7F             // decode[invalid]

//----------------------------------------------------------------------------
//
// Subroutine-
//       rdchar
//
// Purpose-
//       Read character from input line.
//
//----------------------------------------------------------------------------
static int                          // Resultant character
   rdchar(                          // Read line from file
     char*&            C)           // Input character
{
   int                 result;      // Resultant

   result= 0;                       // Default, NULL
   if( *C != '\0' )                 // If not end of line
   {
     result= *C;                    // Return the current character
     C++;                           // Update position
   }

   result &= 0x00ff;                // Insure positive value
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Base64Codec::~Base64Codec
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Base64Codec::~Base64Codec( void )// Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Base64Codec::Base64Codec
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   Base64Codec::Base64Codec( void ) // Default constructor
:  Codec()
{
   int                 i;

   if( init != 0 )
     return;

   AutoBarrier lock(barrier);       // Only one initializer allowed
   if( init == 0 )
   {
     init= 1;

     for(i= 0; i<ELEMENTS(DECODE); i++)
       DECODE[i]= PAD_CERROR;

     for(i=0; i<ELEMENTS(ENCODE); i++)
       ENCODE[i]= ENCHAR[i] & 0x00ff;

     for(i=0; i<ELEMENTS(ENCODE); i++)
       DECODE[(ENCODE[i]&0x00ff)]= i;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Base64Codec::decode
//
// Purpose-
//       Default (NULL) encoding.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Base64Codec::decode(             // Decode a file
     Reader&           inp,         // Input file
     Writer&           out)         // Output file
{
   char*               C;           // -> Buffer
   char                inpLine[128];// Input line
   unsigned            inpWord[4];  // Input buffer
   unsigned            outWord;     // Output word

   int                 rc;          // Called routine's return code

   //-------------------------------------------------------------------------
   // Load the first data line
   //-------------------------------------------------------------------------
   rc= inp.readLine(inpLine, sizeof(inpLine)); // Read a file line
   if( rc < 0 )                     // End of file or error
     return DC_NOH;

   C= inpLine;

   //-------------------------------------------------------------------------
   // Decode
   //-------------------------------------------------------------------------
   setEcode(EC_0);                  // Set the default error code
   for(;;)                          // Decode the data
   {
     inpWord[0]= rdchar(C);         // Next character sequence
     inpWord[1]= rdchar(C);
     inpWord[2]= rdchar(C);
     inpWord[3]= rdchar(C);

     if( inpWord[0] == '\0' )       // If end of line
     {
       rc= inp.readLine(inpLine, sizeof(inpLine)); // Read next file line
       if( rc < 0 )                 // If end of file or error
         break;

       C= inpLine;                  // Next line
       if( *C == PAD_CHAR )         // If end of valid data
         break;

       continue;
     }

     outWord=                  DECODE[inpWord[0]];
     outWord= (outWord << 6) | DECODE[inpWord[1]];
     outWord= (outWord << 6) | DECODE[inpWord[2]];
     outWord= (outWord << 6) | DECODE[inpWord[3]];
     #ifdef HCDM
       fprintf(stderr, "%.6x = {%c,%c,%c,%c}: ", outWord,
                       inpWord[0], inpWord[1], inpWord[2], inpWord[3]);
     #endif

     if( DECODE[inpWord[0]] > 0x003f
         ||DECODE[inpWord[1]] > 0x003f )
     {
       setEcode(DC_ICS);
       fprintf(stderr, "%s\n"
                       "Invalid code sequence [%c%c%c%c]\n", inpLine,
                       inpWord[0], inpWord[1], inpWord[2], inpWord[3]);
       break;
     }

     if( DECODE[inpWord[2]] > 0x003f
         ||DECODE[inpWord[3]] > 0x003f )
     {
       if( inpWord[3] != PAD_CHAR )
       {
         setEcode(DC_ICS);
         fprintf(stderr, "%s\n"
                         "Invalid code sequence [%c%c%c%c]\n", inpLine,
                         inpWord[0], inpWord[1], inpWord[2], inpWord[3]);
         break;
       }

       out.put(outWord >> 16);
       #ifdef HCDM
         fprintf(stderr, ".");
       #endif
       if( inpWord[2] != PAD_CHAR )
       {
         if( DECODE[inpWord[2]] > 0x003f )
         {
           setEcode(DC_ICS);
           fprintf(stderr, "%s\n"
                           "Invalid code sequence [%c%c%c%c]\n", inpLine,
                           inpWord[0], inpWord[1], inpWord[2], inpWord[3]);
           break;
         }
         out.put(outWord >> 8);
         #ifdef HCDM
           fprintf(stderr, ".");
         #endif
       }
       #ifdef HCDM
         fprintf(stderr, "\n");
       #endif

       break;
     }

     out.put(outWord >> 16 );
     out.put(outWord >>  8 );
     out.put(outWord       );
     #ifdef HCDM
       fprintf(stderr, "...\n");
     #endif
   }

   if( getEcode() != DC_OK )
     return RC_NG;

   return RC_OK;
}

//----------------------------------------------------------------------------
//
// Method-
//       Base64Codec::encode
//
// Purpose-
//       Default (NULL) decoding.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Base64Codec::encode(             // Encode a file
     Reader&           inp,         // Input file
     Writer&           out)         // Output file
{
   char                inpBuff[57]; // Input buffer
   char                outBuff[80]; // Output buffer
   unsigned            outWord;     // Output word

   int                 iX;          // Input index
   int                 oX;          // Output index
   int                 L;           // Number of bytes read

   //-------------------------------------------------------------------------
   // Encode
   //-------------------------------------------------------------------------
   setEcode(EC_0);                  // Set the default error code
   for(;;)                          // Encode the data
   {
     L= inp.read(inpBuff, sizeof(inpBuff));
     if( L <= 0 )                   // If error or end of file
     {
       if( L < 0 )                  // If error
       {
         setEcode(EC_RDR);
         printf("==== READ ERROR\n");
         perror("I/O error");
       }
       break;
     }

     oX= 0;
     for(iX=0; iX<=(L-3); iX+=3)
     {
       outWord  = (inpBuff[iX+0] & 0x00FF) << 16;
       outWord |= (inpBuff[iX+1] & 0x00FF) <<  8;
       outWord |= (inpBuff[iX+2] & 0x00FF);

       outBuff[oX++]= ENCODE[(outWord >> 18) & 0x003F];
       outBuff[oX++]= ENCODE[(outWord >> 12) & 0x003F];
       outBuff[oX++]= ENCODE[(outWord >>  6) & 0x003F];
       outBuff[oX++]= ENCODE[(outWord      ) & 0x003F];
       #if 0
         printf("{%.8X= I(%2d,%2d,%2d,%2d) [%c%c%c%c]}",
                 outWord,
                        (outWord >> 18) & 0x003F,
                        (outWord >> 12) & 0x003F,
                        (outWord >>  6) & 0x003F,
                        (outWord      ) & 0x003F,
                 ENCODE[(outWord >> 18) & 0x003F],
                 ENCODE[(outWord >> 12) & 0x003F],
                 ENCODE[(outWord >>  6) & 0x003F],
                 ENCODE[(outWord      ) & 0x003F]);
       #endif
     }
     #if 0
       printf("\n");
     #endif

     outWord  = (inpBuff[iX+0] & 0x00FF) << 16;
     outWord |= (inpBuff[iX+1] & 0x00FF) <<  8;
     outWord |= (inpBuff[iX+2] & 0x00FF);
     if( iX < sizeof(inpBuff) )     // Short last line
     {
       switch(L-iX)                 // Number of bytes remaining to process
       {
         case 0:
           break;

         case 1:
           outBuff[oX++]= ENCODE[(outWord >> 18) & 0x003F];
           outBuff[oX++]= ENCODE[(outWord >> 12) & 0x003F];
           outBuff[oX++]= PAD_CHAR;
           outBuff[oX++]= PAD_CHAR;

           break;

         case 2:
           outBuff[oX++]= ENCODE[(outWord >> 18) & 0x003F];
           outBuff[oX++]= ENCODE[(outWord >> 12) & 0x003F];
           outBuff[oX++]= ENCODE[(outWord >>  6) & 0x003F];
           outBuff[oX++]= PAD_CHAR;
           break;

         default:                   // OK, I can't code
           fprintf(stderr, "%s %4d: bug(%d)\n", __FILE__, __LINE__, L-iX);
       }
     }

     outBuff[oX++]= '\0';
     out.printf("%s\n", outBuff);
   }

   if( getEcode() != EC_0 )
     return RC_NG;
   return RC_OK;
}

