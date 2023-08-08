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
clean: defined-root
	(cd $(CPP_); $(MAKE) clean)

##----------------------------------------------------------------------------
pristine: defined-root
	(cd $(CPP_); $(MAKE) pristine)

##############################################################################
## TARGET: environment: Insure $SDL_ROOT is defined and valid
.PHONY: environment defined-root valid-path

##----------------------------------------------------------------------------
## Insure required subdirectories exist
environment: defined-root
environment: $(SDL_ROOT)/src $(SDL_ROOT)/obj $(SDL_ROOT)/bin
environment: $(CPP_) $(JAVA_) $(MCS_) $(PY_)

$(SDL_ROOT)/bin: ;
	@echo "Missing file: '$(SDL_ROOT)/bin', SDL_ROOT improperly defined"
	@false

$(SDL_ROOT)/obj: ;
	@echo "Missing file: '$(SDL_ROOT)/obj', SDL_ROOT improperly defined"
	@false

$(SDL_ROOT)/src: ;
	@echo "Missing file: '$(SDL_ROOT)/src', SDL_ROOT improperly defined"
	@false

$(CPP_): ;
	@echo "Missing file: '$(CPP_)', SDL_ROOT improperly defined"
	@false

$(JAVA_): ;
	@echo "Missing file: '$(JAVA_)', SDL_ROOT improperly defined"
	@false

$(MCS_): ;
	@echo "Missing file: '$(MCS_)', SDL_ROOT improperly defined"
	@false

$(PY_): ;
	@echo "Missing file: '$(PY_)', SDL_ROOT improperly defined"
	@false

##----------------------------------------------------------------------------
## Insure SDL_ROOT is defined
ifeq "" "$(SDL_ROOT)"
defined-root: ;
	@echo "Environment variable SDL_ROOT is undefined."
	@echo "Use 'source setupSDL' to initialize required Environment variables."
	@false
else
defined-root: ;
	@true
endif

##----------------------------------------------------------------------------
## Insure $(SDL_ROOT)/bin is in $PATH
environment: valid-path
valid-path: $(SDL_ROOT)/bat/sys/is-valid-path
	@$(SDL_ROOT)/bat/sys/is-valid-path

$(SDL_ROOT)/bat/sys/is-valid-path: ;
	@echo "Missing file: '$(SDL_ROOT)/bat/sys/is-valid-path', SDL_ROOT improperly defined"
	@false
