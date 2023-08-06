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
##       2023/08/04
##
## Implementation notes-
##       Use '. setupSDL' to initialize your environment, then 'make'.
##
##############################################################################

##############################################################################
## Target directories
CPP_   := $(SDL_ROOT)/obj/cpp
JAVA_  := $(SDL_ROOT)/obj/java
PY_    := $(SDL_ROOT)/obj/py

##############################################################################
## Default action
.PHONY: default
default: environment
	(cd $(CPP_); $(MAKE))

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
ifeq "" "$(SDL_ROOT)"
environment: ;
	@echo "SDL_ROOT is undefined. Use '. setupSDL' to set it"
	@false

else
##----------------------------------------------------------------------------
## Insure required subdirectories exist
environment: $(SDL_ROOT)/src $(SDL_ROOT)/obj $(SDL_ROOT)/bin
environment: $(CPP_) $(PY_) $(JAVA_)

##----------------------------------------------------------------------------
## Insure $(SDL_ROOT)/bin is in $PATH
environment: is-valid-path
is-valid-path: ;
	@(is_valid=`echo "$(PATH)" | grep $(SDL_ROOT)/bin:`; [[ -n "$(is_valid)" ]] && false; true)
endif
