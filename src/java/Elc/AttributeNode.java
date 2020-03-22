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
//       AttributeNode.java
//
// Purpose-
//       AttributeNode descriptor.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       AttributeNode
//
// Purpose-
//       Node base class.
//
//----------------------------------------------------------------------------
class AttributeNode extends Node    // Node base class
{
//----------------------------------------------------------------------------
// Node.Attributes
//----------------------------------------------------------------------------
String                 name;        // The ATTRIBUTE name
String                 data;        // The ATTRIBUTE value

//----------------------------------------------------------------------------
// AttributeNode.Constructors
//----------------------------------------------------------------------------
public
   AttributeNode(                   // Default constructor
     String            name,        // The ATTRIBUTE name
     String            data)        // The ATTRIBUTE value
{
   this.name= name;
   this.data= data;
}

//----------------------------------------------------------------------------
// AttributeNode::Accessors
//----------------------------------------------------------------------------
public String                       // The data String
   getData( )                       // Get data String
{
   return data;
}

public String                       // The name String
   getName( )                       // Get name String
{
   return name;
}

int                                 // The Node Type
   getType( )                       // Get Node Type
{
   return TYPE_ATTR;
}
}; // class AttributeNode

