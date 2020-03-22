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
//       Context.cs
//
// Purpose-
//       Forth working data areas.
//
// Last change date-
//       2015/01/12
//
// Implementation notes-
//       ** CODE FORMAT NOT UPDATED **
//
//----------------------------------------------------------------------------
using System;                       // (Univerally required)

namespace Forth {
    //------------------------------------------------------------------------
    // Context (Forth working data)
    //------------------------------------------------------------------------
    public class Context            // Forth Context
    {
        // Constants
        const int MEMORY_SIZE = 0x01000000; // Working memory
        internal static Word zeroWord = new Word();

        public Stack<Code>   CodeMemory = new Stack<Code>("CodeMemory");
     // public Stack<Word>   DataMemory = new Stack<Word>("DataMemory");
        public byte[]        UserMemory = new byte[MEMORY_SIZE];

        public Stack<Word>   DataStack  = new Stack<Word>("DataStack");
        public Stack<Word>   CallStack  = new Stack<Word>("CallStack");
        public Dictionary    Dictionary = new Dictionary();

        public int           Allocated;    // UserMemory allocated bytes
        public Word          BASE = 10;    // Current base
        public Word          IAW;          // Current Instruction Address Word
        public bool          RUNNING;      // Instruction processing active?
        public bool          TRACE = true; // Instruction trace active?

        public Context()
        {
            zeroWord = 0;

            // Initialize Dictionary
            // (Entries are automatically inserted by Code constructor)
            new C_NOP(this);        // [0] Always NOP

            new C_Add(this);
            new C_Sub(this);
            new C_Mul(this);
            new C_Div(this);
            new C_Mod(this);

            new C_CONST(this, "-1", -1);
            new C_CONST(this, "0", 0);
            new C_CONST(this, "1", 1);

            new C_CALL(this);
            new C_DOT(this);
            new C_DUP(this);
            new C_GOTO(this);
            new C_RET(this);
            new C_TOP(this);
            new C_POP(this);

            new C_BASE(this);
            new C_DEC(this);
            new C_HEX(this);
            new C_OCT(this);
            new C_PEEK(this);
            new C_POKE(this);

            new C_QUIT(this);
        }

        public C_LABEL AllocateWord(string s = null) { // Returns associated Label name
            // Round up allocation  to Word boundary
            Allocated += Word.Size - 1;
            Allocated &= ~(Word.Size - 1);
            zeroWord.Poke(this, Allocated);

            C_LABEL label = null;
            if( s != null )
                label = new C_LABEL(this, s, Allocated);

            Allocated += Word.Size;

            return label;
        }
    }
} // namespace Forth
