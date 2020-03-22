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
//       tlc_refs.hpp
//
// Purpose-
//       BRINGUP: Reference unused stuff.
//
// Last change date-
//       2019/01/01
//
// Implementation note-
//       For bringup. Reference functions whether or not they are used.
//       This prevents compiler errors for unused code.
//
//----------------------------------------------------------------------------
Word INSURE_REFERENCED[]=           // Insure references exist
{  nullptr
,  Word(BASE)
,  Word(cvttab)
,  Word(CVTTAB)
,  Word(DEF_CON)
,  Word(DEF_SUB)
,  Word(DEF_VAR)
,  TABS
,  TADD
,  TAND
,  TCLS
,  TDEBUG
,  TDEBUG_DUMP
,  TDEBUG_IMMW
,  TDEBUG_THIS
,  TDEC
,  TDIV
,  TDIVR
,  TDOT
,  TDUP
,  TEXIT
,  TGET
,  TGOTO
,  TIFEQZ
,  TIFGEZ
,  TIFGTZ
,  TIFLEZ
,  TIFLTZ
,  TIFNEZ
,  TIFEQ
,  TIFGE
,  TIFGT
,  TIFLE
,  TIFLT
,  TIFNE
,  TIMMW
,  TINC
,  TMAX
,  TMIN
,  TMOD
,  TMUL
,  TNEG
,  TNEXT
,  TNOP
,  TNOT
,  TOR
,  TOUTC
,  TOVER
,  TPEEKC
,  TPEEKW
,  TPOKEC
,  TPOKEW
,  TPOP
,  TPUTI
,  TPUTS
,  TSWAP
,  TSUB
,  TQUIT
,  TXOR
};
