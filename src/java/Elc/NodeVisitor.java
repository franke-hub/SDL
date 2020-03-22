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
//       NodeVisitor.java
//
// Purpose-
//       NodeVisitor descriptor.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       NodeVisitor
//
// Purpose-
//       Node visitor base class.
//
//----------------------------------------------------------------------------
class NodeVisitor                   // Node Visitor
{
//----------------------------------------------------------------------------
// NodeVisitor::Constructors
//----------------------------------------------------------------------------
public
   NodeVisitor( )                   // Default constructor
{
}

//----------------------------------------------------------------------------
// NodeVisitor::Methods
//----------------------------------------------------------------------------
/**
* This method should be overridden in a derived class. The base method
* does nothing, and does not visit child nodes.
*
**/
public int                          // Return code (0 to visit child nodes)
   visit(                           // Visit
     Node              node)        // This Node
{
   return (-1);
}
}; // class NodeVisitor

