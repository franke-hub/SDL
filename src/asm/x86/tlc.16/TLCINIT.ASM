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
;;       TLCINIT.ASM
;;
;; Purpose-
;;       Threaded Language Compiler (FORTH) Initialization
;;
;; Last change date-
;;       2018/09/30 Added license information.
;;
;;----------------------------------------------------------------------------
         PAGE
         ASSUME CS:SEG@TH,SS:SYSSTACK,DS:NOTHING,ES:NOTHING
;
;        INITIALIZATION CODE
;
INITIAL: SYEL             ; DOS ENTRY LINKAGE
         STI              ; ENABLE INTERRUPTS

;
;        INTERNAL INTERFACE INITIALIZATION
;
INI100:  DSEG  SEG@TH     ; ADDRESS THREAD SEGMENT
         ESEG  SEG@GW     ; ADDRESS GWA SEGMENT
         MOV   XITSS,SS   ; SAVE SS FOR FORCED EXIT
         MOV   XITSP,SP   ; SAVE SP FOR FORCED EXIT

         INT   012H       ; DETERMINE MEMORY SIZE
         MOV   CL,10      ; SET SHIFT COUNT
         ROR   AX,CL      ; COMPUTE MEMORY SIZE IN PARAGRAPHS
         MOV   DX,AX      ; DX = HIGHEST SYSTEM PARAGRAPH ADDRESS
         SUB   DX,00100h  ; ALLOW 4K FOR COMMAND.COM
         CLI              ; DISABLE INTERRUPTS
         MOV   AX,SEG@TH  ; ADDRESS OF DATA SEGMENT
         ADD   AX,01020h  ; FULL SEGMENT SIZE + STACK RESERVATION
         CMP   AX,DX      ; IS FULL DATA SEGMENT AVAILABLE?
         JC    INI120     ; YES, USE FULL DATA SEGMENT

;
;        USE CONSTRAINED MEMORY STACK
;
INI110:  MOV   AX,DX      ; AX = HIGHEST AVAILABLE PARAGRAPH ADDRESS
         SUB   AX,00020h  ; ALLOW 512 BYTE STACK AREA
         MOV   DX,AX      ; SAVE STACK SEGMENT ADDRESS
         SUB   AX,SEG@TH  ; AX = NUMBER OF AVAILABLE PARAGRAPHS
         JC    INIERR     ; IF INSUFFICIENT STORAGE
         MOV   SS,DX      ; SET STACK SEGMENT ADDRESS
         SHL   AX,1       ; COMPUTE BYTE COUNT
         SHL   AX,1       ; COMPUTE BYTE COUNT
         SHL   AX,1       ; COMPUTE BYTE COUNT
         SHL   AX,1       ; COMPUTE BYTE COUNT
         MOV   MEMORY+2,AX; SET END OF MEMORY ADDRESS
         MOV   AX,511     ; GET STACK POINTER VALUE
         MOV   SP,AX      ; SET STACK POINTER
         JMP   SHORT INI200; AND CONTINUE

;
;        UNCONSTRAINED MEMORY STACK
;
INI120:  SUB   AX,00020h  ; BACK ADJUST FOR STACK RESERVATION
         MOV   SS,AX      ; SET STACK SEGMENT ADDRESS
         MOV   MEMORY+2,0FFF0h; SET MAXIMUM AVAILABLE MEMORY
         SUB   DX,AX      ; DX = NUMBER OF PARAGRAPHS STILL AVAILBLE
         SHL   DX,1       ; POSITION
         SHL   DX,1       ; POSITION
         SHL   DX,1       ; POSITION
         SHL   DX,1       ; POSITION
         DEC   DX         ; DECREMENT BY 1 FOR TOP
         MOV   SP,DX      ; SET STACK POINTER
         PAGE
;
;        SET UP STORAGE FOR CALL STACK
;
INI200:  STI              ; ENABLE INTERRUPTS
         MOV   BP,SP      ; ADDRESS PROGRAM CALL STACK
         SUB   SP,128     ; MAKE ROOM FOR PROGRAM CALL STACK
         MOV   INIBP,BP   ; SAVE FOR RESET
         MOV   INISP,SP   ; SAVE FOR RESET
         MOV   CX,OFFSET TH@MAIN; ADDRESS MAINLINE THREAD
         JMP   FAR PTR CNEXT; BEGIN CODE EXECUTION

;
;        ERROR - NOT ENOUGH STORAGE FOR INITIALIZATION
;
INIERR:  JMP   FAR PTR CQUIT; ABORT, INSUFFICIENT STORAGE
