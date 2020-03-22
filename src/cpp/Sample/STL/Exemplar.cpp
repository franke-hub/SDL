//----------------------------------------------------------------------------
//
//       Copyright (c) 2017 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Exemplar.cpp
//
// Purpose-
//       Sample usage exemplars.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#include <sstream>
#include <string>
#include <thread>

#include "Main.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#if defined(HCDM) && !defined(SCDM)
  #define SCDM
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       using_stringstream
//
// Purpose-
//       Use of stringstream (type of this_thread::get_id() may differ.)
//
//----------------------------------------------------------------------------
static void
   using_stringstream( void )
{
   Logger::log("Exemplar::using_stringstream()\n");

   std::stringstream ss; ss << this_thread::get_id();
   Logger::log("%4d thread_id(%s)\n", __LINE__, ss.str().c_str());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       exemplar
//
// Purpose-
//       Usage examples.
//
//----------------------------------------------------------------------------
extern void
   exemplar( void )
{
   wtlc(LevelStd, "Exemplar()\n");

   // Exemplars
   using_stringstream();
}

