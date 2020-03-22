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
;;       TLCTERM.ASM
;;
;; Purpose-
;;       Threaded Language Compiler (FORTH) Termination
;;
;; Last change date-
;;       2018/09/30 Added license information.
;;
;;----------------------------------------------------------------------------
         PAGE
;
;        DEBUGGING INTERFACE CODE
;
CEBUG1:  MOV   AX,-2[BP]  ; GET CALLER'S PROGRAM COUNTER
         PUSH  AX
         JMP   CNEXT      ; AND CONTINUE

;
;        CRESET - RESET THE PROGRAM COUNTER AND THE INTEGER STACK
;
CRESET:  ESEG  SEG@GW     ; ADDRESS GWA SEGMENT
         MOV   BP,INIBP   ; RESTORE BP VALUE
         MOV   SP,INISP   ; RESTORE SP VALUE
         MOV   CX,OFFSET TH@MAIN; ADDRESS MAINLINE THREAD
;        JMP   SHORT CCLEAR

;
;        CCLEAR - CLEAR THE INTEGER STACK
;
CCLEAR:  ESEG  SEG@GW     ; ADDRESS GWA SEGMENT
         MOV   SP,INISP   ; RESTORE SP VALUE
         JMP   CNEXT      ; AND CONTINUE

;
;        SYSTEM EXIT
;
CQUIT:   ESEG  SEG@GW     ; ADDRESS GWA SEGMENT
         CLI              ; DISABLE INTERRUPTS
         MOV   SP,XITSP   ; RESTORE STACK POINTER VALUE
         MOV   SS,XITSS   ; RESTORE STACK SEGMENT VALUE
         STI              ; ENABLE INTERRUPTS
         SYXL             ; DOS RETURN LINKAGE
