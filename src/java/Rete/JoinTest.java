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
//       JoinTest.java
//
// Purpose-
//       JoinNode test element
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
//       JoinTest
//
// Purpose-
//       JoinNode test element
//
// Reference-
//       test-at-join-node
//
// Field names-
//       f1         <= field-of-arg1
//       c2         <= condition-number-of-arg2
//       f2         <= field-of-arg2
//
//----------------------------------------------------------------------------
public class JoinTest extends MapDebugAdaptor { // JoinNode test element
//----------------------------------------------------------------------------
// JoinTest.Typedefs and enumerations
//----------------------------------------------------------------------------
static final int       INDEX_ID=    WME_Key.INDEX_ID;    // Identifier index
static final int       INDEX_ATTR=  WME_Key.INDEX_ATTR;  // Attribute  index
static final int       INDEX_VALUE= WME_Key.INDEX_VALUE; // Value      index

static final String[]  INDEX_NAME=
{  "ID"
,  "ATTR"
,  "VALUE"
};

//----------------------------------------------------------------------------
// JoinTest.Attributes
//----------------------------------------------------------------------------
static int             globalSN= 0; // The GLOBAL serial number
int                    objectSN;    // The OBJECT serial number

int                    f1;          // Argument[1] field type
int                    c2;          // Argument[2] (Relative) Condition number
int                    f2;          // Argument[2] field type

//----------------------------------------------------------------------------
//
// Method-
//       JoinTest.JoinTest
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
public
   JoinTest(                        // Constructor
     int               f1,          // Argument[1] type
     int               c2,          // Argument[2] (Relative) Condition number
     int               f2)          // Argument[2] type
{
   this.f1= f1;
   this.c2= c2;
   this.f2= f2;

   objectSN= ++globalSN;
}

//----------------------------------------------------------------------------
//
// Method-
//       JoinTest.MapDebug
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
   debugln(".. " + toString());
}

public int                          // The object serial number
   getObjectSN( )                   // Get object serial number
{  return objectSN; }

//----------------------------------------------------------------------------
//
// Method-
//       JoinTest.equals
//
// Purpose-
//       Equality test (for Vector<JoinList>.equals)
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff identical tests
   equals(                          // Equality test
     Object            object)      // (Overrides Object.equals)
{
   if( object instanceof JoinTest )
   {
     JoinTest test= (JoinTest)object;
     if( f1 == test.f1 && c2 == test.c2 && f2 == test.f2 )
       return true;
   }

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       JoinTest.joinTest
//
// Purpose-
//       Perform a JoinTest
//
// Reference-
//       perform-join-tests (From JoinNode.joinTest, currently UNUSED)
//
//----------------------------------------------------------------------------
public boolean                      // Resultant
   joinTest(                        // Perform a join test
     WME               wme1,        // WME[1]
     WME               wme2)        // WME[2]
{
   String arg1= wme1.index(f1);
   String arg2= wme2.index(f2);
   if( arg1.equals(arg2) )
     return true;

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       JoinTest.toString
//
// Purpose-
//       Get String representation
//
// Reference-
//       perform-join-tests (From JoinNode.joinTest)
//
//----------------------------------------------------------------------------
public String
   toString( )                      // Perform a join test
{
   return "{" + INDEX_NAME[f1]
        + ",[" + c2 + "]"
        + "," + INDEX_NAME[f2]
        + "}";
}
} // class JoinTest

