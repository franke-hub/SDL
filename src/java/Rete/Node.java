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
//       Node.java
//
// Purpose-
//       Node base class.
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
//       Node
//
// Purpose-
//       Node base class.
//
// Reference-
//       rete-node
//
//----------------------------------------------------------------------------
public class Node extends MapDebugAdaptor { // Node base class
//----------------------------------------------------------------------------
// Node.Attributes
//----------------------------------------------------------------------------
// int                 type;        // (Use instanceof operator)       @REMOVE
boolean                isLinked;    // TRUE iff linked to parent       @INSERT
Node                   parent;      // Parent Node
Vector<Node>           children;    // Child Nodes

static final Vector<Node> empty= new Vector<Node>(); // Empty Vector<Node>

//----------------------------------------------------------------------------
//
// Method-
//       Node.Node
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
protected
   Node(                            // Constructor
     Node              parent)      // The parent Node
{
   this.isLinked= false;
   this.parent= parent;
   this.children= null;

   if( parent != null )             // Insert at HEAD of parent.children
     relink(null);
}

//----------------------------------------------------------------------------
//
// Method-
//       Node.delete
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
protected void
   delete( )                        // Destructor
{
   if( isLinked )                   // Unlink from parent
     unlink();
   parent= null;

   for(Iterator<Node> i= getChildren().iterator(); i.hasNext();)
   {
     Node node= i.next();
     node.delete();
   }
   children= null;
}

//----------------------------------------------------------------------------
//
// Method-
//       Node.Accessor methods
//
// Purpose-
//       Accessor methods
//
//----------------------------------------------------------------------------
Vector<Node>                        // The children vector, never NULL
   getChildren( )                   // Get children vector
{
   if( children == null )
     return empty;

   return children;
}

boolean
   getIsLinked( )                   // Get isLinked attribute
{  return isLinked; }

Node
   getParent( )                     // Get parent Node
{  return parent; }

String
   getReferWME( )                   // Get reference + WME_Key
{  return getReference(); }

//----------------------------------------------------------------------------
//
// Method-
//       Node.MapDebug
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

   debugln(".. isLinked: " + isLinked);
   map.debug(".. parent: ", parent);

   Vector<Node> children= getChildren();
   debugln(".. children: " + children.size());
   for(Iterator<Node> i= children.iterator(); i.hasNext();)
   {
     Object o= i.next();
     map.debug(o);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Node.initialize
//
// Purpose-
//       Update parent Node with matches from above
//       Note: THIS Node is the parent node
//
// Reference-
//       update-new-node-with-matches-from-above
//       (Overridden in subclass)
//
//----------------------------------------------------------------------------
public void
   initialize(                      // Update parent Node with matches from above
     Node              child)       // The new child node
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Node.insert
//
// Purpose-
//       Insert a node (ignores isLinked.)
//
//----------------------------------------------------------------------------
private void
   insert(                          // Insert a Node
     Node              node)        // The node to insert
{
   verify( node != this );

   if( children == null )
     children= new Vector<Node>();

   children.add(node);              // Add to TAIL
}

private void
   insert(                          // Insert a Node
     Node              node,        // The node to insert
     Node              before)      // In front of this Node, null for HEAD
{
   verify( node != this );

   if( children == null )
     children= new Vector<Node>();

   int index= 0;                    // DEFAULT, add to HEAD
   if( before != null )             // If add at position
   {
     index= children.indexOf(before);
     verify( index >= 0 );
   }
   children.add(index, node);
}

//----------------------------------------------------------------------------
//
// Method-
//       Node.nearestParent
//
// Purpose-
//       Locate nearest parent with the same AM_Node
//
// Reference-
//       find-nearest-ancestor-with-same-amem
//       (Only meaningful in JoinNode)
//
//----------------------------------------------------------------------------
public JoinNode                     // The nearest parent with the same AM_Node
   nearestParent(                   // Get nearest parent with the same AM_Node
     AM_Node           amem)        // The associated AM_Node
{
   if( parent == null )
     return null;

   return parent.nearestParent(amem);
}

//----------------------------------------------------------------------------
//
// Method-
//       Node.relink
//
// Purpose-
//       Relink this node back onto its parent node
//
//----------------------------------------------------------------------------
public void
   relink(                          // Relink this Node
     Node              before)      // In front of this Node, null for HEAD
{
   verify( isLinked == false );

   parent.insert(this, before);
   isLinked= true;
}

public void
   relink( )                        // Relink this Node as last child
{
   verify( isLinked == false );

   parent.insert(this);
   isLinked= true;
}

//----------------------------------------------------------------------------
//
// Method-
//       Node.remove
//
// Purpose-
//       Remove a node (ignores isLinked.)
//
//----------------------------------------------------------------------------
private void
   remove(                          // Remove a Node
     Node              node)        // The node to remove
{
   verify( children != null );      // TODO: Verify

   if( children != null )
   {
     children.remove(node);

     if( children.size() == 0 )
       children= null;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Node.unlink
//
// Purpose-
//       Unlink this node from its parent node
//
//----------------------------------------------------------------------------
public void
   unlink( )                        // Unlink this Node
{
   verify( isLinked );

   parent.remove(this);
   isLinked= false;
}
} // class Node

