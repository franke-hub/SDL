//----------------------------------------------------------------------------
//
//       Copyright (C) 2006-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       makeproj.hpp
//
// Function-
//       Define constants and controls required by makeproj.cpp
//
// Last change date-
//       2023/05/19
//
//----------------------------------------------------------------------------
#ifndef MAKEPROJ_HPP_INCLUDED
#define MAKEPROJ_HPP_INCLUDED

#include "makeproj.hpp"             // Circular include (for testing)

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#undef  TRUE
#define TRUE  1
#undef  FALSE
#define FALSE 0

//----------------------------------------------------------------------------
// Machine identification
//----------------------------------------------------------------------------
#if ' ' == 0x40                     // If EBCDIC machine
   #define EBCDIC_MACHINE -1
#else                               // ASCII machine
   #define ASCII_MACHINE -1
#endif

//----------------------------------------------------------------------------
// Compiler dependent controls
//----------------------------------------------------------------------------
#ifdef EBCDIC_MACHINE               // EBCDIC machine
   #define indent(a) fputs("  " ,a)
   #define ASM_FILETYPE ".assemble"
   #define EXEC_FILETYPE ".module"
   #define TEXT_FILETYPE ".text"

   #define BOM_ATTRIBUTE "w,recfm=v,lrecl=80"
   #define BOM_EXTENSION " bom a"

   #define MAKE_ATTRIBUTE "w,recfm=v,lrecl=255"
   #define MAKE_EXTENSION " incl a"

#else                               // ASCII machine
   #define indent(a) fputs("\t" ,a)
   #define ASM_FILETYPE ".s"
   #ifdef _OS_WIN
      #define EXEC_FILETYPE ".exe"
      #define TEXT_FILETYPE ".obj"
   #else
      #define EXEC_FILETYPE ""
      #define TEXT_FILETYPE ".o"
   #endif

   #define BOM_ATTRIBUTE "w"
   #define BOM_EXTENSION ".bom"

   #define MAKE_ATTRIBUTE "w"
   #define MAKE_EXTENSION ".incl"
#endif

#endif // MAKEPROJ_HPP_INCLUDED
