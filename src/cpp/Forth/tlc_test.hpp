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
//       tlc_test.hpp
//
// Purpose-
//       TLC bringup tests.
//
// Last change date-
//       2019/01/01
//
//----------------------------------------------------------------------------
#include "tlc_base.hpp"             // Base functions
#include "tlc_refs.hpp"             // Load references

//----------------------------------------------------------------------------
// CCODE: (Shows how to run a code sequence. Runs inside other code.)
//----------------------------------------------------------------------------
static void CCODE(void) {
   debug_op("CCODE");

// TIMMW, Word(104), TMODE, Word(CNOP), Word(CNOP), Word(CPOP)
   CNOP();
   CNOP();
   CPOP();
}
static Word TCODE[] = {(Word)CCODE};

static Word CON_DATA[]= { DEF_CON, Word(-3) };
static Word VAR_DATA[]= { DEF_VAR, Word(-2) };

//----------------------------------------------------------------------------
// Helper threads
//----------------------------------------------------------------------------
static Word TH_DONE[]=              // Part of GOTO regression test
{  TPUTI, Word("TH_DONE, GOTO regression OK\n")
,  TPOP                             // (Part of GOTO regression test)
,  TEXIT                            // (We were in a subroutine0
,  nullptr, nullptr, nullptr, nullptr // HALT! TH_UNIT should stop
};

static Word TH_FAIL[]=              // Test FAIL GOTO
{  TNOP, TNOP, TNOP, TNOP           // Spacers (handle certain bugs)
,  TPUTI, Word("TH_FAIL: some test failed\n")
,  TQUIT
,  nullptr, nullptr, nullptr, nullptr
};

static Word TH_GOTO[]=              // Testing GOTO
{  DEF_SUB, TDEBUG_THIS             // 0, 1
,  TPUTI, Word("TH_GOTO\n")         // 2, 3
,  TGOTO, TH_GOTO + 8               // 4, 5
,  TPUTI, Word("**GOTO FAILED** Didn't GOTO\n") // 6, 7
,  TPUTI, Word("**GOTO PASSED**\n")      // 8, 9
,  TEXIT                            // 10
,  nullptr, nullptr, nullptr, nullptr
};

static Word TH_HALT[]=              // Testing HALT
{  TNOP, TNOP, TNOP, TNOP           // Spacers (handle certain bugs)
,  TPUTI, Word("TH_HALT: halt expected\n")
,  nullptr, nullptr, nullptr, nullptr
};

static Word TH_NADA[]=              // Does (almost) nothing subroutine
{  DEF_SUB, TPUTI, Word("TH_NADA running\n"), TEXIT
,  nullptr, nullptr, nullptr, nullptr
};

//============================================================================
// TH_UNIT: The unit test
//============================================================================
static Word TH_UNIT[]=              // Unit test subroutine
{  DEF_SUB
,  TPUTI, Word("TH_UNIT...\n")
,  TIMMW, Word(732)
,  TIFEQZ, TH_FAIL
,  TIMMW, Word(732), TDEC, TIMMW, Word(731)
,  TIFNE, TH_FAIL
,  TEXIT
,  nullptr, nullptr, nullptr, nullptr // HALT! (Should not occur)
};

//============================================================================
// TH_REGRESSION
//============================================================================
static Word TH_REGRESSION[]=        // Regression tests
{  DEF_SUB
#if false
,  TH_HALT                          // Debug HALT
#endif

// The regression tests (MOST NOT SELF-VERIFYING)
,  TH_GOTO                          // Self-verifying GOTO test
,  TIMMW, Word(101), TH_NADA, TPOP  // Test DEF_SUB
,  TIMMW, Word(102), VAR_DATA, TPEEKW, TPOP, TPOP // Test DEF_VAR
,  TIMMW, Word(103), CON_DATA, TPOP, TPOP // Test DEF_CON
// TIMMW, Word(104), TMODE, Word(CNOP), Word(CNOP), Word(CPOP)
,  TIMMW, Word(104), TCODE          // Call code from code
,  TIMMW, Word(105), TIMMW, TH_NADA, TNEXT, TPOP // Test TNEXT, run TH_NADA
,  TIMMW, Word(999), TGOTO, TH_DONE // (TGOTO regression test)
,  nullptr, nullptr, nullptr, nullptr // HALT! (Should not occur)
};

//============================================================================
// TH_MAIN
//============================================================================
static Word TH_MAIN[]=
{  DEF_SUB                          // (Must be DEF_SUB)
,  TNOP, TNOP                       // In case start address wrong
,  TPUTI, Word("TH_MAIN started OK\n")
,  TDEBUG_DUMP                      // Take a dump
,  TDEBUG_THIS                      // Debug this thread
,  TPUTI, Word("Calling TH_NADA...\n")
,  TH_NADA
,  TPUTI, Word("...TH_NADA returned OK\n")
,  TH_REGRESSION                    // Run regression tests
,  TPUTI, Word("TH_REGRESSION complete\n")
,  TH_UNIT                          // Run VAR_TEST
,  TPUTI, Word("TH_UNIT complete\n")
,  TEXIT                            // Normal termination
,  Word(0xDeadBeef)                 // Marker
,  nullptr, nullptr, nullptr, nullptr // HALT! TH_UNIT should stop

//----------------------------------------------------------------------------
// Reference possibly unused static Words
,  CON_DATA, VAR_DATA
,  TH_DONE, TH_FAIL, TH_HALT, TH_NADA, TH_REGRESSION, TH_UNIT
,  TCODE
};

//============================================================================
// CC_MAIN
//============================================================================
void CC_MAIN( void ) {
   debugf("\nTH_UNIT\n"); debug_list((void**)TH_UNIT);
   debugf("\nTH_REGR\n"); debug_list((void**)TH_REGRESSION);
   debugf("\nTH_MAIN %p\n", TH_MAIN); debug_list((void**)TH_MAIN);
   debugf("\nTH_NADA %p\n", TH_NADA); debug_list((void**)TH_NADA);

   debugf("\nStarting TH_MAIN...\n");
   CRESET();                        // Reset the environment
   data.push(Data(TH_MAIN));        // Set program word
   CNEXT();                         // Run threaded mode
   if( !operational )
     debugf("ERROR: NOT OPERATIONAL\n");
   debugf("\n...TH_MAIN completed, operational(%d)\n", operational);
}
