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
//       Crypto.i
//
// Purpose-
//       Simple file encryption/decryption routine.
//
// Last change date-
//       2007/01/01                 Version 2, Release 1
//
// Parameters-
//       Argc     = Argument count
//       Argv[1]  = Filename to be encrypted/decrytped
//       Argv[2]  = Output file
//       Argv[3]  = File to be used for key
//
//----------------------------------------------------------------------------
#include "ocrw.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

#include <com/Debug.h>
#include <com/define.h>             // TRUE, FALSE, NULL

#include "Crypto.h"
#include "Stack.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define ctlfile             argv[3] // Control file name
#define inpfile             argv[1] // Input file name
#define outfile             argv[2] // Output file name

//----------------------------------------------------------------------------
// Enumerations and typedefs
//----------------------------------------------------------------------------
struct Tcw                          // Translation control words
{
     Word              curr;        // Current control word
     Word              prev;        // Previous control word, mashed
};                                  // Translation control words

//----------------------------------------------------------------------------
// External references
//----------------------------------------------------------------------------
static Random&         RNG= Random::standard; // Our random number generator

//----------------------------------------------------------------------------
// I/O control area
//----------------------------------------------------------------------------
static int             ctlh;        // Control file handle
static int             ctllen;      // Current size of buffer
static int             ctlndx;      // Current index for buffer
static char*           ctlname;     // Pointer to file name
#ifdef USE_MALLOC_BUFFER
static unsigned char*  ctlfree;     // -> Allocated I/O buffer
static unsigned char*  ctlbuf;      // -> I/O buffer
#else
static unsigned char   ctlbuf[CTLSIZE]; // The I/O buffer itself
#endif
static int             ctlEOF;      // End of file encountered once?
static int             ctlREW;      // Extra buffer read?

static Tcw             x;           // Translation control words
static Word            llast;       // Length of the last word
static int             inpeof;      // (see EOF_x for states)
#define EOF_0                     0 // EOF has not been encountered
#define EOF_1                     1 // EOF has been encountered
#define EOF_2                     2 // EOF has been acknowledged

static int             inph;        // Input file handle
static int             inplen;      // Current size of buffer
static int             inpndx;      // Current index for buffer
static char*           inpname;     // Pointer to file name
#ifdef USE_MALLOC_BUFFER
static unsigned char*  inpfree;     // -> Allocated I/O buffer
static unsigned char*  inpbuf;      // -> I/O buffer
#else
static unsigned char   inpbuf[INPSIZE]; // The I/O buffer itself
#endif

static int             outh;        // Output file handle
static int             outlen;      // Current size of buffer
static int             outndx;      // Current index for buffer
static char*           outname;     // Pointer to file name
#ifdef USE_MALLOC_BUFFER
static unsigned char*  outfree;     // -> Allocated I/O buffer
static unsigned char*  outbuf;      // -> I/O buffer
#else
static unsigned char   outbuf[INPSIZE]; // The I/O buffer itself
#endif

//----------------------------------------------------------------------------
// Inline routines
//----------------------------------------------------------------------------
#include "Stack.i"

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Function-
//       Initialization.
//
//----------------------------------------------------------------------------
static void
   init( void )                     // Initialize
{
   ctlh= inph= outh= (-1);

#ifdef USE_MALLOC_BUFFER
   ctlfree= (unsigned char*)malloc(CTLSIZE+4096);
   inpfree= (unsigned char*)malloc(INPSIZE+4096);
   outfree= (unsigned char*)malloc(OUTSIZE+4096);
   if( ctlfree == NULL || inpfree == NULL || outfree == NULL )
   {
     fprintf(stderr, "No storage\n");
     perror("malloc");
     exit(EXIT_FAILURE);
   }

   // Align on page boundary
   ctlbuf= (unsigned char*)(((long)ctlfree+4095)&(~4095));
   inpbuf= (unsigned char*)(((long)inpfree+4095)&(~4095));
   outbuf= (unsigned char*)(((long)outfree+4095)&(~4095));
#endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Function-
//       Termination cleanup.
//
//----------------------------------------------------------------------------
static int                          // Resultant, 0 OK
   term( void )                     // Terminate
{
   int                 result= (-1);// Resultant

#ifdef USE_MALLOC_BUFFER
   if( outfree != NULL )
   {
     free(outfree);
     outfree= NULL;
     outbuf= NULL;
   }

   if( inpfree != NULL )
   {
     free(inpfree);
     inpfree= NULL;
     inpbuf= NULL;
   }

   if( ctlfree != NULL )
   {
     free(ctlfree);
     ctlfree= NULL;
     ctlbuf= NULL;
   }
#endif

   if( outh >= 0 )
   {
     result= close(outh);           // Close the output file
     if( result != 0 )              // If close failure
     {
       fprintf(stderr, "File(%s): ", outname);
       perror("close error");
     }
     outh= (-1);
   }

   if( inph >= 0 )
   {
     close(inph);
     inph= (-1);
   }

   if( ctlh >= 0 )
   {
     close(ctlh);
     ctlh= (-1);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       frNetFormat
//
// Purpose-
//       Convert an integer from network format into host format.
//
//----------------------------------------------------------------------------
static INLINE Word                  // Resultant word, host format
   frNetFormat(                     // Convert from network format
     Word              source)      // Source word, network format
{
   Word                result;      // Resultant
   WC                  wc;          // Conversion word

   size_t              i;

   result= 0;
   wc.w= source;
   for(i= 0; i<sizeof(Word); i++)
   {
     result <<= BITS_PER_BYTE;
     result |= wc.c[i];
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       toNetFormat
//
// Purpose-
//       Convert an integer from host format into network format.
//
//----------------------------------------------------------------------------
#if 0  // UNUSED
static INLINE Word                  // Resultant word, network format
   toNetFormat(                     // Convert into network format
     Word              source)      // Source word, host format
{
   WC                  wc;          // Conversion word

   int                 i, j;

   for(i= 0; i<sizeof(Word); i++)
   {
     j= sizeof(Word) - i - 1;
     wc.c[i]= (source >> (j * BITS_PER_BYTE));
   }

   return wc.w;
}
#endif // UNUSED

//----------------------------------------------------------------------------
//
// Subroutine-
//      random
//
// Function-
//      Extract a random value.
//
//----------------------------------------------------------------------------
static INLINE Word                  // Resultant
   random(                          // Get a random value
     int               modulus)     // Maximum value
{
   return RNG.get() % modulus;      // Return random value
}

//----------------------------------------------------------------------------
//
// Subroutine-
//      randomize
//
// Function-
//      Set the random number seed to a time-dependent random number.
//
//----------------------------------------------------------------------------
static INLINE void
   randomize( void )                // Initialize the randomizer
{
   RNG.randomize();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//      parmwd
//
// Function-
//      Extract parameter word value.
//
//----------------------------------------------------------------------------
static Word                         // Resultant
   parmwd(                          // Parameter word value
     char*             param)       // -> String
{
   Word                resultant= 0;// The parameter word resultant

   while( *param )                  // Process each character
   {
     resultant= lrot(resultant, 6); // Position the resultant
     resultant += *(signed char*)param; // Add the character value
     param++;                       // Address next character
   }

   return resultant;                // Return, function complete
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ctlChar
//
// Function-
//       Get next control file character, with wrap.
//
//----------------------------------------------------------------------------
static INLINE int                   // The next control file character
   ctlChar( void )                  // Read next control file byte
{
   int                 L;           // Read length

   if( ctlndx >= ctllen )           // If the buffer is empty
   {
     if( !ctlEOF || ctlREW )        // If read required
     {
       L= read(ctlh, ctlbuf, CTLSIZE); // Read the new buffer
       if( L > 0 )                  // If not end of file
       {
         ctlREW= TRUE;
         ctllen= L;
       }
       else                         // If end of file
       {
         if( L < 0 )                // If cannot read file
         {
           fprintf(stderr, "File(%s): ", ctlname);
           perror("read error");
           term();
           exit(1);
         }
         ctlEOF= TRUE;              // EOF encountered
         if( ctlREW )               // If rewind required
         {
           if( lseek(ctlh, 0L, SEEK_SET) != 0L ) // If cannot rewind
           {
             fprintf(stderr, "File(%s): ", ctlname);
             perror("lseek error");
             term();
             exit(1);
           }

           ctllen= read(ctlh, ctlbuf, CTLSIZE); // Read the new buffer
           if( ctllen < CTLMINS )   // If cannot read file
           {
             fprintf(stderr, "File(%s): ", ctlname);
             perror("reread error");
             term();
             exit(1);
           }
         }
       }
     }

     ctlndx= 0;                     // Now at top of buffer
   }

#if INSTRUMENT_CTLFILE              // If CTLFILE trace active
   printf("%c",ctlbuf[ctlndx]);     // Trace CTLFILE character
#endif                              // INSTRUMENT_CTLFILE
   return ctlbuf[ctlndx++];         // Exit, update buffer index
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ctlWord
//
// Function-
//       Get next control file word, with wrap.
//
//----------------------------------------------------------------------------
static INLINE Word                  // Resultant Word
   ctlWord( void )                  // Read the next control file word
{
   Word                result;      // Resultant
   size_t              i;

   result= 0;
   for(i= 0; i<sizeof(Word); i++)
   {
     result <<= BITS_PER_BYTE;
     result |= ctlChar();
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       inpChar
//
// Function-
//       Get next input file character.
//
//----------------------------------------------------------------------------
static INLINE int                   // Resultant
   inpChar( void )                  // Read next input file byte
{
   if( inpndx >= inplen )           // If the buffer is empty
   {
     inplen= read(inph, inpbuf, INPSIZE); // Read the new buffer
     if( inplen <= 0 )              // If end of file
     {
       if( inpeof == EOF_0 )        // If EOF has not been set yet
       {
         if(inplen < 0 )
         {
           fprintf(stderr, "File(%s): ", inpname);
           perror("read error");
           term();
           exit(1);
         }
         inpeof= EOF_1;             // Indicate end of file
       }
       return random(256);          // Return gibberish
     }

     inpndx= 0;                     // Now at top of buffer
   }

#if INSTRUMENT_INPFILE              // If INPFILE trace active
   printf("%c",inpbuf[inpndx]);     // Trace INPFILE character
#endif                              // INSTRUMENT_INPFILE
   return inpbuf[inpndx++];         // Exit, update buffer index
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       inpWord
//
// Function-
//       Get next input file word.  At EOF, stick in garbage.
//
//----------------------------------------------------------------------------
static INLINE Word                  // Resultant Word
   inpWord( void )                  // Read next input file word
{
   Word                result;      // Resultant
   size_t              i;           // General index variable

   result= 0;
   for(i= 0; i<sizeof(Word); i++)
   {
     result <<= BITS_PER_BYTE;
     result |= inpChar();

     if( inpeof == EOF_1 )          // If end of file encountered
                                    // for the first time
     {
       llast= i;                    // Set the valid character count
       inpeof= EOF_2;               // Indicate that the end of file
                                    // has been processed
     }
   }

   return result;
}

extern Word outline_inpWord( void ); // (Not very far) Forward reference
extern Word                         // Resultant Word
   outline_inpWord( void )          // Read next input file word
{
   return inpWord();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       openout
//
// Function-
//       Open the output file.
//
//----------------------------------------------------------------------------
static void
   openout( void )                  // Open the output file
{
   outh= open64(outname, O_RDONLY|O_BINARY, 0); // Open the output file (for input)
   if( outh >= 0 )                  // If we can open the file, it must exist
   {
     fprintf(stderr, "File(%s): Exists\n", outname);
     term();
     exit(EXIT_FAILURE);
   }

   outh= open64(outname,            // Open the output file
              O_WRONLY|O_BINARY|O_TRUNC|O_CREAT,// (in write-only binary mode)
              S_IREAD|S_IWRITE);    // (with full write access)
   if( outh < 0 )                   // If the open failed
   {
     fprintf(stderr, "File(%s): ", outname);
     perror("open error");
     term();
     exit(EXIT_FAILURE);
   }

   outndx= 0;                       // Now at top of buffer
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       wflush
//
// Function-
//       Flush the output buffer.
//
//----------------------------------------------------------------------------
static void
   wflush( void )                   // Flush the output buffer
{
   if( outndx == 0 )                // If the buffer is empty
     return;                        // Nothing to do

   outlen= write(outh, outbuf, outndx); // Write the current buffer
   if( outlen != outndx )           // If I/O error
   {
     fprintf(stderr, "File(%s): ", outname);
     perror("write error");
     term();
     exit(EXIT_FAILURE);
   }

   outndx= 0;                       // Now at top of buffer
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       WRITER
//
// Function-
//       Write next output file word.
//
//----------------------------------------------------------------------------
static INLINE void
   writer(                          // Write next output file word
     Word              oword)       // The word to be written
{
   size_t              i, j;

   if( OUTSIZE-outndx < 4 )         // If the buffer is full
     wflush();                      // Flush the output buffer

   for(i= 0; i<sizeof(Word); i++)
   {
     j= sizeof(Word) - i - 1;
     outbuf[outndx++]= (oword >> (j * BITS_PER_BYTE));
#if INSTRUMENT_OUTFILE              // If OUTFILE trace active
     printf("%c", outbuf[outndx-1]);// Trace OUTFILE character
#endif                              // INSTRUMENT_OUTFILE
   }
}

static void
   outline_writer(                  // Write next output file word
     Word              oword)       // The word to be written
{
   writer(oword);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       encipher
//
// Function-
//       Encrypt one word.
//
//----------------------------------------------------------------------------
static INLINE Word                  // The encrypted resultant
   encipher(                        // Encrypt one word
     Word              source)      // The source word
{
   Word                result;      // The encrypted resultant

   x.prev ^= lrot(x.prev, 5);       // Randomize previous value
   x.prev += x.curr;
   x.curr= ctlWord();               // Get next encryption key word
   result= ((source) + x.curr) + x.prev; // Encrypt the word
   x.prev += lrot((source), 27);    // Add plaintext randomizer

   return result;
}

extern Word outline_encipher(Word); // (Not very far) Forward reference
extern Word                         // The encrypted resultant
   outline_encipher(                // Encrypt one word
     Word              source)      // The source word
{
   return encipher(source);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       decipher
//
// Function-
//       Decrypt one word.
//
//----------------------------------------------------------------------------
static INLINE Word                  // The decrypted resultant
   decipher(                        // Decrypt one word
     Word              source)      // The source word
{
   Word                result;      // The encrypted resultant

   x.prev ^= lrot(x.prev, 5);       // Randomize previous value
   x.prev += x.curr;
   x.curr= ctlWord();               // Get next decryption key word
   result= ((source) - x.prev) - x.curr; // Decrypt the word
   x.prev += lrot((result), 27);    // Add plaintext randomizer

   return result;
}

extern Word outline_decipher(Word); // (Not very far) Forward reference
extern Word                         // The decrypted resultant
   outline_decipher(                // Decrypt one word
     Word              source)      // The source word
{
   return decipher(source);
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
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   Word                ctext;       // Encrypted (cipher) text
   Word                ptext;       // Unencrypted (plain) text word
   WC                  xword;       // Verification word

   int                 returncd;    // This routine's return code
   int                 i;           // General index variable

#if( ENCRYPT )                      // If encrypting
   Word                xlast;       // Encrypted value of llast

#else                               // If decrypting
   Stack<Word, 4>      xstack;      // Translation stack
   Word                vlast;       // Length verification word
#endif                              // ENCRYPT

   //-------------------------------------------------------------------------
   // Initialization
   //-------------------------------------------------------------------------
   init();                          // Initialize the file handles
   ctlname= ctlfile;                // Save the file names
   inpname= inpfile;
   outname= outfile;
   randomize();

   //-------------------------------------------------------------------------
   // Argument validation control
   //-------------------------------------------------------------------------
   if( argc < 4 )                   // If invalid parameter count
   {
     fprintf(stderr, "Use: %s from-file to-file control-file\n", argv[0]);
     return 1;
   }

   //-------------------------------------------------------------------------
   // Key initialization
   //-------------------------------------------------------------------------
   x.prev= 0;                       // Set last encryption key value
   x.curr= 0;                       // Set first encryption word
   if( argc > 4 )                   // If initial key specified
   {
     for(i=4; i<argc; i++)          // Set initial seed value
     {
       x.prev ^= lrot(x.prev, 5);   // Randomize previous value
       x.prev += parmwd(argv[i]);   // Add key value
       x.prev += lrot(x.prev, 27);  // Randomize previous value
     }

#if INSTRUMENT_KEYWORD              // If Key instrumentation
     fprintf(stderr, "Extended key:");
     for(i=4; i<argc; i++)
       fprintf(stderr, " '%s'", argv[i]);
     fprintf(stderr, "\n");
#endif                              // INSTRUMENT_KEYWORD

#if INSTRUMENT_KEYCODE              // If Key instrumentation
     fprintf(stderr, "Extended code: 0x%.8lX\n", (long)x.prev);
#endif                              // INSTRUMENT_KEYCODE
   }

   //-------------------------------------------------------------------------
   // File initialization
   //-------------------------------------------------------------------------
   ctlh= open64(ctlname, O_RDONLY|O_BINARY, 0); // Open the file
   if( ctlh < 0 )                   // If we cannot open the file
   {
     fprintf(stderr, "File(%s): ", ctlname);
     perror("open error");
     term();
     return 1;
   }

   ctlndx= 0;                       // Now at top of buffer
   ctlEOF= FALSE;                   // EOF not encountered
   ctlREW= FALSE;                   // REWIND not required
   ctllen= read(ctlh, ctlbuf, CTLSIZE); // Read from the file
   if( ctllen < 0 )                 // If the file is broken
   {
     fprintf(stderr, "File(%s): ", ctlname);
     perror("read error");
     term();
     return 1;
   }
   if( ctllen == 0 )                // If the file is empty
   {
     fprintf(stderr, "File(%s): Empty\n", ctlname);
     term();
     return 1;
   }
   if( ctllen <= CTLMINS )          // If the file is too small
   {
     fprintf(stderr, "File(%s): Too small\n", ctlname);
     term();
     return 1;
   }

   //-------------------------------------------------------------------------
   inph= open64(inpname, O_RDONLY|O_BINARY, 0); // Open the file
   if( inph < 0 )                   // If we cannot open the file
   {
     fprintf(stderr, "File(%s): ", inpname);
     perror("open error");
     term();
     return 1;
   }

   inpndx= 0;                       // Now at top of buffer
   inplen= read(inph, inpbuf, INPSIZE); // Read from the file
   if( inplen < 0 )                 // If the file is broken
   {
     fprintf(stderr, "File(%s): ", inpname);
     perror("read error");
     term();
     return 1;
   }
   if( inplen == 0 )                // If the file is empty
   {
     fprintf(stderr, "File(%s): Empty\n", inpname);
     term();
     return 1;
   }

   //-------------------------------------------------------------------------
   // Transform the input file
   //-------------------------------------------------------------------------
   inpeof= EOF_0;                   // Default, no end of file
   llast= 0;                        // Default, no last word length

   x.curr= ctlWord();               // Get first encryption key word

#if( ENCRYPT )                      // If encrypting
   openout();                       // Open the output file
   for(i=0; size_t(i)<sizeof(WC); i++) // Set verification word
     xword.c[i]= random(256);       // Fill with random characters

   ctext= outline_encipher(frNetFormat(xword.w)); // Encipher the word
   outline_writer(ctext);           // Write it out

#else                               // If decrypting
   xword.w= outline_decipher(outline_inpWord()); // Decrypt the verification word

   xstack.fifo(outline_inpWord());  // Prime the stack
   xstack.fifo(outline_inpWord());
   xstack.fifo(outline_inpWord());
   ctext= outline_inpWord();        // Prime the mini-stack

   if( inpeof != EOF_0 )            // If end of file already
   {
     fprintf(stderr, "File(%s): Too small\n", inpname);
     term();
     return 1;
   }

   openout();                       // Open the output file
#endif                              // ENCRYPT

   while( inpeof == EOF_0 )         // Transform the input file
   {
     // Encrypt/decrypt function
#if( ENCRYPT )                      // If encrypting
     ptext= inpWord();              // Get plain text word
     ctext= encipher(ptext);        // Encrypt a word
     writer(ctext);                 // Write the cipher text

#else                               // If decrypting
     ptext= decipher(xstack.pull());// Decrypt the next word
     writer(ptext);                 // Write the plain text
     xstack.fifo(ctext);            // Add at the end of the stack
     ctext= inpWord();              // Read the next word, or EOF
#endif                              // ENCRYPT
   }

   //-------------------------------------------------------------------------
   // Handle the file trailer
   //-------------------------------------------------------------------------
   returncd= 0;                     // Default, normal return code
#if( ENCRYPT )                      // If encrypting
   xlast= outline_encipher(llast);  // Encrypt the length word
   outline_writer(xlast);           // Write the length word
   ctext= outline_encipher(frNetFormat(xword.w)); // Encrypt the wrecksum word
   outline_writer(ctext);           // Write it out

#else                               // If decrypting
   ptext= outline_decipher(xstack.pull()); // Decrypt the last file word
   vlast= outline_decipher(xstack.pull()); // Decrypt the length word
   ctext= outline_decipher(xstack.pull()); // Decrypt the wrecksum word
   if( llast != 0                   // If invalid file length
       ||vlast > sizeof(Word)       // or invalid length word
       ||ctext != xword.w )         // or invalid wrecksum
   {
     returncd= 1;
     fprintf(stderr, "**WARNING** Invalid wrecksum\n");
     vlast= 0;
   }

   wflush();                        // Flush the buffer
   outline_writer(ptext);           // Write the last word
   outndx -= (sizeof(Word)-vlast);  // Drop the garbage characters
#endif                              // ENCRYPT

   wflush();                        // Flush the output buffer
   if( term() != 0 )                // Terminate
     returncd= 2;
   return returncd;
}

