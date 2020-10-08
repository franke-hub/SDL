//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       tlc_main.hpp
//
// Purpose-
//       TLC mainline. Runs TLC interpreter.
//
// Last change date-
//       2020/10/04
//
// Implementation notes-
//       Interpretation code is almost wholly C++.
//
//----------------------------------------------------------------------------
#include <pub/Console.h>

#include "tlc_base.hpp"             // Base functions
#include "tlc_code.hpp"             // Threaded code
#include "tlc_refs.hpp"             // Handle unused refs

//----------------------------------------------------------------------------
// Data areas
//----------------------------------------------------------------------------
static Data            line_used;   // Number of bytes used
static Data            line_size;   // Number of bytes read
static unsigned char   input[1024]; // Input data line

static Data            next_size;   // The next token size
static unsigned char   token[1024]; // Next token

//----------------------------------------------------------------------------
//
// Subroutine-
//       load_map
//
// Purpose-
//       Load the map
//
//----------------------------------------------------------------------------
static inline void
   load_map( void )                 // Load the map
{
   wordMap.insert("base",    BASE);
   wordMap.insert("abs",     TABS);
   wordMap.insert("+",       TADD);
   wordMap.insert("and",     TAND);
   wordMap.insert("1-",      TDEC);
   wordMap.insert("debug",   TDEBUG);
   wordMap.insert("ddump",   TDEBUG_DUMP);
   wordMap.insert("dthis",   TDEBUG_THIS);
   wordMap.insert("/",       TDIV);
   wordMap.insert("//",      TDIVR);
   wordMap.insert(".",       TDOT);
   wordMap.insert("dup",     TDUP);
   wordMap.insert("tget",    TGET);
   wordMap.insert("cr",      TH_CR);
   wordMap.insert("dec",     TH_DEC);
   wordMap.insert("hex",     TH_HEX);
   wordMap.insert("sp",      TH_SP);
   wordMap.insert("top",     TH_TOP);
   wordMap.insert("1+",      TINC);
   wordMap.insert("max",     TMAX);
   wordMap.insert("min",     TMIN);
   wordMap.insert("/mod",    TMOD);
   wordMap.insert("*",       TMUL);
   wordMap.insert("minus",   TNEG);
   wordMap.insert("nop",     TNOP);
   wordMap.insert("not",     TNOT);
   wordMap.insert("or",      TOR);
   wordMap.insert("echo",    TOUTC);
   wordMap.insert("outs",    TPUTS);
   wordMap.insert("over",    TOVER);
   wordMap.insert("_c",      TPEEKC);
   wordMap.insert("_",       TPEEKW);
   wordMap.insert("!c",      TPOKEC);
   wordMap.insert("!",       TPOKEW);
   wordMap.insert("pop",     TPOP);
   wordMap.insert("swap",    TSWAP);
   wordMap.insert("-",       TSUB);
   wordMap.insert("xor",     TXOR);

   // All supported aliases for quit
   wordMap.insert("bye",     TQUIT);
   wordMap.insert("end",     TQUIT);
   wordMap.insert("exit",    TQUIT);
   wordMap.insert("halt",    TQUIT);
   wordMap.insert("quit",    TQUIT);
   wordMap.insert("BYE",     TQUIT);
   wordMap.insert("END",     TQUIT);
   wordMap.insert("EXIT",    TQUIT);
   wordMap.insert("QUIT",    TQUIT);
   wordMap.insert("HALT",    TQUIT);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_value
//
// Purpose-
//       Get numeric value.
//
//----------------------------------------------------------------------------
static bool                         // TRUE IFF valid number
   get_value(                       // Get numeric value
     const unsigned char*
                       token,       // The input string
     Data&             value)       // The *OUTPUT* (and working) value
{
   Data                base= Data(BASE[1]); // Current numeric base
   int                 negate= false; // TRUE if negation required
   unsigned            X= 0;        // Current token index

   if( token[0] == '0' && toupper(token[1]) == 'X' && token[2] != '\0' )
   {
     base= 16;
     X= 2;
   } else if( token[0] == '-' && token[1] != '\0' ) {
     negate= true;
     X= 1;
   }

   // Compute value (unless invalid character found)
   Data result= 0;
   while( token[X] != '\0' )
   {
     int C= token[X++];
     int V;
     const char* P= strchr(cvttab, C);
     if( P != nullptr )
     {
       V= P - cvttab;
       if( V >= base )
         return false;
     } else {
       P= strchr(CVTTAB, C);
       if( P == nullptr )
         return false;
       V= P - CVTTAB;
     }

     if( V >= base )
       return false;

     result *= base;
     result += V;

     if( result < 0 )               // If overflow
       return false;
   }

   if( negate )
     result= -result;

   value= result;
   return true;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       handle_line
//
// Purpose-
//       Read and handle next input line
//
//----------------------------------------------------------------------------
static void
   handle_line( void )              // Read and handle next input line
{
   int                 C;           // Current input character

   line_used= line_size= 0;

   printf("\nTLC\n");
   if( pub::Console::gets((char*)input, sizeof(input)) == nullptr )
   {
     printf("EOF\n");
     operational= false;            // QUIT if EOF
     return;
   }

   line_size= strlen((char*)input);
   if( line_size > 0 && input[line_size-1] == '\n' )
     input[--line_size]= '\0';

   // Handle tokens one by one
   for(;;)                          // Handle tokens one by one
   {
     next_size= 0;
     for(;;)                        // Skip over blanks
     {
       if( line_used >= line_size )
         return;

       C= input[line_used++];       // Get next input character
       if( C != ' ' ) break;        // break if non-blank
     }

     while( C != ' ' )              // Copy token to token area
     {
       token[next_size++]= C;
       if( line_used >= line_size )
         break;

       C= input[line_used++];
     }
     token[next_size]= '\0';

     // Process the token
     // Implementation note: Symbols take precedence.
     // Otherwise, for example, "dec" interpreted as value in hex mode.
     Data value= 0;

     Word* word= wordMap.locate((char*)token);
     if( word != nullptr )          // If valid token
     {
       data.push(Data(word));
       CNEXT();
     } else if( get_value(token, value) ) // If numeric constant
     {
       data.push(value);
     } else
       printf("\nInvalid symbol(%s)\n", token);
   }
}
static Word TLOOP[] = {Word(handle_line)};

//----------------------------------------------------------------------------
// TH_LOOP: Operational program
//----------------------------------------------------------------------------
static Word TH_LOOP[]= {TLOOP, TGOTO, TH_LOOP}; // The main program loop

//----------------------------------------------------------------------------
// TH_MAIN: Initial startup
//----------------------------------------------------------------------------
static Word TH_MAIN[]=              // Initial program
{  DEF_SUB                          // {} // (MUST BE DEF_SUB)
,  TGOTO, TH_LOOP                   // {} // Run program loop
};

//============================================================================
// CC_MAIN: Startup code
//============================================================================
static void CC_MAIN( void ) {
   load_map();                      // Load the symbol table
   CRESET();                        // Reset the environment
   data.push(Data(TH_MAIN));        // Set program word
   CNEXT();                         // Run threaded mode
}
