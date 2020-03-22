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
;;       TLC.ASM
;;
;; Purpose-
;;       (16 bit mode) Threaded Language Compiler (FORTH)
;;
;; Last change date-
;;       2018/09/30 Added license information.
;;
;;----------------------------------------------------------------------------
         PAGE  80,132
         PAGE
         TITLE TLC V00.0 - Threaded Language Compiler
         NAME  TLC
         IF1
         INCLUDE S:SYSMAC.ASM
         DOSPSP           ; DOS PROGRAM SEGMENT PREFIX
         ENDIF

;
;        EQUATES
;
CH@LF    EQU   10
CH@NL    EQU   13
CH@SP    EQU   ' '


SYSSTACK SEGMENT PARA STACK 'STACK'
         DW    32 DUP(-1) ; DEFINE STACK SPACE
SYSSTACK ENDS             ; END STACK SEGMENT


SEG@GW   SEGMENT PARA     ; GENERAL WORK AREAS
XITSS    DW    0          ; SS ON EXIT
XITSP    DW    0          ; SP ON EXIT
INIBP    DW    0          ; RESET BP VALUE
INISP    DW    0          ; RESET SP VALUE

SEGCS    DW    SEG@CS     ; CODE SEGMENT POINTER
SEGDS    DW    SEG@TH     ; DATA SEGMENT POINTER
SEGES    DW    SEG@GW     ; EXTRA SEGMENT POINTER
SEGSS    DW    SYSSTACK   ; STACK SEGMENT POINTER
SEG@GW   ENDS             ; END GENERAL WORK SEGMENT
         PAGE
SEG@TH   SEGMENT PARA     ; THREAD (DATA) SEGMENT
         INCLUDE TLCTHCD.ASM; THREADED CODE
         INCLUDE TLCDICT.ASM; DICTIONARY
         INCLUDE TLCINIT.ASM; INITIALIZATION CODE
SEG@TH   ENDS             ; DATA THREAD SEGMENT
         PAGE
SEG@CS   SEGMENT PARA     ; THREADED LANGUAGE COMPILER
         ASSUME CS:SEG@CS,SS:NOTHING,DS:SEG@TH,ES:NOTHING
;
; Subsection-
;        MAINLINE
;
; Function-
;        Code execution functions
;

MAINLINE PROC             ; INTERFACE TESTING
;
;        DEBUGGING TRAP POINT
;
CSTOP:   NOP
         JMP   SHORT CNEXT; BEGIN CODE EXECUTION

;
;        TCALL - THREADED CALL
;
TCALL:   DEC   BP         ; DECREMENT PROGRAM COUNTER STACK INDEX
         DEC   BP
         MOV   [BP],CX    ; SAVE PROGRAM COUNTER
         MOV   CX,BX      ; ADDRESS THE TCALL WORD
         INC   CX         ; INCREMENT PROGRAM COUNTER
         INC   CX
;        JMP   CNEXT      ; GET NEXT THREAD INSTRUCTION

;
;        CNEXT - GET NEXT THREAD INSTRUCTION
;
CNEXT:   MOV   BX,CX      ; GET PROGRAM COUNTER
         INC   CX         ; INCREMENT PROGRAM COUNTER
         INC   CX
         MOV   BX,[BX]    ; GET INDIRECT PROGRAM COUNTER
         MOV   AX,[BX]    ; GET NEXT MACHINE INSTRUCTION ADDRESS
         JMP   AX         ; PROCESS NEXT MACHINE INSTUCTION

;
;        CEXIT - EXIT FROM THREAD
;
CEXIT:   MOV   CX,[BP]    ; RESTORE PROGRAM COUNTER
         INC   BP         ; INCREMENT PROGRAM COUNTER STACK INDEX
         INC   BP
         JMP   CNEXT      ; PROCESS NEXT MACHINE INSTUCTION
         PAGE
;
;        CPEEKW - INTEGER LOAD
;
CPEEKW:  POP   BX         ; GET VARIABLE ADDRESS
         MOV   AX,[BX]    ; GET VARIABLE
         PUSH  AX         ; SET VARIABLE ON STACK
         JMP   CNEXT      ; AND CONTINUE

;
;        CPOKEW - INTEGER STORE
;
CPOKEW:  POP   BX         ; GET VARIABLE ADDRESS
         POP   AX         ; GET VARIABLE
         MOV   [BX],AX    ; STORE VARIABLE
         JMP   CNEXT      ; AND CONTINUE

;
;        CPEEKC - CHARACTER LOAD
;
CPEEKC:  POP   BX         ; GET VARIABLE ADDRESS
         MOV   AL,[BX]    ; GET VARIABLE
         MOV   AH,0       ; ZERO AH
         PUSH  AX         ; SET VARIABLE ON STACK
         JMP   CNEXT      ; AND CONTINUE

;
;        CPOKEC - CHARACTER STORE
;
CPOKEC:  POP   BX         ; GET VARIABLE ADDRESS
         POP   AX         ; GET VARIABLE
         MOV   [BX],AL    ; STORE VARIABLE
         JMP   CNEXT      ; AND CONTINUE
         PAGE
;
;        CEGET - GET ES REGISTER ONTO STACK
;
CEGET:   PUSH  ES         ; GET ES
         JMP   CNEXT      ; AND CONTINUE
;
;        CESET - SET ES REGISTER FROM STACK
;
CESET:   POP   ES         ; SET ES
         JMP   CNEXT      ; AND CONTINUE

;
;        CPEEKS - INTEGER SEGMENT LOAD
;
CPEEKWS: POP   BX         ; GET VARIABLE ADDRESS
         MOV   AX,ES:[BX] ; GET VARIABLE
         PUSH  AX         ; SET VARIABLE ON STACK
         JMP   CNEXT      ; AND CONTINUE

;
;        CPOKES - INTEGER SEGMENT STORE
;
CPOKEWS: POP   BX         ; GET VARIABLE ADDRESS
         POP   AX         ; GET VARIABLE
         MOV   ES:[BX],AX ; STORE VARIABLE
         JMP   CNEXT      ; AND CONTINUE

;
;        CPEEKS - CHARACTER SEGMENT LOAD
;
CPEEKCS: POP   BX         ; GET VARIABLE ADDRESS
         MOV   AL,ES:[BX] ; GET VARIABLE
         MOV   AH,0       ; ZERO AH
         PUSH  AX         ; SET VARIABLE ON STACK
         JMP   CNEXT      ; AND CONTINUE

;
;        CPOKES - CHARACTER SEGMENT STORE
;
CPOKECS: POP   BX         ; GET VARIABLE ADDRESS
         POP   AX         ; GET VARIABLE
         MOV   ES:[BX],AL ; STORE VARIABLE
         JMP   CNEXT      ; AND CONTINUE
         PAGE
;
;        TVAR - INTEGER VARIABLE
;
TVAR:    INC   BX         ; ADDRESS THE VARIABLE
         INC   BX
         PUSH  BX         ; SET VARIABLE ADDRESS ON STACK
         JMP   CNEXT      ; AND CONTINUE

;
;        TCON - INTEGER CONSTANT
;
TCON:    INC   BX         ; ADDRESS THE CONSTANT
         INC   BX
         MOV   AX,[BX]    ; GET THE CONSTANT
         PUSH  AX         ; SET CONSTANT ON STACK
         JMP   CNEXT      ; AND CONTINUE

;
;        CIMMW - PUSH THE NEXT PROGRAM WORD ONTO THE STACK
;
CIMMW:   MOV   BX,CX      ; GET WORD ADDRESS
         INC   CX         ; BUMP PC PAST THE CONSTANT
         INC   CX
         MOV   AX,[BX]    ; GET WORD
         PUSH  AX         ; ADD VARIABLE TO STACK
         JMP   CNEXT      ; AND CONTINUE

;
;        CPOP - REMOVE THE TOP ENTRY FROM THE STACK
;
CPOP:    POP   AX         ; REMOVE TOP OF STACK
         JMP   CNEXT      ; AND CONTINUE

;
;        CDUP - DUPLICATE THE TOP ENTRY ON THE STACK
;
CDUP:    POP   AX         ; GET TOP OF STACK
         PUSH  AX         ; RESTORE IT
         PUSH  AX         ; DUPLICATE IT
         JMP   CNEXT      ; AND CONTINUE

;
;        CSWAP - SWAP THE TOP TWO ELEMENTS ON THE STACK
;
CSWAP:   POP   AX         ; GET TOP OF STACK
         POP   BX         ; GET TOP-1
         PUSH  AX         ; STORE OLD TOP
         PUSH  BX         ; STORE OLD TOP-1
         JMP   CNEXT      ; AND CONTINUE

;
;        COVER - COPY THE NEXT TO THE TOP ELEMENT TO THE TOP OF THE STACK
;
COVER:   POP   AX         ; GET TOP OF STACK
         POP   BX         ; GET TOP-1
         PUSH  BX         ; STORE OLD TOP-1
         PUSH  AX         ; STORE OLD TOP
         PUSH  BX         ; STORE OLD TOP-1 AGAIN
         JMP   CNEXT      ; AND CONTINUE
         PAGE
;
;        CINC - INCREMENT TOP
;
CINC:    POP   AX         ; GET TOP OF STACK
         INC   AX         ; AX:= AX + 1
         PUSH  AX         ; STORE RESULT
         JMP   CNEXT      ; AND CONTINUE

;
;        CDEC - DECREMENT TOP
;
CDEC:    POP   AX         ; GET TOP OF STACK
         DEC   AX         ; AX:= AX - 1
         PUSH  AX         ; STORE RESULT
         JMP   CNEXT      ; AND CONTINUE

;
;        CADD - ADD TOP + TOP-1
;
CADD:    POP   AX         ; GET TOP OF STACK
         POP   BX         ; GET TOP-1
         ADD   AX,BX      ; AX:= TOP + TOP-1
         PUSH  AX         ; STORE RESULT
         JMP   CNEXT      ; AND CONTINUE

;
;        CSUB - SUBTRACT TOP FROM TOP-1
;
CSUB:    POP   BX         ; GET TOP OF STACK
         POP   AX         ; GET TOP-1
         SUB   AX,BX      ; AX:= TOP-1 - TOP
         PUSH  AX         ; STORE RESULT
         JMP   CNEXT      ; AND CONTINUE

;
;        CMUL - MULTIPLY TOP * TOP-1
;
CMUL:    POP   AX         ; GET TOP OF STACK
         POP   BX         ; GET TOP-1
         MUL   BX         ; AX:= TOP * TOP-1
         PUSH  AX         ; STORE RESULT
         JMP   CNEXT      ; AND CONTINUE

;
;        CDIV - DIVIDE TOP BY TOP-1
;
CDIV:    POP   BX         ; GET TOP OF STACK
         POP   AX         ; GET TOP-1
         CWD              ; CONVERT TO DOUBLEWORD
         DIV   BX         ; AX:= TOP-1 / TOP
         PUSH  AX         ; STORE RESULT
         JMP   CNEXT      ; AND CONTINUE

;
;        CMOD - MODULUS TOP BY TOP-1
;
CMOD:    POP   BX         ; GET TOP OF STACK
         POP   AX         ; GET TOP-1
         CWD              ; CONVERT TO DOUBLEWORD
         DIV   BX         ; DX:= TOP-1 // TOP
         PUSH  DX         ; STORE RESULT
         JMP   CNEXT      ; AND CONTINUE

;
;        CDIVR - DIVIDE TOP BY TOP-1, KEEP QUOTIENT AND REMAINDER
;
CDIVR:   POP   BX         ; GET TOP OF STACK
         POP   AX         ; GET TOP-1
         CWD              ; CONVERT TO DOUBLEWORD
         DIV   BX         ; AX:= TOP-1 / TOP
         PUSH  AX         ; STORE RESULT
         PUSH  DX         ; STORE RESULT
         JMP   CNEXT      ; AND CONTINUE
         PAGE
;
;        CAND - INTEGER AND
;
CAND:    POP   AX         ; GET TOP OF STACK
         POP   BX         ; GET TOP-1
         AND   AX,BX      ; AX:= TOP & TOP-1
         PUSH  AX         ; STORE RESULT
         JMP   CNEXT      ; AND CONTINUE

;
;        COR - INTEGER OR
;
COR:     POP   AX         ; GET TOP OF STACK
         POP   BX         ; GET TOP-1
         OR    AX,BX      ; AX:= TOP | TOP-1
         PUSH  AX         ; STORE RESULT
         JMP   CNEXT      ; AND CONTINUE

;
;        CXOR - INTEGER XOR
;
CXOR:    POP   AX         ; GET TOP OF STACK
         POP   BX         ; GET TOP-1
         XOR   AX,BX      ; AX:= TOP && TOP-1
         PUSH  AX         ; STORE RESULT
         JMP   CNEXT      ; AND CONTINUE

;
;        CNEG - INTEGER NEGATE
;
CNEG:    POP   AX         ; GET TOP OF STACK
         NEG   AX         ; AX:= -AX
         PUSH  AX         ; STORE RESULT
         JMP   CNEXT      ; AND CONTINUE

;
;        CNOT - INTEGER NOT
;
CNOT:    POP   AX         ; GET TOP OF STACK
         NOT   AX         ; AX:= NOT AX
         PUSH  AX         ; STORE RESULT
         JMP   CNEXT      ; AND CONTINUE

;
;        CMAXF - MAXIMUM(TOP,TOP-1)
;
CMAXF:   POP   BX         ; GET TOP OF STACK
         POP   AX         ; GET TOP-1
         CMP   AX,BX      ; TOP:TOP-1
         JLE   CMIN1      ; IF AX<=BX
CMAX1:   PUSH  AX         ; STORE RESULT
         JMP   CNEXT      ; AND CONTINUE

;
;        CMINF - MINIMUM(TOP,TOP-1)
;
CMINF:   POP   BX         ; GET TOP OF STACK
         POP   AX         ; GET TOP-1
         CMP   AX,BX      ; TOP:TOP-1
         JLE   CMAX1      ; IF AX<=BX
CMIN1:   PUSH  BX         ; STORE RESULT
         JMP   CNEXT      ; AND CONTINUE

;
;        CABSF - ABS(TOP)
;
CABSF:   POP   AX         ; GET TOP-1
         CMP   AX,0       ; TOP:0
         JGE   CABS1      ; IF AX>=0
         NEG   AX         ; GET ABSOLUTE VALUE
CABS1:   PUSH  AX         ; STORE RESULT
         JMP   CNEXT      ; AND CONTINUE
         PAGE
;
;        COMPARE (TOP-1) STRING : (TOP) STRING
;
CCLS:    POP   SI         ; GET (TOP) STRING
         POP   DI         ; GET (TOP-1) STRING
         PUSH  CX         ; SAVE CX
         PUSH  ES         ; SAVE ES
         PUSH  DS         ; SET ES:= DS
         POP   ES         ; +
         SUB   CH,CH      ; ZERO CH
         MOV   BH,CH      ; ZERO BH
         MOV   BL,[SI]    ; SET (TOP) STRING LENGTH
         INC   SI         ; ADDRESS (TOP) STRING
         MOV   CL,[DI]    ; SET (TOP-1) STRING LENGTH
         INC   DI         ; ADDRESS (TOP-1) STRING
         CMP   CL,0       ; IS STRING LENGTH 0?
         JE    CLSE       ; YES, STRINGS ARE EQUAL
         CLD              ; COMPARE LEFT TO RIGHT
CLS1:    CMP   BL,0       ; IS STRING LENGTH 0?
         JE    CLSE       ; YES, STRINGS ARE EQUAL
         CMPSB            ; COMPARE STRINGS
         JL    CLSL       ; IF (TOP-1) STRING < (TOP) STRING
         JG    CLSH       ; IF (TOP-1) STRING > (TOP) STRING
         DEC   BL         ; DECREMENT (TOP) STRING LENGTH
         LOOP  CLS1       ; AND CONTINUE SCAN
CLSE:    CMP   CL,BL      ; COMPARE STRING LENGTHS
         JL    CLSL       ; IF L'(TOP-1) < L'(TOP)
         JG    CLSH       ; IF L'(TOP-1) > L'(TOP)
         SUB   AX,AX      ; STRINGS ARE EXACTLY EQUAL
CLSX:    POP   ES         ; RESTORE ES
         POP   CX         ; RESTORE CX
         PUSH  AX         ; STORE RESULTANT
         JMP   CNEXT      ; AND CONTINUE
CLSL:    MOV   AX,-1      ; SET RESULTANT
         JMP   CLSX       ; AND RETURN
CLSH:    MOV   AX,1       ; SET RESULTANT
         JMP   CLSX       ; AND RETURN

;
;        SET (TOP-1) STRING = (TOP) STRING
;
CMVS:    POP   SI         ; GET (TOP) STRING
         POP   DI         ; GET (TOP-1) STRING
         PUSH  CX         ; SAVE CX
         PUSH  ES         ; SAVE ES
         PUSH  DS         ; SET ES:= DS
         POP   ES         ; +
         SUB   CH,CH      ; ZERO CH
         MOV   CL,[SI]    ; SET (TOP) STRING LENGTH
         INC   SI         ; ADDRESS (TOP) STRING
         MOV   [DI],CL    ; SET STRING LENGTH
         INC   DI         ; ADDRESS (TOP-1) STRING
         CMP   CL,0       ; IS STRING LENGTH 0?
         JE    MVSE       ; YES, MOVE COMPLETE
         CLD              ; MOVE LEFT TO RIGHT
         REP   MOVSB      ; MOVE STRING
MVSE:    POP   ES         ; RESTORE ES
         POP   CX         ; RESTORE CX
         JMP   CNEXT      ; AND CONTINUE
         PAGE
;
;        CEXEC - EXECUTE INSTRUCTION ADDRESSED BY TOP OF STACK
;
CEXEC:   POP   BX         ; GET THREAD ADDRESS
         MOV   AX,[BX]    ; GET CODE ADDRESS
         JMP   AX         ; EXECUTE IT

;
;        IEXEC - EXECUTE INSTRUCTION FOLLOWING CURRENT INSTRUCTION
;
IEXEC:   INC   BX         ; ADDRESS NEXT INSTRUCTION
         INC   BX
         MOV   BX,[BX]    ; GET THREAD ADDRESS
         MOV   AX,[BX]    ; GET CODE ADDRESS
         JMP   AX         ; EXECUTE IT
         PAGE
;
;        CINPC - READ CHARACTER FROM CONSOLE
;
CINPC:   MOV   AH,7       ; CONSOLE INPUT CODE
         PUSH  BP         ; SAVE BP AROUND CALL
         INT   33         ; DOS FUNCTION CALL
         POP   BP         ; RESTORE BP
         MOV   AH,0
         PUSH  AX
         JMP   CNEXT      ; AND CONTINUE

;
;        COUTC - WRITE CHARACTER TO SCREEN
;
COUTC:   POP   DX         ; GET CHARACTER
         MOV   AH,2       ; SCREEN OUTPUT CODE
         PUSH  BP         ; SAVE BP AROUND CALL(S)
         INT   33         ; DOS FUNCTION CALL
         CMP   DL,CH@NL   ; CARRIAGE RETURN?
         JNE   COUTX      ; NO, EXIT
         MOV   DL,CH@LF   ; SET LINE FEED
         MOV   AH,2       ; SCREEN OUTPUT CODE
         INT   33         ; DOS FUNCTION CALL
COUTX:   POP   BP         ; RESTORE BP
         JMP   CNEXT      ; AND CONTINUE

;
;        CGET - GET INPUT STRING
;
CGET:    POP   DX         ; GET STRING ADDRESS
         MOV   AH,10      ; GET BUFFERED INPUT
         PUSH  BP         ; SAVE BP AROUND CALL
         INT   33         ; DOS FUNCTION CALL
         POP   BP         ; RESTORE BP
         JMP   CNEXT      ; AND CONTINUE

;
;        TCVC - CONVERT CHARACTER
;
CCVC:    POP   AX         ; GET NEXT CHARACTER
         PUSH  CX         ; SAVE CODE POINTER
         SUB   BX,BX      ; CHARACTER VALUE
         MOV   CX,16      ; LOOP COUNT

CVC1:    MOV   AH,CVTTAB[BX]; ADDRESS CHARACTER
         CMP   AL,AH      ; CHARACTER FOUND?
         JE    CVC2       ; YES, CONTINUE
         INC   BX         ; INCREMENT INDEX
         LOOP  CVC1       ; AND CONTINUE SCAN
CVCEX:   POP   CX         ; RESTORE CODE POINTER
         SUB   AX,AX      ; INDICATE ERROR
         PUSH  AX
         JMP   CNEXT      ; AND CONTINUE

CVC2:    CMP   BL,BASE    ; IS VALUE VALID?
         JNL   CVCEX      ; NO, INDICATE ERROR
         POP   CX         ; RESTORE CODE POINTER
         PUSH  BX         ; SAVE CONVERTED VALUE
         MOV   AX,1       ; INDICATE SUCESS
         PUSH  AX
         JMP   CNEXT      ; AND CONTINUE
         PAGE
;
;        CAUXPUSH - ADD TO AUXILLIARY STACK
;
CAUXPUSH:MOV   SI,AUXSTACK; GET STACK INDEX
         INC   SI         ; INCREMENT STACK INDEX
         INC   SI         ; INCREMENT STACK INDEX
         MOV   AUXSTACK,SI; AND RESET IT
         POP   AX         ; GET CURRENT TOP
         MOV   AUXSTACK[SI],AX; ADD TO AUXILLIARY STACK
         JMP   CNEXT

;
;        CAUXPOP - REMOVE FROM AUXILLIARY STACK
;
CAUXPOP: MOV   SI,AUXSTACK; GET STACK INDEX
         MOV   AX,AUXSTACK[SI]; GET TOP OF STACK
         DEC   SI         ; DECREMENT STACK INDEX
         DEC   SI         ; DECREMENT STACK INDEX
         MOV   AUXSTACK,SI; AND RESET IT
CPUSH:   PUSH  AX         ; ADD AX TO TOP OF STACK
         JMP   CNEXT      ; AND CONTINUE

;
;        CV@FORT - TEST FOR LOOP TERMINATION
;
CV@FORT: MOV   SI,AUXSTACK; GET CURRENT STACK INDEX
         MOV   AX,AUXSTACK[SI]; GET CURRENT AUX ENTRY
         INC   AX         ; INCREMENT IT
         CMP   AX,AUXSTACK-2[SI]; END OF LOOP?
         JNL   FORTEND    ; YES, TERMINATE LOOP

         MOV   AUXSTACK[SI],AX; SET UPDATED STACK ENTRY
         XOR   AX,AX      ; INDICATE LOOP CONTINUES
         JMP   CPUSH      ; AND CONTINUE

FORTEND: SUB   AUXSTACK,4 ; DECREMENT STACK INDEX
         MOV   AX,1       ; INDICATE END OF LOOP
         JMP   CPUSH      ; AND CONTINUE

;
;        CV@I - GET INDEX VALUE
;        CV@J - GET INDEX VALUE
;        CV@K - GET INDEX VALUE
;
CV@I:    MOV   SI,AUXSTACK; GET CURRENT STACK INDEX
         MOV   AX,AUXSTACK-0[SI]; GET INDEX VALUE
         JMP   CPUSH      ; AND CONTINUE

CV@J:    MOV   SI,AUXSTACK; GET CURRENT STACK INDEX
         MOV   AX,AUXSTACK-4[SI]; GET INDEX VALUE
         JMP   CPUSH      ; AND CONTINUE

CV@K:    MOV   SI,AUXSTACK; GET CURRENT STACK INDEX
         MOV   AX,AUXSTACK-8[SI]; GET INDEX VALUE
         JMP   CPUSH      ; AND CONTINUE

         INCLUDE TLCGOTO.ASM; BRANCH LOGIC
         INCLUDE TLCTERM.ASM; TERMINATION CODE
MAINLINE ENDP
SEG@CS   ENDS
         END   INITIAL
