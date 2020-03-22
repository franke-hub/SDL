;;----------------------------------------------------------------------------
;;
;;       Copyright (c) 2010-2018 Frank Eskesen.
;;
;;       This file is free content, distributed under the GNU General
;;       Public License, version 3.0.
;;       (See accompanying file LICENSE.GPL-3.0 or the original
;;       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
;;
;;----------------------------------------------------------------------------
;;
;;
;; Title-
;;       TLCDICT.ASM
;;
;; Purpose-
;;       Threaded Language Compiler (FORTH) Dictionary
;;
;; Last change date-
;;       2018/09/30 Added license information.
;;
;;----------------------------------------------------------------------------
         IF1
DICT     MACRO P0,P1      ; DICTIONARY ENTRY
         LOCAL NEXT_ENTRY
         DW    NEXT_ENTRY
         DW    P1
         DB    NEXT_ENTRY-$-1,P0
NEXT_ENTRY EQU THIS BYTE
         ENDM
         ENDIF
         PAGE
;
;        COMPILER DICTIONARY
;
TCDICT   DW    TVAR
         DICT  'IF',TH@IF
         DICT  'THEN',TH@THEN
         DICT  'ELSE',TH@ELSE
         DICT  'REPEAT',TH@PEAT
         DICT  'UNTIL',TH@TILL
         DICT  'LOOP',TH@LOOP
         DICT  'FOR',TH@FOR
         DW    0          ; END OF DICTIONARY
         DW    TH@SEMI
         DB    1,';'
         PAGE
;
;        STANDARD DICTIONARY
;
TSDICT   DW    TVAR
         DW    DS@CMPL

DS@CMPL  EQU   THIS BYTE
         DICT  ':',TH@CMPL
         DICT  'VAR',TH@VAR
         DICT  'CON',TH@CON
         DICT  '@',TPEEKW
         DICT  '!',TPOKEW
         DICT  '@C',TPEEKC
         DICT  '!C',TPOKEC
         DICT  'ES@',TEGET
         DICT  'ES!',TESET
         DICT  'ES:@',TPEEKWS
         DICT  'ES:!',TPOKEWS
         DICT  'ES:@C',TPEEKCS
         DICT  'ES:!C',TPOKECS
         DICT  '-1',TCVFF
         DICT  '0',TCV00
         DICT  '1',TCV01
         DICT  'AND',TAND
         DICT  'OR',TOR
         DICT  'XOR',TXOR
         DICT  'NOT',TNOT
         DICT  '1+',TINC
         DICT  '1-',TDEC
         DICT  '+',TADD
         DICT  '-',TSUB
         DICT  '*',TMUL
         DICT  '/',TDIV
         DICT  '//',TDIVR
         DICT  '/MOD',TMOD
         DICT  '=0',TH@EQZ
         DICT  '<0',TH@LTZ
         DICT  '=',TH@EQ
         DICT  '<',TH@LT
         DICT  'L<',TH@LTL
         DICT  'MINUS',TNEG
         DICT  'MAX',TMAXF
         DICT  'MIN',TMINF
         DICT  'ABS',TABSF
         DICT  '::S',TCLS
         DICT  '=S',TMVS
         DICT  'GNC',TGNC
         DICT  'SWAP',TSWAP
         DICT  'OVER',TOVER
         DICT  'DUP',TDUP
         DICT  'POP',TPOP
         DICT  'EXEC',TEXEC
         DICT  'NOP',TNEXT
         DICT  'BASE',TBASE
         DICT  'PAD',TPAD
         DICT  'CVP',TCVP
         DICT  'CVI',TCVI
         DICT  'CLEAR',TCLEAR
         DICT  'RESET',TRESET
         DICT  'TPUT',TPUT
         DICT  'TGET',TGET
         DICT  'ECHO',TOUTC
         DICT  'KEY',TINPC
         DICT  'DEBUG',TDEBUG
         DICT  'TOP',TH@TOP
         DICT  'SP',TH@SP
         DICT  'CR',TH@CR
         DICT  '.',TH@PRTV
         DICT  '.X',TH@PRTX
         DICT  '.D',TH@PRTD
         DICT  'HEX',TH@HEX
         DICT  'DEC',TH@DEC
         DICT  'OCT',TH@OCT
         DICT  'BIN',TH@BIN
         DICT  'I',TH@I
         DICT  'J',TH@J
         DICT  'K',TH@K
         DICT  'FORGET',TH@FGET
         DICT  'ENTER',TH@NTER
         DICT  'LOOKUP',TH@LOOK
         DICT  'FIND',TH@FIND
         DICT  'CMPW',TH@CMPW
         DICT  'MEMORY',MEMORY
         DICT  'DICT',TSDICT
         DICT  'MAIN',TMAIN
         DICT  'END',TQUIT
         DICT  'EXIT',TQUIT
         DICT  'QUIT',TQUIT

         DW    0          ; END OF DICTIONARY
         DW    TH@TEST
         DB    4,'TEST'
@@ENDD   EQU   THIS BYTE
