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
#        tlcvars.m4
#
# Purpose-
#        Threaded Language Compiler: Variables
#
# Notes-
#        Read/write
#
##############################################################################
         .align 4
#
#        Controls
#
BASEGET: tword  TCALL,TBASE,TPEEKC,TEXIT
BASESET: tword  TCALL,TBASE,TPOKEC,TEXIT

PADV0:   tword  TCALL,TPAD,TPEEKC,TEXIT
PADS0:   tword  TCALL,TPAD,TPOKEC,TEXIT

IMAGEA1: tword  TCON,IMAGE-1
IMAGEA2: tword  TCON,IMAGE-0
IMAGEV0: tword  TCALL,IMAGEA0,TPEEKC,TEXIT
IMAGEV1: tword  TCALL,IMAGEA1,TPEEKC,TEXIT
IMAGES0: tword  TCALL,IMAGEA0,TPOKEC,TEXIT
IMAGES1: tword  TCALL,IMAGEA1,TPOKEC,TEXIT

OUTS.GETL:                          # Get length of OUTS
         tword  TCALL,OUTS,TPEEKC,TEXIT
OUTS.SETL:                          # Set length of OUTS
         tword  TCALL,OUTS,TPOKEC,TEXIT

MEMGET:  tword  TCALL,MEMORY,TPEEKW,TEXIT
MEMSET:  tword  TCALL,MEMORY,TPOKEW,TEXIT

#
#        Variables
#
TBASE:   tword  TVAR                # Current base, range 2..36
BASE:    .byte  10, 0, 0, 0         # Base, pad, pad, pad

TPAD:    tword  TVAR                # Input word
         .space 512

IMAGEA0: tword  TVAR                # Input text line
         .byte  0, 0
IMAGE:   .space SIZE_IMAGE
         .align 4

CFLAG:   tword  TVAR                # (FALSE) if compiling
         .long  0

##
##        Variables used in numeric conversions
##
OUTS:    tword  TVAR                # Output string
         .space 40                  # (Large enough for base(2))

CVIVAL:  tword  TVAR                # Input value (absolute)
         .long  0

CVISGN:  tword  TVAR                # (sign) CVIVAL
         .long  0

##
##        Memory control words
##
MEMORG:  tword  TCON                # Memory origin
         .long  0-0
MEMOLD:  tword  TVAR                # Prior value for MEMORY
         .long  0-0
MEMORY:  tword  TVAR                # The current compilation address
         .long  0-0
MEMEND:  tword  TCON                # The end of available memory
         .long  0-0

##
##        Dynamic dictionary origin
##
TSDICT:  tword  TVAR                # VAR TSDICT
         .long  DS_CMPL

##
##       Dynamic data controls
##
DICTIONARY:                         # Dictionary (MEMORY)
         .long  16777216            # (Length)
         .long  (0-0)               # (Address)

DATASTACK:                          # Data stack
         .long  16777216            # (Length)
         .long  (0-0)               # (Address)

CALLSTACK:                          # Call stack
         .long  65536               # (Length)
         .long  (0-0)               # (Address)

COMPSTACK:                          # Compiler stack
         .long  65536               # (Length)
         .long  (0-0)               # (Address)

