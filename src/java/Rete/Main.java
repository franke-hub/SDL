//----------------------------------------------------------------------------
//
//       Copyright (C) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Main.java
//
// Purpose-
//       Test compile.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
import java.lang.*;
import java.util.*;
import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       Main
//
// Purpose-
//       Test compile.
//
// Reference-
//       (None: Added for testing)
//
//----------------------------------------------------------------------------
class Main extends Debug {
//----------------------------------------------------------------------------
// Main.Controls
//----------------------------------------------------------------------------
static final boolean   DISPLAY_JOIN_TEST= true ; // Display JoinTest resultant?

//----------------------------------------------------------------------------
// Main.Attributes (Compile test)
//----------------------------------------------------------------------------
Action                 action;
AME                    ame;
AM_Hash                amHash;
AM_Node                am;
BM_Node                bm;
Condition              cond;
JoinNode               joinNode;
JoinTest               joinTest;
Node                   node;
PosCondition           posCond;
Rete                   rete;
Token                  token;
WME                    wme;
WME_Key                wme_key;
WM_Hash                wmHash;

//----------------------------------------------------------------------------
//
// Method-
//       Main.memoryDiagnostics
//
// Purpose-
//       Display memory diagnostics.
//
//----------------------------------------------------------------------------
public void
   memoryDiagnostics(               // Display memory diagnostics
     String            TESTCASE,    // The associated testcase
     Rete              rete)        // The base RETE object
{
   debugln("\n\n\n\n");
   debugln("VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV");
   debugln("VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV");
   debugln("VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV");
   debugln(TESTCASE + " memory diagnostics");
   DebugMap map= new DebugMap();    // Display allocated memory

   map.debug("rete: ", rete);
   map.unwind();
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.performJoinTests
//
// Purpose-
//       Perform and display Condition.getJoinTests resultant
//
//----------------------------------------------------------------------------
public void
   performJoinTests(                // Display Condition.getJoinTests resultant
     Condition         cond,        // The condition
     Vector<Condition> earlier)     // Prior condition vector
{
   Vector<JoinTest> test= cond.getJoinTests(earlier);

   if( DISPLAY_JOIN_TEST )
   {
     debugln("performJoinTests()");
     debugln("<< [0] " + cond.key);
     for(int i= 0; i<earlier.size(); i++)
       debugln("<< [" + (i+1) + "] " + earlier.elementAt(i).key);

     for(int i= 0; i<test.size(); i++)
       debugln(">> " + test.elementAt(i));

     if( test.size() == 0 )
       debugln(">> <NONE>");

     debugln("");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.test0000
//
// Purpose-
//       Object verification tests.
//
//----------------------------------------------------------------------------
public void
   test0000( )                      // Testcase
   throws Exception
{
   String TESTCASE= "test0000";
   boolean QUICK_EXIT= true ;       // Exit without running test?
   boolean ABORT_EXIT= true ;       // Abort after running test?
   if( QUICK_EXIT ) return;         // Avoid changing global sequence numbers
   debugln(TESTCASE + " started");

   // Debugging controls
   HCDM= true ;                     // Hard-Core-Debug-Mode?
   SCDM= true ;                     // Soft-Core-Debug-Mode?
   INFO= true ;                     // Verbose mode?

   WME w1= new WME("B1",      "^ON",    "B2");
   WME w2= new WME("B1",      "^ON",    "B3");
   WME w3= new WME("B1",   "^COLOR",   "RED");
   WME w4= new WME("B2",      "^ON", "TABLE");
   WME w5= new WME("B2", "^LEFT-OF",    "B3");
   WME w6= new WME("B2",   "^COLOR",  "BLUE");
   WME w7= new WME("B3", "^LEFT-OF",    "B4");
   WME w8= new WME("B3",      "^ON", "TABLE");
   WME w9= new WME("B3",   "^COLOR",   "RED");

   verify( w1.index(-1) == null );
   verify( w1.index(0).equals("B1") );
   verify( w1.index(1).equals("^ON") );
   verify( w1.index(2).equals("B2") );
   verify( w1.index(3) == null );

   verify( w6.index(0).equals("B2") );
   verify( w6.index(1).equals("^COLOR") );
   verify( w6.index(2).equals("BLUE") );

   Token t1= new Token(null, null, w1);
   Token t2= new Token(  t1, null, w2);
   Token t3= new Token(  t2, null, w3);
   Token t4= new Token(  t3, null, w4);
   Token t5= new Token(  t4, null, w5);
   Token t6= new Token(  t5, null, w6);
   Token t7= new Token(  t6, null, w7);
   Token t8= new Token(  t7, null, w8);
   Token t9= new Token(  t8, null, w9);

   verify( t5.index(0) == t5 );
   verify( t5.index(1) == t4 );
   verify( t5.index(2) == t3 );
   verify( t5.index(3) == t2 );
   verify( t5.index(4) == t1 );
   verify( t5.index(5) == null );

   verify( t1.size() == 1 );
   verify( t2.size() == 2 );
   verify( t5.size() == 5 );
   verify( t9.size() == 9 );

   if( true  )                      // Verify delete?
   {
     t1.delete();
     verify( t1.children == null );
     verify( t5.children == null );
     verify( t1.size() == 1 );
     verify( t2.size() == 1 );
     verify( t5.size() == 1 );
     verify( t9.size() == 1 );
   }

   Condition c1= new PosCondition("<x>",      "^ON",   "<y>");
   Condition c2= new PosCondition("<y>", "^LEFT-OF",   "<z>");
   Condition c3= new PosCondition("<z>",   "^COLOR",   "RED");
   Condition c4= new PosCondition("<z>",   "^COLOR",   "<z>");
   Condition c5= new PosCondition("<w>",   "^COLOR",   "<z>");

   verify( c1.indexOf("foo") == (-1) );
   verify( c1.indexOf("<x>") == 0 );
   verify( c1.indexOf("^ON") == 1 );
   verify( c1.indexOf("<y>") == 2 );

   Vector<Condition> v1= new Vector<Condition>();
   Vector<Condition> v2= new Vector<Condition>();
   v2.add(c1);
   Vector<Condition> v3= new Vector<Condition>();
   v3.add(c1);
   v3.add(c2);
   v3.add(c3);
   v3.add(c4);

   performJoinTests(c1, v1);
   performJoinTests(c2, v2);
   performJoinTests(c4, v3);
   performJoinTests(c5, v3);

   if( ABORT_EXIT ) throw new Exception("Testing complete");
   debugln(TESTCASE + " complete");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.test0001
//
// Purpose-
//       Bringup
//
//----------------------------------------------------------------------------
public void
   test0001( )                      // Testcase
   throws Exception
{
   String TESTCASE= "test0001";
   boolean QUICK_EXIT= true ;       // Exit without running test?
   boolean ABORT_EXIT= true ;       // Abort after running test?
   if( QUICK_EXIT ) return;         // Avoid changing global sequence numbers
   debugln(TESTCASE + " started");

   // Debugging controls
// HCDM= true ;                     // Hard-Core-Debug-Mode?
// SCDM= true ;                     // Soft-Core-Debug-Mode?
// INFO= true ;                     // Verbose mode?

   // Bringup test
   Rete rete= new Rete();

   // Data elements
   WME w1= rete.insertData("THAT", "^IS", "THAT");
   WME w2= rete.insertData("THIS", "^IS", "THAT");
   WME w3= rete.insertData("WHAT", "^IS", "THAT");
   WME w4= rete.insertData( "WHO", "^IS", "THAT");
   WME w5= rete.insertData( "WHY", "^IS", "THAT");

   // Actions
   Vector<Action> a1= new Vector<Action>();
   a1.add(new Action("Default"));
   a1.add(new Action("Additional"));

   // Conditions
   Condition c0= new PosCondition("<z>", "^IS",  "<z>");
   Condition c1= new PosCondition("<x>", "^IS", "THIS");
   Condition c2= new PosCondition("<x>", "^IS",  "<z>");

   Condition c3= new PosCondition("<y>", "^IS", "THAT");
   Condition c4= new PosCondition("<y>", "^IS",  "<z>");
   Condition c5= new PosCondition("<z>", "^IS",  "<y>");

   Vector<Condition> v1= new Vector<Condition>();
   v1.add(c0);                     // NO tests
   v1.add(c1);                     // NO result

   Vector<Condition> v2= new Vector<Condition>();
   v2.add(c0);                     //  1 test
   v2.add(c2);                     //  5 results

   Vector<Condition> v3= new Vector<Condition>();
   v3.add(c0);                     //  1 test (same as v2)
   v3.add(c1);                     // NO results
   v3.add(c2);

   Vector<Condition> v4= new Vector<Condition>();
   v4.add(c0);                     //  3 tests
   v4.add(c3);                     //  1 result
   v4.add(c4);
   v4.add(c5);

   // Productions
   Production p1= rete.insertProd(v1, new Action("P1"));
   Production p2= rete.insertProd(v2, new Action("P2"));
   Production p3= rete.insertProd(v3, new Action("P3"));
   Production p4= rete.insertProd(v4, a1);

   // Modifications
   debugln("insert: WHAT THE HECK, ^IS, THIS");
   rete.insertData("WHAT THE HECK", "^IS", "THIS");

   debugln("insert: THIS, ^IS, THIS");
   rete.insertData("THIS", "^IS", "THIS");

   memoryDiagnostics(TESTCASE, rete);

   if( ABORT_EXIT ) throw new Exception("Testing complete");
   debugln(TESTCASE + " complete");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.test0010
//
// Purpose-
//       Page 10 with additional "never-match" test.
//
//----------------------------------------------------------------------------
public void
   test0010( )                      // Testcase
   throws Exception
{
   String TESTCASE= "Page 10";      // Verify page 10 network
   boolean QUICK_EXIT= false;       // Exit without running test?
   boolean ABORT_EXIT= false;       // Abort after running test?
   if( QUICK_EXIT ) return;         // Skip test?
   debugln(TESTCASE + " started");
   Bringup.debug();

   // Debugging controls
   HCDM= true ;                     // Hard-Core-Debug-Mode?
   SCDM= true ;                     // Soft-Core-Debug-Mode?
   INFO= true ;                     // Verbose mode?

   Rete rete= new Rete();

   // Rete network, page 10
   WME w1= rete.insertData("B1",      "^ON",    "B2");
   WME w2= rete.insertData("B1",      "^ON",    "B3");
   WME w3= rete.insertData("B1",   "^COLOR",   "RED");
   WME w4= rete.insertData("B2",      "^ON", "TABLE");
   WME w5= rete.insertData("B2", "^LEFT-OF",    "B3");
   WME w6= rete.insertData("B2",   "^COLOR",  "BLUE");
   WME w7= rete.insertData("B3", "^LEFT-OF",    "B4");
   WME w8= rete.insertData("B3",      "^ON", "TABLE");
   WME w9= rete.insertData("B3",   "^COLOR",   "RED");

   Condition c1= new PosCondition("<x>",      "^ON",   "<y>");
   Condition c2= new PosCondition("<y>", "^LEFT-OF",   "<z>");
   Condition c3= new PosCondition("<z>",   "^COLOR",   "RED");
// Condition c4= new PosCondition("<a>", "testfail",   "<a>");

   Vector<Condition> v1= new Vector<Condition>();
   v1.add(c1);
   v1.add(c2);
   v1.add(c3);

// Vector<Condition> v2= new Vector<Condition>();
// v2.add(c4);

   Vector<Action> a1= new Vector<Action>();
   a1.add(new Action("Default"));

   Production p1= rete.insertProd(v1, a1); p1.debug();
// Production p2= rete.insertProd(v2, a1);

   //-------------------------------------------------------------------------
   // Diagnostics
   memoryDiagnostics(TESTCASE, rete);

   if( ABORT_EXIT ) throw new Exception("Testing complete");
   debugln(TESTCASE + " complete");
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.main
//
// Purpose-
//       Mainline code
//
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
   throws Exception
{
   Main main= new Main();           // Instantition object

   try {
     main.test0000();
     main.test0001();
     main.test0010();
   } catch(Exception x) {
     debugln("Main: Exception: " + x);
     x.printStackTrace();
   }

   debugln("Tests complete");
}
} // class Main

