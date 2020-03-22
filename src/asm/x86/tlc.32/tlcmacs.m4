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
#        tlcmacs.m4
#
# Purpose-
#        Threaded Language Compiler: Macros
#
##############################################################################

##############################################################################
#
# Macro-
#        CONSTANTS
#
# Purpose-
#        Constant definitions.
#
##############################################################################
#define SIZE_IMAGE 510              /* sizeof(input text line)              */
#define WORDSIZE     4              /* sizeof(word)                         */

##############################################################################
#
# Macro-
#        REGISTERS
#
# Purpose-
#        Register definitions.
#
##############################################################################
#define dStack  %ebp                /* Data stack                           */
#define iAddr   %ebx                /* Instruction address                  */
#define cStack  %ecx                /* Instruction address stack            */
#define iWord   %edx                /* Instruction word (temporary)         */

##############################################################################
#
# Macro-
#        DEBUGGING
#
# Purpose-
#        Debugging conditional.
#
# Usage-
#        DEBUGGING(`
#          ## Debugging code
#        ')
#
##############################################################################
#if 0
define(DEBUGGING, ` ###### macro DEBUGGING()
         $1
') dnl

#else
define(DEBUGGING, ` ###### macro DEBUGGING()
') dnl
#endif

##############################################################################
#
# Macro-
#        tword
#
# Function-
#        Define a word (list).
#
# Usage-
#        tword name(,...)
#
##############################################################################
define(`tword',.long  ) dnl

##############################################################################
#
# Macro-
#        dEntry
#
# Function-
#        Define a dictionary entry
#
# Usage-
#        dictEntry("Name", Thread)
#
##############################################################################
define(`NEXT_DICT_LABEL', 0) dnl

define(dEntry, ` ###### macro dEntry($1,$2)
         define(`NEXT_DICT_LABEL', incr(NEXT_DICT_LABEL))
         .long  `NEXT_DICT_ENTRY'NEXT_DICT_LABEL  ## Chain pointer
         .long  $2                  ## -> Handler
         .byte  `SIZE_DICT_ENTRY'NEXT_DICT_LABEL-`NAME_DICT_ENTRY'NEXT_DICT_LABEL ## Length
`NAME_DICT_ENTRY'NEXT_DICT_LABEL:   ## The byte of the entry
         .ascii $1                  ## Name
`SIZE_DICT_ENTRY'NEXT_DICT_LABEL:   ## The byte after the entry
         .align 4                   ## Word alignment
`NEXT_DICT_ENTRY'NEXT_DICT_LABEL:   ## The next entry
') dnl                              ## Macro DICT

##############################################################################
#
# Macro-
#        dFetch/dStore
#
# Function-
#        Fetch/store top entry on data stack.  The stack offset is unchanged.
#
# Usage-
#        dFetch(register)
#        dStore(register)
#
##############################################################################
define(dFetch, ` ###### macro dFetch($1)
         movl   (dStack), $1        ## Fetch value from stack
') dnl

define(dStore, ` ###### macro dStore($1)
         movl   $1, (dStack)        ## Store value onto stack
') dnl

##############################################################################
#
# Macro-
#        dPush/dPop
#
# Function-
#        Push/pop register onto/from data Stack.
#
# Usage-
#        push(register)
#        pop(register)
#
##############################################################################
define(dPush, ` ###### macro push($1)
         add    $(WORDSIZE), dStack ## Grow the stack
         movl   $1, (dStack)        ## Push value onto stack
') dnl

define(dPop, ` ###### macro pop($1)
         movl   (dStack), $1        ## Pop value from stack
         sub    $(WORDSIZE), dStack ## Shrink the stack
') dnl

##############################################################################
#
# Macro-
#        pushALL, popALL
#
# Function-
#        Push/pop all volatile registers.
#
# Usage-
#        pushALL
#        popALL
#
##############################################################################
define(pushALL, ` ###### macro pushALL()
         pushl  dStack
         pushl  cStack
         pushl  iAddr
         pushl  iWord
') dnl

define(popALL, ` ###### macro popALL()
         popl   iWord
         popl   iAddr
         popl   cStack
         popl   dStack
') dnl

