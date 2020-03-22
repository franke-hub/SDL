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
#        tlcbase.m4
#
# Purpose-
#        Threaded Language Compiler: Base Code
#
# Notes-
#        Read/only
#
##############################################################################

##############################################################################
#
# Thread-
#        TH_MAIN
#
# Purpose-
#        Mainline control thread.
#
##############################################################################
TH_MAIN:
         tword  VMAIN, TPEEKW       # [VMAIN]
         tword  TEXEC               # {} ((Stack modified))
         tword  TGOTO,TH_MAIN       # {} ((Stack modified))

VMAIN:                              # VAR VMAIN
######## tword  TVAR,TH_TEST        # (If bringup testing)
         tword  TVAR,TH_SCAN        # (Production thread)

##############################################################################
#
# Thread-
#        TH_SCAN
#
# Purpose-
#        Console scan loop.
#
# Usage-
#        ((Stack modified))
#
##############################################################################
TH_SCAN: tword  TCALL               # {}
         tword  TIMMW,SCANM1        # (@(SCANM1))
         tword  TH_PUT              # {}
         tword  TH_NXTL             # {} ((initializes IMAGE))

SCAN1:
         tword  TH_NXTW             # (0) | (@PAD[0]) (1)
         tword  TIFEQZ,TH_EXIT      # {} | (@PAD[0])
         tword  TH_CMNT             # (@PAD[0]) (0) | (1)
         tword  TIFNEZ,SCAN1        # (@PAD[0])
         tword  TSDICT, TPEEKW      # (@PAD[0]) (@DS_CMPL)
         tword  TH_LOOK             # (@THREAD) (1) | (@PAD[0]) (0)
         tword  TIFNEZ,SCAN2        # (@THREAD) | (@PAD[0])
         tword  TH_CVI              # (value) (-1) | (@PAD[0]) (0)
         tword  TIFNEZ,SCAN1        # (value) | (@PAD[0])
         tword  TIMMW,SCANM2        # (@PAD[0]) (@SCANM2)
         tword  TH_PUT              # >> "\nUNDEFINED SYMBOL: "
         tword  TH_PUT              # >> @PAD[0]
TH_EXIT: tword  TEXIT               # {} ((remainder of line discarded))

SCAN2:                              # (@THREAD)
         tword  TEXEC               # {}
         tword  TGOTO,SCAN1         # {}

########                            # Constants used in TH_SCAN
SCANM1:  .byte  5,CH_NL             # 5, "\nTLC\n"
         .ascii "TLC"
         .byte  CH_NL

SCANM2:  .byte  19,CH_NL            # 19, "\nUNDEFINED SYMBOL: "
         .ascii "UNDEFINED SYMBOL: "
         .align 4

##############################################################################
#
# Thread-
#        TH_NXTL
#
# Purpose-
#        Get new input line.
#
# Usage-
#        -
#
# IMAGE-
#        [0] Size
#        [1] Used
#        [2] Text
#
##############################################################################
TH_NXTL: tword  TCALL               # {}
         tword  TIMMW,SIZE_IMAGE    # (sizeof(image))
         tword  IMAGES0             # ((IMAGE[0]= sizeof(image)))
         tword  IMAGEA0             # (@IMAGEA0)
         tword  TGET                # {} ((Sets IMAGEA0[1]))
         tword  TH_CR               # {}
         tword  TCV00               # (0)
         tword  IMAGES0             # {} ((IMAGE[0]= 0))
         tword  TEXIT               # {}

##############################################################################
#
# Thread-
#        TH_GETW
#
# Purpose-
#        Get next word in input image buffer, reading lines as required.
#
# Usage-
#        - (@PAD[0])
#        ((See TH_NXTW))
#
##############################################################################
TH_GETW: tword  TCALL               # {}

GETW1:   tword  TH_NXTW             # (0) | (@PAD[0]) (1)
         tword  TIFNEZ,TH_EXIT      # {} | (@PAD[0])
         tword  TH_NXTL             # {} ((Read next line))
         tword  TGOTO,GETW1         # {}

##############################################################################
#
# Thread-
#        TH_NXTW
#
# Purpose-
#        Get next word in input image buffer.
#
# Usage-
#        - {(0)} | {(@PAD[0]) (1)}
#
# IMAGE-
#        [0] Used (updated)
#        [1] Size
#        [2] Text
#
# PAD-
#        [0] Size (set)             # Number of bytes in output word
#        [1] Text (set)             # The next output word
#
##############################################################################
TH_NXTW: tword  TCALL               # {}
NXTW1:   tword  TH_NXTC             # (0) | (CHAR) (1)
         tword  TIFEQZ,NXTW5        # {} | (CHAR)
         tword  TDUP                # (CHAR) (CHAR)
         tword  TIMMW,(' ')         # (CHAR) (CHAR) (' ')
         tword  TIFNE,NXTW2         # (CHAR)
         tword  TPOP                # {}
         tword  TGOTO,NXTW1         # {}

NXTW2:   tword  TCV00               # (CHAR) (0)
         tword  PADS0               # (CHAR) ((pad.size= 0))
NXTW3:   tword  PADV0               # (CHAR) (pad.size)
         tword  TINC                # (CHAR) (pad.size+1)
         tword  TDUP                # (CHAR) (pad.size+1) (pad.size+1)
         tword  PADS0               # (CHAR) (pad.size) ((pad.size= (pad.size+1)))
         tword  TPAD                # (CHAR) (pad.size) (@PAD[0])
         tword  TADD                # (CHAR) (@PAD[pad.size])
         tword  TPOKEC              # {}
         tword  TH_NXTC             # (0) | (CHAR) (1)
         tword  TIFEQZ,NXTW4        # {} | (CHAR)
         tword  TDUP                # (CHAR) (CHAR)
         tword  TIMMW,(' ')         # (CHAR) (CHAR) (' ')
         tword  TIFNE,NXTW3         # (CHAR)
         tword  TPOP                # {}
NXTW4:   tword  TPAD                # (@PAD[0])
         tword  TCV01               # (@PAD[0]) (1)
         tword  TEXIT               # (@PAD[0]) (1)

NXTW5:   tword  TCV00               # (0)
         tword  TEXIT               # (0)

##############################################################################
#
# Thread-
#        TH_NXTC
#
# Purpose-
#        Get next character in input image buffer.
#
# Usage-
#        - {(0)} | {(CHAR) (1)}
#
# IMAGE-
#        [0] Used (updated)
#        [1] Size
#        [2] Text
#
##############################################################################
TH_NXTC: tword  TCALL               # {}
         tword  IMAGEV0             # (used)
         tword  IMAGEV1             # (used) (size)
         tword  TIFGE,NXCH1         # {}
         tword  IMAGEV0             # (used)
         tword  IMAGEA2             # (used) (@IMAGE[2])
         tword  TADD                # (@IMAGE[used+2])
         tword  TPEEKC              # (CHAR)
         tword  IMAGEV0             # (CHAR) (used)
         tword  TINC                # (CHAR) (used+1)
         tword  IMAGES0             # (CHAR) ((used= (used+1)))
         tword  TCV01               # (CHAR) (1)
         tword  TEXIT               # (CHAR) (1)

NXCH1:                              # {}
         tword  TCV00               # (0)
         tword  TEXIT               # (0)

##############################################################################
#
# Thread-
#        TH_CVI
#
# Purpose-
#        Convert string to integer using current base.
#
# Usage-
#        (@STRING) - {(value) (-1)} | {(@STRING) (0)}
#
##############################################################################
TH_CVI:  tword  TCALL               # (@STRING)
         tword  TCV00,CVISGN,TPOKEC # (@STRING) ((CVISGN= 0))
         tword  TCV00,CVIVAL,TPOKEW # (@STRING) ((CVIVAL= 0))
         tword  TDUP                # (@STRING) (@STRING)
         tword  TDUP                # (@STRING) (@STRING) (@STRING)
         tword  TPEEKC              # (@STRING) (@STRING) (length)
         tword  TH_GNC              # (@STRING) {(-1) | (@STRING+1) (length-1) (CHAR) (0)}
         tword  TIFEQZ,CVIEX        # (@STRING) {{} | (@STRING+1) (length-1) (CHAR)}
         tword  TDUP                # (@STRING) (@STRING+1) (length-1) (CHAR) (CHAR)
         tword  TIMMW,('+')         # .. (length-1) (CHAR) (CHAR) ('+')
         tword  TIFEQ,CVI1          # (@STRING) (@STRING+1) (length-1) (CHAR)
         tword  TDUP                # .. (length-1) (CHAR) (CHAR)
         tword  TIMMW,('-')         # .. (length-1) (CHAR) (CHAR) ('-')
         tword  TIFNE,CVI2          # .. (length-1) (CHAR)
         tword  TCVFF               # .. (length-1) (CHAR) (-1)
         tword  CVISGN              # .. (length-1) (CHAR) (-1) (@CVISGN)
         tword  TPOKEC              # (@STRING) (@STRING+1) (length-1) (CHAR)
CVI1:    tword  TPOP                # (@STRING) (@STRING+1) (length-1)
         tword  TH_GNC              # (@STRING) {(0) | (@STRING+1) (length-1) (CHAR) (-1)}
         tword  TIFEQZ,CVIEX        # (@STRING) {{} | (@STRING+1) (length-1) (CHAR)}
CVI2:    tword  TCVC                # (@STRING) (@STRING+1) (length-1) {(0) | (c-value) (1)}
         tword  TIFEQZ,CVIE1        # (@STRING) (@STRING+1) (length-1) {{} | (c-value)}
         tword  CVIVAL,TPEEKW       # (@STRING) (@STRING+1) (length-1) (c-value) [CVIVAL]
         tword  TBASE,TPEEKC        # (@STRING) (@STRING+1) (length-1) (c-value) [CVIVAL] [BASE]
         tword  TMUL                # (@STRING) (@STRING+1) (length-1) (c-value) ([CVIVAL]*[BASE])
         tword  TADD                # (@STRING) (@STRING+1) (length-1) (c-value)+([CVIVAL]*[BASE])
         tword  CVIVAL,TPOKEW       # (@STRING) (@STRING+1) (length-1)
         tword  TH_GNC              # (@STRING) {(0) | (@STRING+1) (length-1) (CHAR) (-1)}
         tword  TIFNEZ,CVI2         # (@STRING) {{} | (@STRING+1) (length-1) (CHAR)}
         tword  TPOP                # {}
         tword  CVIVAL,TPEEKW       # [CVIVAL]
         tword  CVISGN,TPEEKC       # [CVIVAL] [CVISGN]
         tword  TIFEQZ,CVI3         # [CVIVAL]
         tword  TNEG                # -[CVIVAL]

CVI3:    tword  TCVFF               # [(+/-)CVIVAL] (-1)
         tword  TEXIT               # [(+/-)CVIVAL] (-1)

CVIE1:                              # (@STRING) (@STRING+1) (length-1)
         tword  TPOP                # (@STRING) (@STRING++)
         tword  TPOP                # (@STRING)

CVIEX:   tword  TCV00               # (@STRING) (0)
         tword  TEXIT               # (@STRING) (0)

##############################################################################
#
# Thread-
#        TH_CMNT
#
# Purpose-
#        Process a comment: ( any text )
#
# Usage-
#        (@STRING) - {(@STRING) (0)} | {(1)}
#
##############################################################################
TH_CMNT: tword  TCALL               # (@STRING)
         tword  TDUP                # (@STRING) (@STRING)
         tword  TIMMW, CMNTS        # (@STRING) (@STRING) ("(")
         tword  TCLS                # (@STRING) {(-1) | (0) | (+1)}
         tword  TIFNEZ,IS_FALSE     # (@STRING)
         tword  TPOP                # {}

CMNT1:   tword  TH_NXTW             # {(0) | (@PAD[0]) (1)}
         tword  TIFEQZ,CMNT2        # {} | (@PAD[0])
         tword  TIMMW,CMNTE         # (@PAD[0]) (")")
         tword  TCLS                # (-1) | (0) | (+1)
         tword  TIFNEZ,CMNT1        # {}
         tword  TGOTO,IS_TRUE       # {} ((=> (1)))

CMNT2:   tword  TIMMW,CMNTM1        # ("\nCOMMENT\n")
         tword  TH_PUT              # {}
         tword  TH_NXTL             # {}
         tword  TGOTO,CMNT1         # {}

CMNTS:   .byte  1,'('               # Comment start delimiter
CMNTE:   .byte  1,')'               # Comment end delimiter
CMNTM1:  .byte  9,CH_NL             # 9, "\nCOMMENT\n"
         .ascii "COMMENT"
         .byte  CH_NL
         .align 4

##############################################################################
#
# Thread-
#        TH_LOOK
#
# Purpose-
#        Look for matching dictionary entry.
#
# Usage-
#        (@STRING) (@DICTIONARY) - {(@THREAD) (1)} | {(@STRING) (0)}
#
##############################################################################
TH_LOOK: tword  TCALL               # (@STRING) (@DICT)
LOOK1:   tword  TDUP                # (@STRING) (@DICT) (@DICT)
         tword  TIFEQZ,LOOK3        # (@STRING) (@DICT)
         tword  TOVER               # (@STRING) (@DICT) (@STRING)
         tword  TOVER               # (@STRING) (@DICT) (@STRING) (@DICT)
         tword  DICT.STRING         # (@STRING) (@DICT) (@STRING) (@DICT.STRING)
         tword  TCLS                # (@STRING) (@DICT) (compare)
         tword  TIFEQZ,LOOK2        #
         tword  TPEEKW              # (@STRING) ([@DICT])
         tword  TGOTO,LOOK1         # (@STRING) ([@DICT])

LOOK2:   tword  TSWAP               # (@DICT) (@STRING)
         tword  TPOP                # (@DICT)
         tword  DICT.THREAD         # (@THREAD)
         tword  TCV01               # (@THREAD) (1)
         tword  TEXIT               # (@THREAD) (1)

LOOK3:   tword  TPOP                # (@STRING)
         tword  TCV00               # (@STRING) (0)
         tword  TEXIT               # (@STRING) (0)

##
##       DICT.STRING: (@DICT) - (@DICT.STRING)
##
DICT.STRING:
         tword  TCALL               # (@DICT)
         tword  TIMMW, (dict.string-dictionary)
         tword  TADD                # (@DICT.STRING)
         tword  TEXIT               # (@DICT.STRING)

##
##       DICT.THREAD: (@DICT) - ([@DICT.THREAD])
##
DICT.THREAD:
         tword  TCALL               # (@DICT)
         tword  TIMMW, (dict.thread-dictionary)
         tword  TADD                # (@DICT.THREAD)
         tword  TPEEKW              # ([@DICT.THREAD])
         tword  TEXIT               # ([@DICT.THREAD])

##############################################################################
#
# Thread-
#        TH_FIND
#
# Purpose-
#        Look for matching dictionary entry.
#
# Usage-
#        (@STRING) (@DICTIONARY) - {(@DICT) (1)} | {(@STRING) (0)}
#
##############################################################################
TH_FIND: tword  TCALL               # (@STRING) (@DICT)

FIND1:   tword  TDUP                # (@STRING) (@DICT) (@DICT)
         tword  TIFEQZ,FIND3        # (@STRING) (@DICT)
         tword  TOVER               # (@STRING) (@DICT) (@STRING)
         tword  TOVER               # (@STRING) (@DICT) (@STRING) (@DICT)
         tword  DICT.STRING         # (@STRING) (@DICT) (@STRING) (@DICT.STRING)
         tword  TCLS                # (@STRING) (@DICT) {(<0) | (0) | (>0)}
         tword  TIFEQZ,FIND2        # (@STRING) (@DICT)
         tword  TPEEKW              # (@STRING) (@DICT.NEXT)
         tword  TGOTO,FIND1         # (@STRING) (@DICT.NEXT)

FIND2:   tword  TSWAP               # (@DICT) (@STRING)
         tword  TPOP                # (@DICT)
         tword  TCV01               # (@DICT) (1)
         tword  TEXIT               # (@DICT) (1)

FIND3:   tword  TPOP                # (@STRING)
         tword  TCV00               # (@STRING) (0)
         tword  TEXIT               # (@STRING) (0)

##############################################################################
#
# Thread-
#        TH_PUT
#
# Purpose-
#        Write an output line.
#
# Usage-
#        (@STRING) -
#
##############################################################################
#
#        TH_PUT - WRITE AN OUTPUT LINE
#
TH_PUT:  tword  TCALL               # (@STRING)
         tword  TDUP                # (@STRING) (@STRING)
         tword  TPEEKC              # (@STRING) (length)

PUT1:    tword  TH_GNC              # (0) | (@STRING+1) (length-1) (CHAR) (-1)
         tword  TIFEQZ,TH_EXIT      # =>
         tword  TOUTC               # (@STRING+1) (length-1) (CHAR)
         tword  TGOTO,PUT1          # (@STRING+1) (length-1)

##############################################################################
#
# Thread-
#        TH_GNC
#
# Purpose-
#        Get next character from string.
#
# Usage-
#        (@STRING) (length) - {(0)} | {(@STRING+1) (length-1) (CHAR) (-1)}
#
##############################################################################
TH_GNC:  tword  TCALL               # (@STRING) (length)
         tword  TDUP                # (@STRING) (length) (length)
         tword  TIFLEZ, GNCEX       # (@STRING) (length)
         tword  TDEC                # (@STRING) (length-1)
         tword  TSWAP               # (length-1) (@STRING)
         tword  TINC                # (length-1) (@STRING+1)
         tword  TSWAP               # (@STRING+1) (length-1)
         tword  TOVER               # (@STRING+1) (length-1) (@STRING+1)
         tword  TPEEKC              # (@STRING+1) (length-1) (CHAR)
         tword  TCVFF               # (@STRING+1) (length-1) (CHAR) (-1)
         tword  TEXIT               # (@STRING+1) (length-1) (CHAR) (-1)

GNCEX:                              # (@STRING) (length)
         tword  TPOP                # (@STRING)
         tword  TPOP                # {}
         tword  TCV00               # (0)
         tword  TEXIT               # (0)

##############################################################################
#
# Thread-
#        TH_PRTD
#
# Purpose-
#        Print top using base(10).
#
# Usage-
#        (value) -
#
##############################################################################
TH_PRTD: tword  TCALL               # (value)
         tword  BASEGET             # (value) ([base])
         tword  TSWAP               # ([base]) (value)
         tword  TH_DEC              # ([base]) (value) ((BASE= 10))
         tword  TH_PRTV             # ([base])
         tword  BASESET             # {} ((BASE= old base))
         tword  TEXIT               # {}

##############################################################################
#
# Thread-
#        TH_PRTX
#
# Purpose-
#        Print top using base(16).
#
# Usage-
#        (value) -
#
##############################################################################
TH_PRTX: tword  TCALL               # (value)
         tword  BASEGET             # (value) ([base])
         tword  TSWAP               # ([base]) (value)
         tword  TH_HEX              # ([base]) (value) ((BASE= 16))
         tword  TH_PRTV             # ([base])
         tword  BASESET             # {} ((BASE= old base))
         tword  TEXIT               # {}

##############################################################################
#
# Thread-
#        TH_HEX
#
# Purpose-
#        Set BASE= 16
#
##############################################################################
TH_HEX:  tword  TCALL               # {}
         tword  TCV16, BASESET      # {} ((BASE= 16))
         tword  TEXIT               # {}

##############################################################################
#
# Thread-
#        TH_DEC
#
# Purpose-
#        Set BASE= 10
#
##############################################################################
TH_DEC:  tword  TCALL               # {}
         tword  TCV10,BASESET       # {} ((BASE= 10))
         tword  TEXIT               # {}

##############################################################################
#
# Thread-
#        TH_OCT
#
# Purpose-
#        Set BASE= 8
#
##############################################################################
TH_OCT:  tword  TCALL               # {}
         tword  TCV08,BASESET       # {} ((BASE= 8))
         tword  TEXIT               # {}

##############################################################################
#
# Thread-
#        TH_BIN
#
# Purpose-
#        Set BASE= 2
#
##############################################################################
TH_BIN:  tword  TCALL               # {}
         tword  TCV02,BASESET       # {} ((BASE= 2))
         tword  TEXIT               # {}

##############################################################################
#
# Thread-
#        TH_TOP
#
# Purpose-
#        Print top without changing it.
#
# Usage-
#        (value) - (value)
#
##############################################################################
TH_TOP:  tword  TCALL               # (value)
         tword  TDUP                # (value) (value)
         tword  TH_PRTV             # (value)
         tword  TEXIT               # (value)

##############################################################################
#
# Thread-
#        TH_PRTV
#
# Purpose-
#        "." Print top using current base.
#
# Usage-
#        (value) -
#
##############################################################################
TH_PRTV: tword  TCALL               # (value)
         tword  TCVP                # (@STRING)
         tword  TH_PUT              # {}
         tword  TH_SP               # {}
         tword  TEXIT               # {}

##############################################################################
#
# Thread-
#        TH_TCVP
#
# Purpose-
#        Convert top to string using current base.
#
# Usage-
#        (value) - (@STRING)
#
##############################################################################
TCVP:    tword  TCALL               # (value)
         tword  TCV00, OUTS.SETL    # (value) ((OUTS.length= 0))
         tword  TDUP                # (value) (value)
         tword  TIFGEZ,CVP1         # (value)
         tword  TNEG                # (abs(value))
         tword  TIMMW,('-')         # (abs(value)) ('-')
         tword  OUTS                # (abs(value)) ('-') (@OUTS)
         tword  TINC                # (abs(value)) ('-') (@OUTS+1)
         tword  TPOKEC              # (abs(value))
         tword  TCV01, OUTS.SETL    # (abs(value)) ((OUTS.length= 1))

CVP1:    tword  TCVFF               # (abs(value)) (-1)
         tword  TSWAP               # (-1) (abs(value))

CVP2:    tword  TBASE, TPEEKC       # (-1) (abs(value)) ([base])
         tword  TDIVR               # (-1) (abs(value)/[base]) (abs(value)%[base])
         tword  TABSF               # (-1) (abs(value)/[base]) (abs(value)%[base])
         tword  TSWAP               # (-1) (abs(value)%[base]) (abs(value)/[base])
         tword  TDUP                # (-1) (abs(value)%[base]) (abs(value)/[base]) (abs(value)/[base])
         tword  TIFNEZ,CVP2         # (-1) (abs(v)%[base]) ... (abs(v)%[base]) (abs(value)/[base])
         tword  TPOP                # (-1) (abs(v)%[base]) ... (abs(v)%[base])

CVP3:    tword  TDUP                # (-1) (abs(v)%[base]) ... (abs(v)%[base]) (abs(v)%[base])
         tword  TIFLTZ,CVP4         # (-1) (abs(v)%[base]) ... (abs(v)%[base])
         tword  TIMMW,CVTTAB        # (-1) (abs(v)%[base]) ... (abs(v)%[base]) (@CVTTAB)
         tword  TADD                # (-1) (abs(v)%[base]) ... (abs(v)%[base]+@CVTTAB)
         tword  TPEEKC              # (-1) (abs(v)%[base]) ... ([abs(v)%[base]+@CVTTAB])
         tword  OUTS.GETL           # (-1) (abs(v)%[base]) ... ([abs(v)%[base]+@CVTTAB]) (length)
         tword  TINC                # (-1) (abs(v)%[base]) ... ([abs(v)%[base]+@CVTTAB]) (length+1)
         tword  TDUP                # (-1) (abs(v)%[base]) ... ([abs(v)%[base]+@CVTTAB]) (length+1) (length+1)
         tword  OUTS.SETL           # (-1) (abs(v)%[base]) ... ([abs(v)%[base]+@CVTTAB]) (length)
         tword  OUTS                # (-1) (abs(v)%[base]) ... ([abs(v)%[base]+@CVTTAB]) (length) (@OUTS)
         tword  TADD                # (-1) (abs(v)%[base]) ... ([abs(v)%[base]+@CVTTAB]) (length+@OUTS)
         tword  TPOKEC              # (-1) (abs(v)%[base]) ... ([abs(v)%[base]+@CVTTAB]) (length+@OUTS)
         tword  TGOTO,CVP3          # (-1) (abs(v)%[base]) ...

CVP4:    tword  TPOP                # {}
         tword  OUTS                # (@OUTS)
         tword  TEXIT               # (@OUTS)

##############################################################################
#
# Thread-
#        TH_CR
#
# Purpose-
#        Write a carriage return.
#
# Usage-
#        -
#
##############################################################################
TH_CR:   tword  TCALL               # {}
         tword  TIMMW,CH_NL         # ('\n')
         tword  TOUTC               # {}
         tword  TEXIT               # {}

##############################################################################
#
# Thread-
#        TH_SP
#
# Purpose-
#        Write a space.
#
# Usage-
#        -
#
##############################################################################
TH_SP:   tword  TCALL               # {}
         tword  TIMMW,CH_SP         # (' ')
         tword  TOUTC               # {}
         tword  TEXIT               # {}

##############################################################################
#
# Thread-
#        TH_ROUND
#
# Purpose-
#        Round length to next word boundary.
#
# Usage-
#        (size) - ((size+(WORDSIZE-1))&(-WORDSIZE))
#
##############################################################################
#
#        TH_ROUND - Round size to next word boundary
#
TH_ROUND:
         tword  TCALL               # (size)
         tword  TIMMW, (WORDSIZE-1) # (size) (WORDSIZE-1)
         tword  TADD                # (size+(WORDSIZE-1))
         tword  TIMMW, (-(WORDSIZE))# (size+(WORDSIZE-1)) (-WORDSIZE)
         tword  TAND                # ((size+(WORDSIZE-1))&(-WORDSIZE))
         tword  TEXIT               # ((size+(WORDSIZE-1))&(-WORDSIZE))

##############################################################################
#
# Thread-
#        TH_CON
#
# Purpose-
#        Compile a constant.
#
# Usage-
#        (value) -
#
##############################################################################
TH_CON:  tword  TCALL               # (constant)
         tword  TH_GETW             # (constant) (@PAD[0])
         tword  TH_NTER             # (constant) ((Name added to dictionary))
         tword  TIMMW,TCON          # (constant) (TCON)
         tword  TH_CMPW             # (constant)
         tword  TH_CMPW             # {}
         tword  TEXIT               # {}

##############################################################################
#
# Thread-
#        TH_VAR
#
# Purpose-
#        Compile a variable.
#
# Usage-
#        (initialValue) -
#
##############################################################################
TH_VAR:  tword  TCALL               # (initialValue)
         tword  TH_GETW             # (initialValue) (@PAD[0])
         tword  TH_NTER             # (initialValue) ((Name added to dictionary))
         tword  TIMMW,TVAR          # (initialValue) (TVAR)
         tword  TH_CMPW             # (initialValue)
         tword  TH_CMPW             # {}
         tword  TEXIT               # {}

##############################################################################
#
# Thread-
#        TH_CMPL
#
# Purpose-
#        Compile: Begin compilation.
#
# Usage-
#        -
#
##############################################################################
TH_CMPL: tword  TCALL               # {}
         tword  TCV00,CFLAG,TPOKEW  # {} ((CFLAG= 0))
         tword  TH_GETW             # (@PAD[0])
         tword  TH_NTER             # {} ((Name added to dictionary))
         tword  TIMMW,TCALL         # (TCALL)
         tword  TH_CMPW             # {}

CMPL1:   tword  TH_NXTW             # {(0) | (@PAD[0]) (1)}
         tword  TIFEQZ,CMPL6        # (@PAD[0])
         tword  TH_CMNT             # (@PAD[0]) (0) | (1)
         tword  TIFNEZ,CMPL1        # (@PAD[0])
         tword  TCDICT              # (@PAD[0]) (@DICT)
         tword  TH_LOOK             # (@THREAD) (1) | (@PAD[0]) (0)
         tword  TIFEQZ,CMPL2        # (@THREAD)
         tword  TEXEC               # {}
         tword  TGOTO,CMPL5         # {}

CMPL2:   tword  TSDICT,TPEEKW       # (@PAD[0]) (@DICT)
         tword  TH_LOOK             # (@THREAD) (1) | (@PAD[0]) (0)
         tword  TIFEQZ,CMPL3        # (@THREAD)
         tword  TH_CMPW             # {}
         tword  TGOTO,CMPL5         # {}

CMPL3:   tword  TH_CVI              # (value) (-1) | (@PAD[0]) (0)
         tword  TIFEQZ,CMPL4        # (value)
         tword  TIMMW,TIMMW         # (value) (TIMMW)
         tword  TH_CMPW             # (value)
         tword  TH_CMPW             # {}
         tword  TGOTO,CMPL5         # {}

CMPL4:   tword  TIMMW,SCANM2        # (@PAD[0]) ("\nUNDEFINED SYMBOL: ")
         tword  TH_PUT              # (@PAD[0])
         tword  TH_UERR             # (@PAD[0])

CMPL5:   tword  CFLAG,TPEEKW        # ([CFLAG])
         tword  TIFEQZ,CMPL1        # {}
         tword  TEXIT               # {}

CMPL6:   tword  TIMMW,CMPLM1        # {} ("\n:\n")
         tword  TH_PUT              # {}
         tword  TH_NXTL             # {}
         tword  TGOTO,CMPL1         # {}

CMPLM1:  .byte  3,CH_NL,':',CH_NL
         .align 4

##############################################################################
#
# Thread-
#        TH_SEMI
#
# Purpose-
#        Compile: Compilation complete.
#
# Usage-
#        -
#
##############################################################################
TH_SEMI: tword  TCALL               # {}
         tword  TIMMW,TEXIT         # (TEXIT)
         tword  TH_CMPW             # {}
         tword  TCVFF,CFLAG,TPOKEW  # ((CFLAG= (-1))
         tword  TEXIT               # {}

##############################################################################
#
# Thread-
#        TH_IF
#
# Purpose-
#        Compile: Skip code if false.
#
# Usage-
#        - ([MEMORY])
#
##############################################################################
TH_IF:   tword  TCALL               # {}
         tword  TIMMW,TIFEQZ        # (TIFEQZ)
         tword  TH_CMPW             # {}
         tword  MEMGET              # ([MEMORY])
         tword  TCV00               # ([MEMORY]) (0)
         tword  TH_CMPW             # ([MEMORY])
         tword  TEXIT               # ([MEMORY])

##############################################################################
#
# Thread-
#        TH_ELSE
#
# Purpose-
#        Compile: Complete previous IF, generate ELSE.
#
# Usage-
#        ([MEMORY]IF) - ([MEMORY]ELSE)
#
##############################################################################
TH_ELSE: tword  TCALL               # ([MEMORY]IF)
         tword  TIMMW,TGOTO         # ([MEMORY]IF) (TGOTO)
         tword  TH_CMPW             # ([MEMORY]IF)
         tword  MEMGET              # ([MEMORY]IF) ([MEMORY]ELSE)
         tword  TCV00               # ([MEMORY]IF) ([MEMORY]ELSE) (0)
         tword  TH_CMPW             # ([MEMORY]IF) ([MEMORY]ELSE)
         tword  TSWAP               # ([MEMORY]ELSE) ([MEMORY]IF)
         tword  MEMGET              # ([MEMORY]ELSE) ([MEMORY]IF) ([MEMORY](0))
         tword  TSWAP               # ([MEMORY]ELSE) ([MEMORY](0)) ([MEMORY]IF)
         tword  TPOKEW              # ([MEMORY]ELSE)
         tword  TEXIT               # ([MEMORY]ELSE)

##############################################################################
#
# Thread-
#        TH_THEN
#
# Purpose-
#        Compile: Complete previous IF or ELSE.
#
# Usage-
#        ([MEMORY]ELSE) -
#
##############################################################################
TH_THEN: tword  TCALL               # ([MEMORY]ELSE)
         tword  MEMGET              # ([MEMORY]ELSE) ([MEMORY])
         tword  TSWAP               # ([MEMORY]) ([MEMORY]ELSE)
         tword  TPOKEW              # {}
         tword  TEXIT               # {}

##############################################################################
#
# Thread-
#        TH_PEAT
#
# Purpose-
#        Compile: Target for LOOP instruction.
#
# Usage-
#        - ([MEMORY]PEAT)
#
##############################################################################
TH_PEAT: tword  TCALL               # {}
         tword  MEMGET              # ([MEMORY]PEAT)
         tword  TEXIT               # ([MEMORY]PEAT)

##############################################################################
#
# Thread-
#        TH_TILL
#
# Purpose-
#        Compile: Terminate loop if true.
#
# Usage-
#        - ([MEMORY]TILL)
#
##############################################################################
TH_TILL: tword  TCALL               # {}
         tword  TIMMW,TIFNEZ        # (IFNEZ)
         tword  TH_CMPW             # {}
         tword  MEMGET              # ([MEMORY]TILL)
         tword  TCV00               # ([MEMORY]TILL) (0)
         tword  TH_CMPW             # ([MEMORY]TILL)
         tword  TEXIT               # ([MEMORY]TILL)

##############################################################################
#
# Thread-
#        TH_FOR
#
# Purpose-
#        Compile: Beginning of FOR loop.
#
# Usage-
#        - ([MEMORY]FORI) ([MEMORY]FORT)
#
##############################################################################
TH_FOR:  tword  TCALL               # {}
         tword  TIMMW,TH_FORI       # (TH_FORI)
         tword  TH_CMPW             # {}
         tword  MEMGET              # ([MEMORY]FORI)
         tword  TIMMW,TH_FORT       # ([MEMORY]FORI) (TH_FORT)
         tword  TH_CMPW             # ([MEMORY]FORI)
         tword  TIMMW,TIFNEZ        # ([MEMORY]FORI) (TIFNEZ)
         tword  TH_CMPW             # ([MEMORY]FORI)
         tword  MEMGET              # ([MEMORY]FORI) ([MEMORY]FORT)
         tword  TCV00               # ([MEMORY]FORI) ([MEMORY]FORT) (0)
         tword  TH_CMPW             # ([MEMORY]FORI) ([MEMORY]FORT)
         tword  TEXIT               # ([MEMORY]FORI) ([MEMORY]FORT)

##############################################################################
#
# Thread-
#        TH_FORI
#
# Purpose-
#        Compile: Initialize FOR loop.
#
# Usage-
#        ??? - ???
#
##############################################################################
TH_FORI: tword  TCALL               #
         tword  TSWAP               #
         tword  TINC                #
         tword  TAUXPUSH            #
         tword  TDEC                #
         tword  TAUXPUSH            #
         tword  TEXIT               #

##############################################################################
#
# Thread-
#        TH_LOOP
#
# Purpose-
#        Compile: Target for UNTIL; unconditional return to repeat.
#
# Usage-
#        ??? - ???
#
##############################################################################
TH_LOOP: tword  TCALL               # ??2 ??1
         tword  TIMMW,TGOTO         # ??2 ??1 (TGOTO)
         tword  TH_CMPW             # ??2 ??1
         tword  TSWAP               # ??1 ??2
         tword  TH_CMPW             # ??1
         tword  MEMGET              # ??1 ([MEMORY])
         tword  TSWAP               # ([MEMORY]) ??1
         tword  TPOKEW              # {}
         tword  TEXIT               # {}

##############################################################################
#
# Thread-
#        TH_UERR
#
# Purpose-
#        Compile: User error.
#
# Usage-
#        (@STRING) -
#        ((All stacks are reset))
#
##############################################################################
TH_UERR: tword  TCALL               # (@STRING)
         tword  TSDICT              # (@STRING) (@DICT)
         tword  TPEEKW              # (@STRING) ([@DICT])
         tword  TPEEKW              # (@STRING) ([[@DICT]])
         tword  TSDICT              # (@STRING) ([[@DICT]]) (@DICT)
         tword  TPOKEW              # (@STRING)
         tword  MEMOLD, TPEEKW      # (@STRING) ([MEMOLD])
         tword  MEMSET              # (@STRING)
         tword  TH_CERR             # (@STRING)

##############################################################################
#
# Thread-
#        TH_CERR (abnormal)
#
# Purpose-
#        Compile: Compilation error.
#
# Usage-
#        (@STRING) -
#        ((All stacks are reset))
#
##############################################################################
TH_CERR: tword  TCALL               # (@STRING)
         tword  TH_PUT              # {}
         tword  TCVFF,CFLAG,TPOKEW  # {} ((CFLAG= (-1)))
         tword  TRESET              # {}

##############################################################################
#
# Thread-
#        TH_CMPW
#
# Purpose-
#        Compile: Compile next word (adding it to memory)
#
# Usage-
#        (word) -
#
##############################################################################
TH_CMPW: tword  TCALL               # (word)
         tword  MEMGET              # (word) ([MEMORY])
         tword  TDUP                # (word) ([MEMORY]) ([MEMORY])
         tword  TIMMW, (WORDSIZE)   # (word) ([MEMORY]) ([MEMORY]) (WORDSIZE)
         tword  TADD                # (word) ([MEMORY]) ([MEMORY]+WORDSIZE)
         tword  TDUP                # (word) ([MEMORY]) ([MEMORY]+WORDSIZE) ([MEMORY]+WORDSIZE)
         tword  MEMEND              # .. ([MEMORY]+WORDSIZE) (MEMEND)
         tword  TIFGEL,CMPW1        # (word) ([MEMORY]) ([MEMORY]+WORDSIZE)
         tword  MEMSET              # (word) ([MEMORY]) (([MEMORY]+=WORDSIZE))
         tword  TPOKEW              # {} (([[MEMORY]]= (word)))
         tword  TEXIT               # {}

CMPW1:   tword  TIMMW,NTERM1        # (word) ([MEMORY]) "\nDICTIONARY FULL"
         tword  TH_UERR             # (word) ([MEMORY]) "\nDICTIONARY FULL"

##############################################################################
#
# Thread-
#        TH_NTER
#
# Purpose-
#        Compile: Enter a name into the dictionary.
#
# Usage-
#        (@STRING) -
#
##############################################################################
TH_NTER: tword  TCALL               # (@STRING)
######## tword  TSTOP               # (@STRING) ((Debugging trap point))
         tword  TH_CHKD             # (@STRING) ((Message if duplicate))
         tword  MEMGET              # (@STRING) ([MEMORY])
         tword  MEMOLD, TPOKEW      # (@STRING) ((MEMOLD= [MEMORY]))
         tword  TDUP                # (@STRING) (@STRING)
         tword  TPEEKC              # (@STRING) (length(STRING))
         tword  TIMMW, (dict.base - dictionary)
         tword  TADD                # (@STRING) (size(entry))
         tword  TH_ROUND            # (@STRING) (rounded(size(entry)))
         tword  MEMGET              # (@STRING) (rounded) ([MEMORY])
         tword  TADD                # (@STRING) ([MEMORY]+size)
         tword  TDUP                # (@STRING) ([MEMORY]+size) ([MEMORY]+size)
         tword  MEMEND              # (@STRING) ([MEMORY]+size) ([MEMORY]+size) (@MEMEND)
         tword  TIFGEL,NTER1        # (@STRING) ([MEMORY]+size)
         tword  TSDICT, TPEEKW      # (@STRING) ([MEMORY]+size) (@DICT)
         tword  MEMGET, TPOKEW      # (@STRING) ([MEMORY]+size)
         tword  MEMGET              # (@STRING) ([MEMORY]+size) ([MEMORY])
         tword  TSDICT, TPOKEW      # (@STRING) ([MEMORY]+size)
         tword  TDUP                # (@STRING) ([MEMORY]+size) ([MEMORY]+size)
         tword  MEMGET              # (@STRING) ([MEMORY]+size) ([MEMORY]+size) ([MEMORY])
         tword  TIMMW, (dict.thread - dictionary)
         tword  TADD                # (@STRING) ([MEMORY]+size) ([MEMORY]+size) ([MEMORY]+.thread)
         tword  TPOKEW              # (@STRING) ([MEMORY]+size)
         tword  MEMSET              # (@STRING) (([MEMORY]= [MEMORY]+size))
         tword  MEMOLD, TPEEKW      # (@STRING) ([MEMOLD])
         tword  TIMMW, (dict.string - dictionary)
         tword  TADD                # (@STRING) ([MEMOLD]+.string)
         tword  TSWAP               # ([MEMOLD]+.string) (@STRING)
         tword  TMVS                # {}
         tword  TEXIT               # {}

NTER1:   tword  TPOP                # (@STRING)
         tword  TPOP                # {}
         tword  TIMMW, NTERM1       # "\nDICTIONARY FULL"
         tword  TH_CERR             # {}
         tword  TEXIT               # {}

NTERM1:  .byte  16,CH_NL
         .ascii "DICTIONARY FULL"
         .align 4

##############################################################################
#
# Thread-
#        TH_CHKD
#
# Purpose-
#        Compile: Warning message if duplicate symbol.
#
# Usage-
#        (@STRING) - (@STRING)
#
##############################################################################
TH_CHKD: tword  TCALL               # (@STRING)
         tword  TDUP                # (@STRING) (@STRING)
         tword  TSDICT, TPEEKW      # (@STRING) (@STRING) (@DS_CMPL)
         tword  TH_FIND             # (@STRING) {(@DICT) (1) | (@STRING) (0)}
         tword  TIFEQZ,CHKD1        # (@STRING) {(@DICT) | (@STRING)}
         tword  TPOP                # (@STRING)
         tword  TIMMW,CHKDM1        # (@STRING) "\nDUPLICATE SYMBOL: "
         tword  TH_PUT              # (@STRING)
         tword  TDUP                # (@STRING) (@STRING)
         tword  TH_PUT              # (@STRING)
         tword  TEXIT               # (@STRING)

CHKD1:   tword  TPOP                # (@STRING)
         tword  TEXIT               # (@STRING)

CHKDM1:  .byte  19,CH_NL
         .ascii "DUPLICATE SYMBOL: "
         .align 4

##############################################################################
#
# Thread-
#        TH_FGET
#
# Purpose-
#        "FORGET" Forget dictionary entry.
#
# Usage-
#        ??? - ???
#
##############################################################################
TH_FGET: tword  TCALL               #
         tword  TH_GETW             # (@PAD[0])
         tword  TSDICT              # (@PAD[0]) (@@DICT)
         tword  TPEEKW              # (@PAD[0]) (@DICT])
         tword  TH_FIND             # {(@DICT) (1) | (@PAD[0]) (0)}
         tword  TIFEQZ,FGET1        #
         tword  TDUP                #
         tword  TPEEKW              #
         tword  TSDICT              #
         tword  TPOKEW              #
         tword  MEMSET              #
         tword  TEXIT               #

FGET1:   tword  TIMMW,SCANM2        #
         tword  TH_PUT              #
         tword  TH_PUT              #
         tword  TEXIT               #

##############################################################################
#
# Thread-
#        TH_EQZ
#
# Purpose-
#        "=0" Equals zero.
#
# Usage-
#        (value) - {(0) | (1)}
#
##############################################################################
TH_EQZ:  tword  TCALL               #
         tword  TIFNEZ,IS_FALSE     #
         tword  TGOTO,IS_TRUE       #

##############################################################################
#
# Thread-
#        TH_LTZ
#
# Purpose-
#        "<0" Less than zero.
#
# Usage-
#        (value) - {(0) | (1)}
#
##############################################################################
TH_LTZ:  tword  TCALL               #
         tword  TIFLTZ,IS_TRUE      #
         tword  TGOTO,IS_FALSE      #

##############################################################################
#
# Thread-
#        TH_EQ
#
# Purpose-
#        "=" Equal
#
# Usage-
#        (a) (b) - {(0) | (1)}
#
##############################################################################
TH_EQ:   tword  TCALL               #
         tword  TIFEQ,IS_TRUE       #
         tword  TGOTO,IS_FALSE      #

##############################################################################
#
# Thread-
#        TH_LT
#
# Purpose-
#        "<" Arithmetic less than
#
# Usage-
#        (a) (b) - {(0) | (1)}
#
##############################################################################
TH_LT:   tword  TCALL               #
         tword  TIFLT,IS_TRUE       #
         tword  TGOTO,IS_FALSE      #

##############################################################################
#
# Thread-
#        TH_LTL
#
# Purpose-
#        "L<" Logical less than
#
# Usage-
#        (a) (b) - {(0) | (1)}
#
##############################################################################
TH_LTL:  tword  TCALL               #
         tword  TIFLTL,IS_TRUE      #
         tword  TGOTO,IS_FALSE      #

##############################################################################
#
# Label-
#        IS_FALSE
#
# Purpose-
#        Constant: FALSE
#
# Usage-
#        - (0)
#
##############################################################################
IS_FALSE:tword  TIMMW, 0            # (0)
         tword  TEXIT               # (0)

##############################################################################
#
# Label-
#        IS_TRUE
#
# Purpose-
#        Constant: TRUE
#
# Usage-
#        - (1)
#
##############################################################################
IS_TRUE: tword  TIMMW, 1            # (1)
         tword  TEXIT               # (1)

##############################################################################
#
# Thread-
#        TH_DEBUG
#
# Purpose-
#        Debugging utility.
#
# Usage-
#        -
#
##############################################################################
DEBUG1:  tword  CEBUG1              # (Get caller's return address)

TH_DEBUG:tword  TCALL               # {}
         tword  TIMMW,DEBUGM1,TH_PUT# {}
         tword  TIMMW,DEBUGM2,TH_PUT# {}
         tword  DEBUG1              # (Caller's return address)
         tword  TH_PRTX             # {}
         tword  TIMMW,DEBUGM3,TH_PUT# {}
         tword  TDUP                # (top)
         tword  TH_PRTX             # {}
         tword  TH_CR               # {}
         tword  TEXIT               # {}

DEBUGM1: .byte  7
         .ascii "*DEBUG*"
DEBUGM2: .byte  5
         .ascii " PCR="
DEBUGM3: .byte  5
         .ascii " TOP="
         .align 4

##############################################################################
#
# Thread-
#        TH_TEST
#
# Purpose-
#        Bringup self-test function.
#
##############################################################################
TH_TEST: tword  TCALL
         tword  TH_DEBUG

DEBUGGING(`
##################################### Debugging
######## Self-test
TEST000: ############################# Test TIFLT
         tword  TIMMW, (+12345)     ## Stack constant
         tword  TIMMW, (+0), TIMMW, (-1), TIFLT, TH_FAIL
         tword  TIMMW, (+0), TIMMW, (+0), TIFLT, TH_FAIL
         tword  TIMMW, (+0), TIMMW, (+1), TIFLT, TEST001
         tword  TFALSE
         tword  TEXIT

TEST001: ############################# Test TIFLTL
         tword  TIMMW, (-1), TIMMW, (+0), TIFLTL, TH_FAIL
         tword  TIMMW, (+0), TIMMW, (+0), TIFLTL, TH_FAIL
         tword  TIMMW, (+0), TIMMW, (+1), TIFLTL, TEST002
         tword  TFALSE
         tword  TEXIT

TEST002: ############################# Test TDIVR
         tword  TIMMW, (+7), TIMMW, (+3), TDIVR, TNOP
         tword  TIMMW, (+1), TIFNE, TH_FAIL
         tword  TIMMW, (+2), TIFNE, TH_FAIL

###################################### Test TDIV
         tword  TIMMW, (+7), TIMMW, (+3), TDIV, TNOP
         tword  TIMMW, (+2), TIFNE, TH_FAIL
         tword  TGOTO, TH_PASS

TH_PASS: ############################# PASS!
         tword  TIMMW, (+12345), TIFNE, TH_FAIL
         tword  TTRUE
         tword  TEXIT

TH_FAIL:
         tword  TFALSE
         tword  TEXIT

TTRUE:   tword  CTRUE
TFALSE:  tword  CFALSE
##################################### Debugging
')
         tword  TEXIT

