//----------------------------------------------------------------------------
//
//       Copyright (C) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Lower.java
//
// Purpose-
//       Lower level function -- follow the list.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Lower
//
// Purpose-
//       Follow the list.
//
//----------------------------------------------------------------------------
public class Lower
{
//----------------------------------------------------------------------------
// Lower.Attributes
//----------------------------------------------------------------------------
String                 name= null;  // The Name of this Object
Lower                  parent= null;// Parent Object
Lower                  peer= null;  // Peer Object
Lower                  child= null; // Child Object

//----------------------------------------------------------------------------
//
// Method-
//       Lower.Lower
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Lower(                           // Constructor
     String            name,        // The name of the Object
     Lower             parent)      // The parent Object
{
   System.out.println("Lower.Lower(" + name + ")");
   this.name= name;

   if( parent != null )
   {
     this.parent= parent;
     this.peer= parent.child;
     parent.child= this;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Lower.visit
//
// Purpose-
//       Visit the remaining subtree.
//
//----------------------------------------------------------------------------
public void
   visit( )                         // Visit an Object
{
// System.out.println("Lower.visit(" + name + ")");

   if( child != null )              // If any children
     child.visit();                 // Visit them

   if( peer != null )               // If a peer
     peer.visit();                  // Visit it
}

//----------------------------------------------------------------------------
//
// Method-
//       Lower.toString
//
// Purpose-
//       Displayable
//
//----------------------------------------------------------------------------
public String                       // Resultant
   toString( )                      // Convert to String
{
   return "Lower: " + name;
}
} // class Lower

