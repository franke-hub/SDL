##############################################################################
##
##       Copyright (C) 2015-2024 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##############################################################################
## Makefile.GNU
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

.PHONY: default
default: $(DEFAULT)

##############################################################################
## Make control: Generic rule
%.s: S/%.C
	@echo -e "%.s: S/%.C"
	@echo -e "$$\c"
	@echo -e "@     $@"
	@echo -e "$$\c"
	@echo -e "<     $<"
	@echo -e "$$\c"
	@echo -e "^     $^"
	@echo -e "$$\c"
	@echo -e "?     $?"
	@echo -e "$$\c"
	@echo -e "+     $+"
	@echo -e "$$\c"
	@echo -e "*     $*"
	@echo -e "$$\c"
	@echo -e "%     $%"
	@echo -e "$$\c"
	@echo -e "(<D)  $(<D)"
	@echo -e "$$\c"
	@echo -e "(@D)  $(@D)"
	@echo -e "$$\c"
	@echo -e "(<F)  $(<F)"
	@echo -e "$$\c"
	@echo -e "(@F)  $(@F)"
	@echo -e ""
	@echo -e ""

##############################################################################
## Make control: Main
Main.street Main.run: $(MAKOBJ)
	@echo -e "Main.street Main.run: $(MAKOBJ)"
	@echo -e "$$\c"
	@echo -e "@     $@"
	@echo -e "$$\c"
	@echo -e "<     $<"
	@echo -e "$$\c"
	@echo -e "<     $<"
	@echo -e "$$\c"
	@echo -e "^     $^"
	@echo -e "$$\c"
	@echo -e "?     $?"
	@echo -e "$$\c"
	@echo -e "+     $+"
	@echo -e "$$\c"
	@echo -e "*     $*"
	@echo -e "$$\c"
	@echo -e "%     $%"
	@echo -e "$$\c"
	@echo -e "(<D)  $(<D)"
	@echo -e "$$\c"
	@echo -e "(@D)  $(@D)"
	@echo -e "$$\c"
	@echo -e "(<F)  $(<F)"
	@echo -e "$$\c"
	@echo -e "(@F)  $(@F)"
	@echo -e ""
	@echo -e "$$\c"
	@echo -e "@F    $@F  (Yes, the parentheses really are needed.)"
	@echo -e "Main.run: O/Other"
	@echo -e ""
	@echo -e ""

Main.run: O/Other

##############################################################################
## Make control: O/Other
O/Other: subdir/xxx.o yyy.o subdir/xxx.o yyy.o
	@echo -e "O/Other: subdir/xxx.o yyy.o subdir/xxx.o yyy.o"
	@echo -e "$$\c"
	@echo -e "@     $@"
	@echo -e "$$\c"
	@echo -e "<     $<"
	@echo -e "$$\c"
	@echo -e "^     $^"
	@echo -e "$$\c"
	@echo -e "?     $?"
	@echo -e "$$\c"
	@echo -e "+     $+"
	@echo -e "$$\c"
	@echo -e "*     $*"
	@echo -e "$$\c"
	@echo -e "%     $%"
	@echo -e "$$\c"
	@echo -e "(<D)  $(<D)"
	@echo -e "$$\c"
	@echo -e "(@D)  $(@D)"
	@echo -e "$$\c"
	@echo -e "(<F)  $(<F)"
	@echo -e "$$\c"
	@echo -e "(@F)  $(@F)"
	@echo -e ""
	@echo -e "O/Other: Other.s subdir/foo.o bar.o"
	@echo -e "S/Other.C: subdir/bar.o foo.o"
	@echo -e ""
	@echo -e ""

O/Other: Other.s subdir/foo.o bar.o
S/Other.C: subdir/bar.o foo.o;
