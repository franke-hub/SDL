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
//       UuCodeCodec.cpp
//
// Purpose-
//       Instantiate UU Codec Object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Barrier.h>
#include <com/Reader.h>
#include <com/Writer.h>
#include "UuCodeCodec.h"

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
static int             ENCODE[65];

#define PAD_CHAR '='                // Pad character

static Barrier         barrier= BARRIER_INIT; // Barrier
static int             init= 0;     // Non-zero when initialized
static int             DECODE[256]; // Initialized using encode table
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
//       UuCodeCodec::~UuCodeCodec
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   UuCodeCodec::~UuCodeCodec( void )// Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       UuCodeCodec::UuCodeCodec
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   UuCodeCodec::UuCodeCodec( void ) // Default constructor
:  Codec()
{
   unsigned            i;

   if( init != 0 )
     return;

   AutoBarrier lock(barrier);       // Only one initializer allowed
   if( init == 0 )
   {
     init= 1;

     memset(ENCODE, 0, sizeof(ENCODE));
     for(i=0; i<ELEMENTS(ENCODE)-1; i++)
       ENCODE[i]= ' '+i;
     ENCODE[0]= ' '+ELEMENTS(ENCODE)-1;

     for(i=0; i<ELEMENTS(DECODE); i++)
       DECODE[i]= PAD_CERROR;

     for(i=0; i<ELEMENTS(ENCODE)-1; i++)
       DECODE[ENCODE[i]]= i;
     DECODE[(int)' ']= 0;
     DECODE[0]= 0;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       UuCodeCodec::decode
//
// Purpose-
//       UU decoding.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   UuCodeCodec::decode(             // Decode a file
     Reader&           inp,         // Input file
     Writer&           out)         // Output file
{
   char*               C;           // -> Buffer
   char                inpLine[128];// Input line
   int                 inpWord[4];  // Input buffer
   char                outLine[sizeof(inpLine)]; // Output buffer

   int                 rc;          // Called routine's return code
   int                 i, m;

   //-------------------------------------------------------------------------
   // Decode
   //-------------------------------------------------------------------------
   setEcode(EC_0);                  // Set the default error code
   for(;;)                          // Decode the data
   {
     rc= inp.readLine(inpLine, sizeof(inpLine)); // Read a file line
     if( rc < 0 )                   // End of file or error
     {
       if( rc == Reader::RC_EOF )
         break;

       return DC_ICS;
     }

     C= inpLine;
     m= DECODE[(int)*C & 0x00ff];
     if( m == 0 )
       break;
     if( m == PAD_CERROR )
     if( rc < 0 )                   // End of file or error
     {
       #ifdef HCDM
         fprintf(stderr, "%4d: ERR 0 '%s'\n", __LINE__, inpLine);
       #endif
       return DC_ICS;
     }

     C++;
     for(i= 0; i<m; i+=3)
     {
       inpWord[0]= rdchar(C);       // Next character sequence
       inpWord[1]= rdchar(C);
       inpWord[2]= rdchar(C);
       inpWord[3]= rdchar(C);
       if( DECODE[inpWord[0]] == PAD_CERROR
           || DECODE[inpWord[1]] == PAD_CERROR
           || DECODE[inpWord[2]] == PAD_CERROR
           || DECODE[inpWord[3]] == PAD_CERROR )
       {
         #ifdef HCDM
           fprintf(stderr, "%4d: ERR %d/%d '%s'\n", __LINE__, i, m, inpLine);
           fprintf(stderr, "'%c'=>%d  ", inpWord[0], DECODE[inpWord[0]]);
           fprintf(stderr, "'%c'=>%d  ", inpWord[1], DECODE[inpWord[1]]);
           fprintf(stderr, "'%c'=>%d  ", inpWord[2], DECODE[inpWord[2]]);
           fprintf(stderr, "'%c'=>%d\n", inpWord[3], DECODE[inpWord[3]]);
         #endif
         return DC_ICS;
       }

       outLine[i+0]= (DECODE[inpWord[0]] << 2)
                   | (DECODE[inpWord[1]] >> 4);
       outLine[i+1]= (DECODE[inpWord[1]] << 4)
                   | (DECODE[inpWord[2]] >> 2);
       outLine[i+2]= (DECODE[inpWord[2]] << 6)
                   | (DECODE[inpWord[3]] >> 0);
     }

     out.write(outLine, m);
   }

   return DC_OK;
}

//----------------------------------------------------------------------------
//
// Method-
//       UuCodeCodec::encode
//
// Purpose-
//       UU encoding.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   UuCodeCodec::encode(             // Encode a file
     Reader&           inp,         // Input file
     Writer&           out)         // Output file
{
   char                inpBuff[45]; // Input buffer
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
         out.printf("==== READ ERROR\n");
         perror("I/O error");
       }
       break;
     }

     if( size_t(L) < sizeof(inpBuff) )
       memset(&inpBuff[L], 0, sizeof(inpBuff)-L);
     oX= 0;
     outBuff[oX++]= ENCODE[L];
     for(iX=0; iX<L; iX+=3)
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
//// printf("\n");

     outBuff[oX++]= '\0';
     out.printf("%s\n", outBuff);
   }

   if( getEcode() != EC_0 )
     return RC_NG;
   return RC_OK;
}

