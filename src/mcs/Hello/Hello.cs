//----------------------------------------------------------------------------
//
//       Copyright (c) 2015 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Hello.cs
//
// Purpose-
//       Hello, C# world.
//
// Last change date-
//       2015/01/23
//
//----------------------------------------------------------------------------
using System;

namespace NotRequired {
//----------------------------------------------------------------------------
//
// Class-
//       Hello
//
// Purpose-
//       C# "Hello, World" example.
//
//----------------------------------------------------------------------------
class Hello
{
    static void Main(              // Mainline code
        string[]       args)       // Command arguments
    {
        int i = 0;
        Console.WriteLine("Hello, C# World");
        foreach(string arg in args)
        {
            Console.WriteLine("[{0}] {1}", i++, arg);
        }
    }
}
} // namespace NotRequired

