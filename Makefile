##############################################################################
##
##       Copyright (C) 2023 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##############################################################################
##
## Title-
##       (~/)Makefile
##
## Purpose-
##       Build control makefile.
##
## Last change date-
##       2023/08/06
##
##############################################################################

##############################################################################
## Default action when $SDL_ROOT is not initialized
ifeq "" "$(SDL_ROOT)"               ## If $SDL_ROOT is undefined
.PHONY: not-initialized
not-initialized: ;
	@echo "Environment variable SDL_ROOT is undefined."
	@echo "Use 'source setupSDL' to initialize required Environment variables."
endif

##############################################################################
## Default action
.PHONY: list-options
list-options: ;
	@echo "Available options:"
	@echo "  make install: Build all installation prerequisites and libraries."
	@echo "    Use this to initialize or update your installation."
	@echo "  make uninstall: Remove prerequisites and build libraries."
	@echo "  make reinstall: (make uninstall; make install)"
	@echo "    Use this after installing a new Linux version."
	@echo
	@echo "  make check: Compile and run all test suites."
	@echo "  make compile: Compile all source files."
	@echo
	@echo "  make clean: Remove all 'make check' and 'make compile' files."
	@echo "  make pristine: Performs 'make clean' and 'make uninstall' "
	@echo

##############################################################################
## Target directories
CPP_   := $(SDL_ROOT)/obj/cpp
JAVA_  := $(SDL_ROOT)/obj/java
MCS_   := $(SDL_ROOT)/obj/mcs
PY_    := $(SDL_ROOT)/obj/py

##############################################################################
## TARGETS: install, reinstall, uninstall, check, compile, clean, pristine
.PHONY: install uninstall reinstall check compile clean pristine

##----------------------------------------------------------------------------
install: environment
	(cd $(CPP_); $(MAKE) install)

##----------------------------------------------------------------------------
reinstall: environment
	(cd $(CPP_); $(MAKE) reinstall)

##----------------------------------------------------------------------------
uninstall: environment
	(cd $(CPP_); $(MAKE) uninstall)

##----------------------------------------------------------------------------
check: environment
	(cd $(CPP_); $(MAKE) check)

##----------------------------------------------------------------------------
compile: environment
	(cd $(CPP_); $(MAKE) compile)

##----------------------------------------------------------------------------
clean: ;
	(cd $(CPP_); $(MAKE) clean)

##----------------------------------------------------------------------------
pristine: ;
	(cd $(CPP_); $(MAKE) pristine)

##############################################################################
## TARGET: environment: Insure $SDL_ROOT is defined and valid
.PHONY: environment
##----------------------------------------------------------------------------
## Insure required subdirectories exist
environment: $(SDL_ROOT)/src $(SDL_ROOT)/obj $(SDL_ROOT)/bin
environment: $(CPP_) $(JAVA_) $(MCS_) $(PY_)

##----------------------------------------------------------------------------
## Insure $(SDL_ROOT)/bin is in $PATH
environment: is-valid-path
is-valid-path: ;
	@(is_valid=`echo "$(PATH)" | grep $(SDL_ROOT)/bin:`; [ -n "$(is_valid)" ] && false; true)
