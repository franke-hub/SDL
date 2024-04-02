##############################################################################
##
##       Copyright (C) 2015-2024 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##############################################################################
## Makefile
##############################################################################

##############################################################################
## Built-in rules
.SUFFIXES:
%.o:      ;
alpha/%:  ;
beta/%:   ;
subdir/%: ;

##############################################################################
## Local module list
MAKRUN = \
    Main.run

MAKOBJ = \
    Main.o     \
    Alpha.o    \
    alpha/One.foo(Q)     \
    beta/Two.bar         \

##############################################################################
## Set default target
ifeq "" "$(DEFAULT)"
DEFAULT = $(MAKRUN)
endif

.PHONY: default info
default: info $(DEFAULT)

##############################################################################
## Make control: Rule list
info: ;
	@echo -e "------------------------------------------------------------------"
	@echo -e "info: ;"
	@echo -e "  :: (default: info)"
	@echo -e "%.s: S/%.C other.s: S/Other.C"
	@echo -e "  :: (other.s: S/Other.C)"
	@echo -e "Main.street Main.run: $(MAKOBJ)"
	@echo -e "  :: (default: Main.run)"
	@echo -e "O/Other: subdir/xxx.o yyy.o subdir/xxx.o yyy.o"
	@echo -e "  :: (Main.run: O/other)"
	@echo -e ""
	@echo -e "default: info $(DEFAULT)"
	@echo -e "Main.run: O/Other"
	@echo -e "O/Other: Other.s subdir/foo.o bar.o"
	@echo -e "S/Other.C: subdir/bar.o foo.o"
	@echo -e ""
	@echo -e "Never need to build:"
	@echo -e "%.o alpha/% beta/% subdir/%: ;" ## (Never need to build these)
	@echo -e ""
	@echo -e ""

Main.run: O/Other
O/Other: Other.s subdir/foo.o bar.o
S/Other.C: subdir/bar.o foo.o;

##############################################################################
## Pattern: %.s: S/%.C
%.s: S/%.C
	@echo -e "------------------------------------------------------------------"
	@echo -e "%.s: S/%.C"
	@echo -e ":: (Main.run: O/Other) (O/Other: other.s ...) (S/Other.C: <done>)"
## VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
	@echo -e "$$\c"
	@echo -e "@     $@"    ## The (selected) target
	@echo -e "$$\c"
	@echo -e "<     $<"    ## 1st prerequisite
	@echo -e "$$\c"
	@echo -e "^     $^"    ## Non order-only prerequisites w/o duplicates
	@echo -e "$$\c"
	@echo -e "%     $%"    ## Target member name
	@echo -e "$$\c"
	@echo -e "?     $?"    ## Newer prerequisites
	@echo -e "$$\c"
	@echo -e "+     $+"    ## Non order-only prerequisites including duplicates
	@echo -e "$$\c"
	@echo -e "*     $*"    ## The implicit rule matching stem
	@echo -e "$$\c"
	@echo -e "|     $|"    ## All order-only prerequisites
## ---------------------------------------------------------------------------
	@echo -e "$$\c"
	@echo -e "(@D)  $(@D)" ## The (selected) target's directory name
	@echo -e "$$\c"
	@echo -e "(@F)  $(@F)" ## The (selected) target's file name
	@echo -e "$$\c"
	@echo -e "(<D)  $(<D)" ## 1st prerequisite's directory name
	@echo -e "$$\c"
	@echo -e "(<F)  $(<F)" ## 1st prerequisite's file name
	@echo -e "$$\c"
	@echo -e "(^D)  $(^D)" ## Non order-only prerequisite directory names
	@echo -e "$$\c"
	@echo -e "(^F)  $(^F)" ## Non order-only prerequisite file names
	@echo -e "$$\c"
	@echo -e "(%D)  $(%D)" ## Target member directory names
	@echo -e "$$\c"
	@echo -e "(%F)  $(%F)" ## Target member file names
	@echo -e "$$\c"
	@echo -e "(?D)  $(?D)" ## Newer prerequisite directory names
	@echo -e "$$\c"
	@echo -e "(?F)  $(?F)" ## Newer prerequisite file names
	@echo -e "$$\c"
	@echo -e "(+D)  $(+D)" ## All prerequisite directory names
	@echo -e "$$\c"
	@echo -e "(+F)  $(+F)" ## All prerequisite file names
	@echo -e "$$\c"
	@echo -e "(*D)  $(*D)" ## The implicit rule matching stem's directory name
	@echo -e "$$\c"
	@echo -e "(*F)  $(*F)" ## The implicit rule matching stem's file name
## $(|D) $(|F) ** NOT SUPPORTED, ALWAYS EMPTY **
	@echo -e "$$\c"
	@echo -e "(|D)  $(|D)" ## All order-only prerequisite directory names
	@echo -e "$$\c"
	@echo -e "(|F)  $(|F)" ## All order-only prerequisite file names
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	@echo -e ""
	@echo -e ""

##############################################################################
## Make control: Main.run
Main.street Main.run: $(MAKOBJ)
	@echo -e "------------------------------------------------------------------"
	@echo -e "Main.street Main.run: $(MAKOBJ)"
	@echo -e ":: (default: Main.run)"
## VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
	@echo -e "$$\c"
	@echo -e "@     $@"    ## The (selected) target
	@echo -e "$$\c"
	@echo -e "<     $<"    ## 1st prerequisite
	@echo -e "$$\c"
	@echo -e "^     $^"    ## Non order-only prerequisites w/o duplicates
	@echo -e "$$\c"
	@echo -e "%     $%"    ## Target member name
	@echo -e "$$\c"
	@echo -e "?     $?"    ## Newer prerequisites
	@echo -e "$$\c"
	@echo -e "+     $+"    ## Non order-only prerequisites including duplicates
	@echo -e "$$\c"
	@echo -e "*     $*"    ## The implicit rule matching stem
	@echo -e "$$\c"
	@echo -e "|     $|"    ## All order-only prerequisites
## ---------------------------------------------------------------------------
	@echo -e "$$\c"
	@echo -e "(@D)  $(@D)" ## The (selected) target's directory name
	@echo -e "$$\c"
	@echo -e "(@F)  $(@F)" ## The (selected) target's file name
	@echo -e "$$\c"
	@echo -e "(<D)  $(<D)" ## 1st prerequisite's directory name
	@echo -e "$$\c"
	@echo -e "(<F)  $(<F)" ## 1st prerequisite's file name
	@echo -e "$$\c"
	@echo -e "(^D)  $(^D)" ## Non order-only prerequisite directory names
	@echo -e "$$\c"
	@echo -e "(^F)  $(^F)" ## Non order-only prerequisite file names
	@echo -e "$$\c"
	@echo -e "(%D)  $(%D)" ## Target member directory names
	@echo -e "$$\c"
	@echo -e "(%F)  $(%F)" ## Target member file names
	@echo -e "$$\c"
	@echo -e "(?D)  $(?D)" ## Newer prerequisite directory names
	@echo -e "$$\c"
	@echo -e "(?F)  $(?F)" ## Newer prerequisite file names
	@echo -e "$$\c"
	@echo -e "(+D)  $(+D)" ## All prerequisite directory names
	@echo -e "$$\c"
	@echo -e "(+F)  $(+F)" ## All prerequisite file names
	@echo -e "$$\c"
	@echo -e "(*D)  $(*D)" ## The implicit rule matching stem's directory name
	@echo -e "$$\c"
	@echo -e "(*F)  $(*F)" ## The implicit rule matching stem's file name
## ====== MACROS $(|D) and $(|F) ** ARE NOT SUPPORTED and ALWAYS EMPTY **
	@echo -e "$$\c"
	@echo -e "(|D)  $(|D)" ## All order-only prerequisite directory names
	@echo -e "$$\c"
	@echo -e "(|F)  $(|F)" ## All order-only prerequisite file names
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	@echo -e ""
	@echo -e "NOTE:  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = ="
	@echo -e "$$\c"
	@echo -e "@D    $@D  ($$\c"; echo -e "@D := '$$\c"; echo "@' :: 'D')"
	@echo -e "(The parentheses are required.)"
	@echo -e ""
	@echo -e ""

##############################################################################
## Make control: O/Other
O/Other: subdir/xxx.o yyy.o subdir/xxx.o yyy.o
	@echo -e "------------------------------------------------------------------"
	@echo -e "O/Other: subdir/xxx.o yyy.o subdir/xxx.o yyy.o"
	@echo -e ":: (default: Main.run) (Main.run: O/Other)"
## VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
	@echo -e "$$\c"
	@echo -e "@     $@"    ## The (selected) target
	@echo -e "$$\c"
	@echo -e "<     $<"    ## 1st prerequisite
	@echo -e "$$\c"
	@echo -e "^     $^"    ## Non order-only prerequisites w/o duplicates
	@echo -e "$$\c"
	@echo -e "%     $%"    ## Target member name
	@echo -e "$$\c"
	@echo -e "?     $?"    ## Newer prerequisites
	@echo -e "$$\c"
	@echo -e "+     $+"    ## Non order-only prerequisites including duplicates
	@echo -e "$$\c"
	@echo -e "*     $*"    ## The implicit rule matching stem
	@echo -e "$$\c"
	@echo -e "|     $|"    ## All order-only prerequisites
## ---------------------------------------------------------------------------
	@echo -e "$$\c"
	@echo -e "(@D)  $(@D)" ## The (selected) target's directory name
	@echo -e "$$\c"
	@echo -e "(@F)  $(@F)" ## The (selected) target's file name
	@echo -e "$$\c"
	@echo -e "(<D)  $(<D)" ## 1st prerequisite's directory name
	@echo -e "$$\c"
	@echo -e "(<F)  $(<F)" ## 1st prerequisite's file name
	@echo -e "$$\c"
	@echo -e "(^D)  $(^D)" ## Non order-only prerequisite directory names
	@echo -e "$$\c"
	@echo -e "(^F)  $(^F)" ## Non order-only prerequisite file names
	@echo -e "$$\c"
	@echo -e "(%D)  $(%D)" ## Target member directory names
	@echo -e "$$\c"
	@echo -e "(%F)  $(%F)" ## Target member file names
	@echo -e "$$\c"
	@echo -e "(?D)  $(?D)" ## Newer prerequisite directory names
	@echo -e "$$\c"
	@echo -e "(?F)  $(?F)" ## Newer prerequisite file names
	@echo -e "$$\c"
	@echo -e "(+D)  $(+D)" ## All prerequisite directory names
	@echo -e "$$\c"
	@echo -e "(+F)  $(+F)" ## All prerequisite file names
	@echo -e "$$\c"
	@echo -e "(*D)  $(*D)" ## The implicit rule matching stem's directory name
	@echo -e "$$\c"
	@echo -e "(*F)  $(*F)" ## The implicit rule matching stem's file name
## $(|D) $(|F) ** NOT SUPPORTED, ALWAYS EMPTY **
	@echo -e "$$\c"
	@echo -e "(|D)  $(|D)" ## All order-only prerequisite directory names
	@echo -e "$$\c"
	@echo -e "(|F)  $(|F)" ## All order-only prerequisite file names
## ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	@echo -e ""
	@echo -e ""
