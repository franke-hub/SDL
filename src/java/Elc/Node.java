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
//       Node.java
//
// Purpose-
//       Node base class.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Node
//
// Purpose-
//       Node base class.
//
//----------------------------------------------------------------------------
class Node                          // Node base class
{
//----------------------------------------------------------------------------
// Node.Constants for parameterization
//----------------------------------------------------------------------------
static final int       TYPE_NULL= 0;// Type: Invalid
static final int       TYPE_ELEM= 1;// Type: ElementNode
static final int       TYPE_ATTR= 2;// Type: AttributeNode
static final int       TYPE_TEXT= 3;// Type: TextNode

//----------------------------------------------------------------------------
// Node.Attributes
//----------------------------------------------------------------------------
ElementNode            parent;      // Parent Node
Node                   peer;        // Peer Node

//----------------------------------------------------------------------------
// Node.Constructors
//----------------------------------------------------------------------------
public
   Node( )                          // Default constructor
{
}

//----------------------------------------------------------------------------
// Node::Accessors
//----------------------------------------------------------------------------
public Node                         // The associated attribute Node
   getAttr(                         // Get attribute Node
     String            attr)        // For this attribute name
{
   return null;
}

public String                       // The data String
   getData( )                       // Get data String
{
   return null;
}

public String                       // The name String
   getName( )                       // Get name String
{
   return null;
}

public Node                         // The first child Node
   getChild( )                      // Get first child Node
{
   return null;
}

public Node                         // The parent Node
   getParent( )                     // Get parent Node
{
   return parent;
}

public Node                         // The next peer Node
   getPeer( )                       // Get next peer Node
{
   return peer;
}

int                                 // The Node Type
   getType( )                       // Get Node Type
{
   return TYPE_NULL;
}

//----------------------------------------------------------------------------
// Node::Methods
//----------------------------------------------------------------------------
/**
* Visit the node subtree.
*
* For each node, the node is visited first. The return code from that visit
* determines whether child nodes are visited. If so, each child node begins
* a new visit subtree.
*
**/
public void
   visit(                           // Visit the node tree
     NodeVisitor       visitor)     // Using this Visitor
{
   Node                node;        // Working Node
   int                 rc;          // Return code

   rc= visitor.visit(this);         // Visit this Node
   node= getChild();                // Visit all child Node
   if( rc == 0 && node != null )
   {
     node.visit(visitor);           // Visit the child, pushing the stack
     node= node.getPeer();          // Visit all the child's peer Nodes
     while( node != null )
     {
       node.visit(visitor);
       node= node.getPeer();
     }
   }
}

/**
* Reset the Node. (Remove it from the tree.)
**/
public void
   reset( )                         // Reset the Node
{
   parent= null;
}
}; // class Node

