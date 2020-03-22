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
//       TextNode.java
//
// Purpose-
//       TextNode descriptor.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       TextNode
//
// Purpose-
//       TextNode descriptor.
//
//----------------------------------------------------------------------------
class TextNode extends Node         // TextNode descriptor
{
//----------------------------------------------------------------------------
// TextNode.Attributes
//----------------------------------------------------------------------------
String                 data;        // The text data

//----------------------------------------------------------------------------
// TextNode.Constructors
//----------------------------------------------------------------------------
public
   TextNode(                        // Constructor
     String            text)        // The text data
{
   data= text;
}

//----------------------------------------------------------------------------
// TextNode::Accessors
//----------------------------------------------------------------------------
public String                       // The data String
   getData( )                       // Get data String
{
   return data;
}

int                                 // The Node Type
   getType( )                       // Get Node Type
{
   return TYPE_TEXT;
}
}; // class TextNode

