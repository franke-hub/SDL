##############################################################################
##
##       Copyright (C) 2010 Frank Eskesen.
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
##       Makefile for JAVA programs
##
## Last change date-
##       2010/01/01
##
## Preparation-
##       In your source directory,
##         ln -s <Source_directory>  S
##         ln -s <This_directory>    M
##         ln -s M/Makefile Makefile
##
## Commands-
##       make do        Execute Main.class.
##       make clean     Remove *.class
##       make pristine  "make clean" + Remove *.jar
##
## Reserved file names-
##       *WARNING* These files are subject to automatic deletion.
##
##       *.class        Object files.
##       *.jar          Library files.
##
##############################################################################

##############################################################################
## Set default paths
OBJDIR = .######################### Object directory
SRCDIR = S######################### Source directory
MAKDIR = M######################### M(akefile) directory

##############################################################################
## SOURCEDIR, OBJECTDIR (Used in compilation, can be overridden)
export SOURCEDIR=$(SRCDIR)
export OBJECTDIR=$(OBJDIR)

##############################################################################
## Include option control file
INCSRC :=Makefile.OPT############## Default Control file
ifneq "$(findstring $(INCSRC),$(wildcard Makefile.*))" "$(INCSRC)"
INCSRC := $(MAKDIR)/$(INCSRC)
endif
include $(INCSRC)

##############################################################################
## Default rules
vpath

.SUFFIXES:
.SUFFIXES: .java .class
Make% : ;
make% : ;

%.class : %.java
	$(JC) $(JFLAGS) "$(shell cygpath --path --windows "$<")"

$(OBJDIR)/%.class : $(SRCDIR)/%.java
	$(JC) $(JFLAGS) "$(shell cygpath --path --windows "$<")"

##############################################################################
## Default source and object files
MAKSRC := $(wildcard $(SRCDIR)/*.java)
MAKOBJ := $(patsubst $(SRCDIR)/%.java,$(OBJDIR)/%.class,$(MAKSRC))

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
## TARGET: do
ifeq "" "$(MAKRUN)"
MAKRUN := Main
endif

.PHONY: do
do: $(MAKOBJ)
	$(JRUN) -classpath "$(shell cygpath --path --windows "$(CLASSPATH)")" $(MAKRUN)

##############################################################################
## TARGET: CLEAN, PRISTINE
.PHONY: pristine clean
pristine: clean
	-@rm *.class    >/dev/null 2>/dev/null

clean: ;
	-@rm *.jar      >/dev/null 2>/dev/null

