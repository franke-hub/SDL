//-----------------------------------------------------------------------------
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
//       Sample.cs
//
// Purpose-
//       Language samples with associated verification code.
//
// Last change date-
//       2019/02/15
//
// Usage notes-
//       Use `Sample --help' for parameter information.
//       Options and controls are defined in Options.cs
//
//-----------------------------------------------------------------------------
#define GENERATE_UNREACHABLE_CODE   // (Example duplicate)
#define GENERATE_UNREACHABLE_CODE   // (Example duplicate)
#undef  GENERATE_UNREACHABLE_CODE   // (Example duplicate)
#undef  GENERATE_UNREACHABLE_CODE   // (Example duplicate)

using System;                       // (Almost always required)
using System.Collections.Generic;   // For IEnumeration
using System.Diagnostics;           // For System.Diagnostics.StackTrace
using System.Runtime.Serialization; // For ISerializable, SerializationInfo
using System.Threading;             // For Thread.Sleep

using Shared;                       // For Debug

//-----------------------------------------------------------------------------
/// <summary>
/// The Sample namespace contains a set of classes that implement the Test
/// interface as defined in Test_interface.cs. Some tests are implemented in
/// this source, others are implemented separately and included from a
/// netmodule (or dll) library.
///
/// It also contins the Sample class, which defines the Main program that
/// drives the individual tests.
/// </summary>
//-----------------------------------------------------------------------------
namespace Sample {                  // The Sample namespace
//=============================================================================
/// Since Debug exists both in Shared and in System.Diagnostics, we have to
/// tell the compiler which one to use.
///
/// We could use
/// <code>
/// internal class Debug: Shared.Debug {}
/// </code>
/// But this methodology HIDES the Shared.System.Debug class constructors.
/// We use a using statement instead, leaving the constructors visible.
//=============================================================================
using Debug = Shared.Debug;

//=============================================================================
/// This demonstrates the recommended way to override the (System.)Exception
/// class. A Simpler method is also demonstrated.
//=============================================================================
// CustomException is not used. It is for demonstration only.
public class CustomException: Exception, ISerializable { // MSDN recommends
public CustomException() : base() {}
public CustomException(string s) : base(s) {}
public CustomException(string s,Exception inner) : base(s,inner) {}
protected CustomException(SerializationInfo info, StreamingContext context)
   : base(info, context) {}
} // class CustomException

// SimpleException is not used. It is for demonstration only.
public class SimpleException: Exception {
public SimpleException(string s) : base(s) {}
} // class SimpleException

//=============================================================================
// Demo_arguments: Test Main argument object
//=============================================================================
class Demo_arguments: Test {
public void Test(object obj) {
   string[] args= (string[])obj;

   Debug.WriteLine("*DEMO*: [{0,2}] Demo_arguments...", args.Length);
   for(int i= 0; i<args.Length; i++) // Note: brackets not required
       Debug.WriteLine("        [{0,2}] {1}", i, args[i]);
   Debug.WriteLine("*DEMO*: ...Demo_arguments");
}
} // class Demo_arguments

//=============================================================================
// Test_backtrace (Causes test failure)
//=============================================================================
class Test_backtrace: Test {
public void Test(Object obj)
{
   if( Options.test_backtrace ) {
       Debug.WriteLine("START>: Test_backtrace (always fails!)");
       throw new Exception("FAILED: As expected");
   }
}
} // class Test_backtrace

//=============================================================================
// Test_callbyref
//=============================================================================
class Test_callbyref: Test {
static void Test_out(int a, int b, out int c)
{
   c= b - a;
}

static void Test_ref(ref int a, ref int b)
{
   int t= a;
   a= b;
   b= t;
}

public void Test(object obj)
{
   int i= 1;
   int j= 2;
   int k= 3;

   Test_ref(ref i, ref j);
   Debug.assert( i == 2 && j == 1);

   Test_out(1, 2, out k);
   Debug.assert( k == 1 );

   Debug.WriteLine("PASSED: Test_callbyref");
}
} // class Test_callbyref

//=============================================================================
// Test_delegates
//=============================================================================
class Test_delegates: Test {
int                    invocationCount;

delegate void          Function(int i);

void FunctionSample(int i)
{
   invocationCount++;
   Debug.assert( i == 8191 );
}

void SampleFunction(int i)
{
   invocationCount++;
   Debug.assert( i == 4095 );
}

void ApplyDelegator(Function f, int i)
{
   f(i);
}

public void Test(object obj)
{
   int i= 8191;

   ApplyDelegator(SampleFunction, 4095);
   ApplyDelegator(FunctionSample, i);
   Debug.assert( invocationCount == 2 );

   Debug.WriteLine("PASSED: Test_delegates");
}
} // class Test_delegates

//=============================================================================
// Test_derived01
//=============================================================================
class VirtualBase {
public int Length { get; protected set; }

public virtual string Method() { return "VirtualBase"; }
public virtual VirtualBase Resize (int n) { Length= n; return this; }
}

public class Test_derived01: Test {
// C# Does not allow protected inheritance
class Derived : /* protected */ VirtualBase {
    public override string Method() { return "Derived"; }
    public new virtual int Resize (int n) { Length= n+1; return n; }
}

public void Test(object obj)
{
   VirtualBase vb= new VirtualBase();
   Debug.assert( vb.Method() == "VirtualBase" );
   Debug.assert( vb.Resize(123) ==  vb );
   Debug.assert( vb.Length == 123 );

   Derived de= new Derived();
   Debug.assert( de.Method() == "Derived" );
   Debug.assert( de.Resize(321) == 321 );
   Debug.assert( de.Length == 322 );

   // Notes:
   //   vb ALWAYS uses VirtualBase.Resize rather than Derived.Resize
   //   de ALWAYS uses Derived.Resize rather than Derived.Resize
   vb= de; // upcast (towards root of hierarchy) not required
   Debug.assert( vb.Method() == "Derived" );
   Debug.assert( vb.Resize(732) == vb );
   Debug.assert( vb.Length == 732 );

   de= (Derived)vb; // downcast (away from root) is required
   Debug.assert( de.Method() == "Derived" );
   Debug.assert( de.Resize(555) == 555 );
   Debug.assert( de.Length == 556 );

   Debug.WriteLine("PASSED: Test_derived01");
}
} // class Test_derived01

//=============================================================================
// Test_operators
//=============================================================================
public static class Extensions_ClassEnclosureRequiredButNameIsNotImportant {
public static int MyInt32(this string s) { return Int32.Parse(s); }
}

class Op {
int                    opValue= 0;

public static implicit operator int(Op L) // Cast to integer
{  return L.opValue; }

public static implicit operator Op(int L) // Cast from integer
{
   Op R= new Op();
   R.opValue= L;
   return R;
}

public static Op operator ++(Op L)  // Prefix/postfix operator ++
{
   ++L.opValue;
   return L;
}
} // class Op

//-----------------------------------------------------------------------------
// Test_operators (Test class)
//-----------------------------------------------------------------------------
class Test_operators: Test {
// Implementation notes-
//     It is unclear whether or not the current results are valid.
//     In C++, results for this test are explicitly undefined.
//     If this test fails, modify or remove the test.
void Test_intermediate_postfix(int i, int j)
{
   Debug.assert( i == 1 && j == 2 ); // (Inverted from current GNU C++)!
}

public void Test(object obj)
{
   int    i;
   string s= "string";
   object o= 1;
   Op    op= (Op)1;

   Debug.assert((o as string) == null);
   Debug.assert((o is string) == false);

   Debug.assert((s as string) == s);
   Debug.assert((s as string) == "string");
   Debug.assert((s is string) == true);

   Debug.assert((int)op == 1);
   Debug.assert((int)++op == 2);
   Debug.assert((int)op++ == 3);    // (Acts like prefix operator)
   Debug.assert((int)(Op)7 == 7);   // (Multiple casts)
   i= op;                           // Implicit cast
   Debug.assert(i == 3);

   // Test postfix operator
   i= 1;
   Test_intermediate_postfix(i++,i++);

   // Note: This part of the test is valid.
   // (The current value of i is well-defined.)
   Debug.assert( i == 3 );

   // Verify the type extension facility
   Debug.assert( "12345".MyInt32() == 12345 );

   Debug.WriteLine("PASSED: Test_operators");
}
} // class Test_operators

//=============================================================================
// Demo_options: Demos NDesk.Options resultant (excepting test_* booleans)
//=============================================================================
class Demo_options: Test {
public void Test(object obj)
{
   if( Options.test_options ) {
       Debug.debug.putLine("*DEMO*: Demo_options...");
       Debug.debug.putLine("opt_help({0})", Options.opt_help);
       Debug.debug.putLine("level({0})", Options.level);

       int M= Options.extra.Length;
       Debug.debug.putLine("[{0,2}] extras:", M);
       for(int i= 0; i<M; i++)
           Debug.debug.putLine("[{0,2}] '{1}'", i, Options.extra[i]);
       Debug.debug.putLine("*DEMO*: ...Demo_options");
   }
}  // Test()
}  // class Demo_options

//=============================================================================
// Test_statement
//=============================================================================
public class EventHandlerContainer {
protected event EventHandler handler;

public void AddHandler(EventHandler eh) { handler += eh; }

public void RemHandler(EventHandler eh) { handler -= eh; }

public void UseHandler(EventArgs args= null)
{
   if( args == null )
       args = EventArgs.Empty;

   if( handler != null )
       handler(this, args);
}

public void Reset() { handler= null; }
} // class EventHandlerContainer

public class Noisy: IDisposable {
static int             noisyCount= 0; // Number of allocated objects

public Noisy           next;        // For chaining
public Noisy           prev;
string                 name;

public string          Name { get { return name; } }
public static int      NoisyCount { get { return noisyCount; } }

public Noisy(string name= null)
{
   if( Options.level > 4 )
       Debug.WriteLine("{0}[{1}].Noisy()", typeof(Noisy), name);

   this.name= name;

   noisyCount++;
}

~Noisy( )
{
   if( Options.level > 4 )
       Debug.WriteLine("Noisy[{0}].~Noisy()", name);

   noisyCount--;
}

public void Dispose( )
{
   if( Options.level > 4 )
       Debug.WriteLine("Noisy[{0}].Dispose()", name);

   next= null;
   prev= null;
}
} // class Noisy

//-----------------------------------------------------------------------------
// Test_statement (Test class)
//-----------------------------------------------------------------------------
class Test_statement: Test {
// This mostly just demonstrates C# unique statments
delegate int     delInt(int i);
delegate decimal delDec(decimal d);

int whoop_te_do= 0;

public int this[int index]          // Indexer
{
    get { return whoop_te_do; }
    set { whoop_te_do= value + index; }
}

public int WhoopTeDo                // Property
{
    get { return whoop_te_do; }
    set { whoop_te_do= value + 5; }
}

private void EventListener(object sender, EventArgs e)
{
    if( Options.level > 4 )
        Debug.WriteLine(">>>>>>: EventListener({0},{1})", sender, e);

    whoop_te_do++;
}

// This function returns multiple values, or breaks
static IEnumerable<int> YieldStmtTest(int from, int to)
{
    for(var i= from; i<to; i++)     // Implicitly an int
        yield return i;

    yield break;
}

public static void Nop() {}         // Conditionally used, does nothing

public void Test(object obj)
{
    Debug.WriteLine("START>: Test_statement");

    int i= 1, j= 2, k;
    checked {                       // Exception on overflow
        i++;
        j++;
    }
    Debug.assert(i == 2 && j == 3);

    unchecked {                     // No exception on overflow
        i++;
        j++;
        k= i + j;
    }
    Debug.assert(i == 3 && j == 4 && k == 7);

    lock(this) {                    // Object lock
        i++;
        j++;
        k= j - i;
    }
    Debug.assert(i == 4 && j == 5 && k == 1);

    // Indexing
    this[27]= 14;
    Debug.assert(this[128] == 41);  // (14 + 27)

    // Property
    Debug.assert(WhoopTeDo == 41);  // (The index variable is used)
    WhoopTeDo= 19;
    Debug.assert(WhoopTeDo == 24);  // (19 + const(5))

    // Event handling
    EventHandlerContainer ehc= new EventHandlerContainer();
    ehc.UseHandler();               // No registered handlers
    Debug.assert(whoop_te_do == 24); // Unchanged
    ehc.AddHandler(new EventHandler(EventListener));
    ehc.UseHandler();               // One registered handler
    Debug.assert(whoop_te_do == 25);
    ehc.AddHandler(new EventHandler(EventListener));
    ehc.UseHandler();               // Two registered handlers
    Debug.assert(whoop_te_do == 27);
    ehc.RemHandler(new EventHandler(EventListener));
    ehc.UseHandler();               // One registered handler
    Debug.assert(whoop_te_do == 28);
    ehc.RemHandler(new EventHandler(EventListener));
    ehc.UseHandler();               // No registered handlers
    Debug.assert(whoop_te_do == 28);
    ehc= null;                      // Cleanup

    // Lambda expression (simple)
    delInt myInt= x => x*x;
    delDec myDec= x => (x+0.1m)*x;

    Debug.assert(myInt( 5) == 25);
    Debug.assert(myDec(5m) == 25.5m);

    // Null coalescing
    string s= null;
    int?  ni= 4;
    Debug.assert( (s ?? "NULL") == "NULL" );
    Debug.assert( (ni ?? -1) == 4 );

    // Using statement + garbage collection test
    // We create a ring of noisy objects, then dispose of the anchor
    Debug.WriteLine(">>>>>>: using statement");
    // int generation= 0;
    using (Noisy n= new Noisy("use")) { // Create the anchor
        Noisy  one= new Noisy("one");
        Noisy  two= new Noisy("two");
        n.next= one;
        n.prev= two;

        one.next= two;
        one.prev= two;

        two.next= one;
        two.prev= one;

        // generation= GC.GetGeneration(n);
    }

    #if true
        // GC timing depends (at least) on compile-time options
        // (FAILS) csc /debug (Not collected until program ends)
        // (WORKS) csc /optimize
        // (WORKS) csc /debug /optimize
        Debug.WriteLine("<<<<<<: Running GC");
        GC.Collect();       // *REQUIRED*
        Debug.WriteLine("<<<<<<: GC.Collect()");
/////// GC.Collect(generation);     // ** NO EFFECT **
/////// Debug.WriteLine("<<<<<<: GC.Collect(int)");
/////// GC.WaitForFullGCComplete(); // ** NO EFFECT **
/////// Debug.WriteLine("<<<<<<: GC.WaitForFullGCComplete()");
        GC.WaitForPendingFinalizers(); // *REQUIRED*
        Debug.WriteLine("<<<<<<: GC.WaitForPendingFinalizer()");
/////// Thread.Sleep(5000);         // ** NO EFFECT **

        if( Noisy.NoisyCount != 0 ) // Verify GC completion
                Console.WriteLine("*INFO*: GC incomplete");
#endif

    // Test yield statement
    int expect= (-10);
    foreach(int x in YieldStmtTest(-10,+10))
    {
        Debug.assert( expect == x );
        expect++;
    }
    Debug.assert( expect == 10 );

    // Test compiler directives (#if, #pragma)
    #if GENERATE_UNREACHABLE_CODE
        // #pragma warning disable 162 // The warning comes later!
        goto create_unreachable_code;

        // Unreachable code, error CS0162
        #pragma warning disable 162
        ; // Apparently this is not enough "unreachable code" to matter
        {}  // Neither is this
        Nop(); // But this is!
        #pragma warning restore 162

create_unreachable_code:
        Debug.WriteLine(">>>>>>: Unreachable code tested");
    #endif

    Debug.WriteLine("PASSED: Test_statement");
}

//=============================================================================
// Test statement syntax (COMPILE-ONLY TEST)
//=============================================================================
public interface One { Two two(One o); } // Interface statement
public interface Two { One one(Two o); }
public interface All: One, Two { }

public partial class Foo<T> where T: One, Two {
   public Two two(One one)      { return MakeTwo(); }
   public One OneMethod(Two o)  { return (One)this; }
}

partial class Foo<T> where T: One, Two {
   public One one(Two two)      { return MakeOne(); }
   public Two TwoMethod(One o)  { return (Two)this; }
}

partial class Foo<T> {
   internal string WhatAmI()    { return "FOO"; }

   internal One MakeOne()       { return (One)new Foo<T>(); }
   internal Two MakeTwo()       { return (Two)new Foo<T>(); }
}

// Note: Here the where clause is required. (It's not implicit.)
public sealed class Bar<T>: Foo<T> where T: One, Two {} // Non-extendable Foo<T>
} // class Test_statement

//=============================================================================
// Test_syntactic
//=============================================================================
class Test_syntactic: Test {
public class One {          // For inheritance verification
   public          string Ident()  { return "IsOne"; }
   public virtual  string WhoAmI() { return "AmOne"; }
} // class One

public class Two: One {     // keywords "new" and "override" required
   public new      string Ident()  { return "IsTwo"; }
   public override string WhoAmI() { return "AmTwo"; }
   public          string Oldie()  { return base.WhoAmI(); }
} // class Two

public void Test(object obj)
{
   // Test declaration and usage of class derivation
   Two t= new Two();
   One o= t;

   Debug.assert( t.WhoAmI() == "AmTwo" );        // virtual
   Debug.assert( o.WhoAmI() == "AmTwo" );        // virtual
   Debug.assert( ((One)t).WhoAmI() == "AmTwo" ); // virtual (still)
   Debug.assert( ((One)o).WhoAmI() == "AmTwo" ); // virtual (still)
   Debug.assert( t.Oldie()  == "AmOne" ); // Programatic base access

   Debug.assert( t.Ident() == "IsTwo" );         // in class
   Debug.assert( o.Ident() == "IsOne" );         // in class
   Debug.assert( ((One)t).Ident() == "IsOne" );  // in class (downcast)
   Debug.assert( ((Two)t).Ident() == "IsTwo" );  // in class (nopcast)
   Debug.assert( ((One)o).Ident() == "IsOne" );  // in class (nopcast)
   Debug.assert( ((Two)o).Ident() == "IsTwo" );  // in class (upcast)
   Debug.assert( t == o );          // (They are the same object!)

   Debug.WriteLine("PASSED: Test_syntactic");
}
} // class Test_syntactic

//=============================================================================
// SAMPLE: Mainline code
//=============================================================================
public class Sample {
//-----------------------------------------------------------------------------
// Sample.Attributes
//-----------------------------------------------------------------------------
Test[]                 tests= //===============================================
{  new Demo_arguments()
,  new Test_attribute()
,  new Test_backtrace()
,  new Test_callbyref()
,  new Test_delegates()
,  new Test_derived01()
,  new Demo_exception()
,  new Test_operators()
,  new Demo_options()
,  new Test_regressed()
,  new Test_something()
,  new Test_queryexpr()
,  new Test_statement()
,  new Test_syntactic()
,  new Test_threading()
}; // tests[] =================================================================

//-----------------------------------------------------------------------------
// Sample.Methods
//-----------------------------------------------------------------------------
public void test(string[] args)     // Run the tests
{
   foreach(Test t in tests)
       t.Test(args);
}

//-----------------------------------------------------------------------------
// Sample.Main: Mainline code
//-----------------------------------------------------------------------------
static void Main(string[] args)     // Mainline code
{
   Debug debug= new Debug("debug.log");

   try {
       if( Options.parm(args) ) {
           Sample sample= new Sample();
           sample.test(args);
           Debug.WriteLine("PASSED: All tests complete");
       }
   } catch( Exception e) {
       Debug.WriteLine("Exception: " + e.Message);
       Debug.WriteLine(e.StackTrace);

       Debug.WriteLine("FAILED: Exception!!");
       Debug.Flush();
   } finally {
       // This gets called even if there was a return statement somewhere in
       // the above code or if an Exception occurs within the catch statement.
       debug.close();
   }
} // void Main
} // class Sample
} // namespace Sample
