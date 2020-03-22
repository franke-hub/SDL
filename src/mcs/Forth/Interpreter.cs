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
//       Interpreter.cs
//
// Purpose-
//       Interpret Forth words.
//
// Last change date-
//       2015/01/25
//
// Implementation notes-
//       ** CODE FORMAT NOT UPDATED **
//
//----------------------------------------------------------------------------
#if true
    #define HCDM                    // Hard-Core Debug Mode
#endif

#if true
    #define STACK_PROMPT            // Display the Stack before the prompt
#endif

using System;                       // (Universally required)
using Common;                       // For ConsoleReader

namespace Forth {
    public class Interpreter
    {
        Context        context;     // The working Context
        System.IO.TextReader reader; // The source reader

        public Interpreter(System.IO.TextReader r, Context c) {
            context = c;
            reader  = r;
        }

        static string replace(string s) // Replace special characters in string
        {
            return s.Replace("\b","\\b").Replace("\r","\\r").Replace("\n","\\n");
        }

        string normalize(string s)  // Handle special characters in string
        {
            string current = s;
            while( true )
            {
                int x = current.IndexOf('\b'); // Backspace
                if( x >= 0 )
                {
                    #if HCDM
                        Debug.WriteLine("current({0})", replace(current));
                    #endif

                    if( x == 0 )
                        current = current.Substring(1);
                    else if( x == 1 )
                        current = current.Substring(2);
                    else
                        current = current.Substring(0,x-1) + current.Substring(x+1);

                    #if HCDM
                        Debug.WriteLine("current({0})", replace(current));
                    #endif
                    continue;
                }

                x = current.IndexOf('\r'); // Carriage return
                if( x  >= 0 )
                {
                    #if HCDM
                        Debug.WriteLine("current({0})", replace(current));
                    #endif

                    current = current.Substring(x+1);

                    #if HCDM
                        Debug.WriteLine("current({0})", replace(current));
                    #endif
                    continue;
                }

                // Nothing special found, return
                return current;
            }
        }

        public void Prompt()        // Console prompt
        {
            #if STACK_PROMPT
                Context c = context;
                long L = c.DataStack.Length - 1;
                Console.WriteLine("");
                if( L >= 1 )
                    Console.WriteLine(">>>> [{0,4:D}] {1}", L-1, c.DataStack[L-1]);
                if( L >= 0 )
                    Console.WriteLine(">>>> [{0,4:D}] {1}", L, c.DataStack.Peek());
            #endif

            Console.Write("TLC> ");
        }

        public void Run()           // Run the Interpreter
        {
            Context c = context;

            while( c.RUNNING )
            {
                string[] Tokens;     // The input line tokens
                try {
                   if( reader == Console.In )
                       Prompt();

                   // string line = normalize(reader.ReadLine());
                   string line = normalize(ConsoleReader.ReadLine());
                   Tokens = line.Split(default(Char[]), StringSplitOptions.RemoveEmptyEntries);
                } catch( System.IO.EndOfStreamException ) {
                   return;
                }

                foreach(string token in Tokens)
                {
                    Code code = c.Dictionary[token];
                    if( code != null )
                        code.Op(c);
                    else
                    {
                        try {
                            c.DataStack.Push(Word.Parse(token, c));
                        } catch( Exception ) {
                            Console.WriteLine("Invalid Token({0})", token);
                            break;
                        }
                    }
                }
            }
        }
    } // class Interpreter
} // namespace Forth

