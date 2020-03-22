         .file   "tlc.S"
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
##
## Title-
##       tlc.S
##
## Purpose-
##       Threaded Language Compiler
##
## Register assignments-
##       ebp   dStack: Data Stack
##       ebx   iAddr:  Instruction Address
##       ecx   cStack: Call Stack
##       edx   iWord:  Instruction word (temporary)
##
##############################################################################
#include "H/standard.m4"            /* Standard includes                    */
#include "tlcmacs.m4"               /* Macros                               */

##############################################################################
#
#        Macros
#
#if 1
define(MDEBUG, ` ###### macro DEBUGGING()
         call  DEBUGALL
') dnl

#else
define(MDEBUG, ` ###### macro DEBUGGING()
') dnl
#endif

##############################################################################
#
#        Externals
#
#ifndef _OS_CYGWIN
#ifdef _OS_BSD
#define _tlc      tlc
#define _getchar  getchar
#define _getSTDIN getSTDIN
#define _malloc   malloc
#define _printf   printf
#define _putchar  putchar
#define _strlen   strlen
#define _snap     snap
#define _debugf   debugf
#define _tracef   tracef
#define _setIntensiveMode setIntensiveMode
#endif
#endif

         .globl  _tlc

         .extern _getchar           ## C-library: getchar()
         .extern _malloc            ## C-library: malloc()
         .extern _printf            ## C-library: printf()
         .extern _putchar           ## C-library: putchar()
         .extern _strlen            ## C-library: strlen()

         .extern _getSTDIN          ## MAIN: fgets(addr, size, stdin)
         .extern _snap              ## DEBUG: snap()
         .extern _debugf            ## DEBUG: debugf()
         .extern _tracef            ## DEBUG: tracef()
         .extern _setIntensiveMode  ## DEBUG: setIntensiveMode

##############################################################################
#
#        Equates
#
         .set   CH_LF, 10           ## LineFeed character
         .set   CH_NL, 13           ## NewLine character
         .set   CH_SP, 32           ## Space character

##############################################################################
#
# Subsection-
#        TEXT
#
# Function-
#        Executable code.
#
##############################################################################
         .text
         .align 4096
CODEAREA:

#
#        Entry linkage
#
_tlc:
         pushl  %ebp
         movl   %esp, %ebp
         movl   %ebp, saveEBP

DEBUGGING(`
##################################### Debugging
######## call   _setIntensiveMode

         movl   ptrData, %eax
         pushl  %eax
         movl   ptrCode, %eax
         pushl  %eax
         pushl  $string.debugINIT
         call   __debug
         addl   $12, %esp
##################################### Debugging
')

#if 0
DEBUGGING(`
##################################### Debugging
         pushl  $(CODEENDS-CODEAREA)
         pushl  $CODEAREA
         call   _snap
         addl   $8, %esp

         pushl  $(DATAENDS-DATAAREA)
         pushl  $DATAAREA
         call   _snap
         addl   $8, %esp
##################################### Debugging
')
#endif

#
#        Allocate working storage
#
         pushl  DICTIONARY+0        # Sizeof(dictionary)
         call   _malloc             # Allocate storage
         addl   $4,%esp
         movl   %eax, DICTIONARY+WORDSIZE # Set dictionary origin
         test   %eax, %eax          # Any storage available
         jz     noStorageExit       # No, exit

         movl   %eax, MEMORG+WORDSIZE # Dynamic address origin
         movl   %eax, MEMORY+WORDSIZE # Dynamic address origin
         addl   DICTIONARY+0, %eax  # Dynamic address size
         movl   %eax, MEMEND+WORDSIZE # End of storage

         pushl  DATASTACK+0         # Sizeof(data stack)
         call   _malloc             # Allocate storage
         addl   $4,%esp
         movl   %eax, DATASTACK+WORDSIZE # Set data stack origin
         test   %eax, %eax          # Any storage available
         jz     noStorageExit       # No, exit

         pushl  CALLSTACK+0         # Sizeof(call stack)
         call   _malloc             # Allocate storage
         addl   $4,%esp
         movl   %eax, CALLSTACK+WORDSIZE # Set call stack origin
         test   %eax, %eax          # Any storage available
         jz     noStorageExit       # No, exit

         pushl  COMPSTACK+0         # Sizeof(comp stack)
         call   _malloc             # Allocate storage
         addl   $4,%esp
         movl   %eax, COMPSTACK+WORDSIZE # Set comp stack origin
         test   %eax, %eax          # Any storage available
         jz     noStorageExit       # No, exit

#
#        CRESET- Initialize/Reset the environment
#
CRESET:
         movl   $TH_MAIN, iAddr     # Address the Main thread
         movl   CALLSTACK+WORDSIZE, cStack # Restore the call stack
         movl   COMPSTACK+WORDSIZE, %eax # Restore the compiler stack
         movl   %eax, xStack

#
#        CCLEAR - CLEAR THE DATA STACK
#
CCLEAR:
         movl   DATASTACK+WORDSIZE, dStack # Restore the data stack
         xorl   %eax, %eax          # (Constant)
         decl   %eax                # (-1)
         dPush(%eax)
         dPush(%eax)
         dPush(%eax)
         dPush(%eax)
         jmp    CNEXT               # Next interpreted instruction

#
#        No Storage Exit
#
noStorageExit:
         pushl  string.nostorage
         call   _printf
         addl   $4,%esp
######## jmp    CQUIT

#
#        Return to caller
#
CQUIT:
         movl   saveEBP, %ebp
         xorl   %eax, %eax
         movl   %ebp, %esp
         popl   %ebp
         ret

#
#        Debugging trap point
#
CNOP:
CSTOP:
         nop                        # Debugging trap point
         jmp    CNEXT               # Continue code execution

##############################################################################
#
#        TCALL - Thread Call
#
TCALL:
DEBUGGING(`
##################################### Debugging
         pushALL

         pushl  iWord
         pushl  $string.debugTCALL
         call   __debug
         addl   $8,%esp

         popALL
         pushALL

         pushl  iAddr
         pushl  cStack
         pushl  $string.debugSTACK
         call   __debug
         addl   $12,%esp

         popALL
##################################### Debugging
')

         movl   iAddr, (cStack)     # Push value onto stack
         addl   $(WORDSIZE), cStack # Grow the stack
         leal   WORDSIZE(iWord), iAddr # New instruction address

######## jmp    CNEXT               # Get next thread word

##############################################################################
#
#        CNEXT - GET NEXT THREAD INSTRUCTION
#
CNEXT:
DEBUGGING(`
##################################### Debugging
         pushALL

         pushl  iAddr
         pushl  $string.debugIaddr
         call   __debug
         addl   $8,%esp

         popALL
         pushALL

         pushl  0(iAddr)
         pushl  $string.debugIword
         call   __debug
         addl   $8,%esp

         popALL
         pushALL

         movl   0(iAddr), iWord
         pushl  (iWord)
         pushl  $string.debugInstr
         call   __debug
         addl   $8,%esp

         popALL
         pushALL

         dFetch(%eax)
         pushl  %eax
         pushl  dStack
         pushl  $string.debugSTACK
         call   __debug
         addl   $12,%esp

         popALL
##################################### Debugging
')

         movl   (iAddr), iWord      # Current instruction word address
         addl   $(WORDSIZE), iAddr  # Bump the instruction counter
         movl   (iWord), %eax       # Next instruction address
         jmp    *%eax               # Next instruction

##############################################################################
#
#        CEXIT - EXIT FROM THREAD
#
CEXIT:
         subl   $(WORDSIZE), cStack # Shrink the stack
         movl   (cStack), iAddr     # Pop instruction address

DEBUGGING(`
##################################### Debugging
         pushALL

         pushl  iAddr
         pushl  $string.debugCEXIT
         call   __debug
         addl   $8, %esp

         popALL
         pushALL

         pushl  iAddr
         pushl  cStack
         pushl  $string.debugSTACK
         call   __debug
         addl   $12, %esp

         popALL
##################################### Debugging
')

         jmp    CNEXT

##############################################################################
#
#        CEXEC - EXECUTE INSTRUCTION ADDRESSED BY TOP OF STACK
#
CEXEC:
DEBUGGING(`
##################################### Debugging
         pushALL

         dFetch(%eax)
         pushl  (%eax)
         pushl  %eax
         pushl  $string.debugCEXEC
         call   __debug
         addl   $12, %esp

         popALL
         pushALL

         dFetch(%eax)
         pushl  %eax
         pushl  dStack
         pushl  $string.debugSTACK
         call   __debug
         addl   $12, %esp

         popALL
##################################### Debugging
')

         dPop(iWord)                # Get thread address
         movl   (iWord), %eax       # Get code address
         jmp    *%eax               # (Next instruction)

##############################################################################
#
#        IEXEC - EXECUTE INSTRUCTION FOLLOWING CURRENT INSTRUCTION
#
IEXEC:
DEBUGGING(`
##################################### Debugging
         pushALL

         pushl  (iAddr)
         pushl  iAddr
         pushl  $string.debugIEXEC
         call   __debug
         addl   $12, %esp

         popALL
##################################### Debugging
')

         movl   (iAddr), %eax       # Current Machine instruction
         jmp    *%eax

##############################################################################
#
#        CGOTO - PROCESS THREADED GOTO
#
CGOTO:
DEBUGGING(`
##################################### Debugging
         pushALL

         pushl  (iAddr)
         pushl  iAddr
         pushl  $string.debugCGOTO
         call   __debug
         addl   $12, %esp

         popALL
##################################### Debugging
')

         movl   (iAddr), iAddr      # Set new instruction address
         jmp    CNEXT

#
#        CIFEQZ - IF (TOP) == 0, PROCESS GOTO FUNCTION
#
CIFEQZ:
         dPop(%eax)                 # Pop (top) value
         cmpl   $0, %eax            # Compare :: (0)
         jz     CGOTO               # EQ, process as GOTO
######## jmp    CSKIP

CSKIP:
         addl   $(WORDSIZE), iAddr  # Skip GOTO target
         jmp    CNEXT

#
#        CIFNEZ - IF (TOP) != 0, PROCESS GOTO FUNCTION
#
CIFNEZ:
         dPop(%eax)                 # Pop (top) value
         cmpl   $0, %eax            # Compare :: (0)
         jnz    CGOTO               # NE, process as GOTO
         jmp    CSKIP

#
#        CIFLTZ - IF (TOP) <  0, PROCESS GOTO FUNCTION
#
CIFLTZ:
         dPop(%eax)                 # Pop (top) value
         cmpl   $0, %eax            # Compare :: (0)
         jnl    CSKIP               # LT, process as GOTO
         jmp    CGOTO

#
#        CIFLEZ - IF (TOP) <= 0, PROCESS GOTO FUNCTION
#
CIFLEZ:
         dPop(%eax)                 # Pop (top) value
         cmpl   $0, %eax            # Compare :: (0)
         jng    CGOTO               # LE, process as GOTO
         jmp    CSKIP

#
#        CIFGEZ - IF (TOP) >= 0, PROCESS GOTO FUNCTION
#
CIFGEZ:
         dPop(%eax)                 # Pop (top) value
         cmpl   $0, %eax            # Compare :: (0)
         jnl    CGOTO               # GE, process as GOTO
         jmp    CSKIP

#
#        CIFEQ - IF (TOP-1) == (TOP), PROCESS GOTO FUNCTION
#
CIFEQ:
         dPop(%eax)                 # Pop (top) value
         dPop(iWord)                # Pop (top) value
         cmpl   %eax, iWord         # Compare
         je     CGOTO               # EQ, process as GOTO
         jmp    CSKIP

#
#        CIFNE - IF (TOP-1) != (TOP), PROCESS GOTO FUNCTION
#
CIFNE:
         dPop(%eax)                 # Pop (top) value
         dPop(iWord)                # Pop (top) value
         cmpl   %eax, iWord         # Compare
         jne    CGOTO               # EQ, process as GOTO
         jmp    CSKIP

#
#        CIFLT - IF (TOP-1) <  (TOP), PROCESS GOTO FUNCTION
#
CIFLT:
         dPop(%eax)                 # Pop (top) value
         dPop(iWord)                # Pop (top) value
         cmpl   %eax, iWord         # Compare
         jl     CGOTO               # LT, process as GOTO
         jmp    CSKIP

#
#        CIFGE - IF (TOP-1) >= (TOP), PROCESS GOTO FUNCTION
#
CIFGE:
         dPop(%eax)                 # Pop (top) value
         dPop(iWord)                # Pop (top) value
         cmpl   %eax, iWord         # Compare
         jnl    CGOTO               # GE, process as GOTO
         jmp    CSKIP

#
#        CIFLTL - IF (unsigned)(TOP-1) < (unsigned)(TOP), PROCESS GOTO FUNCTION
#
CIFLTL:
         dPop(%eax)                 # Pop (top) value
         dPop(iWord)                # Pop (top) value
         cmpl   %eax, iWord         # Compare
         jb     CGOTO               # LT, process as GOTO
         jmp    CSKIP

#
#        CIFGEL - IF (unsigned)(TOP-1) >= (unsigned)(TOP), PROCESS GOTO FUNCTION
#
CIFGEL:
         dPop(%eax)                 # Pop (top) value
         dPop(iWord)                # Pop (top) value
         cmpl   %eax, iWord         # Compare
         jae    CGOTO               # GE, process as GOTO
         jmp    CSKIP

#
#        TCON - INTEGER CONSTANT
#
TCON:
         movl   WORDSIZE(iWord), %eax # Get the constant
         dPush(%eax)                # And put it on the stack
         jmp    CNEXT

#
#        TVAR - INTEGER VARIABLE
#
TVAR:
         leal   WORDSIZE(iWord), %eax # Get the variable's address
         dPush(%eax)                # And put it on the stack
         jmp    CNEXT

#
#        CIMMW - PUSH THE NEXT PROGRAM WORD ONTO THE STACK
#
CIMMW:
         mov    (iAddr), %eax       # Get the value
         addl   $(WORDSIZE), iAddr  # Skip past the value
         dPush(%eax)                # Put the value on the stack
         jmp    CNEXT

#
#        CPEEKW - WORD LOAD
#
CPEEKW:
         dFetch(iWord)              # Get address
         movl  (iWord), %eax        # Load variable
         dStore(%eax)               # Store result
         jmp    CNEXT

#
#        CPOKEW - WORD STORE
#
CPOKEW:
         dPop(iWord)                # Get address
         dPop(%eax)                 # Get value
         movl   %eax, (iWord)       # Store variable
         jmp    CNEXT

#
#        CPEEKC - BYTE LOAD
#
CPEEKC:
         dFetch(iWord)              # Get address
         xor    %eax, %eax          # Initialize to zero
         movb   (iWord), %al        # Load variable
         dStore(%eax)               # Store result
         jmp    CNEXT

#
#        CPOKEC - BYTE STORE
#
CPOKEC:
         dPop(iWord)                # Get address
         dPop(%eax)                 # Get value
         movb   %al, (iWord)        # Store variable
         jmp    CNEXT

#
#        CDUP - DUPLICATE THE TOP ENTRY ON THE STACK
#
CDUP:
         dFetch(%eax)               # Get the top word
         dPush(%eax)                # Duplicate it on the stack
         jmp    CNEXT

#
#        COVER - COPY THE NEXT TO THE TOP ELEMENT TO THE TOP OF THE STACK
#
COVER:
         dPop(iWord)
         dFetch(%eax)
         dPush(iWord)
         dPush(%eax)
         jmp    CNEXT

#
#        CSWAP - SWAP THE TOP TWO ELEMENTS ON THE STACK
#
CSWAP:
         dPop(iWord)
         dFetch(%eax)
         dStore(iWord)
         dPush(%eax)
         jmp    CNEXT

#
#        CPOP - REMOVE THE TOP ENTRY FROM THE STACK
#
CPOP:
         dPop(%eax)                 # Discard top
         jmp    CNEXT

#
#        CINC - INCREMENT TOP
#
CINC:
         incl   (dStack)
         jmp    CNEXT

#
#        CDEC - DECREMENT TOP
#
CDEC:
         decl   (dStack)
         jmp    CNEXT

#
#        CADD: a= pop[TOP]; b= pop[TOP-1]; push[a + b]
#
CADD:
         dPop(%eax)
         addl   %eax, (dStack)
         jmp    CNEXT

#
#        CSUB: a= pop[TOP]; b= pop[TOP-1]; push[b - a]
#
CSUB:
         dPop(%eax)
         subl   %eax, (dStack)
         jmp    CNEXT

#
#        CMUL: a= pop[TOP]; b= pop[TOP-1]; push[a * b]
#
CMUL:
         dPop(iWord)
         dFetch(%eax)
         imull  iWord, %eax
         dStore(%eax)
         jmp    CNEXT

#
#        CDIV: a= pop[TOP]; b= pop[TOP-1]; push[b / a]
#
CDIV:
         pushl  %ebx
         pushl  %ecx
         pushl  %edx

         dPop(%ecx)
         dFetch(%eax)
         cdq
         idivl  %ecx
         dStore(%eax)

         popl   %edx
         popl   %ecx
         popl   %ebx
         jmp    CNEXT

#
#        CMOD: a= pop[TOP]; b= pop[TOP-1]; push[b % a]
#
CMOD:
         pushl  %ebx
         pushl  %ecx
         pushl  %edx

         dPop(%ecx)
         dFetch(%eax)
         cdq
         idivl  %ecx
         dStore(%edx)

         popl   %edx
         popl   %ecx
         popl   %ebx
         jmp    CNEXT

#
#        CDIVR: a= pop[TOP]; b= pop[TOP-1]; push[b / a]; push[b % a]
#
CDIVR:
         pushl  %ebx
         pushl  %ecx
         pushl  %edx

         dPop(%ecx)
         dFetch(%eax)
         cdq
         idivl  %ecx
         dStore(%eax)
         dPush(%edx)

         popl   %edx
         popl   %ecx
         popl   %ebx
         jmp    CNEXT

#
#        CAND - INTEGER AND
#
CAND:
         dPop(%eax)
         andl   %eax, (dStack)
         jmp    CNEXT

#
#        COR - INTEGER OR
#
COR:
         dPop(%eax)
         orl    %eax, (dStack)
         jmp    CNEXT

#
#        CXOR - INTEGER XOR
#
CXOR:
         dPop(%eax)
         xorl   %eax, (dStack)
         jmp    CNEXT

#
#        CNEG - INTEGER NEGATE
#
CNEG:
         negl   (dStack)
         jmp    CNEXT

#
#        CNOT - INTEGER NOT
#
CNOT:
         notl   (dStack)
         jmp    CNEXT

#
#        CMAXF - MAXIMUM(TOP,TOP-1)
#
CMAXF:
         dPop(%eax)
         dFetch(iWord)
         cmpl   iWord, %eax
         jge    CMAX100
         jmp    CNEXT

CMAX100:
         dStore(%eax)
         jmp    CNEXT

#
#        CMINF - MINIMUM(TOP,TOP-1)
#
CMINF:
         dPop(%eax)
         dFetch(iWord)
         cmpl   iWord, %eax
         jle    CMIN100
         movl   iWord, %eax

CMIN100:
         dStore(%eax)
         jmp    CNEXT

#
#        CABSF - ABS(TOP)
#
CABSF:
         dFetch(%eax)
         cmpl   $0, %eax
         jge    CNEXT

         neg    %eax
         dStore(%eax)
         jmp    CNEXT

##############################################################################
#
# WARNING!: CAUXPUSH, CAUXPOP, CV_FORT, CV_I, CV_J and CV_K
#        are NOT tested, nor are the corresponding thread words.
#
#        Note that CAUXPUSH is called but not CAUXPOP!!
#
##############################################################################

#
#        CAUXPUSH - ADD TO AUXILLIARY STACK
#
CAUXPUSH:
MDEBUG() #
         dPop(%eax)
MDEBUG() #
         movl   xStack, iWord
MDEBUG() #
         subl   $(WORDSIZE), iWord
MDEBUG() #
         movl   iWord, xStack
MDEBUG() #
         movl   %eax, (iWord)
MDEBUG() #
         jmp    CNEXT

#
#        CAUXPOP - REMOVE FROM AUXILLIARY STACK
#
CAUXPOP:
MDEBUG() #
         movl   xStack, iWord
MDEBUG() #
         movl   (iWord), %eax
MDEBUG() #
         addl   $(WORDSIZE), iWord
MDEBUG() #
         movl   iWord, xStack
MDEBUG() #
         dPush(%eax)
MDEBUG() #
         jmp    CNEXT

#
#        CV_FORT - TEST FOR LOOP TERMINATION
#
CV_FORT:
         movl   xStack, iWord
         movl   (iWord), %eax
         incl   %eax
         cmpl   -WORDSIZE(iWord), %eax
         jnl    FORTEND

         movl   %eax, (iWord)
         dPush($0)
         jmp    CNEXT

FORTEND:
         subl   $(-1), iWord
         movl   iWord, xStack
         dPush($1)
         jmp    CNEXT

#
#        CV_I - GET INDEX VALUE
#        CV_J - GET INDEX VALUE
#        CV_K - GET INDEX VALUE
#
CV_I:
         movl   xStack, iWord
         movl   (iWord), %eax
         dPush(%eax)
         jmp    CNEXT

CV_J:
         movl   xStack, iWord
         movl   -WORDSIZE*2(iWord), %eax
         dPush(%eax)
         jmp    CNEXT

CV_K:
         movl   xStack, iWord
         movl   -WORDSIZE*4(iWord), %eax
         dPush(%eax)
         jmp    CNEXT

##############################################################################
#
# Thread-
#        CCLS
#
# Purpose-
#        COMPARE (TOP-1) STRING : (TOP) STRING
#
# Inputs/outputs-
#        @(string-1) @(string-0) - { (-1) | (+0) | (+1) }
#
##############################################################################
CCLS:
         pushl  %ebx
         pushl  %ecx
         pushl  %edx
         pushl  %edi
         pushl  %esi

         dPop(%esi)                 # esi= addr(string-0)
         dFetch(%edi)               # edi= addr(string-1)

         xorl   %ebx, %ebx
         xorl   %ecx, %ecx
         movb   (%esi), %bl         # ebx= length(string-0)
         movb   (%edi), %cl         # ecx= length(string-1)
         cld
         cmpsb                      # Compare string lengths
         jl     CCLS110             # (string-0 is shorter)
         je     CCLS120             # (strings lengths are identical)

CCLS100:                            # (string-1) is shorter
         test   %ecx, %ecx          # Is string-1 empty?
         jz     CLSL                # Yes, (string-1) < (string-0)

         repz  cmpsb                # Compare strings
         jng    CLSL                # If <=, (string-1) < (string-0)
         jmp    CLSH                # If >,  (string-1) > (string-0)

CCLS110:                            # (string-0) is shorter
         movl   %ebx, %ecx          # Use its length
         test   %ecx, %ecx          # Is (string-0) empty?
         jz     CLSH                # Yes, (string-1) > (string-0)

         repz  cmpsb                # Compare strings
         jnl    CLSH                # If >=, (string-1) > (string-0)
         jmp    CLSL                # If <,  (string-1) < (string-0)

CCLS120:                            # String lengths are identical
         xor    %eax, %eax          # Default, equal
         test   %ecx, %ecx          # Are strings empty?
         jz     CLSX                # Yes, they're identical

         repz  cmpsb                # Compare strings
         jz     CLSX                # If ==, (string-1) == (string-0)
         jg     CLSH                # If >,  (string-1) >  (string-0)
######## jmp    CLSL

CLSL:                               # (string-1) < (string-0)
         movl   $(-1), %eax
         jmp    CLSX

CLSH:                               # (string-1) > (string-0)
         movl   $(+1), %eax
######## jmp    CLSX

CLSX:
         dStore(%eax)               # Set resultant

         popl   %esi
         popl   %edi
         popl   %edx
         popl   %ecx
         popl   %ebx
         jmp    CNEXT

##############################################################################
#
# Thread-
#        CMVS
#
# Purpose-
#        Copy (string-1) into (string-0)
#
# Inputs/outputs-
#        @(string-1) @(string-0) - {}
#        string-0 = string-1
#
##############################################################################
CMVS:
         pushl  %ebx
         pushl  %ecx
         pushl  %edx
         pushl  %edi
         pushl  %esi

         dPop(%esi)                 # esi= addr(string-1)
         dPop(%edi)                 # edi= addr(string-0)

         cld
         xorl   %ecx, %ecx
         movb   (%esi), %cl
         movsb

         cmpl   $0, %ecx
         jz     MVSX

         rep movsb

MVSX:
         popl   %esi
         popl   %edi
         popl   %edx
         popl   %ecx
         popl   %ebx
         jmp    CNEXT

##############################################################################
#
# Thread-
#        CCVC
#
# Purpose-
#        Convert character from base.
#
# Inputs/outputs-
#        (character) - 0            # Invalid character
#        (character) - value 1      # Valid character
#
##############################################################################
CCVC:
         pushl  %ebx
         pushl  %ecx
         pushl  %edx

         dFetch(%eax)               # Conversion character
         movl   $CVTTAB, %ebx
         xor    %ecx, %ecx          # %ecx = index
         movb   BASE, %cl

CCVC100:
         movl   %ebx, %edx
         addl   %ecx, %edx
         cmpb   -1(%edx), %al
         je     CCVC200

         loop   CCVC100
         movl   $cvttab, %ebx
         movb   BASE, %cl

CCVC110:
         movl   %ebx, %edx
         addl   %ecx, %edx
         cmpb   -1(%edx), %al
         je     CCVC200

         loop   CCVC110
         dStore(%ecx)
         jmp    CVCX

CCVC200:
         decl   %ecx
         dStore(%ecx)
         dPush($1)

CVCX:
         popl   %edx
         popl   %ecx
         popl   %ebx
         jmp    CNEXT

##############################################################################
#
# Thread-
#        CINPC
#
# Purpose-
#        Read character from console.
#        WARNING! : Not tested
#
# Inputs/outputs-
#        - (character)
#
##############################################################################
CINPC:
         pushl  %ebx
         pushl  %ecx
         pushl  %edx

         call   _getchar
         dPush(%eax)

         popl   %edx
         popl   %ecx
         popl   %ebx

DEBUGGING(`
##################################### Debugging
         pushALL

         pushl  %eax
         pushl  %eax
         pushl  $string.debugCINPC
         call   __debug
         addl   $12, %esp

         popALL
##################################### Debugging
')

         jmp    CNEXT

##############################################################################
#
# Thread-
#        COUTC
#
# Purpose-
#        Write character onto console.
#        '\n' converted into "\n\r"
#
# Inputs/outputs-
#        (character) -
#
##############################################################################
COUTC:
DEBUGGING(`
##################################### Debugging
         pushALL

         dFetch(%eax)
         pushl  %eax
         pushl  %eax
         pushl  $string.debugCOUTC
         call   __debug
         addl   $12, %esp

         popALL
##################################### Debugging
')

         dPop(iWord)

         pushALL
         pushl  iWord
         call   _putchar
         addl   $4, %esp
         popALL

         cmpl   $(CH_NL), iWord
         jne    CNEXT

         pushALL
         pushl  $(CH_LF)
         call   _putchar
         addl   $4, %esp
         popALL

         jmp    CNEXT

##############################################################################
#
# Thread-
#        CGET
#
# Purpose-
#        Get input string
#
# Inputs/outputs-
#        @string -
#        I: string[0] (maximum length)
#        O: string[1] (actual  length)
#        O: string[2] (text begins)
#
##############################################################################
CGET:
         pushl  %ebx
         pushl  %ecx
         pushl  %edx

         dPop(%ebx)
         movb   $0, 1(%ebx)
         pushl  %ebx

         xorl   %eax, %eax          # Set length
         movb   0(%ebx), %al
         pushl  %eax
         leal   2(%ebx), %eax       # Set address
         pushl  %eax
         call   _getSTDIN
         addl   $8, %esp

         popl   %ebx
         test   %eax, %eax
         jz     GETX
         pushl  %ebx

         pushl  %eax
         call   _strlen
         addl   $4, %esp

         popl   %ebx
         movb   %al, 1(%ebx)

GETX:
DEBUGGING(`
##################################### Debugging
         leal   2(%ebx), %eax
         pushl  %eax
         xor    %eax, %eax
         movb   1(%ebx), %al
         pushl  %eax
         pushl  $string.debugCGETS
         call   __debug
         addl   $12, %esp
##################################### Debugging
')

         popl   %edx
         popl   %ecx
         popl   %ebx
         jmp    CNEXT

##############################################################################
#
# Thread-
#        CEBUG1
#
# Purpose-
#        Debugging: Get caller's return address
#
# Inputs/outputs-
#        - (returnAddress)
#
##############################################################################
CEBUG1:
         movl   -WORDSIZE(cStack), %eax
         dPush(%eax)
         jmp    CNEXT

##############################################################################
#
# Thread-
#        CSNAP
#
# Purpose-
#        Debugging: Dump storage.
#
# Inputs/outputs-
#        (address) (length) -
#
##############################################################################
CSNAP:
         dPop(%eax)                 # Address
         dPop(iWord)                # Length

         pushl  iWord
         pushl  %eax
         call   _snap
         addl   $8, %esp
         jmp    CNEXT

##############################################################################
#
# Thread-
#        CTRUE/CFALSE
#
# Purpose-
#        Debugging: Print message "TRUE" or "FALSE"
#
# Inputs/outputs-
#        None
#
##############################################################################
DEBUGGING(`
##################################### Debugging
#
#        CTRUE: Debugging: TRUE
#
CTRUE:
         pushALL
         pushl  $string.debugTRUE
         call   _debugf
         addl   $4, %esp
         popALL

         jmp    CNEXT

string.debugTRUE:
         .asciz "TRUE\n";

#
#        CFALSE: Debugging: FALSE
#
CFALSE:
         pushALL
         pushl  $string.debugFALSE
         call   _debugf
         addl   $4, %esp
         popALL

         jmp    CNEXT

string.debugFALSE:
         .asciz "FALSE\n";
##################################### Debugging
')

##############################################################################
#
# Subroutine-
#        __debug
#
# Purpose-
#        Debugging: Diagnostic printf function.
#
##############################################################################
#
#        __debug: debugf || tracef || nop
#
__debug:
         ret                        # NOP    (no action)
######## jmp    _tracef             # TRACEF (trace)
######## jmp    _debugf             # DEBUGF (debug)

##############################################################################
#
# Subroutine-
#        DEBUGALL
#
# Purpose-
#        Debugging: Print all machine registers.
#
##############################################################################
DEBUGALL:
######## ret                        # Comment out for active debugging
         pushl  %ebp                # Save registers (as parameters)
         pushl  %esp
         pushl  %esi
         pushl  %edi
         pushf
         pushl  %edx
         pushl  %ecx
         pushl  %ebx
         pushl  %eax
         pushl  $string.debug
         call   _debugf

         popl   %eax                # Pop $string.debug
         popl   %eax                # Restore registers
         popl   %ebx
         popl   %ecx
         popl   %edx
         popf
         popl   %edi
         popl   %esi
         popl   %esp
         popl   %ebp
         ret                        # and return
CODEENDS:

##############################################################################
#
# Section-
#        CONSTANTS
#
# Function-
#        Define constants.
#
##############################################################################
         .data                      ## Constants
         .align 4096
DATAAREA:

#include "tlcbase.m4"               /* Code                                 */
#include "tlcvecs.m4"               /* Code vectors                         */
#include "tlcdict.m4"               /* Static dictionaries                  */

string.nostorage:
         .ascii "No Storage"
         .byte  CH_LF, 0

string.debug:
         .ascii "eax(%.8x) "
         .ascii "ebx(%.8x) "
         .ascii "ecx(%.8x) "
         .ascii "edx(%.8x) "
         .ascii "flg(%.4x) "
         .ascii "\n"
         .ascii "edi(%.8x) "
         .ascii "esi(%.8x) "
         .ascii "esp(%.8x) "
         .ascii "ebp(%.8x) "
         .ascii "ret(%.8x) "
         .byte  CH_LF, 0

string.debugGaddr:
         .ascii "addr(%.8x)"
         .byte  CH_LF, 0

string.debugIaddr:
         .ascii "CNEXT: "
         .ascii "iaddr(%.8x) "
         .byte  0

string.debugIword:
         .ascii "iword(%.8x) "
         .byte  0

string.debugInstr:
         .ascii "instr(%.8x) "
         .byte  0

string.debugCEXEC:
         .ascii "CEXEC: "
         .ascii "iword(%.8x) "
         .ascii "instr(%.8x) "
         .byte  0

string.debugCGOTO:
         .ascii "CGOTO: "
         .ascii "iaddr(%.8x) "
         .ascii "naddr(%.8x) "
         .byte  CH_LF, 0

string.debugIEXEC:
         .ascii "IEXEC: "
         .ascii "iaddr(%.8x) "
         .ascii "instr(%.8x) "
         .byte  CH_LF, 0

string.debugSTACK:
         .ascii "SP(%.8x) "
         .ascii ".(%.8x)"
         .byte  CH_LF, 0

string.debugTCALL:
         .ascii "TCALL: "
         .ascii "iaddr(%.8x) "
         .byte  0

string.debugCEXIT:
         .ascii "CEXIT: "
         .ascii "iaddr(%.8x) "
         .byte  0

string.debugCINPC:
         .ascii "CINPC: %3d '%c'<<"
         .byte  CH_LF, 0

string.debugCOUTC:
         .ascii "COUTC: %3d >>'%c'"
         .byte  CH_LF, 0

string.debugCGETS:
         .ascii "CGETS: %3d '%s'"
         .byte  CH_LF, 0

string.debugINIT:
         .ascii ".code(%.8x) "
         .ascii ".data(%.8x) "
         .byte  CH_LF, 0

         .long
ptrCode: .long  (CODEAREA)
ptrData: .long  (DATAAREA)
saveEBP: .long  (0)                 ## Saved EBP

##############################################################################
#
# Section-
#        VARIABLES
#
# Function-
#        Define variables.
#
##############################################################################
         .data                      ## Variables
         .align 4096
#include "tlcvars.m4"               /* Variables                            */

         .align 4
xStack:  .long  0                   ## Auxiliary stack

DATAENDS:

##############################################################################
#
# Dsect-
#        dictionary
#
# Function-
#        Define dictionary entry
#
##############################################################################
         .section tlcdict
dictionary:                         ## Dictionary entry
dict.next:                          ## -> Next entry (must be first)
         .long  0
dict.thread:                        ## -> Associated thread
         .long  0
dict.string:                        ## Name
         .byte  4                   ## (length(name))
dict.base:                          ## (Used in size calculation)
         .ascii "name"              ## (The actual name)
         .align 4                   ## (Next entry is on word boundary)

