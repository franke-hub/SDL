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
//       Condition.java
//
// Purpose-
//       Condition descriptor
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
//       Condition
//
// Purpose-
//       Condition descriptor
//
// Reference-
//       condition
//
//----------------------------------------------------------------------------
public class Condition extends MapDebugAdaptor { // Condition
//----------------------------------------------------------------------------
// Condition.Attributes
//----------------------------------------------------------------------------
static int             globalSN= 0; // The GLOBAL serial number
int                    objectSN;    // The OBJECT serial number

WME_Key                key;         // Associated AM_Node key

//----------------------------------------------------------------------------
//
// Method-
//       Condition.Condition
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
private void
   constructor(                     // Constructor initializer
     WME_Key           key)         // The associated key
{
   this.key= key;

   objectSN= ++globalSN;
}

protected
   Condition(                       // Constructor
     WME_Key           key)         // Associated key
{
   constructor(key);
}

protected
   Condition(                       // Copy constructor
     Condition         copy)        // Source
{
   constructor(copy.key);
}

//----------------------------------------------------------------------------
//
// Method-
//       Condition.delete
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
public void
   delete( )                        // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Condition.Accessor methods
//
// Purpose-
//       Accessor methods
//
//----------------------------------------------------------------------------
public WME_Key                      // The associated WME_Key
   getKey( )                        // Get associated WME_Key
{  return key; }

public String
   getReferWME( )
{  return getReference()+toString(); }

public String                       // Resultant String, or null
   getVariable(                     // Get variable name
     int               index)       // The String to test
{
   // Returns String iff both
   //   1) The cond.key(index) contains a variable name
   //   2) No earlier duplicate of the variable name exists
   String result= key.index(index);
   if( result.charAt(0) != '<' )    // If not a variable name
     return null;

   for(int f2= 0; f2<index; f2++)   // Any earlier instances of name?
   {
     if( result.equals(key.index(f2)) )
       return null;
   }

   return result;
}

static public boolean               // True iff string contains a variable name
   isVariable(                      // Does string contain a variable name?
     String            string)      // The String to test
{  return string.charAt(0) == '<'; }

//----------------------------------------------------------------------------
//
// Method-
//       Condition.MapDebug
//
// Purpose-
//       Override MapDebugAdaptor methods
//
//----------------------------------------------------------------------------
public void
   debug(                           // Debugging display
     DebugMap          map)         // DebugMap extention
{
   super.debug(map);                // Display this entry

   debugln(".. key: " + key);
}

public int                          // The object serial number
   getObjectSN( )                   // Get object serial number
{  return objectSN; }

//----------------------------------------------------------------------------
//
// Method-
//       Condition.debugMatch
//
// Purpose-
//       Show debug information when a match is found
//
//----------------------------------------------------------------------------
public void
   debugMatch(                      // Show debug match information
     Vector<Condition> earlier,     // The list of earlier Conditions
     int               f1,          // The index in THIS Condition
     int               c2,          // The earlier  THAT Condition
     int               f2)          // The index in THAT Condition
{
   if( false || (HCDM && INFO) )
   {
     debugln(getReferWME() + ".debugMatch("+f1+","+c2+","+f2+")");

     c2= 1;
     for(Iterator<Condition> iC= earlier.iterator(); iC.hasNext(); c2++)
     {
       Condition cond= iC.next();
       debugln("<< [" + c2 + "] " + cond.getReferWME());
    }
  }
}

//----------------------------------------------------------------------------
//
// Method-
//       Condition.getJoinTests
//
// Purpose-
//       Extract associated JoinTests
//
// Reference-
//       get-join-tests-from-condition
//
//----------------------------------------------------------------------------
public Vector<JoinTest>             // The associated JoinTest Vector
   getJoinTests(                    // Get associated JoinTests
     Vector<Condition> earlier)     // The list of earlier Conditions
{
   if( SCDM ) debugln(getReferWME() + ".getJoinTests()");

   final int M= key.size();
   Vector<JoinTest> result= new Vector<JoinTest>();

   // Self-tests
   int c2= 0;                       // Self-test index
   for(int f1= 0; f1<M; f1++)
   {
     String s= key.index(f1);
     if( isVariable(s) )            // If a variable name
     {
       for(int f2= f1+1; f2<M; f2++)
       {
         if( s.equals(key.index(f2)) ) // If the SAME variable name
         {
           result.add(new JoinTest(f1, c2, f2));
           debugMatch(earlier, f1, c2, f2);
           break;
         }
       }
     }
   }

   // Earlier condition tests
   for(int f1= 0; f1<M; f1++)
   {
     String s= getVariable(f1);     // Get field[f1] (if unique variable)
     if( s != null )                // If a unique variable name
     {
       c2= 1;
       for(Iterator<Condition> iC= earlier.iterator(); iC.hasNext(); c2++)
       {
         Condition cond= iC.next();
         if( cond instanceof PosCondition )
         {
           int f2= cond.indexOf(s);
           if( f2 >= 0 )
           {
             // Per the spec, all that's really needed is to find the EARLIEST
             // condition (the last one in the list) that meets this criteria
             // because any earlier conditions duplicate the tests
             for(int x2= c2+1; iC.hasNext(); x2++)
             {
               cond= iC.next();
               int y2= cond.indexOf(s);
               if( y2 >= 0 )
               {
                 c2= x2;
                 f2= y2;
               }
             }

             result.add(new JoinTest(f1, c2, f2));
             debugMatch(earlier, f1, c2, f2);
             break;               // Only this one test is needed
           } // if( f2 >= 0 )
         } // if( cond instanceof PosCondition )
       } // for( Interator<Condition> ... )
     } // if( s != null ) // Unique variable name
   } // for( int f1= 0; f1<M; f1++ )

   if( HCDM && INFO )               // Hard-Core-Debug-Mode
   {
     if( !SCDM )                    // Avoid duplicate display
       debugln(getReferWME()+".getJoinTests()");
     for(int i= 0; i<earlier.size(); i++)
       debugln("<< [" + (i+1) + "] " + earlier.elementAt(i).key);

     if( earlier.size() == 0 )
       debugln("<< <NONE>");

     for(int i= 0; i<result.size(); i++)
       debugln(">> " + result.elementAt(i));

     if( result.size() == 0 )
       debugln(">> <NONE>");

     debugln("");
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Condition.index
//
// Purpose-
//       Return String of associated Index
//
//----------------------------------------------------------------------------
public String                       // The String of the associated index
   index(                           // Get String of the associated index
     int               x)           // The associated index
{
   return key.index(x);
}

//----------------------------------------------------------------------------
//
// Method-
//       Condition.indexOf
//
// Purpose-
//       Return index of associated String (-1 if not present)
//
//----------------------------------------------------------------------------
public int                          // The index of the associated String
   indexOf(                         // Get index of the associated String
     String            s)           // The associated String
{
   int result= (-1);                // Default, NO match

   int M= key.size();
   for(int i= 0; i<M; i++)
   {
     if( key.index(i).equals(s) )
     {
       result= i;
       break;
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Condition.toString
//
// Purpose-
//       Diagnostic String
//
//----------------------------------------------------------------------------
public String
   toString( )
{
   return key.toString();
}
} // class Condition

