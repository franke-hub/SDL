##############################################################################
##
##       Copyright (C) 2015-2019 Frank Eskesen.
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
%.o : ;
S/%.C: ;

##############################################################################
## Local module list
MAKRUN = \
    Main.run

MAKOBJ = \
    Main.s     \
    Alpha.o    \
    ../subdir/Beta.o     \
    Gamma.o    \

##############################################################################
## Set default target
ifeq "" "$(DEFAULT)"
DEFAULT = $(MAKRUN)
endif

.PHONY: default
default: $(DEFAULT)

##############################################################################
## Make control: Generic rule
%.s : S/%.C
	@echo -e "%.s : S/%.C"
	@echo -e "$$\c"
	@echo -e "(\@)  $@"
	@echo -e "$$\c"
	@echo -e "(\<)  $<"
	@echo -e "$$\c"
	@echo -e "(\^)  $^"
	@echo -e "$$\c"
	@echo -e "(\?)  $?"
	@echo -e "$$\c"
	@echo -e "(\+)  $+"
	@echo -e "$$\c"
	@echo -e "(\*)  $*"
	@echo -e "$$\c"
	@echo -e "(\%)  $%"
	@echo -e "$$\c"
	@echo -e "(\<D) $(<D)"
	@echo -e "$$\c"
	@echo -e "(\<F) $(<F)"
	@echo -e ""
	@echo -e "%o: ;"
	@echo -e "S/%.C: ;"
	@echo -e ""
	@echo -e ""

##############################################################################
## Make control: Main
Main.run: $(MAKOBJ)
	@echo -e "Main.run: $(MAKOBJ)"
	@echo -e "$$\c"
	@echo -e "(\@)  $@"
	@echo -e "$$\c"
	@echo -e "(\<)  $<"
	@echo -e "$$\c"
	@echo -e "(\^)  $^"
	@echo -e "$$\c"
	@echo -e "(\?)  $?"
	@echo -e "$$\c"
	@echo -e "(\+)  $+"
	@echo -e "$$\c"
	@echo -e "(\*)  $*"
	@echo -e "$$\c"
	@echo -e "(\%)  $%"
	@echo -e "$$\c"
	@echo -e "(\<D) $(<D)"
	@echo -e "$$\c"
	@echo -e "(\<F) $(<F)"
	@echo -e ""
	@echo -e "Main.run: Other"

Main.run: Other

##############################################################################
## Make control: Other
Other: foo.o bar.o foo.o bar.o
	@echo -e "Other: foo.o bar.o foo.o bar.o"
	@echo -e "$$\c"
	@echo -e "(\@)  $@"
	@echo -e "$$\c"
	@echo -e "(\<)  $<"
	@echo -e "$$\c"
	@echo -e "(\^)  $^"
	@echo -e "$$\c"
	@echo -e "(\?)  $?"
	@echo -e "$$\c"
	@echo -e "(\+)  $+"
	@echo -e "$$\c"
	@echo -e "(\*)  $*"
	@echo -e "$$\c"
	@echo -e "(\%)  $%"
	@echo -e "$$\c"
	@echo -e "(\<D) $(<D)"
	@echo -e "$$\c"
	@echo -e "(\<F) $(<F)"
	@echo ""

