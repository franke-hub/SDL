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
//       Production.java
//
// Purpose-
//       Production descriptor
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
//       Production
//
// Purpose-
//       Production descriptor
//
// Reference-
//       production
//
//----------------------------------------------------------------------------
public class Production extends BM_Node { // Production
//----------------------------------------------------------------------------
// Production.Attributes
//----------------------------------------------------------------------------
static int             globalSN= 0; // The GLOBAL serial number
int                    objectSN;    // The OBJECT serial number

Vector<Condition>      lhs;         // List of Conditions
Vector<Action>         rhs;         // List of Actions

//----------------------------------------------------------------------------
//
// Method-
//       Production.Production
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
public
   Production(                      // Constructor
     JoinNode          parent,      // The parent JoinNode
     Vector<Condition> lhs,         // List of Conditions
     Vector<Action>    rhs)         // List of Actions
{
   super(parent);

   this.lhs= lhs;
   this.rhs= rhs;

   objectSN= ++globalSN;
}

//----------------------------------------------------------------------------
//
// Method-
//       Production.delete
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
//       Production.MapDebug
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

   debugln(".. lhs: " + lhs.size());
   for(Iterator<Condition> i= lhs.iterator(); i.hasNext();)
   {
     Condition cond= i.next();
     map.debug(".... ", cond, cond.toString());
   }

   debugln(".. rhs: " + rhs.size());
   for(Iterator<Action> i= rhs.iterator(); i.hasNext();)
   {
     Object o= i.next();
     map.debug(o);
   }
}

public int                          // The object serial number
   getObjectSN( )                   // Get object serial number
{  return objectSN; }

//----------------------------------------------------------------------------
//
// Method-
//       Production.activateLHS
//
// Purpose-
//       Activate this Production
//
// Reference-
//       beta-memory-left-activation, overrides BM_Node.activateLHS
//
//----------------------------------------------------------------------------
public void
   activateLHS(                     // LEFT activate this BM_Node
     Token             token,       // Activation Token
     WME               wme)         // Associated WME
{
   for(Iterator<Action> i= rhs.iterator(); i.hasNext();)
   {
     Action action= i.next();
     action.act(this, token, wme);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Production.debug
//
// Purpose-
//       Extensive debugging display
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Debugging display
{
   debugln(getReference() + ".debug()");
   for(int x= 0; x<lhs.size(); x++)
   {
     debugln("[" + x + "] " + lhs.elementAt(x).getReferWME());
     BM_Node beta= this;
     JoinNode join= null;
     for(int u= 0; u <= x; u++)     // Locate upper node pair
     {
       join= (JoinNode)beta.parent;
       beta= (BM_Node)join.parent;
     }

     debugln(".. " + beta.getReference());
     debugln(".... items: " + beta.items.size());
     for(Iterator<Token> i= beta.items.iterator(); i.hasNext();)
     {
       Token token= i.next();
       debugln("...... " + token.getReferWME());
     }

     debugln(".. " + join.getReference()+", RHS:"+join.linkedRHS+" LHS:"+join.linkedLHS);
     join.debug(".... ");
   }
}
} // class Production

