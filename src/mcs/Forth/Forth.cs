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
//       Forth.cs
//
// Purpose-
//       Forth console.
//
// Last change date-
//       2015/01/25
//
// Implementation notes-
//       ** CODE FORMAT NOT UPDATED **
//
// Usage-
//       Forth <-debug>
//
//----------------------------------------------------------------------------
#if false                           // Hard-Core Debug Mode?
    #define HCDM
#endif

#if false                           // Test environment?
    #define TEST
#endif

using System;                       // (Universally required)
using System.Diagnostics;           // For System.Diagnostics.StackTrace

namespace Forth {
    //------------------------------------------------------------------------
    // Forth mainline
    //------------------------------------------------------------------------
    public class Forth
    {
        internal int ReturnCode = 0; // ReturnCode property

        Context        c;           // The (internal) Context
        Loader         loader;      // The code Loader

        Forth()                     // Constructor
        {
            c = new Context();
            loader = new Loader(c);
        }

        void Test()                 // Test the CodeStack
        {
            Debug.WriteLine("Forth.Test()..");
            c.RUNNING = true;
            while( c.RUNNING )
            {
                Word IAW  = c.IAW;
                Code code = c.CodeMemory[(int)IAW];
                if( code == null )
                    throw new Exception(String.Format("[{0,4:D}] No associate operation", IAW));

                if( c.TRACE )       // Instruction trace active?
                {
                    Debug.WriteLine("IAW[{0,4:D}] {1}", IAW, code);
                    long L = c.DataStack.Length - 1;
                    if( L >= 0 )
                        Debug.WriteLine(">>>[{0,4:D}] {1}", L--, c.DataStack.Peek());
                    if( L >= 0 )
                        Debug.WriteLine(">>>[{0,4:D}] {1}\n", L, c.DataStack[L]);
                }

                c.IAW++;
                code.Op(c);
            }
        }

        void Run()                  // Operate the interpreter
        {
            #if TEST
                Test();             // CodeStack test
            #endif

            // Debug.WriteLine("..Starting Interpreter..");
            c.RUNNING = true;
            Interpreter interpreter = new Interpreter(Console.In, c);
            interpreter.Run();
        }

        static int Main(string[] args) // Mainline code
        {
            int Result = 0;

            Console.WriteLine("Forth version {0}", Core.VERSION_ID);

            try {
                foreach(string arg in args) // Process arguments
                {
                    if( Core.StringApproxEquals(arg, "-quiet") )
                        Debug.DEBUGGING = false;

                    else if( Core.StringApproxEquals(arg, "-verbose") )
                        Debug.DEBUGGING = true;
                    else
                        Debug.WriteLine("Argument({0}) ignored", arg);
                }

                Forth forth = new Forth();

/////////////// Debug.WriteLine("{0,4:D} {1} HCDM", Core.Line(), Core.File());
                forth.loader.Load();
                forth.Run();
                Result = forth.ReturnCode;
            } catch( Exception e) {
                Result = 3;
                Console.WriteLine("Exception({0})", e.Message);

                Console.WriteLine(e.StackTrace);

                StackTrace st = new System.Diagnostics.StackTrace();
                foreach(StackFrame f in st.GetFrames()) {
                    Console.WriteLine(f);
                }

                Console.Out.Flush();
            } finally {
            }

/////////// Debug.WriteLine("{0,4:D} {1} HCDM Return({2})", Core.Line(), Core.File(), Result);
            return Result;

        }
    } // class Forth
} // namespace Forth

