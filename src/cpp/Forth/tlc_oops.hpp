//----------------------------------------------------------------------------
//
//       Copyright (c) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       tlc_oops.hpp
//
// Purpose-
//       TLC built-in threaded code.
//
// Last change date-
//       2019/01/01
//
// Implementation notes-
//       This is a *FAILED* attempt to duplicate the TLC assembler version.
//       While it currently completes, expected functionality not verified.
//       It is only minimally maintained, if that.
//
//----------------------------------------------------------------------------
#include "tlc_base.hpp"             // Base functions
#include "tlc_refs.hpp"             // Load references

//----------------------------------------------------------------------------
// FIXES
//----------------------------------------------------------------------------
#define IFDEBUG(X) { X }
#define ELDEBUG(X) {   }

//----------------------------------------------------------------------------
// Common data
//----------------------------------------------------------------------------
static Word            TCVFF[] = {DEF_CON, Word(-1)};
static Word            TCV00[] = {DEF_CON, Word( 0)};
static Word            TCV01[] = {DEF_CON, Word( 1)};
static Word            TCV02[] = {DEF_CON, Word( 2)};
static Word            TCV04[] = {DEF_CON, Word( 4)};
static Word            TCV08[] = {DEF_CON, Word( 8)};
static Word            TCV10[] = {DEF_CON, Word(10)};
static Word            TCV16[] = {DEF_CON, Word(16)};
static Word            TSIZE[] = {DEF_CON, Word(sizeof(void*))};

//----------------------------------------------------------------------------
// Common exits/subroutines
//----------------------------------------------------------------------------
static Word TH_EXIT0[]= {TCV00, TEXIT, TCV01, TEXIT};
static Word TH_EXIT1=   TH_EXIT0 + 2;
static Word TH_EXIT=    TH_EXIT0 + 1;

static Word TH_NG[]= {DEF_SUB, TPUTI, Word("NG\n"), TQUIT};
static Word TH_OK[]= {DEF_SUB, TPUTI, Word("OK\n"), TEXIT};

//----------------------------------------------------------------------------
// Data areas
//----------------------------------------------------------------------------
static char            _LINE[1024]; // Input data line (IMAGE + PREFIX)
static char            _OUTS[128];  // Numeric output string
static char            _TPAD[1024]; // Padding string

#if 1
static Word            LINE_ADDR[]= {DEF_CON, Word(_LINE)}; // The text
static Word            LINE_SIZE[]= {DEF_CON, Word(sizeof(_LINE))};

static Word            LINE_INPS[]= {DEF_VAR, Word(0)}; // Input size
static Word            LINE_USED[]= {DEF_VAR, Word(0)}; // Number used

static Word            LINE_INIT[]= // Initialize new line
{  DEF_SUB, TCV00, TDUP, LINE_INPS, TPOKEW, LINE_USED, TPOKEW, TEXIT};

static Word            LINE_GET[]=  // Get character from line
{  DEF_SUB, LINE_USED, TPEEKW, LINE_INPS, TPEEKW, TIFGE, TH_EXIT0 // If done
,  LINE_USED, TPEEKW, TDUP, TINC, LINE_USED, TPOKEW // Increment LINE_USED
,  LINE_ADDR, TADD, TPEEKC,  TEXIT}; // Fetch character

static Word            LINE_FULL[]= // Line full error condition
{  TPUTI, Word("\nLINE FULL\n"), TPOP, TEXIT};
static Word            LINE_PUT[]=  // Put character into line
{  DEF_SUB, LINE_INPS, TPEEKW, LINE_SIZE, TIFGE, LINE_FULL // If full
,  LINE_INPS, TPEEKW, TDUP, TINC, LINE_INPS, TPOKEW // Increment LINE_INPS
,  LINE_ADDR, TADD, TPOKEC, TEXIT}; // Store character

#else
// ASM defines LINES as:
//     IMAGEA0 tword  VAR
//             bute   0, 0
//     IMAGE   .space 512
static Word            IMAGEV0[]=   {DEF_SUB, IMAGEA0, TPEEKC, TEXIT};
static Word            IMAGES0[]=   {DEF_SUB, IMAGEA0, TPOKEC, TEXIT};
static Word            IMAGEV1[]=   {DEF_SUB, IMAGEA1, TPEEKC, TEXIT};
static Word            IMAGES1[]=   {DEF_SUB, IMAGEA1, TPOKEC, TEXIT};

static Word            IMAGE[]=     {DEF_CON, Word(_LINE+2)}; // (@IMAGE)
static Word            IMAGEA0[]=   {DEF_CON, Word(_LINE+0)}; // (@IMAGE-2)
static Word            IMAGEA1[]=   {DEF_CON, Word(_LINE+1)}; // (@IMAGE-1)
static Word            IMAGEA2[]=   {DEF_CON, Word(_LINE+2)}; // (@IMAGE-0)
#endif

// ASM defines OUTS as OUTS: tword VAR; .space 40
static Word            OUTS[]=      {DEF_CON, Word(_OUTS)};
static Word            OUTS_GETL[]= {DEF_SUB, OUTS, TPEEKC, TEXIT};
static Word            OUTS_SETL[]= {DEF_SUB, OUTS, TPOKEC, TEXIT};

// ASM defines TPAD as TPAD: tword VAR; .space 512
static Word            TPAD[]=      {DEF_CON, Word(_TPAD)};
static Word            PADV0[]=     {DEF_SUB, TPAD, TPEEKC, TEXIT};
static Word            PADS0[]=     {DEF_SUB, TPAD, TPOKEC, TEXIT};

//----------------------------------------------------------------------------
// Variables and accessors
//----------------------------------------------------------------------------
static Word            CFLAG[]= {DEF_VAR, Word(0)}; // TRUE iff compiling

//                     // TODO: RENAME TBASE=>VBASE
static Word            TBASE[]= {DEF_VAR, Word(10)}; // Numeric base, default 10
static Word            BASEGET[]= {DEF_SUB, TBASE, TPEEKW, TEXIT};
static Word            BASESET[]= {DEF_SUB, TBASE, TPOKEW, TEXIT};

//----------------------------------------------------------------------------
// TEMPORARY
//----------------------------------------------------------------------------
Word DATA_REFS[]=
{  DATA_REFS
,  BASEGET, BASESET
,  Word(OUTS)
};

//----------------------------------------------------------------------------
// CHELP: Bringup debugging
//----------------------------------------------------------------------------
static void CHELP(void) {
   debug_op("THELP");

   using pub::utility::dump;
   Data used= Data(LINE_USED[1]);
   debugf("_LINE %p %ld\n", _LINE, used); dump(stdout, _LINE, used);
   debugf("_TPAD\n"); dump(stdout, _TPAD, 64);
}
static Word THELP[] = {(Word)CHELP};

//----------------------------------------------------------------------------
//
// Thread-
//       TH_GNC
//
// Purpose-
//       Get next character from string
//
//----------------------------------------------------------------------------
static Word GNC_EX[]=               // (@STRING) (length)
{  TPOP                             // (@STRING)
,  TPOP                             // {}
,  TCV00                            // (0)
,  TEXIT                            // (0)
};

static Word TH_GNC[]=               // (@STRING) (length)
{  DEF_SUB                          // (@STRING) (length)
,  TDUP                             // (@STRING) (length) (length)
,  TIFLEZ, GNC_EX                   // (@STRING) (length)
,  TDEC                             // (@STRING) (length-1)
,  TSWAP                            // (length-1) (@STRING)
,  TINC                             // (length-1) (@STRING+1)
,  TSWAP                            // (@STRING+1) (length-1)
,  TOVER                            // (@STRING+1) (length-1) (@STRING+1)
,  TPEEKC                           // (@STRING+1) (length-1) (CHAR)
,  TCVFF                            // (@STRING+1) (length-1) (CHAR) (-1)
,  TEXIT                            // (@STRING+1) (length-1) (CHAR) (-1)
};

//----------------------------------------------------------------------------
//
// Thread-
//       TH_PUT
//
// Purpose-
//       Write an output line
//
//----------------------------------------------------------------------------
static Word TH_PUT[]=               // (@STRING)
{  DEF_SUB                          // (@STRING)
,  TDUP                             // (@STRING) (@STRING)
,  TPEEKC                           // (@STRING) (length)
// TNOP                             // ... FOR ALIGNMENT ...
};

static Word PUT_00[]=               // (@STRING) (length)
{  TH_GNC                           // (0) || (@STRING+1) (length-1) (CHAR) (-1)
,  TIFEQZ, TH_EXIT                  // (0) => TEXIT
,  TOUTC                            // (@STRING+1) (length-1) (CHAR)
,  TGOTO, PUT_00                    // (@STRING+1) (length-1)
,  nullptr
};

//----------------------------------------------------------------------------
//
// Thread-
//       TH_SUB
//
// Purpose-
//       Empty subroutine, used for copying
//
//----------------------------------------------------------------------------
static Word TH_SUB[]=
{  DEF_SUB                          // {}
// ==== INSERT COPY HERE =====================================================
,  TEXIT                            // {}
};

//----------------------------------------------------------------------------
//
// Thread-
//       TH_CR
//
// Purpose-
//       Write a carriage return
//
//----------------------------------------------------------------------------
static Word TH_CR[]=
{  DEF_SUB                          // {}
,  TIMMW, Word('\n')                // ('\n')
,  TOUTC                            // {}
,  TEXIT                            // {}
};

//----------------------------------------------------------------------------
//
// Thread-
//       TH_NXTC
//
// Purpose-
//       Get next character from input line
//
// Outputs-
//       (0) || (CHAR) (1)
//       LINE_USED (updated)
//
//----------------------------------------------------------------------------
static Word TH_NXTC[]=
{  DEF_SUB                          // {}
,  LINE_GET                         // (char)
,  TDUP, TIFNEZ, TH_EXIT1           // If (char) != '\0', EXIT TRUE
,  TEXIT                            // (0)
};

//----------------------------------------------------------------------------
//
// Thread-
//       TH_MXTL
//
// Purpose-
//       Initialize input line
//
// Output-
//       LINE_USED (line length)
//
//----------------------------------------------------------------------------
static Word TH_NXTL[]=
{  DEF_SUB                          // {}
,  LINE_INIT                        // {}
,  TPUTI, Word("\nTLC\n")           // {}
,  LINE_ADDR                        // (@LINE)
,  LINE_SIZE                        // (@LINE) (sizeof(LINE))
,  TGET                             // {} (Input length)
,  TPUTI, Word("DUN\n")             // {} (Input length)
,  THELP
,  LINE_INPS, TPOKEW                // {}
,  TH_CR                            // {}
,  TEXIT                            // {}
};

//----------------------------------------------------------------------------
//
// Thread-
//       TH_NXTW
//
// Purpose-
//       Get next word in LINE buffer. (Maximum length 255)
//
// Outputs-
//       (0) || (@PAD[0]) (1)
//       LINE_USED updated
//       PAD[0]= length
//       PAD[1]= text
//
//----------------------------------------------------------------------------
// Exit, word found
static Word NXTW4[]=                // {}
{  TPAD                             // (@PAD[0])
,  TCV01                            // (@PAD[0]) (1)
,  TEXIT                            // (@PAD[0]) (1)
};

// Copy non-blank characters
static Word NXTW3[]=                // (CHAR)
{  PADV0                            // (CHAR) (pad.size)
,  TINC                             // (CHAR) (pad.size+1)
,  TDUP                             // (CHAR) (pad.size+1) (pad.size+1)
,  PADS0                            // (CHAR) (pad.size) ((pad.size= (pad.size+1)))
,  TPAD                             // (CHAR) (pad.size) (@PAD[0])
,  TADD                             // (CHAR) (@PAD[pad.size])
,  TPOKEC                           // {}
,  TH_NXTC                          // (0) || (CHAR) (1)
,  TIFEQZ, NXTW4                    // (CHAR)
,  TDUP                             // (CHAR) (CHAR)
,  TIMMW, Word(' ')                 // (CHAR) (CHAR) (' ')
,  TIFNE, NXTW3                     // (CHAR)
,  TPOP                             // {}
,  TGOTO, NXTW4                     // {}
,  nullptr // TEMPORARY
};

// ==== TH_NXTW ==============================================================
static Word TH_NXTW[]=              // {}
{  DEF_SUB                          // {}
,  TCV00, PADS0                     // {} Initialize PAD[0]
,  TPUTI, Word("NXTW4\n"), TDEBUG_IMMW, NXTW4
,  TPUTI, Word("NXTW3\n"), TDEBUG_IMMW, NXTW3
,  TPUTI, Word("TH_NXTW\n"), TDEBUG_IMMW, TH_NXTW
,  TPUTI, Word("NXTW1\n"), TDEBUG_IMMW, &TH_NXTW[22]
,  TPUTI, Word("NXTW1\n"), TDEBUG
// FALL-THROUGH                     // {}
};

// Skip blank characters
static Word NXTW1[]=                // {}
{  TH_NXTC                          // (0) || (CHAR) (1)
,  TIFEQZ, TH_EXIT0                 // (CHAR)
,  TDUP                             // (CHAR) (CHAR)
,  TIMMW, Word(' ')                 // (CHAR) (CHAR) (' ')
,  TIFNE, NXTW3                     // (CHAR) // TODO: TIFNEI, Word(' '), NXTW2
,  TPOP                             // {}
,  TGOTO, NXTW1                     // {}
,  Word(-2), Word(-1) // TEMPORARY
,  nullptr // TEMPORARY
};

// static Word TH_NXTW[]= {DEF_SUB, TGOTO, NXTW1 }; // {}

//----------------------------------------------------------------------------
// CMAIN: Bringup debugging
//----------------------------------------------------------------------------
static void CMAIN(void) {
   IFDEBUG( debugf("CMAIN  Bringup debugging\n"); )

   debugf("TH_MAIN\n");
   debug_list((void**)i_addr);

   debugf("%p _LINE\n", _LINE);
}
static Word TMAIN[] = {(Word)CMAIN};

//----------------------------------------------------------------------------
//
// Thread-
//       TH_MAIN
//
// Purpose-
//       Main program loop
//
//----------------------------------------------------------------------------
static Word TH_QUIT[]= { TPUTI, Word("..QUIT..\n"), TQUIT}; // !!QUIT!!

static Word TH_MAIN[]=              // {}
{  DEF_SUB                          // {} // (MUST BE DEF_SUB)
#if 0
,  TH_NXTL
,  THELP
,  TGOTO, TH_QUIT
#endif

#if 1 // LINE REGRESSION TEST
,  TPUTI, Word("Testing LINE_PUT\n")
,  TPUTI, Word("A"), TIMMW, Word('A'), LINE_PUT
,  TPUTI, Word("B"), TIMMW, Word('B'), LINE_PUT
,  TPUTI, Word("C"), TIMMW, Word('C'), LINE_PUT
,  TPUTI, Word("D"), TIMMW, Word('D'), LINE_PUT

,  TPUTI, Word("\nLength "), TCV04, LINE_INPS, TPEEKW, TIFNE, TH_NG, TH_NG
,  TPUTI, Word("A "), TIMMW, Word('A'), LINE_GET, TIFNE, TH_NG, TH_NG
,  TPUTI, Word("B "), TIMMW, Word('B'), LINE_GET, TIFNE, TH_NG, TH_NG
,  TPUTI, Word("C "), TIMMW, Word('C'), LINE_GET, TIFNE, TH_NG, TH_NG
,  TPUTI, Word("D "), TIMMW, Word('D'), LINE_GET, TIFNE, TH_NG, TH_NG

// LAST BUG FIX (After adding test from empty LINE_GET)
// TPUTI, Word("! "), TIMMW, TCVF00,    LINE_GET, TIFNE, TH_NG, TH_NG
// SEG_FAULT!

,  TPUTI, Word("! "), TIMMW, Word(0),   LINE_GET, TIFNE, TH_NG, TH_NG
#endif

,  TMAIN
,  TDEBUG_DUMP
,  TH_NXTL                          // Initialize LINE
,  TPUTI, Word("Testing NXTW\n")
,  TH_NXTW                          // => 0 || (@PAD[0]) (1)
,  TIFEQZ, TH_MAIN                  // (@PAD[0])
// We should have a TH_PUT-able string on the stack
,  TDEBUG_DUMP
,  TPUTI, Word("Testing NXTW output\n")
,  TH_PUT                           //
,  THELP
,  TGOTO, TH_QUIT
,  TGOTO, TH_MAIN

// TEMPORARY: REFERENCE POSSIBLY UNUSED
,  TH_EXIT, TH_EXIT0, TH_EXIT1
,  TH_OK, TH_NG
,  TMAIN
,  TH_SUB
};

void CC_MAIN( void ) {
   debugf("TLC started\n");
   CRESET();                        // Reset the environment
   data.push(Data(TH_MAIN));        // Set program word
   CNEXT();                         // Run threaded mode
}
