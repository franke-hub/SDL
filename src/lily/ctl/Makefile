##############################################################################
##
##       Copyright (C) 2007-2020 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##############################################################################
##
## Title-
##       Makefile
##
## Purpose-
##       Makefile for lilypond programs
##
## Last change date-
##       2020/02/03
##
## Preparation-
##       In your object directory,
##         ln -s <Source_directory>  S
##         ln -s <This_directory>    M
##         ln -s M/Makefile Makefile
##
## Commands-
##       make clean    Remove garbage files and resultants.
##       make pristine "make clean" plus remove ...
##
## Reserved file names-
##       *WARNING* These files are subject to automatic deletion.
##
##       *.eps         Postscript intermeditate files.
##       *.midi        MIDI output files.
##       *.pdf         PDF acroread files.
##       *.png         Display files.
##       *.ps          Postscript files.
##       core          System core dump.
##       errs          Error output from certain script files.
##
##############################################################################

##############################################################################
## Set default paths
MAKDIR = M######################### Makefile directory
OBJDIR = .######################### Object directory
SRCDIR = S######################### Source directory

##############################################################################
## CMD      (Commands)
ifeq "CYGWIN" "$(findstring CYGWIN,$(shell uname))"
LILY   := lilypond ################ Lilypond
MIDI   := timidity ################ Midi player
PDF    := xpdf #################### PDF player
PS     := gv ###################### PS player
endif

ifeq "Linux"  "$(findstring Linux,$(shell uname))"
LILY   := lilypond ################ Lilypond
MIDI   := timidity ################ Midi player
PDF    := xpdf #################### PDF player
PS     := gv ###################### PS player
endif

##############################################################################
## LOPTS    (Compiler options)
LOPTS  += ######################### Options
LFLAGS := $(LOPTS)

##############################################################################
## Default rules
vpath

.SUFFIXES:
.SUFFIXES: .eps .pdf .png .ps
## Make% : ;
## make% : ;

%.midi : $(SRCDIR)/%.ly
	$(LILY) $(LFLAGS) $<
	@touch $@

%.pdf : $(SRCDIR)/%.ly
	$(LILY) $(LFLAGS) $<
	@touch $@

%.ps : $(SRCDIR)/%.ly
	$(LILY) --ps $(LFLAGS) $<
	@touch $@

##############################################################################
## Default source, executable and object files
ifeq "" "$(SOURCE)"
  SOURCE := $(wildcard $(SRCDIR)/*.ly)
endif
OBJMID := $(patsubst $(SRCDIR)/%.ly,$(OBJDIR)/%.midi,$(SOURCE))
OBJPDF := $(patsubst $(SRCDIR)/%.ly,$(OBJDIR)/%.pdf,$(SOURCE))
OBJPS  := $(patsubst $(SRCDIR)/%.ly,$(OBJDIR)/%.ps,$(SOURCE))
OBJECT := $(OBJMID) $(OBJPDF) $(OBJPS)

##############################################################################
## Include implementation file
INCSRC :=Makefile.BSD############## Default Control file
ifeq "$(findstring $(SRCDIR)/$(INCSRC),$(wildcard $(SRCDIR)/Makefile.*))" "$(SRCDIR)/$(INCSRC)"
INCSRC := $(SRCDIR)/$(INCSRC)
else
INCSRC := $(MAKDIR)/$(INCSRC)
endif
include $(INCSRC)

##############################################################################
## TARGET: nothing (Default when no default.)
.PHONY: nothing
nothing: ;
	-@echo "No default target specified"

##############################################################################
## TARGET: midi (Play midi files.)
.PHONY: midi
midi: $(OBJMID)
	$(MIDI) $?

##############################################################################
## TARGET: pdf (Play pdf files.)
.PHONY: pdf
pdf: $(OBJPDF)
	$(PDF) $?

##############################################################################
## TARGET: ps (Play ps files.)
.PHONY: ps
ps: $(OBJPS)
	$(PS) --media=letter $?

##############################################################################
## TARGET: CLEAN, PRISTINE
.PHONY: pristine clean
pristine: ;
	-@rm *.pdf *.ps *.midi             >/dev/null 2>/dev/null
	-@rm core errs *.eps               >/dev/null 2>/dev/null

clean: ;
	-@rm $(OBJECT)                     >/dev/null 2>/dev/null

