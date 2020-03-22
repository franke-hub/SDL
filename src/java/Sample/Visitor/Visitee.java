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
//       Visitee.java
//
// Purpose-
//       An object that can be Visitee.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Visitee
//
// Purpose-
//       Define an Object that can be Visitee.
//
//----------------------------------------------------------------------------
public class Visitee
{
//----------------------------------------------------------------------------
// Visitee.Attributes
//----------------------------------------------------------------------------
String                 name= null;  // The Name of this Object
Visitee                parent= null;// Parent Object
Visitee                peer= null;  // Peer Object
Visitee                child= null; // Child Object

//----------------------------------------------------------------------------
//
// Method-
//       Visitee.Visitee
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Visitee(                         // Constructor
     String            name,        // The name of the Object
     Visitee           parent)      // The parent Object
{
   System.out.println("Visitee.Visitee(" + name + ")");
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
//       Visitee.visit
//
// Purpose-
//       Visit the remaining subtree.
//
//----------------------------------------------------------------------------
public void
   visit(                           // Visit an Object
     AbstractVisitor   function)    // The function to perform there
{
   System.out.println("Visitee.visit(" + name + ")");

   function.visit(this);            // Visit this Object

   if( child != null )              // If any children
     child.visit(function);         // Visit them

   if( peer != null )               // If a peer
     peer.visit(function);          // Visit it

// function.visit(this);            // Visit this Object
}

//----------------------------------------------------------------------------
//
// Method-
//       Visitee.toString
//
// Purpose-
//       Displayable
//
//----------------------------------------------------------------------------
public String                       // Resultant
   toString( )                      // Convert to String
{
   return name;
}
} // class Visitee

