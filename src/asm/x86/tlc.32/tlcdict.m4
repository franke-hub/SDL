##############################################################################
##
##       Copyright (c) 2010-2018 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
#
# Title-
#        tlcdict.m4
#
# Purpose-
#        Threaded Language Compiler: Dictionary
#
# Notes-
#        Read/only
#
##############################################################################

#
#        COMPILER DICTIONARY
#
TCDICT:  .long  TVAR
         dEntry("if",      TH_IF  )
         dEntry("then",    TH_THEN)
         dEntry("else",    TH_ELSE)
         dEntry("repeat",  TH_PEAT)
         dEntry("until",   TH_TILL)
         dEntry("loop",    TH_LOOP)
         dEntry("for",     TH_FOR )

         .long  0                   ## Last entry
         .long  TH_SEMI
         .byte  1,';'
         .align 4

#
#        STANDARD DICTIONARY
#
DS_CMPL:
         dEntry(":",       TH_CMPL)
         dEntry("var",     TH_VAR )
         dEntry("con",     TH_CON )
         dEntry("_",       TPEEKW )
         dEntry("!",       TPOKEW )
         dEntry("_c",      TPEEKC )
         dEntry("!c",      TPOKEC )
         dEntry("-1",      TCVFF  )
         dEntry("0",       TCV00  )
         dEntry("1",       TCV01  )
         dEntry("and",     TAND   )
         dEntry("or",      TOR    )
         dEntry("xor",     TXOR   )
         dEntry("not",     TNOT   )
         dEntry("1+",      TINC   )
         dEntry("1-",      TDEC   )
         dEntry("+",       TADD   )
         dEntry("-",       TSUB   )
         dEntry("*",       TMUL   )
         dEntry("/",       TDIV   )
         dEntry("//",      TDIVR  )
         dEntry("/mod",    TMOD   )
         dEntry("=0",      TH_EQZ )
         dEntry("<0",      TH_LTZ )
         dEntry("=",       TH_EQ  )
         dEntry("<",       TH_LT  )
         dEntry("L<",      TH_LTL )
         dEntry("minus",   TNEG   )
         dEntry("max",     TMAXF  )
         dEntry("min",     TMINF  )
         dEntry("abs",     TABSF  )
         dEntry("::s",     TCLS   )
         dEntry("=s",      TMVS   )
         dEntry("gnc",     TH_GNC )
         dEntry("swap",    TSWAP  )
         dEntry("over",    TOVER  )
         dEntry("dup",     TDUP   )
         dEntry("pop",     TPOP   )
         dEntry("exec",    TEXEC  )
         dEntry("nop",     TNEXT  )
         dEntry("base",    TBASE  )
         dEntry("pad",     TPAD   )
         dEntry("cvp",     TCVP   )
         dEntry("cvi",     TH_CVI )
         dEntry("clear",   TCLEAR )
         dEntry("reset",   TRESET )
         dEntry("tput",    TH_PUT )
         dEntry("tget",    TGET   )
         dEntry("echo",    TOUTC  )
         dEntry("key",     TINPC  )
         dEntry("debug",   TH_DEBUG)
         dEntry("top",     TH_TOP )
         dEntry("sp",      TH_SP  )
         dEntry("cr",      TH_CR  )
         dEntry(".",       TH_PRTV)
         dEntry(".x",      TH_PRTX)
         dEntry(".d",      TH_PRTD)
         dEntry("hex",     TH_HEX )
         dEntry("dec",     TH_DEC )
         dEntry("oct",     TH_OCT )
         dEntry("bin",     TH_BIN )
         dEntry("i",       TH_I   )
         dEntry("j",       TH_J   )
         dEntry("k",       TH_K   )
         dEntry("forget",  TH_FGET)
         dEntry("enter",   TH_NTER)
         dEntry("lookup",  TH_LOOK)
         dEntry("find",    TH_FIND)
         dEntry("cmpw",    TH_CMPW)
         dEntry("memorg",  MEMORG )
         dEntry("memory",  MEMORY )
         dEntry("memend",  MEMEND )
         dEntry("dict",    TSDICT )
         dEntry("snap",    TSNAP  )

         dEntry("bye",     TQUIT  )
         dEntry("end",     TQUIT  )
         dEntry("exit",    TQUIT  )
         dEntry("quit",    TQUIT  )
         dEntry("BYE",     TQUIT  )
         dEntry("END",     TQUIT  )
         dEntry("EXIT",    TQUIT  )
         dEntry("QUIT",    TQUIT  )

         .long  0                   ## Last entry
         .long  TH_TEST             ## (Test function)
         .byte  4
         .ascii "TEST"
         .align 4

