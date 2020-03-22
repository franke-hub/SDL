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
//       TestLib.cs
//
// Purpose-
//       Test library objects.
//
// Last change date-
//       2015/01/29
//
// Implementation notes-
//       ** NOT MAINTAINED **
//
// Test results-
//       Test Array[] timing
//              NOP   624,999,218
//           +Store 1,249,998,437
//           +Index   555,554,629
//           -Store   999,998,000
//           -Index   454,544,834
//              NOP   666,665,777
//
//       Test Array<T> (Timing)
//              NOP   606,026,080       (Rejected)
//           +Store   322,580,000 25%   46,422,600
//           +Index   285,714,000 50%   62,613,824
//           -Store   277,777,000 25%   43,895,645
//           -Index   250,000,000 50%   61,046,568
//              NOP   621,082,521
//
//       Test Stack<T> (Timing)
//              NOP   621,082,521       (Rejected)
//             Push   142,857,000       93,457,946
//           +Index   227,272,000       66,225,441
//           -Index   217,391,000       62,892,074
//              Pop   238,095,000      172,423,508
//              NOP   621,082,521
//
//----------------------------------------------------------------------------
#if false                           // Hard-Core Debug Mode?
    #define HCDM
#endif

using System;                       // (Universally required)
using Common;                       // For Debug, Stack<T>
using UserLib;                      // For Thing

namespace Simple {                  // Not required
    //========================================================================
    // Common.System classes that override System class
    //========================================================================
    internal class Array<T>: Common.System.Array<T> {
        public Array(string name = null) : base(name) {}
        public Array(long size) : base(size) {}
    }
    internal class Debug: Common.System.Debug {}
    internal class Stack<T>: Common.System.Stack<T> {
        public Stack(string name = null) : base(name) {}
    }

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
        // Test Array<T> Base timing test
        //====================================================================
        public static void ArrayBase() // Array base timing test
        {
            Console.WriteLine("\nArray: Base timing test...");

            DateTime time;
            TimeSpan span;

            const int M = 100000000;   // One Hundred Million elements
            int[] a = new int[M];

            time = DateTime.Now;    // NOP(i)
            for(int i= 0; i<M; i++)
            {
                NOP(i);
            }
            span = DateTime.Now - time;
            Console.WriteLine("   NOP {0:N}", (double)M / Seconds(span));

            time = DateTime.Now;    // Store+
            for(int i= 0; i<M; i++)
            {
                a[i] = i;
            }
            span = DateTime.Now - time;
            Console.WriteLine("+Store {0:N}", (double)M / Seconds(span));

            time = DateTime.Now;    // Forward indexing
            for(int i= 0; i<M; i++)
            {
                Debug.Assert( a[i] == i );
            }
            span = DateTime.Now - time;
            Console.WriteLine("+Index {0:N}", (double)M / Seconds(span));

            a = new int[M];         // Refresh the array

            time = DateTime.Now;    // Store-
            for(int i= 0; i<M; i++)
            {
                a[M-(i+1)] = i;
            }
            span = DateTime.Now - time;
            Console.WriteLine("-Store {0:N}", (double)M / Seconds(span));

            time = DateTime.Now;    // Reverse indexing
            for(int i= 0; i<M; i++)
            {
                Debug.Assert( a[M-(i+1)] == i );
            }
            span = DateTime.Now - time;
            Console.WriteLine("-Index {0:N}", (double)M / Seconds(span));

            time = DateTime.Now;    // NOP(i) reprise
            for(int i= 0; i<M; i++)
            {
                NOP(i);
            }
            span = DateTime.Now - time;
            Console.WriteLine("   NOP {0:N}", (double)M / Seconds(span));

            // Time Random with Debug.Assert
            time = DateTime.Now;
            Random random = new Random();
            for(int i= 0; i<M; i++)
            {
                int v = random.Next();
                v %= 2;
                if( v == 0 )
                    Debug.Assert( v == 0 );
                else
                    Debug.Assert( v >  0 );
            }
            span = DateTime.Now - time;
            Console.WriteLine("Random {0:N} + Debug.Assert", (double)M / Seconds(span));

            Console.WriteLine("PASSED: Test_ArrayBase");
        } // ArrayBase()

        //====================================================================
        // Test Array<T> (Easy)
        //====================================================================
        public static void ArrayEasy() // Stack easy test
        {
            Console.WriteLine("\nArray: Easy test");

            bool DEBUGGING = Debug.DEBUGGING;
            Array<int> a = new Array<int>("Array.Easy");
            Array<int> b = new Array<int>(3);
            b[0] = 13;
            b[1] = 23;
            b[2] = 33;

            #if !HCDM
                Debug.DEBUGGING = false;
            #endif

            int M = 1000;
            a.resize(M);
            #if HCDM
                a.debug(true);
            #endif
            a[0] = (-1);
            for(int i= 1; i<M; i++)
                a[i] = i;

            // Perform insert/remove
#if CURRENTLY_UNUSED
            a.remove(211, 271);      // Test Remove
            a.insert(209, -732);     // Test single insert
            a.insert(-1, -733);      // Test single insert at last
            a.insert(209, b);        // Test multiple insert
            #if HCDM
                a.debug(true);
            #endif

            // Verify insert/remove
            Debug.Assert( a[-1] == (M-1) );    // Last index
            Debug.Assert( a[-2] == (-733) );   // Insert at end
            Debug.Assert( a[208] == 208 );     // (unchanged)
            Debug.Assert( a[209] == 13 );      // insert b[0] @ 209 (later)
            Debug.Assert( a[210] == 23 );      // insert b[1]
            Debug.Assert( a[211] == 33 );      // insert b[2]
            Debug.Assert( a[212] == (-732) );  // insert @ 209
            Debug.Assert( a[213] == 209 );     // (unchanged, moved)
            Debug.Assert( a[214] == 210 );     // (unchanged, moved) 211-
            Debug.Assert( a[215] == 272 );     // (unchanged, moved) -271
            Debug.Assert( a[216] == 273 );     // (unchanged, moved)
            Debug.Assert( a.Length == (M + (211-271-1) + 1 + 1 + 3) );
#endif

            M = 200;
            a.resize(M);
            #if HCDM
                a.debug(true);

            #endif
            Debug.Assert( a[0] == (-1) );
            for(int i= 1; i<M; i++)
                Debug.Assert( a[i] == i );

            // Test exceptions
            bool exceptionDriven = false;
            Debug.WriteLine("\n================");
            Debug.WriteLine("Exception tests:");

            try {
                Debug.WriteLine("\n================");
                exceptionDriven = false;
                foreach(int i in a)
                {
                    if( i == 30 )
                        a[35] = 35;

                    if( i == 31 )
                        break;
                }

                throw new Exception("ShouldNotOccur");
            } catch (InvalidOperationException e) {
                exceptionDriven = true;
                Debug.WriteLine("Expected: " + e.Message);
                Debug.WriteLine(e.StackTrace);
            } finally {
                Debug.Assert( exceptionDriven );
            }

            a.Reset();

            try {
                exceptionDriven = false;
                Debug.WriteLine("RangeException " + a[0]);
            } catch (IndexOutOfRangeException e) {
                Debug.WriteLine("\n================");
                exceptionDriven = true;
                Debug.WriteLine("Expected: " + e.Message);
                Debug.WriteLine(e.StackTrace);
            } finally {
                Debug.Assert( exceptionDriven );
            }

            try {
                Debug.WriteLine("\n================");
                exceptionDriven = false;
                Debug.WriteLine("RangeException " + a[-1]);
            } catch (IndexOutOfRangeException e) {
                exceptionDriven = true;
                Debug.WriteLine("Expected: " + e.Message);
                Debug.WriteLine(e.StackTrace);
            } finally {
                Debug.Assert( exceptionDriven );
            }

            Debug.DEBUGGING = DEBUGGING;
            Console.WriteLine("PASSED: Test_ArrayEasy");
        } // ArrayEasy()

        //====================================================================
        // Test Array<T> (Standard)
        //====================================================================
        public static void ArrayTest() // Array unit test
        {
            Console.WriteLine("\nArray: Random function test");
            int  v;                 // (temporary)
            int  x;                 // (temporary)

            DateTime time;
            TimeSpan span;

            int M = 10000000;       // Large enough to be interesting
            Array<int> a = new Array<int>((long)M);

            for(int i= 0; i<M; i++)
                a[i] = i+11;

            Debug.Assert( a.Length == M );
            #if HCDM
                a.debug(true);
            #endif

            x = 0;
            foreach(int i in a)
            {
                Debug.Assert( i == (x+11) );
                x++;
            }
            Debug.Assert( x == M );

            for(int i= 0; i<M; i++) // Verify forward/reverse indexing
            {
                Debug.Assert( a[i] == (i+11) );
                Debug.Assert( a[-(i+1)] == (M-i+10) );
            }

            //======== Randomized function test ==============================
            Random random = new Random();
            M = 100000000;
            a = new Array<int>(M);
            int[] b = new int[M];

            time = DateTime.Now;
            for(int i= 0; i<M; i++)
            {
                x = random.Next() % M;
                v = random.Next();
                v %= 2;
                if( v == 0 )
                {
                    v = random.Next();
                    a[x] = v;
                    b[x] = v;
                }
                else
                    Debug.Assert( a[x] == b[x] );
            }
            span = DateTime.Now - time;

            Console.WriteLine("Random {0:N}", (double)M / Seconds(span));
            Console.WriteLine("PASSED: Test_ArrayTest");
        } // ArrayTest()

        //====================================================================
        // Test Array<T> (Timing)
        //====================================================================
        public static void ArrayTime() // Array timing test
        {
            Console.WriteLine("\nArray: Timing test...");

            DateTime time;
            TimeSpan span;

            const int M = 100000000;   // One Hundred Million elements
            Array<int> a = new Array<int>(M);

            time = DateTime.Now;    // NOP(i)
            for(int i= 0; i<M; i++)
            {
                NOP(i);
            }
            span = DateTime.Now - time;
            Console.WriteLine("   NOP {0:N}", (double)M / Seconds(span));

            time = DateTime.Now;    // Store+
            for(int i= 0; i<M; i++)
            {
                a[i] = i;
            }
            span = DateTime.Now - time;
            Console.WriteLine("+Store {0:N}", (double)M / Seconds(span));

            time = DateTime.Now;    // Forward indexing
            for(int i= 0; i<M; i++)
            {
                Debug.Assert( a[i] == i );
            }
            span = DateTime.Now - time;
            Console.WriteLine("+Index {0:N}", (double)M / Seconds(span));

            a.Reset();
            a.resize(M);

            time = DateTime.Now;    // Store-
            for(int i= 0; i<M; i++)
            {
                a[-(i+1)] = i;
            }
            span = DateTime.Now - time;
            Console.WriteLine("-Store {0:N}", (double)M / Seconds(span));

            time = DateTime.Now;    // Reverse indexing
            for(int i= 0; i<M; i++)
            {
                Debug.Assert( a[-(i+1)] == i );
            }
            span = DateTime.Now - time;
            Console.WriteLine("-Index {0:N}", (double)M / Seconds(span));

            time = DateTime.Now;    // NOP(i) reprise
            for(int i= 0; i<M; i++)
            {
                NOP(i);
            }
            span = DateTime.Now - time;
            Console.WriteLine("   NOP {0:N}", (double)M / Seconds(span));

            Console.WriteLine("PASSED: Test_ArrayTime");
        } // ArrayTime()

        //====================================================================
        // Test Stack<T> (Easy)
        //====================================================================
        public static void StackEasy() // Stack easy test
        {
            Console.WriteLine("\nStack: Easy test...");
            Stack<int> s = new Stack<int>("Stack.Easy");
            bool exceptionDriven = false;

            bool DEBUGGING = Debug.DEBUGGING;

            #if !HCDM
                Debug.DEBUGGING = false;
            #endif
            int M = 10;
            for(int i= 0; i<M; i++) {
                int value = i;
                if( value == 0 ) value = (-1);

                Debug.WriteLine("\nPUSH {0}[{1}]", s, value);

                s.Push(value);
                #if HCDM
                    s.debug(true);
                #endif
            }

            Debug.WriteLine("================");
            for(int i= 0; i<M; i++) {
                Debug.WriteLine("\n{0}= INDEX {1}[{2}]", s[i], s, i);
            }

            #if HCDM
                Debug.WriteLine("================");
                s.debug(true);
            #endif

            Debug.WriteLine("================");
            while( s.Length > 0 ) {
                int i = s.Pop();
                Debug.WriteLine("\nPOP {0}[{1}]", s, i);
                #if HCDM
                    s.debug(true);
                #endif
            }

            Debug.WriteLine("================\n");
            M = 100;
            for(int i= 0; i<M; i++) {
                int value = i;
                if( value == 0 ) value = (-1);

                Debug.WriteLine("PUSH {0}[{1}]", s, value);
                s.Push(value);
            }

            Debug.WriteLine("================\n");
            for(int i= 0; i<M; i++) {
                Debug.WriteLine("{0}= INDEX {1}[{2}]", s[i], s, i);
            }

            try {
                Debug.WriteLine("\n================");
                exceptionDriven = false;
                foreach(int i in s)
                {
                    if( i == 30 )
                        s.Push((int)s.Length);

                    if( i == 31 )
                        break;
                }

                throw new Exception("ShouldNotOccur");
            } catch (InvalidOperationException e) {
                exceptionDriven = true;
                Debug.WriteLine("Expected: " + e.Message);
                Debug.WriteLine(e.StackTrace);
            } finally {
                Debug.Assert( exceptionDriven );
            }

            Debug.WriteLine("================\n");
            while( s.Length > 0 ) {
                int i = s.Pop();
                Debug.WriteLine("POP {0}[{1}]", s, i);
            }

            // Test exception
            Debug.WriteLine("\n================");
            Debug.WriteLine("Exception tests:");

            try {
                exceptionDriven = false;
                s.Pop();
            } catch (IndexOutOfRangeException e) {
                Debug.WriteLine("\n================");
                exceptionDriven = true;
                Debug.WriteLine("Expected: " + e.Message);
                Debug.WriteLine(e.StackTrace);
            } finally {
                Debug.Assert( exceptionDriven );
            }

            try {
                exceptionDriven = false;
                Debug.WriteLine("RangeException " + s[0]);
            } catch (IndexOutOfRangeException e) {
                Debug.WriteLine("\n================");
                exceptionDriven = true;
                Debug.WriteLine("Expected: " + e.Message);
                Debug.WriteLine(e.StackTrace);
            } finally {
                Debug.Assert( exceptionDriven );
            }

            try {
                Debug.WriteLine("\n================");
                exceptionDriven = false;
                Debug.WriteLine("RangeException " + s[-1]);
            } catch (IndexOutOfRangeException e) {
                exceptionDriven = true;
                Debug.WriteLine("Expected: " + e.Message);
                Debug.WriteLine(e.StackTrace);
            } finally {
                Debug.Assert( exceptionDriven );
            }

            Debug.DEBUGGING = DEBUGGING;
            Console.WriteLine("PASSED: Test_StackEasy");
        } // StackEasy()

        //====================================================================
        // Test Stack<T> (Standard)
        //====================================================================
        public static void StackTest() // Stack unit test
        {
            Console.WriteLine("\nStack: Function test...");

            DateTime time;
            TimeSpan span;

            Stack<int> s = new Stack<int>("Stack.Test");

            int M = 10000000;          // Large enough to be interesting
            for(int i= 0; i<M; i++)
                s.Push(i+11);

            Debug.Assert( s.Length == M );
            #if HCDM
                s.debug(true);
            #endif

            int x = 0;
            foreach(int i in s)
            {
                Debug.Assert( i == (x+11) );
                x++;
            }
            Debug.Assert( x == M );

            for(int i= 0; i<M; i++) // Verify forward/reverse indexing
            {
                Debug.Assert( s[i] == (i+11) );
                Debug.Assert( s[-(i+1)] == (M-i+10) );
            }

            for(int i= M; i>0; i--)
            {
                int value = s.Pop();
                Debug.Assert( value == i+10 );
            }

            Debug.Assert( s.Length == 0 );

            //======== Randomized function test ==============================
            Random random = new Random();
            M = 1000000000;
            int X = 555555;
            time = DateTime.Now;
            for(int i= 0; i<M; i++)
            {
                int v = random.Next();
                v %= 2;
                if( s.Length == 0 )
                    v = 0;

                if( v == 0 )
                    s.Push(X++);
                else
                {
                    v = s.Pop();
                    X--;
                    Debug.Assert( v == X );
                }
            }
            span = DateTime.Now - time;
            Debug.Assert( s.Length == (X - 555555) );

            Console.WriteLine("Random {0:N}", (double)M / Seconds(span));
            Console.WriteLine("PASSED: Test_StackTest");
        } // StackTest()

        //====================================================================
        // Test Stack<T> (Timing)
        //====================================================================
        public static void StackTime() // Stack timing test
        {
            Console.WriteLine("\nStack: Timing test...");

            DateTime time;
            TimeSpan span;

            const int M = 100000000;   // One Hundred Million elements
            Stack<int> s = new Stack<int>("Stack.Test");

            time = DateTime.Now;    // NOP(i)
            for(int i= 0; i<M; i++)
            {
                NOP(i);
            }
            span = DateTime.Now - time;
            Console.WriteLine("   NOP {0:N}", (double)M / Seconds(span));

            time = DateTime.Now;    // Push
            for(int i= 0; i<M; i++)
            {
                s.Push(i);
            }
            span = DateTime.Now - time;
            Console.WriteLine("  Push {0:N}", (double)M / Seconds(span));

            time = DateTime.Now;    // Forward indexing
            for(int i= 0; i<M; i++)
            {
                Debug.Assert( s[i] == i );
            }
            span = DateTime.Now - time;
            Console.WriteLine("+Index {0:N}", (double)M / Seconds(span));

            time = DateTime.Now;    // Reverse indexing
            for(int i= 0; i<M; i++)
            {
                Debug.Assert( s[-(i+1)] == (M-(i+1)) );
            }
            span = DateTime.Now - time;
            Console.WriteLine("-Index {0:N}", (double)M / Seconds(span));

            time = DateTime.Now;    // Pop
            for(int i= 0; i<M; i++)
            {
                Debug.Assert( s.Pop() == (M-(i+1)) );
            }
            span = DateTime.Now - time;
            Console.WriteLine("   Pop {0:N}", (double)M / Seconds(span));

            time = DateTime.Now;    // NOP(i) reprise
            for(int i= 0; i<M; i++)
            {
                NOP(i);
            }
            span = DateTime.Now - time;
            Console.WriteLine("   NOP {0:N}", (double)M / Seconds(span));

            Console.WriteLine("PASSED: Test_StackTime");
        } // StackTime()

        //====================================================================
        // Test Thing<T> (Easy)
        //====================================================================
        public static void ThingEasy() // Thing unit test
        {
            Console.WriteLine("\nThing: Easy test...");

            try {
                Thing thing = new Thing("Oh boy, oh boy!");
                thing.Something();
            } catch( Exception e) {
                Console.WriteLine("Exception({0})", e.Message);
                Console.WriteLine(e.StackTrace);
                Console.Out.Flush();
                throw e;
            }

            Console.WriteLine("PASSED: Test_ThingEasy");
        } // ThingEasy()
    } // static class Test

    //========================================================================
    // PROGRAM: Mainline code
    //========================================================================
    public class Program
    {
        static void Main(string[] args)// Mainline code
        {
            Test.ArrayBase();
            Test.ArrayEasy();
            Test.ArrayTest();
            Test.ArrayTime();
            Test.StackEasy();
            Test.StackTest();
            Test.StackTime();
            // Test.ThingEasy();
        }
    } // class Program
} // namespace Simple

