##############################################################################
##
##       Copyright (c) 2019 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##############################################################################
## ~/src/py/ctl/Makefile
##############################################################################

.PHONY: default dirty do

default: do

dirty: ;
	-Main dirty 2>err.debug
	@echo -e "\n\nSTDERR:"
	@cat err.debug

do: ;
	-Main main 2>err.debug
	@echo -e "\n\nSTDERR:"
	@cat err.debug

.PHONY: clean pristine
pristine: clean
	-(cd ~/src/py; .Cleanup)

clean: ;
	-@rm -f debug.*
	-@rm -f err.debug
	-@rm -f *.stackdump

