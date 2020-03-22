//----------------------------------------------------------------------------
//
//       Copyright (c) 2015 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Core.cs
//
// Purpose-
//       Core definitions.
//
// Last change date-
//       2015/01/23
//
// Implementation notes-
//       ** CODE FORMAT NOT UPDATED **
//
//----------------------------------------------------------------------------
using System;                       // (Univerally required)
using System.Runtime.CompilerServices; // For [Caller...] Attributes

namespace Forth {
    //------------------------------------------------------------------------
    // Core constants and methods
    //------------------------------------------------------------------------
    public static class Core        // Constants
    {
        public const string VERSION_ID = "0.0.0";
        public static string File([CallerFilePath] string file = "")
            {   int L = file.Length;
                while( L > 0 ) {
                    char C = file[L - 1];
                    if( C == '\\' || C == '/' )
                        break;

                    L--;
                }

                return file.Substring(L);
            }
        public static int    Line([CallerLineNumber] int line = 0) { return line; }
        public static string Path([CallerFilePath] string file = "") { return file; }
        public static bool   StringApproxEquals(string L, string R)
            { return string.Equals(L, R, StringComparison.CurrentCultureIgnoreCase); }
    } // class Core
} // namespace Forth
