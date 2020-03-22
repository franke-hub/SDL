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
#        tlcvecs.m4
#
# Purpose-
#        Threaded Language Compiler: Code Thread Vectors
#
# Notes-
#        Read/only
#
##############################################################################

#
#        CODE THREAD VECTORS
#
TEXIT:   tword  CEXIT
TNEXT:   tword  CNEXT

TPEEKW:  tword  CPEEKW
TPOKEW:  tword  CPOKEW
TPEEKC:  tword  CPEEKC
TPOKEC:  tword  CPOKEC

TEXEC:   tword  CEXEC
TGOTO:   tword  CGOTO
TIFEQZ:  tword  CIFEQZ
TIFNEZ:  tword  CIFNEZ
TIFLTZ:  tword  CIFLTZ
TIFLEZ:  tword  CIFLEZ
TIFGEZ:  tword  CIFGEZ
TIFEQ:   tword  CIFEQ
TIFLT:   tword  CIFLT
TIFGE:   tword  CIFGE
TIFNE:   tword  CIFNE
TIFLTL:  tword  CIFLTL
TIFGEL:  tword  CIFGEL

TIMMW:   tword  CIMMW
TPOP:    tword  CPOP
TSWAP:   tword  CSWAP
TOVER:   tword  COVER
TDUP:    tword  CDUP
TCLEAR:  tword  CCLEAR
TRESET:  tword  CRESET
TQUIT:   tword  CQUIT
TAND:    tword  CAND
TOR:     tword  COR
TXOR:    tword  CXOR
TNOT:    tword  CNOT
TINC:    tword  CINC
TDEC:    tword  CDEC
TADD:    tword  CADD
TSUB:    tword  CSUB
TMUL:    tword  CMUL
TDIV:    tword  CDIV
TMOD:    tword  CMOD
TDIVR:   tword  CDIVR
TNEG:    tword  CNEG
TMAXF:   tword  CMAXF
TMINF:   tword  CMINF
TABSF:   tword  CABSF
TCLS:    tword  CCLS
TMVS:    tword  CMVS
TSNAP:   tword  CSNAP

TGET:    tword  CGET
TINPC:   tword  CINPC
TOUTC:   tword  COUTC
TCVC:    tword  CCVC

TAUXPUSH:tword  CAUXPUSH
TAUXPOP: tword  CAUXPOP
TH_FORT: tword  CV_FORT
TH_I:    tword  CV_I
TH_J:    tword  CV_J
TH_K:    tword  CV_K

TNOP:    tword  CNOP
TSTOP:   tword  CSTOP

#
#        CONSTANTS
#
TWORDSIZE:
         tword  TCON
         tword  WORDSIZE

TCVFF:   tword  TCON,-1
TCV00:   tword  TCON, 0
TCV01:   tword  TCON, 1
TCV02:   tword  TCON, 2
TCV04:   tword  TCON, 4
TCV08:   tword  TCON, 8
TCV10:   tword  TCON,10
TCV16:   tword  TCON,16
CVTTAB:  .ascii "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
cvttab:  .ascii "0123456789abcdefghijklmnopqrstuvwxyz"

