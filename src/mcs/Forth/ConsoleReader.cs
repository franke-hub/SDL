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
//       ConsoleReader.cs
//
// Purpose-
//       Read from Console, properly handling \b and \r characters.
//
// Last change date-
//       2015/01/25
//
// Implementation notes-
//       ** CODE FORMAT NOT UPDATED **
//
//----------------------------------------------------------------------------
using System;
using Common.System;

namespace Common {
    //------------------------------------------------------------------------
    //
    // Class-
    //   ConsoleReader
    //
    // Purpose-
    //   Read from Console, properly echoing characters.
    //
    // Implementation notes-
    //   Backspace display does not work properly for Cygwin xterm consoles.
    //   (that were started before the window manager is started!)
    //   Console.ReadKey(true) cannot be used because it throws an Exception
    //   if the associated console isn't a Windows console.
    //
    //------------------------------------------------------------------------
    public static class ConsoleReader
    {
        const char EOF = unchecked((char)(-1));

        public static string ReadLine()
        {
            // stack holds the input line
            Common.System.Stack<char> stack = new Common.System.Stack<char>();

            while( true )
            {
                char C = (char)Console.Read();
                if( C == '\n' || C == EOF ) // If end of line
                    break;

                if( C == '\b' )     // Backspace removes the prior character
                {
                    if( stack.Length > 0 )
                        stack.Pop();

                    continue;
                }

                if( C != '\r' )     // Carriage return is ignored
                    stack.Push(C);
            }

            string result = "";
            if( stack.Length != 0 )
            {
                char[] output = new char[stack.Length];
                int i = 0;
                foreach(char C in stack)
                    output[i++] = C;

                result = new string(output);
            }

            return result;
        }
    } // class ConsoleReader
} // namespace Common

