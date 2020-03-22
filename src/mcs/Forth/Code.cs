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
//       Code.cs
//
// Purpose-
//       Code (operator) definitions.
//
// Last change date-
//       2015/01/25
//
// Implementation notes-
//       ** CODE FORMAT NOT UPDATED **
//
//----------------------------------------------------------------------------
using System;                       // (Univerally required)

namespace Forth {
    public class Code {
        private string token;
        public  string Token { get { return token; } }

        public  virtual void Op(Context c) {}
        internal Code(Context c, string token) {
            this.token = token;
            if( token != null )
            {
                // Insert label into dictionary
                c.Dictionary.Push(this);
            }
        }

        public override string ToString() { return token ?? "*"; }
    }

    //------------------------------------------------------------------------
    // Arithmetic operators
    public class C_Add: Code {
        // L= pop[TOP]; R= pop[TOP-1]; push(L + R)
        public C_Add(Context c) : base(c, "+") {}

        public sealed override void Op(Context c) {
            c.DataStack.Poke(c.DataStack.Pop() + c.DataStack.Peek());
        }
    }

    public class C_Sub: Code {
        // L= pop[TOP]; R= pop[TOP-1]; push(R - L)
        public C_Sub(Context c) : base(c, "-") {}

        public sealed override void Op(Context c) {
            Word L = c.DataStack.Pop();
            c.DataStack.Poke(c.DataStack.Peek() - L);
        }
    }

    public class C_Mul: Code {
        // L= pop[TOP]; R= pop[TOP-1]; push(L * R)
        public C_Mul(Context c) : base(c, "*") {}

        public sealed override void Op(Context c) {
            c.DataStack.Poke(c.DataStack.Pop() * c.DataStack.Peek());
        }
    }

    public class C_Div: Code {
        // L= pop[TOP]; R= pop[TOP-1]; push(R / L)
        public C_Div(Context c) : base(c, "/") {}

        public sealed override void Op(Context c) {
            Word L = c.DataStack.Pop();
            c.DataStack.Poke(c.DataStack.Peek() / L);
        }
    }

    public class C_Mod: Code {      // Modulus
        // L= pop[TOP]; R= pop[TOP-1]; push(R % L)
        public C_Mod(Context c) : base(c, "%") {}

        public sealed override void Op(Context c) {
            Word L = c.DataStack.Pop();
            c.DataStack.Poke(c.DataStack.Peek() % L);
        }
    }

    //------------------------------------------------------------------------
    // Utility operators
    public class C_CALL: Code {     // CALL (executable)
        public C_CALL(Context c) : base(c, "call") { }

        public sealed override void Op(Context c) {
            c.CallStack.Push(c.IAW); // Set return address
            c.IAW = c.DataStack.Pop(); // goto (address from data stack)
        }
    }

    public class C_CONST: Code {    // Named constant
        Word value;

        public C_CONST(Context c, string s, Word v) : base(c, s) { value= v; }

        public sealed override void Op(Context c) {
            c.DataStack.Push(value);
        }

        public override string ToString() {
            return String.Format("{0} {1}", base.ToString(), value);
        }
    }

    public class C_DOT: Code {      // Display (and consume) TOP stack element
        // L= pop[TOP]; <Display L using current base>
        public C_DOT(Context c) : base(c, ".") {}

        public sealed override void Op(Context c) {
            Word w = c.DataStack.Pop();
            Console.Write("{0} ", w.ToString(c));
        }
    }

    public class C_DUP: Code {      // Duplicate TOP stack element
        public C_DUP(Context c) : base(c, "dup") {}

        public sealed override void Op(Context c) {
            c.DataStack.Push(c.DataStack.Peek());
        }
    }

    public class C_GOTO: Code {     // GOTO (executable)
        public C_GOTO(Context c) : base(c, "goto") { }

        public sealed override void Op(Context c) {
            c.IAW = c.DataStack.Pop(); // goto (address from data stack)
        }
    }

    public class C_IMMED: Code {    // Immediate (constant) value
        Word value;

        public C_IMMED(Context c, Word v) : base(c, null) { value= v; }

        public sealed override void Op(Context c) {
            c.DataStack.Push(value);
        }

        public override string ToString() { return "<push> " + value; }
    }

    public class C_LABEL: Code {    // Address (set once) value
        public long Addr;           // Allows set

        public C_LABEL(Context c, string s = null, long a = 0) : base(c, s) { Addr= a; }

        public sealed override void Op(Context c) {
            if( Addr == 0 )
                throw new Exception("Undefined Label(" + Token + ")");

            c.DataStack.Push(Addr);
        }

        public override string ToString() {
            return String.Format("{0}: ({1})", base.ToString(), Addr);
        }
    }

    public class C_NOP: Code {      // No OPeration
        public C_NOP(Context c) : base(c, "nop") {}

        public sealed override void Op(Context c) {
        }
    }

    public class C_BASE: Code {     // Set the base
        // A= pop[TOP]; BASE= A;
        public C_BASE(Context c) : base(c, "base") {}

        public sealed override void Op(Context c) {
            Word A = c.DataStack.Pop();
            if( A < 2 || A > 16 )
                Console.WriteLine("Invalid Base({0})", A);
            else
                c.BASE = A;
        }
    }

    public class C_DEC: Code {      // Set the base = 10
        // BASE= 10;
        public C_DEC(Context c) : base(c, "dec") {}

        public sealed override void Op(Context c) {
            c.BASE = 10;
        }
    }

    public class C_HEX: Code {      // Set the base = 16
        // BASE= 16;
        public C_HEX(Context c) : base(c, "hex") {}

        public sealed override void Op(Context c) {
            c.BASE = 16;
        }
    }

    public class C_OCT: Code {      // Set the base = 8
        // BASE= 8;
        public C_OCT(Context c) : base(c, "oct") {}

        public sealed override void Op(Context c) {
            c.BASE = 8;
        }
    }

    public class C_PEEK: Code {     // Fetch from UserMemory
        // A= pop[TOP]; push((word)UserMemory[A])
        public C_PEEK(Context c) : base(c, "_") {}

        public sealed override void Op(Context c) {
            int  A = (int)c.DataStack.Pop();
            Word R = new Word();
            R.Peek(c, A);
            c.DataStack.Push(R);
        }
    }

    public class C_POKE: Code {     // Store into UserMemory
        // A= pop[TOP]; V= pop[TOP-1]; (word)UserMemory[A]= V
        public C_POKE(Context c) : base(c, "!") {}

        public sealed override void Op(Context c) {
            int  A = (int)c.DataStack.Pop();
            Word V = c.DataStack.Pop();
            V.Poke(c, A);
        }
    }

    public class C_RET: Code {      // RETURN (executable)
        public C_RET(Context c) : base(c, "ret") { }

        public sealed override void Op(Context c) {
            c.IAW = c.CallStack.Pop();
        }
    }

    public class C_TOP: Code {      // Display TOP element (stack unchanged)
        public C_TOP(Context c) : base(c, "top") {}

        public sealed override void Op(Context c) {
            c.Dictionary["dup"].Op(c);
            c.Dictionary["."].Op(c);
        }
    }

    public class C_POP: Code {     // Discard the top element
        // A= pop[TOP]; // (Discarded)
        public C_POP(Context c) : base(c, "pop") {}

        public sealed override void Op(Context c) {
            c.DataStack.Pop();
        }
    }

    // Quit operation and aliases
    public class C_ALIAS: Code {
        Code alias;

        public C_ALIAS(Context c, Code code, string t) : base(c, t) { alias= code; }

        public sealed override void Op(Context c) {
            alias.Op(c);
        }
    }

    public class C_QUIT: Code {
        public C_QUIT(Context c) : base(c, "quit") {
            new C_ALIAS(c, this, "QUIT");
            new C_ALIAS(c, this, "exit");
            new C_ALIAS(c, this, "EXIT");
            new C_ALIAS(c, this, "bye");
            new C_ALIAS(c, this, "BYE");
            new C_ALIAS(c, this, "end");
            new C_ALIAS(c, this, "END");
        }

        public sealed override void Op(Context c) {
            c.RUNNING = false;
        }
    }
} // namespace Forth
