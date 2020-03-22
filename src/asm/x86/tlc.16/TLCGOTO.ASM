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
;;       TLCGOTO.ASM
;;
;; Purpose-
;;       Threaded Language Compiler (FORTH) GOTO, IF statements.
;;
;; Last change date-
;;       2018/09/30 Added license information.
;;
;;----------------------------------------------------------------------------
         PAGE
;
;        CGOTO - PROCESS THREADED GOTO
;
CGOTO:   MOV   BX,CX      ; GET PROGRAM COUNTER
         MOV   CX,[BX]    ; UPDATE PROGRAM COUNTER
         JMP   CNEXT      ; PROCESS NEXT MACHINE INSTUCTION

;
;        CIFEQZ - IF (TOP) = 0, PROCESS GOTO FUNCTION
;
CIFEQZ:  POP   AX         ; GET TOP
         CMP   AX,0       ; TEST TOP
         JE    CGOTO      ; IF = 0
CSKIP:   INC   CX         ; SKIP GOTO TARGET
         INC   CX
         JMP   CNEXT      ; PROCESS NEXT MACHINE INSTRUCTION

;
;        CIFNEZ - IF (TOP) <> 0, PROCESS GOTO FUNCTION
;
CIFNEZ:  POP   AX         ; GET TOP
         CMP   AX,0       ; TEST TOP
         JNE   CGOTO      ; IF <> 0
         JMP   CSKIP      ; SKIP GOTO TARGET

;
;        CIFLTZ - IF (TOP) < 0, PROCESS GOTO FUNCTION
;
CIFLTZ:  POP   AX         ; GET TOP
         CMP   AX,0       ; TEST TOP
         JL    CGOTO      ; IF < 0
         JMP   CSKIP      ; SKIP GOTO TARGET

;
;        CIFLEZ - IF (TOP) <= 0, PROCESS GOTO FUNCTION
;
CIFLEZ:  POP   AX         ; GET TOP
         CMP   AX,0       ; TEST TOP
         JLE   CGOTO      ; IF <= 0
         JMP   CSKIP      ; SKIP GOTO TARGET

;
;        CIFGEZ - IF (TOP) >= 0, PROCESS GOTO FUNCTION
;
CIFGEZ:  POP   AX         ; GET TOP
         CMP   AX,0       ; TEST TOP
         JGE   CGOTO      ; IF >= 0
         JMP   CSKIP      ; SKIP GOTO TARGET

;
;        CIFEQ - IF (TOP-1) = (TOP), PROCESS GOTO FUNCTION
;
CIFEQ:   POP   BX         ; GET TOP
         POP   AX         ; GET TOP-1
         CMP   AX,BX      ; (TOP-1):(TOP)
         JE    CGOTO      ; IF =
         JMP   CSKIP      ; SKIP GOTO TARGET

;
;        CIFNE - IF (TOP-1) <> (TOP), PROCESS GOTO FUNCTION
;
CIFNE:   POP   BX         ; GET TOP
         POP   AX         ; GET TOP-1
         CMP   AX,BX      ; (TOP-1):(TOP)
         JNE   CGOTO      ; IF <>
         JMP   CSKIP      ; SKIP GOTO TARGET
         PAGE
;
;        CIFLT - IF (TOP-1) < (TOP), PROCESS GOTO FUNCTION
;
CIFLT:   POP   BX         ; GET TOP
         POP   AX         ; GET TOP-1
         CMP   AX,BX      ; (TOP-1):(TOP)
         JL    CGOTO      ; IF <
         JMP   CSKIP      ; SKIP GOTO TARGET

;
;        CIFGE - IF (TOP-1) >= (TOP), PROCESS GOTO FUNCTION
;
CIFGE:   POP   BX         ; GET TOP
         POP   AX         ; GET TOP-1
         CMP   AX,BX      ; (TOP-1):(TOP)
         JGE   CGOTO      ; IF >=
         JMP   CSKIP      ; SKIP GOTO TARGET

;
;        CIFLTL - IF (TOP-1) L< (TOP), PROCESS GOTO FUNCTION
;
CIFLTL:  POP   BX         ; GET TOP
         POP   AX         ; GET TOP-1
         CMP   AX,BX      ; (TOP-1):(TOP)
         JC    CGOTO      ; IF <
         JMP   CSKIP      ; SKIP GOTO TARGET

;
;        CIFGEL - IF (TOP-1) L>= (TOP), PROCESS GOTO FUNCTION
;
CIFGEL:  POP   BX         ; GET TOP
         POP   AX         ; GET TOP-1
         CMP   AX,BX      ; (TOP-1):(TOP)
         JNC   CGOTO      ; IF >=
         JMP   CSKIP      ; SKIP GOTO TARGET
