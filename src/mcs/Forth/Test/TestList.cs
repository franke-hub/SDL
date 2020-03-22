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
//       TestList.cs
//
// Purpose-
//       Test library List object.
//
// Last change date-
//       2015/02/16
//
// Implementation notes-
//       ** NOT MAINTAINED **
//
//----------------------------------------------------------------------------
#if false                           // Hard-Core Debug Mode?
    #define HCDM
#endif

using System;                       // (Universally required)
using Common;                       // For Debug, Stack<T>

namespace Simple {                  // Not required
    //========================================================================
    // Common.System classes that override System class
    //========================================================================
    internal class Debug: Common.System.Debug {}
    internal class List<T>: Common.System.List<T> {}

    //------------------------------------------------------------------------
    //
    // Class-
    //   Thing
    //
    // Purpose-
    //   A Thing that can be put onto a List.
    //
    //------------------------------------------------------------------------
    public class Thing : List<Thing>.Link
    {
        static protected int SerialNumber = 0;

        int    number;
        public Thing()         { number = (SerialNumber++); }
        public override string ToString() {
            return String.Format("Thing({0})", number);
        }
    } // class Thing

    //========================================================================
    // Test cases
    //========================================================================
    internal static class Test {
        public static int BOP;

        internal static double Seconds(TimeSpan interval) { // DateTime to seconds
            return (double)interval.Ticks / (double)TimeSpan.TicksPerSecond;
        }

        public static void NOP(int i) { // (Almost) a No-operation
            BOP += i;                  // (Avoid optimization code removal)
        }

        //====================================================================
        // Test List<T>
        //====================================================================
        public static void ListTest() // List unit test
        {
            Console.WriteLine("\nList: Unit test");

            const int   DIM = 16;
            List<Thing> list = new List<Thing>();
            List<Thing>.Link link;
            Thing       ting;

            Thing[]     thing = new Thing[DIM];
            for(int i= 0; i<DIM; i++)
                thing[i]= new Thing();

            list.Lifo(thing[2]);
            list.Lifo(thing[1]);
            list.debug();

            link = list.Remove();
            ting= (Thing)link; Console.WriteLine("\nRemoved({0})", ting);
            list.debug();

            link = list.Remove();
            ting= (Thing)link; Console.WriteLine("\nRemoved({0})", ting);
            list.debug();

            link = list.Remove();
            ting= (Thing)link; Console.WriteLine("\nRemoved({0})", ting);
            list.debug();
        } // ListTest()
    } // static class Test

    //========================================================================
    // PROGRAM: Mainline code
    //========================================================================
    public class Program
    {
        static void Main(string[] args)// Mainline code
        {
            Test.ListTest();
        }
    } // class Program
} // namespace Simple

