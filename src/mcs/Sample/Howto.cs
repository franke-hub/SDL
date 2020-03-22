//----------------------------------------------------------------------------
//
//       Copyright (c) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       Howto.cs
//
// Purpose-
//       Sample "Howto" tests
//
// Last change date-
//       2019/02/15
//
//-----------------------------------------------------------------------------
// These errors are progressively detected later in the compilation.
#undef  USE_COMPILE_ERRORS_0        // Define to demonstrate what not to do
#undef  USE_COMPILE_ERRORS_1        // To use: #undef USE_COMPILE_ERRORS_0
#undef  USE_COMPILE_ERRORS_2        // To use: #undef USE_COMPILE_ERRORS/_0/_1
#undef  USE_COMPILE_ERRORS_3        // To use: #undef etc

// Control: defines the order in which demonstration methods are invoked
//      define invokes demo_one then demo_two; undef demo_two then demo_one
#define USE_ONE_TWO                 // (Handled) exception occurs either way

using System;

using Sample;                       // For Sample.Test_exception
using Shared;                       // For Debug

// namespace X { // not required here...
//-----------------------------------------------------------------------------
//
// Class-
//        Thing
//
// Purpose-
//        A demonstration class that requires a constructor
//
//-----------------------------------------------------------------------------
class Thing {
public string          name { get; } // The name of the Thing, not settable
public string          that { get; set; } // The name of the Thing, settable

public string          other { get; protected set; } // Set is protected
public string          Other { get {return other;} set{other= value;} }

//-----------------------------------------------------------------------------
// The get and set methods can do ANYTHING
public Thing           Demo {       // A getter/setter demo
get {                               // Doesn't get anything
   Debug.WriteLine("Thing.Demo.get");
   return null;
}  // getter

set {                               // Doesn't set anything
   Debug.WriteLine("Thing.Demo.set, value({0}) only for display",
                   value == null ? "<null>" : value.name);
   if( value != null )
       method(value.that);
}  // setter
}

protected void method(string name)  // A demonstrtion method
{  Debug.WriteLine("method({0}) called", name); }

//-----------------------------------------------------------------------------
// Thing.Constructor
public Thing(string name)           // Constructor
{  this.name= name; this.that= name; this.other= name; }
}  // class Thing

//-----------------------------------------------------------------------------
//
// Class-
//        One, Two
//
// Purpose-
//        Classes that depend on each other when constructed
//
//-----------------------------------------------------------------------------
class One {
public static Two      two= new Two(); // We need a Two
public static Two      wuz;            // What two was when One constructed
public static bool     one_then_two { get { return wuz == null; } }
public static bool     two_then_one { get { return wuz != null; } }
public string          exists= "Exists"; // Something in One

public One( ) {
   Debug.WriteLine(">>>>>>: Constructing One({0})", two);
   wuz= two;
   try {
       Debug.WriteLine(">>>>>>: Using One.wuz({0})", wuz);
       Debug.WriteLine(">>>>>>: Using One.wuz({0})", wuz.exists);
   } catch {
       Debug.WriteLine(">>>>>>: .. oops ..");
   }
}

public override string ToString() {return "One";}
}  // class One

class Two {
const string           prefix= ">>>>>>";
public static One      one= new One(); // We need a One
public static One      wuz;            // What one was when Two constructed
public static bool     two_then_one { get { return wuz == null; } }
public static bool     one_then_two { get { return wuz != null; } }
public string          exists= "Exists"; // Something in Two

public Two( ) {
   Debug.WriteLine(">>>>>>: Two({0})", one);
   wuz= one;
   try {
       Debug.WriteLine(">>>>>>: Using Two.wuz({0})", wuz);
       Debug.WriteLine(">>>>>>: Using Two.wuz({0})", wuz.exists);
   } catch {
       Debug.WriteLine(">>>>>>: .. oops ..");
   }
}

public override string ToString() {return "Two";}
}  // class Two

//-----------------------------------------------------------------------------
//
// Class-
//        Howto
//
// Purpose-
//        Demonstrate how to use C# features.
//
//-----------------------------------------------------------------------------
class Howto {
//-----------------------------------------------------------------------------
// Howto.Attributes
//-----------------------------------------------------------------------------
protected static Debug debug= null; // Our debug object

#if USE_COMPILE_ERRORS_2
static const string    disallowed= "You can't do this";
#endif
const string           allowed= "Allowed: read-only string";

//-----------------------------------------------------------------------------
// Howto.Constructors
//-----------------------------------------------------------------------------
// public Howto( ) { }              // Do-nothing constructor not required

//-----------------------------------------------------------------------------
//
// Method-
//       Howto.demo_arrays
//
// Purpose-
//       Demonstrate array usage
//
//-----------------------------------------------------------------------------
static string[]        name=        // A static array
{  "111", "222", "333", "444"};     // Enough names for now

static Thing[]         builtin=     // A static array of Things
{  new Thing(name[0]), new Thing(name[1])
,  new Thing(name[2]), new Thing(name[3])
};

void demo_arrays( )                 // Demonstrate array usage
{
#if USE_COMPILE_ERRORS_0            // You can't define statics in functions
static Thing           next_big_thing= new Thing("The next one");
#else
   Thing               next_big_thing= new Thing("The big one");
#endif

   Debug.debug.putLine("*DEMO*: array usage");

   Thing[]             thing_1= new Thing[name.Length];
   Thing[]             thing_2=     // A stack array of Things
{  new Thing(name[0]), new Thing(name[1])
,  new Thing(name[2]), new Thing(name[3])
};

   for(int i= 0; i<name.Length; i++)
       thing_1[i]= new Thing(name[i]);

   // Verification
   Debug.assert( next_big_thing.name == "The big one" );

   for(int i= 0; i<name.Length; i++)
   {
       Debug.assert( name[i] == builtin[i].name );
       Debug.assert( name[i] == thing_1[i].name );
       Debug.assert( name[i] == thing_2[i].name );
   }
}

//-----------------------------------------------------------------------------
//
// Method-
//       Howto.demo_getset
//
// Purpose-
//       Demonstrate getters and setters
//
//-----------------------------------------------------------------------------
void demo_getset( )                 // Demonstrate getters and setters
{
   Thing               thingy= new Thing("Thingy"); // A new Thing

#if USE_COMPILE_ERRORS_3
   thingy.name= "New name";         // You can't set when there's no setter
   thingy.other= "New name";        // You can't set when setter's protected
#endif

   Debug.debug.putLine("*DEMO*: getters/setters");
   Debug.assert( thingy.that == "Thingy" ); // Before setting
   thingy.that= "New that";
   Debug.assert( thingy.that == "New that" ); // After setting

   Debug.assert( thingy.other == "Thingy" ); // Before setting
   thingy.Other= "New other";       // You can set using a public alias
   Debug.assert( thingy.other == "New other" ); // Alias set
   Debug.assert( thingy.Other == "New other" ); // Alias get

   // Getter/Setter program demos
   Debug.assert( thingy.Demo == null ); // Getter, always returns null
   thingy.Demo= thingy;             // Setter w/value
   thingy.Demo= null;               // Setter to null
}

//-----------------------------------------------------------------------------
//
// Method-
//       Howto.demo_strings
//
// Purpose-
//       Demonstrate string usage
//
//-----------------------------------------------------------------------------
void demo_strings( )                // Demonstrate string usage
{
#if USE_COMPILE_ERRORS_1
// Contrary to: https://www.tutorialspoint.com/csharp/csharp_constants.htm
// These string literals NOT compile
   Debug.assert( "Hello, dear" == "Hello, \
dear" );
   Debug.assert( "Hello, dear" == "Hello, " "d" "ear" );

#elif USE_COMPILE_ERRORS_3
   // String constructors DO NOT ACCEPT copy constructors
   String              foo= new String("T'is the FOO I am");
   String              bar= "XYYZX"; // (Valid)
   string              xyyzx= new String(bar);

#else
   String              foo= "I am the FOO";
   string              bar= new String("I am the BAR".ToCharArray());
   Char[]              xxyyz= {'X','Y','Y','Z','X'};
   string              xyyzx= new String(xxyyz);
#endif

   Debug.debug.putLine("*DEMO*: string usage");
   Debug.assert( foo == "I am the FOO" );
   Debug.assert( bar == "I am the BAR" );
   Debug.assert( xyyzx == "XYYZX" );

   Debug.assert( "Hello, dear" == "Hello, " + "dear" );
   Debug.assert( "Hello, dear" == @"Hello, dear" );
}

//-----------------------------------------------------------------------------
//
// Method-
//       Howto.demo_stuff
//
// Purpose-
//       Demonstrate miscellaneous stuff
//
//-----------------------------------------------------------------------------
void demo_one( )                    // Demonstrate miscellaneous stuff
{
   Debug.WriteLine("demo_one");
   Debug.WriteLine(">>>>>>: Class({0}) wuz({3}) one_two({1}) two_one({2})",
                   "One", One.one_then_two, One.two_then_one, One.wuz);
   Debug.WriteLine(">>>>>>: Class({0}) wuz({3}) one_two({1}) two_one({2})",
                   "Two", Two.one_then_two, Two.two_then_one, Two.wuz);
}

void demo_two( )                    // Demonstrate miscellaneous stuff
{
   Debug.WriteLine("demo_two");
   Debug.WriteLine(">>>>>>: Class({0}) wuz({3}) one_two({1}) two_one({2})",
                   "Two", Two.one_then_two, Two.two_then_one, Two.wuz);
   Debug.WriteLine(">>>>>>: Class({0}) wuz({3}) one_two({1}) two_one({2})",
                   "One", One.one_then_two, One.two_then_one, One.wuz);
}

void demo_stuff( )                  // Demonstrate miscellaneous stuff
{
#if USE_COMPILE_ERRORS_1
   // How NOT to use keyword identifiers
   Thing wh@ile= new Thing("Wiley"); // @ not valid inside identifiers
   Thing while@= new Thing("Wiley"); // Or at the end
#endif

   // Cross-dependendent classes
   // Implementation note:
   //     Changing the order that the static classes appear for the first
   //     time changes their construction order.
   //     It's not always so easy to see that classes are cross-dependent and,
   //     since the order is usage dependent, testing can easily miss it.
   Debug.debug.putLine("*DEMO*: cross-dependent constructors");
   Debug.debug.put("*INFO*: Source line \"#define USE_ONE_TWO\" ");
   Debug.debug.putLine("controls where exception occurs.");
   #if USE_ONE_TWO                  // if( true/false ) would also work
       demo_one();
       demo_two();
   #else
       demo_two();
       demo_one();
   #endif

   // How to use keyword identifiers
   string @for= "fore";             // Using keyword "for" as an identifier
   Debug.assert( @for == "fore" );

   Thing @while= new Thing("Wiley"); // Not just for strings!
   Debug.assert( @while.name == "Wiley" );

   Thing @something= new Thing("Wahoo"); // Not just for keywords!
   Debug.assert( @something.name == "Wahoo" );
}

//-----------------------------------------------------------------------------
//
// Method-
//       Howto.test_exception
//
// Purpose-
//       Demonstrate invocation of class.
//
// Implementation notes-
//       Test moved to class Sample.Test_exception. That class uses
//       Debug.debug which is initialized by the Debug() constructor, and this
//       is done by its caller: either .Howto.Main or Sample.Sample.Main
//
//-----------------------------------------------------------------------------
bool test_exception( )              // Test exception handling
{
   Debug.WriteLine();
   Sample.Demo_exception tc= new Sample.Demo_exception();
   tc.Test(null);                   // Exception if failure
   return true;
}

//-----------------------------------------------------------------------------
//
// Method-
//       Howto.test_something
//
// Purpose-
//       Sample test
//
//-----------------------------------------------------------------------------
bool test_something( )              // Sample test
{
   debug.put("PASSED: test_something\n");
   return true;
}

//-----------------------------------------------------------------------------
//
// Method-
//       Howto.run
//
// Purpose-
//       Run tests
//
//-----------------------------------------------------------------------------
bool run(string[] args)             // Run tests
{
   bool result = true;

   demo_arrays();
   demo_getset();
   demo_strings();
   demo_stuff();

   result &= test_exception();
   result &= test_something();

   return result;
}

//-----------------------------------------------------------------------------
//
// Method-
//       Howto.Main
//
// Purpose-
//       Mainline code
//
//-----------------------------------------------------------------------------
static void Main(string[] args)     // Mainline code
{
   bool                result = true; // TRUE if result is valid

   debug= new Debug("debug.log");
   try {
       if( args.Length > 0 )
       {
           if( string.Equals(args[0], "-quiet", StringComparison.CurrentCultureIgnoreCase) )
               Debug.DEBUGGING = false;
       }

       Howto main = new Howto();

       result &= main.run(args);
       debug.putLine("PASSED: All tests");
   } catch( Exception e) {
       result = false;
       Debug.WriteLine(e.Message);
       Debug.WriteLine(e.StackTrace);
       Debug.Flush();
       Debug.WriteLine("FAILED: Check debug.log");
   } finally {
       debug.close();
   }
}
} // class Howto
// } // namespace X // not required here...
