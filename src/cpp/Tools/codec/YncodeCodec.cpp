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
//       YncodeCodec.cpp
//
// Purpose-
//       Instantiate yEnc Codec Object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/AutoPointer.h>
#include <com/Barrier.h>
#include <com/Debug.h>
#include <com/Reader.h>
#include <com/Writer.h>
#include "YncodeCodec.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define CODEA 42
#define CODEB 106

#define LINE_SIZE 128               // Nominal line size
#define BUFF_SIZE 8192              // Buffer size

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static Barrier         barrier= BARRIER_INIT; // Barrier
static int             init= 0;     // Non-zero when initialized

static int             ENCODE[256]; // Encoding table
static int             DECODE[256]; // Decoding table
static int             RECODE[256]; // Recoding table

//----------------------------------------------------------------------------
//
// Method-
//       YncodeCodec::~YncodeCodec
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   YncodeCodec::~YncodeCodec( void )// Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       YncodeCodec::YncodeCodec
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   YncodeCodec::YncodeCodec( void ) // Default constructor
:  Codec()
,  checksum()
,  size(0)
{
   unsigned            i;

   // Only initialize static areas once
   if( init != 0 )
     return;

   AutoBarrier lock(barrier);
   if( init == 0 )
   {
     for(i=0; i<ELEMENTS(ENCODE); i++)
       ENCODE[i]= (i + CODEA) & 0x00ff;

     for(i=0; i<ELEMENTS(DECODE); i++)
       DECODE[i]= (i - CODEA) & 0x00ff;

     memset(RECODE, 0, sizeof(RECODE));
     RECODE[(int)'\0']= 2;
     RECODE[(int)'\n']= 2;
     RECODE[(int)'\r']= 2;
     RECODE[(int)'=' ]= 2;
     RECODE[(int)'\t']= 1;
     RECODE[(int)' ' ]= 1;
     RECODE[(int)'.' ]= 1;

     init= 1;
     #ifdef HCDM
       tracef("ENCODE:\n");
       dump(ENCODE, sizeof(ENCODE));

       tracef("DECODE:\n");
       dump(DECODE, sizeof(DECODE));

       tracef("RECODE:\n");
       dump(RECODE, sizeof(RECODE));
     #endif
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       YncodeCodec::decode
//
// Purpose-
//       UU decoding.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   YncodeCodec::decode(             // Decode a file
     Reader&           inp,         // Input file
     Writer&           out)         // Output file
{
   int                 resultant;   // Resulant
   char*               buffer;      // Working buffer
   int                 C;           // Current character
   int                 L;           // Write length
   const char*         ptrC;        // Working message
   int                 swEscape;    // ESCAPE character

// int                 iX;          // Input index
   int                 oX;          // Output index

   //-------------------------------------------------------------------------
   // Decode
   //-------------------------------------------------------------------------
   setEcode(EC_0);                  // Set the default error code
   checksum.reset();
   size= 0;

   buffer= (char*)malloc(BUFF_SIZE); // Allocate buffer
   if( buffer == NULL )
   {
     perror("No storage");
     setEcode(EC_FAULT);
     return DC_ERR;
   }
   AutoPointer aptr(buffer);

   resultant= DC_OK;
   swEscape= FALSE;
// iX= oX= 0;
   oX= 0;
   for(;;)                          // Decode the data
   {
     if( oX >= BUFF_SIZE )
     {
       checksum.accumulate(buffer, oX);
       size += oX;
       L= out.write(buffer, oX);    // Write the data
       if( L != oX )
       {
         resultant= DC_ERR;
         setEcode(EC_WTR);
         perror("I/O error");
         break;
       }
       oX= 0;
     }

     C= inp.get();
     if( C < 0 )                    // If error or EOF
     {
       if( C == EC_EOF )
       {
         if( oX > 0 )
         {
           checksum.accumulate(buffer, oX);
           size += oX;
           L= out.write(buffer, oX); // Write the data
           if( L != oX )
           {
             setEcode(EC_WTR);
             perror("I/O error");
           }
         }
         break;
       }

       resultant= DC_ERR;
       setEcode(EC_RDR);
       out.printf("\n=yend ");
       out.printf("==== READ ERROR\n");
       perror("I/O error");
       break;
     }

     if( swEscape )
     {
       if( C == '\n' || C == '\r' || C == 'y' )
       {
         resultant= DC_ICS;

         ptrC= "y";
         if( C == '\n' )
           ptrC= "\\n";

         else if( C == '\r' )
           ptrC= "\\r";
         fprintf(stdout, "%4d: Sequence: '=%s'\n", __LINE__, ptrC);

         break;
       }

       C= (C - CODEB) & 0x00ff;
       swEscape= FALSE;
     }
     else if( C == '\n' || C == '\r' )
     {
//     iX= 0;
       continue;
     }
     else if( C == '=' )
     {
       swEscape= TRUE;
       continue;
     }
     else
       C= DECODE[C];

     buffer[oX++]= C;
   }

   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       YncodeCodec::encode
//
// Purpose-
//       yEnc encoding.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   YncodeCodec::encode(             // Encode a file
     Reader&           inp,         // Input file
     Writer&           out)         // Output file
{
   char*               buffer;      // Working buffer
   int                 C;           // Current character
   int                 L;           // Read length

   int                 iX;          // Input index
   int                 oX;          // Output index

   //-------------------------------------------------------------------------
   // Encode
   //-------------------------------------------------------------------------
   setEcode(EC_0);                  // Set the default error code
   checksum.reset();
   size= 0;

   buffer= (char*)malloc(BUFF_SIZE); // Allocate buffer
   if( buffer == NULL )
   {
     setEcode(EC_FAULT);
     return RC_NG;
   }

   L= inp.read(buffer, BUFF_SIZE);
   if( L > 0 )
   {
     checksum.accumulate(buffer, L);
     size= L;
   }
   iX= 0;
   for(;;)                          // Encode the data
   {
     if( L <= 0 )                   // If error or end of file
     {
       if( L < 0 )                  // If error
       {
         setEcode(EC_RDR);
         out.printf("\n=yend ");
         out.printf("==== READ ERROR\n");
         perror("I/O error");
       }
       break;
     }

     for(oX= 0; oX<LINE_SIZE; oX++)
     {
       if( iX >= L )
       {
         L= inp.read(buffer, BUFF_SIZE);
         iX= 0;
         if( L <= 0 )
           break;

         checksum.accumulate(buffer, L);
         size += L;
       }

       C= buffer[iX] & 0x00ff;
       C= ENCODE[C];
       if( RECODE[C] != 0 )
       {
         if( RECODE[C] == 2 || oX == 0 || oX >= (LINE_SIZE-1) )
         {
           out.put('=');
           oX++;
           C= (buffer[iX] + CODEB) & 0x00ff;
         }
       }
       out.put(C);
       iX++;
     }
     out.put('\n');
   }

   free(buffer);
   if( getEcode() != EC_0 )
     return RC_NG;
   return RC_OK;
}

//----------------------------------------------------------------------------
//
// Method-
//       YncodeCodec::getSize
//
// Purpose-
//       Get file size.
//
//----------------------------------------------------------------------------
uint64_t                            // The file size
   YncodeCodec::getSize( void ) const // Get file size
{
   return size;
}

//----------------------------------------------------------------------------
//
// Method-
//       YncodeCodec::getSum
//
// Purpose-
//       Get checksum.
//
//----------------------------------------------------------------------------
uint32_t                            // The checksum
   YncodeCodec::getSum( void ) const // Get checksum
{
   return checksum.getValue();
}

