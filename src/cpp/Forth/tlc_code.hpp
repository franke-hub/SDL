//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       tlc_code.hpp
//
// Purpose-
//       TLC built-in Threaded functions
//
// Last change date-
//       2021/05/19
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// TH_CR: Write a carriage return
//----------------------------------------------------------------------------
static Word TH_CR[]= {DEF_SUB, TIMMW, Word('\n'), TOUTC, TEXIT };

//----------------------------------------------------------------------------
// TH_DEC: Set BASE= 10
//----------------------------------------------------------------------------
static Word TH_DEC[]= {DEF_SUB, TIMMW, Word(10), BASE, TPOKEW, TEXIT };

//----------------------------------------------------------------------------
// TH_HEX: Set BASE= 16
//----------------------------------------------------------------------------
static Word TH_HEX[]= {DEF_SUB, TIMMW, Word(16), BASE, TPOKEW, TEXIT };

//----------------------------------------------------------------------------
// TH_OCT: Set BASE= 8
//----------------------------------------------------------------------------
static Word TH_OCT[]= {DEF_SUB, TIMMW, Word(8), BASE, TPOKEW, TEXIT };

//----------------------------------------------------------------------------
// TH_SP: Write a space
//----------------------------------------------------------------------------
static Word TH_SP[]= {DEF_SUB, TIMMW, Word(' '), TOUTC, TEXIT };

//----------------------------------------------------------------------------
// TH_TOP: Print top without changing it
//----------------------------------------------------------------------------
static Word TH_TOP[]= {DEF_SUB, TDUP, TDOT, TEXIT };
