##############################################################################
##
##       Copyright (C) 2023-2024 Frank Eskesen.
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
##       2024/02/01
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
	@echo "  make update: Update an installation."
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
## TARGETS: check, compile, clean, install, pristine, reinstall,
## TARGETS: uninstall, update
.PHONY: check compile clean install pristine reinstall
.PHONY: uninstall update

##----------------------------------------------------------------------------
install: environment
	(cd $(CPP_); $(MAKE) install)
	(cd $(PY_);  $(MAKE) install)

##----------------------------------------------------------------------------
reinstall: environment
	(cd $(CPP_); $(MAKE) reinstall)
	(cd $(PY_);  $(MAKE) reinstall)

##----------------------------------------------------------------------------
uninstall: environment
	(cd $(CPP_); $(MAKE) uninstall)
	(cd $(PY_);  $(MAKE) uninstall)

##----------------------------------------------------------------------------
update: environment
	(cd $(CPP_); $(MAKE) update)
	(cd $(PY_);  $(MAKE) update)

##----------------------------------------------------------------------------
check: environment
	(cd $(CPP_); $(MAKE) check)
	(cd $(PY_);  $(MAKE) check)

##----------------------------------------------------------------------------
compile: environment
	(cd $(CPP_); $(MAKE) compile)
	(cd $(PY_);  $(MAKE) compile)

##----------------------------------------------------------------------------
clean: defined-root
	(cd $(CPP_); $(MAKE) clean)
	(cd $(PY_);  $(MAKE) clean)

##----------------------------------------------------------------------------
pristine: defined-root
	(cd $(CPP_); $(MAKE) pristine)
	(cd $(PY_);  $(MAKE) pristine)

##############################################################################
## TARGET: environment: Insure $SDL_ROOT is defined and valid
.PHONY: environment defined-root valid-path

##----------------------------------------------------------------------------
## Insure required subdirectories exist
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
## Insure *ONLY* that SDL_ROOT is defined
environment: defined-root
ifeq "" "$(SDL_ROOT)"
defined-root: ;
	@echo "Environment variable SDL_ROOT is undefined."
	@echo "Use 'source setupSDL' to initialize the environment variables."
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
