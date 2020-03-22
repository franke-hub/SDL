##############################################################################
##
##       Copyright (c) 2018 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##############################################################################
##
## Title-
##       README.BUILD
##
## Purpose-
##       Modified JPEG library installation instructions.
##
## Implementation notes-
##       These instructions apply only to the installation of the library
##       within the local build environment.
##
##       Requirements:
##         ~/src/c/lib/1st/jpeg     (THIS directory)
##         ~/src/c/lib/jpeg.bsd     (Target for Cygwin/Linux source)
##         ~/src/c/inc/jpeg/bsd     (Target for Cygwin/Linux includes)
##         ~/src/cpp/inc/jpeg/bsd   (Target for Cygwin/Linux includes)
##         ~/src/c/lib/jpeg.win     (Target for WINDOWS source)
##         ~/src/c/inc/jpeg/win     (Target for WINDOWS includes)
##         ~/src/cpp/inc/jpeg/win   (Target for WINDOWS includes)
##
##         Other source and object file directories contained within the
##         local build environment, notably:
##           ~/src/c/lib/Makefile.BSD
##           ~/src/c/lib/Makefile.WIN
##           ~/obj/c/lib/*
##           ~/obj/c/lib/jpeg/*
##           ~/obj/cpp/lib/*
##           ~/obj/cpp/lib/jpeg/*
##
##       The included install scripts must be run from THIS subdirectory:
##           ~/src/c/lib/1st/jpeg
##
## Implementation notes-
##       The install scripts create backup of the current environment:
##         ~/src/c/lib/jpeg.bsd   into ~/src/c/lib/jpeg.bsd.old
##         ~/src/c/inc/jpeg/bsd   into ~/src/c/inc/jpeg/bsd.old
##         ~/src/cpp/inc/jpeg/bsd into ~/src/cpp/inc/jpeg/bsd.old
##         ~/src/c/lib/jpeg.win   into ~/src/c/lib/jpeg.win.old
##         ~/src/c/inc/jpeg/win   into ~/src/c/inc/jpeg/win.old
##         ~/src/cpp/inc/jpeg/win into ~/src/cpp/inc/jpeg/win.old
##
##       Remove these backups at your discretion.
##
##############################################################################
##
## Installation instructions (version jpeg-9c)
## -------------------------------------------
## Import source code from: http://www.ijg.org/
##
## WINDOWS (Visual Studio 2017):
##    Download: jpegsrc9c.zip
##    run INSTALL.BAT
##
## Cygwin/Linux:
##    Download: jpegsrc.v9c.tar.gz
##    run install.bsd
##
## The build scripts create the initial libraries.
## These libraries can be rebuilt from:
##    ~/obj/c/lib
##    ~/obj/cpp/lib
##
## The libraries can be partially rebuilt from:
##    ~/obj/c/lib/jpeg
##    ~/obj/cpp/lib/jpeg
##
## Using these make options-
##    make           (Standard make with dependencies)
##    make clean     removes libraries and links
##    make depend    regenerates dependencies
##    make pristine  removes libraries, links, and object file
