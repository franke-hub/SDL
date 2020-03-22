//-----------------------------------------------------------------------------
//
//       Copyright (c) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       Utility.cs
//
// Purpose-
//       Utility routines.
//
// Last change date-
//       2019/01/17
//
//-----------------------------------------------------------------------------
using System;                       // (Almost always required)

namespace Sample {                  // The Sample namespace
//=============================================================================
// Utility: Utility routines
//=============================================================================
static public class Utility {
//-----------------------------------------------------------------------------
// Utility.Attributes
//-----------------------------------------------------------------------------
static public bool     ALWAYS_TRUE= true; // But the compiler can't know that!
static DateTime        tod_origin= new DateTime(1970,1,1,0,0,0);

//-----------------------------------------------------------------------------
// Utility.Methods
//-----------------------------------------------------------------------------
static public double tod()          // Current unix time (in seconds)
{
   TimeSpan ts= DateTime.UtcNow - tod_origin;
   return ts.TotalSeconds;
}

static public string nullify(string s) // Return: s != null ? s : "<null>"
{
   return s != null ? s : "<null>";
}
}  // class Utility
}  // namespace Sample
