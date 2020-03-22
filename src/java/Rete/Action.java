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
//       Action.java
//
// Purpose-
//       Action descriptor
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
//       Action
//
// Purpose-
//       Action descriptor
//
// Reference-
//       action
//
//----------------------------------------------------------------------------
public class Action extends MapDebugAdaptor { // Action
//----------------------------------------------------------------------------
// Action.Attributes
//----------------------------------------------------------------------------
static int             globalSN= 0; // The GLOBAL serial number
int                    objectSN;    // The OBJECT serial number

String                 name;        // The Action name

//----------------------------------------------------------------------------
//
// Method-
//       Action.Action
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
public
   Action(                          // Constructor
     String            name)        // The Action name
{
   this.name= name;

   objectSN= ++globalSN;
}

//----------------------------------------------------------------------------
//
// Method-
//       Action.delete
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
//       Action.MapDebug
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

   debugln(".. name: " + name);
}

public int                          // The object serial number
   getObjectSN( )                   // Get object serial number
{  return objectSN; }

//----------------------------------------------------------------------------
//
// Method-
//       Action.act
//
// Purpose-
//       Perform an Action
//
//----------------------------------------------------------------------------
public void
   act(                             // Perform an Action
     Production        prod,        // For this production
     Token             token,       // This activation Token, and
     WME               wme)         // This WME
{
   debugln("\n==============================================================");

   debugln("Action(" + name + ").act("+wme.getReferWME()+")");

   debugln("Production: " + prod.getReference());
   for(Iterator<Condition> i= prod.lhs.iterator(); i.hasNext();)
   {
     Condition cond= i.next();
     debugln(".. " + cond.getReferWME());
   }

   debugln("\nToken tree:");
   debugln(".. Token[---WME--]: "+wme.getReferWME());
   while( token != null )
   {
     String ref= "<NULL>";
     if( token.node != null )
       ref= token.node.getReference();
     debugln(".. "+token.getReference()+": "+(token.wme==null?"<NULL>":token.wme.getReferWME())+", "+ref);
     token= (Token)token.getParent();
   }

   debugln("\nNode tree:");
   Node node= prod;
   while( node != null )
   {
     debugln(".. " + node.getReference());
     node= node.getParent();
   }

   debugln("==============================================================\n");
}
} // class Action

