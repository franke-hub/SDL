//----------------------------------------------------------------------------
//
//       Copyright (c) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestUtil.cs
//
// Purpose-
//       Test library Utility class.
//
// Last change date-
//       2019/02/15
//
//----------------------------------------------------------------------------
#undef  USE_DEBUGGING               // For additional runtime information
#undef  VERIFY_COMPILE_ERRORS       // Define: verify expected compile errors

using System;
using System.Threading;             // For Thread.Sleep

using Shared;                       // For Debug

//============================================================================
// Test cases
//============================================================================
static class Test {
static bool match(string wild, string text)
{
#if USE_DEBUGGING
   bool result= Utility.wildmatch(wild, text);
   Debug.WriteLine("{0}= match({1},{2})", result, wild, text);
   return result;

#else
   return Utility.wildmatch(wild, text);
#endif
}

//----------------------------------------------------------------------------
// test_humanify: Tests Utility.humanify
//----------------------------------------------------------------------------
static void test_humanify( ) {      // Test Utility.humanify
   Debug.assert( "9.2E" == Utility.humanify(9223372036854775807) );
   Debug.assert( "1.2E" == Utility.humanify(1234567890123456789) );
   Debug.assert( "123P" == Utility.humanify( 123456789012345678) );
   Debug.assert( " 12P" == Utility.humanify(  12345678901234567) );
   Debug.assert( "1.2P" == Utility.humanify(   1234567890123456) );
   Debug.assert( "123T" == Utility.humanify(    123456789012345) );
   Debug.assert( " 12T" == Utility.humanify(     12345678901234) );
   Debug.assert( "1.2T" == Utility.humanify(      1234567890123) );
   Debug.assert( "123G" == Utility.humanify(       123456789012) );
   Debug.assert( " 12G" == Utility.humanify(        12345678901) );
   Debug.assert( "1.2G" == Utility.humanify(         1234567890) );
   Debug.assert( "123M" == Utility.humanify(          123456789) );
   Debug.assert( " 12M" == Utility.humanify(           12345678) );
   Debug.assert( "1.2M" == Utility.humanify(            1234567) );
   Debug.assert( "123K" == Utility.humanify(             123456) );
   Debug.assert( " 12K" == Utility.humanify(              12345) );
   Debug.assert( "1234" == Utility.humanify(               1234) );
   Debug.assert( " 123" == Utility.humanify(                123) );
   Debug.assert( "  12" == Utility.humanify(                 12) );
   Debug.assert( "   1" == Utility.humanify(                  1) );
   Debug.assert( "   0" == Utility.humanify(                  0) );

   // Special cases
   Debug.assert( "9949" == Utility.humanify(    9949) );
   Debug.assert( " 10K" == Utility.humanify(    9950) ); // up (special case)

   // Rounding
   Debug.assert( " 95K" == Utility.humanify(   94999) );
   Debug.assert( " 95K" == Utility.humanify(   95000) );
   Debug.assert( " 99K" == Utility.humanify(   99499) );
   Debug.assert( "100K" == Utility.humanify(   99500) ); // up
   Debug.assert( "100K" == Utility.humanify(   99949) );
   Debug.assert( "100K" == Utility.humanify(   99950) );

   Debug.assert( "950K" == Utility.humanify(  949999) );
   Debug.assert( "950K" == Utility.humanify(  950000) );
   Debug.assert( "995K" == Utility.humanify(  994999) );
   Debug.assert( "995K" == Utility.humanify(  995000) );
   Debug.assert( "999K" == Utility.humanify(  999499) );
   Debug.assert( "1.0M" == Utility.humanify(  999500) ); // up

   Debug.assert( "9.5M" == Utility.humanify( 9499999) );
   Debug.assert( "9.5M" == Utility.humanify( 9500000) );
   Debug.assert( "9.9M" == Utility.humanify( 9949999) );
   Debug.assert( " 10M" == Utility.humanify( 9950000) ); // up
   Debug.assert( " 10M" == Utility.humanify( 9994999) );
   Debug.assert( " 10M" == Utility.humanify( 9995000) );

   Debug.assert( " 95M" == Utility.humanify(94999999) );
   Debug.assert( " 95M" == Utility.humanify(95000000) );
   Debug.assert( " 99M" == Utility.humanify(99499999) );
   Debug.assert( "100M" == Utility.humanify(99500000) ); // up
   Debug.assert( "100M" == Utility.humanify(99949999) );
   Debug.assert( "100M" == Utility.humanify(99950000) );
}

//----------------------------------------------------------------------------
// test_nullify: Tests Utility.nullify
//----------------------------------------------------------------------------
static void test_nullify( ) {       // Test Utility.nullify
   string thing= null;
   Debug.assert( Utility.nullify(thing) == "<null>" );

   thing= "output";
   Debug.assert( Utility.nullify(thing) == "output" );
}

//----------------------------------------------------------------------------
// test_match: Tests Utility.wildmatch
//----------------------------------------------------------------------------
static void test_match( ) {         // Test Utility.wildmatch
   string test01= "Hello there! It's Yogi Bear.";
   string test02= "String * contains ? and ! characters";

   // Escape sequences
   Debug.assert( true  == match("(**)test it",  "*test it") ); // Expansion test
   Debug.assert( true  == match("test it(??)",  "test it?") ); // Expansion test
   Debug.assert( true  == match("test(!!) it",  "test! it") ); // Expansion test
   Debug.assert( false == match("test!! it",    "test! it") ); // Misused !!
   Debug.assert( false == match("test*!",       "test it!") ); // Expansion test
   Debug.assert( true  == match("test*(!!)",    "test it!") ); // Expansion test
   Debug.assert( true  == match("x((**)(**))y", "x(**)y") ); // Expansion test

   // Expansion tests
   Debug.assert( false == match("*it!!",        "test it!") ); // Misused
   Debug.assert( true  == match("*it(!!)",      "test it!") );
   Debug.assert( true  == match("*it(!!)*",     "test it!") );

   // Not implemented: "!<any case match>!" "exact case match"
   Debug.assert( false == match("!<hello >!*",   test01) ); // Not implemented

   // Wildcard tests
   Debug.assert( true  == match("Hello ?*here*", test01) );
   Debug.assert( true  == match("Hello ?*ere*",  test01) );
   Debug.assert( false == match("Hello *?here*", test01) ); // Invalid sequence
   Debug.assert( false == match("Hello **here*", test01) ); // Invalid sequence
   Debug.assert( false == match("Hello ?*th*",   test01) );

   Debug.assert( false == match("St(**)* con*",  test02) );
   Debug.assert( true  == match("St*(**) con*",  test02) );
}

//----------------------------------------------------------------------------
// test_tod: Tests Utility.tod
//----------------------------------------------------------------------------
static void test_tod( ) {           // Test Utility.tod
#if USE_DEBUGGING
   Debug.WriteLine("\ntest_tod (with one second delay)");
#endif

   double start= Utility.tod();
   Thread.Sleep(1000);
   double elapsed= Utility.tod() - start;

   Debug.assert( elapsed >= 1.0 && elapsed <= 1.03125 );
   Debug.assert( start > 1000000000.0 );
}

//----------------------------------------------------------------------------
// test_verify: Tests compile errors, and properties
//----------------------------------------------------------------------------
static void test_verify( ) {        // Verify expected compile errors
#if VERIFY_COMPILE_ERRORS
   Utility.ALWAYS_TRUE= false;      // Don't try this at home
   Debug.WriteLine("{0}", Utility.tod_origin);   // Not accessible
   Debug.WriteLine("{0}", Utility.have_linux);   // Not accessible
   Debug.WriteLine("{0}", Utility.have_windows); // Not accessible

   Utility.is_linux= false;         // No setter
   Utility.is_windows= false;       // No setter
#endif

   Debug.assert( Utility.ALWAYS_TRUE );

   if( Utility.is_linux )
       Debug.WriteLine("Running LINUX");

   if( Utility.is_windows )
       Debug.WriteLine("Running WINDOWS");

   Debug.assert( Utility.is_linux != Utility.is_windows );
}

static public void all( ) {         // Run all tests
   test_verify();                   // This always displays something
   test_humanify();
   test_match();
   test_nullify();
   test_tod();
}
}  // static class Test

//============================================================================
// PROGRAM: Mainline code
//============================================================================
public class Program {
static void Main(string[] args)     // Mainline code
{
   Test.all();
}
}   // class Program
