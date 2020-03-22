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
//       ElementNode.java
//
// Purpose-
//       ElementNode descriptor.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       ElementNode
//
// Purpose-
//       ElementNode descriptor.
//
//----------------------------------------------------------------------------
class ElementNode extends Node      // ElementNode descriptor
{
//----------------------------------------------------------------------------
// ElementNode.Attributes
//----------------------------------------------------------------------------
String                 name;        // The ELEMENT name
Node                   head;        // Head child Node
Node                   tail;        // Tail child Node

//----------------------------------------------------------------------------
// ElementNode.Constructors
//----------------------------------------------------------------------------
public
   ElementNode(                     // Constructor
     String             name)       // The ELEMENT name
{
   this.name= name;
}

//----------------------------------------------------------------------------
// ElementNode::Accessors
//----------------------------------------------------------------------------
public Node                         // The associated attribute Node
   getAttr(                         // Get attribute Node
     String            attr)        // For this attribute name
{
   Node                node= head;

   while( node != null )
   {
     if( node.getType() != TYPE_ATTR )
     {
       node= null;
       break;
     }

     if( ((AttributeNode)node).name == name )
       break;

     node= node.peer;
   }

   return node;
}

public String                       // The data String
   getData( )                       // Get data String
{
   return null;
}

public String                       // The name String
   getName( )                       // Get name String
{
   return name;
}

public Node                         // The first child Node
   getChild( )                      // Get first child Node
{
   return head;
}

int                                 // The Node Type
   getType( )                       // Get Node Type
{
   return TYPE_ELEM;
}

//----------------------------------------------------------------------------
// ElementNode::Methods
//----------------------------------------------------------------------------
/**
* Insert a child element.
*
* The child element is added at the end of the child node list.
*
**/
void
   insert(                          // Insert child Node
     Node              child)       // The node to insert
   throws Exception
{
   if( child.getParent() != null )
     throw new Exception("ElementNode.DuplicateParentException");

   child.parent= this;
   child.peer= null;

   if( tail == null )
     this.head= child;
   else
     tail.peer= child;
   tail= child;
}

/**
* Reset the Node. (Remove it and all its children from the tree.)
**/
public void
   reset( )                         // Reset the Node
{
   Node                node;        // Working Node

   parent= null;

   while( head != null )
   {
     node= head;
     head= node.peer;
     node.peer= null;
     node.reset();
   }

   tail= null;
}
}; // class ElementNode

