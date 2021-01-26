//----------------------------------------------------------------------------
//
//       Copyright (C) 2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Config.h
//
// Purpose-
//       Configuration controls
//
// Last change date-
//       2021/01/24
//
//----------------------------------------------------------------------------
#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

//----------------------------------------------------------------------------
//
// Namespace-
//       config
//
// Purpose-
//       Configuration controls
//
//----------------------------------------------------------------------------
namespace config {                  // Configuration controls
extern int             opt_hcdm;    // Hard Core Debug Mode?
extern const char*     opt_test;    // Bringup test?
extern int             opt_verbose; // Debugging verbosity
}  // namespace config
#endif // CONFIG_H_INCLUDED
