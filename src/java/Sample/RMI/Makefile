##############################################################################
##
##       Copyright (C) 2007 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##############################################################################
## Makefile
##############################################################################

##############################################################################
## Controls
CODEBASE = -Djava.rmi.server.codebase=http://woodstock.watson.ibm.com/tmp
CODEBASE = -Djava.rmi.server.codebase=file://./
CODEBASE = -Djava.rmi.server.codebase=file://$(PWD)/
SECURITY = -Djava.security.policy=./policy

##############################################################################
## Include compiler controls
include M/Makefile.prefix

##############################################################################
## Local module list
MAKSRC = \
	 Main.java    \
	 ServerIF.java\
	 ServerOB.java\
	 ObjectIF.java\
	 ObjectOB.java\

MAKRUN = $(patsubst %.java,%.class,$(MAKSRC))

SKEL   = ServerOB_Skel.class ObjectOB_Skel.class

STUB   = ServerOB_Stub.class ObjectOB_Stub.class

##############################################################################
## Set default target
ifeq "" "$(DEFAULT)"
DEFAULT = $(MAKRUN) $(SKEL) $(STUB)
endif

.PHONY: default
default: $(DEFAULT)

.PHONY: client
client: ;
	java $(CODEBASE) $(SECURITY) Main client

.PHONY: server
server: ;
	java $(CODEBASE) $(SECURITY) Main server

.PHONY: config
config: ;
	java $(CODEBASE) $(SECURITY) Main config

##############################################################################
## Dependencies
Main.class : *.java
	javac Main.java

ObjectOB_Skel.class ObjectOB_Stub.class : ObjectOB.class
	rmic ObjectOB

ServerOB_Skel.class ServerOB_Stub.class : ServerOB.class
	rmic ServerOB

##############################################################################
## Include default targets
include M/Makefile.suffix

