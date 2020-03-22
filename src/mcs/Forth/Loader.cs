//----------------------------------------------------------------------------
//
//       Copyright (c) 2015-2019 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Loader.cs
//
// Purpose-
//       Forth built-in program loader.
//
// Last change date-
//       2019/01/13
//
// Implementation notes-
//       ** CODE FORMAT NOT UPDATED **
//
//----------------------------------------------------------------------------
using System;                       // (Universally required)

namespace Forth {
    public class Loader
    {
        Context c;                  // The working Context

        public Loader(Context c) { this.c = c; }

        void Push(Code code)        // Push code
        {
            Debug.assert( code != null );
            c.CodeMemory.Push(code);
        }

        void Push(string s)         // Push dictionary entry
        {
            Code code = c.Dictionary[s];
            Push(code);
        }

        public void Load()          // Load the CodeStack
        {
            c.AllocateWord("<00000>"); // (Exception if referenced!)
            c.DataStack.Push(0);   // Add fence words on top of stack
            c.DataStack.Push(-1);
            c.DataStack.Push(-2);
            c.DataStack.Push(-3);

            // Call <origin:>
            C_LABEL origin = new C_LABEL(c);
            Push(origin);
            Push("call");
            Push("EXIT");

            // Initialize
            origin.Addr = c.CodeMemory.Length; // origin= *HERE*

            Test();                 // Load Test, if any

            Push("ret");
        }

        public void Test()          // Load the CodeStack Test
        {
            #if TEST
                // (Bringup test)
                Push(new C_IMMED(c,7));
                Push(new C_IMMED(c,3));
                Push("-");
                Push(".");

                c.AllocateWord("WordOne");
                Push(new C_IMMED(c,11111));
                Push("WordOne");
                Push("!");

                c.AllocateWord("WordTwo");
                Push(new C_IMMED(c,22222));
                Push("WordTwo");
                Push("!");

                Push("WordOne");
                Push("_");
                Push(".");

                Push("WordTwo");
                Push("_");
                Push(".");

                Push("WordOne");
                Push("_");
                Push("WordTwo");
                Push("_");
                Push("+");
                Push(".");

                // Debugging displays
                c.DataStack.debug(true); // Dump the DataStack

                Debug.WriteLine("");
                c.CodeMemory.debug(true);

                Debug.WriteLine("");
                c.Dictionary.debug(true);

                Debug.WriteLine("");
            #endif
        }
    } // class Loader
} // namespace Forth

