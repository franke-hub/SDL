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
//       Fanin.java
//
// Purpose-
//       Fanin Vector.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
package user.node;

import  java.util.*;
import  user.node.Node;

//----------------------------------------------------------------------------
//
// Class-
//       Fanin
//
// Purpose-
//       Fanin Vector.
//
//----------------------------------------------------------------------------
public class Fanin
{
//----------------------------------------------------------------------------
// Fanin.attributes
//----------------------------------------------------------------------------
private Vector         vector;      // Fanin object list

//----------------------------------------------------------------------------
//
// Method-
//       Fanin.Fanin
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
public
   Fanin( )                         // Default constructor
{
   vector= new Vector();
}

//----------------------------------------------------------------------------
//
// Method-
//       Fanin.add
//
// Purpose-
//       Add Object to Fanin list.
//
//----------------------------------------------------------------------------
public void
   add(                             // Add Node
     Node              node)        // Associated Node
{
   vector.add(node);
}

//----------------------------------------------------------------------------
//
// Method-
//       Fanin.get
//
// Purpose-
//       Get the associated Object.
//
//----------------------------------------------------------------------------
public Node                         // The associated Node
   get(                             // Get Node
     int               index)       // Using this index
{
   return (Node)vector.get(index);
}

//----------------------------------------------------------------------------
//
// Method-
//       Fanin.size
//
// Purpose-
//       Get the number of elements in the Vector.
//
//----------------------------------------------------------------------------
public int                          // The number of elements
   size( )                          // Get number of elements
{
   return vector.size();
}
} // class Fanin

