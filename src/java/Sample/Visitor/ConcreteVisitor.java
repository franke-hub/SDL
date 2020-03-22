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
//       ConcreteVisitor.java
//
// Purpose-
//       Concrete visitor.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
import java.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       ConcreteVisitor
//
// Purpose-
//       Really do something when an Object is visited.
//
//----------------------------------------------------------------------------
public class ConcreteVisitor implements AbstractVisitor
{
//----------------------------------------------------------------------------
// ConcreteVisitor.Attributes
//----------------------------------------------------------------------------
Vector                 vector= new Vector(); // The Vector of visited Objects

//----------------------------------------------------------------------------
//
// Method-
//       ConcreteVisitor.ConcreteVisitor
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   ConcreteVisitor( )
{
}

//----------------------------------------------------------------------------
//
// Method-
//       ConcreteVisitor.visit
//
// Purpose-
//       Do something when an Object is visited.
//
//----------------------------------------------------------------------------
public void
   visit(                           // Visit an Object
     Object            obj)         // The visited Object
{
   System.out.println("ConcreteVisitor.visit(" + obj + ")");
   vector.add(obj);
}

//----------------------------------------------------------------------------
//
// Method-
//       ConcreteVisitor.list
//
// Purpose-
//       Do something else.
//
//----------------------------------------------------------------------------
public void
   list( )                          // List the Objects that were Visited
{
   System.out.println("ConcreteVisitor.list()");
   for(int i= 0; i<vector.size(); i++)
     System.out.println("[" + i + "] " + vector.get(i));
}
} // class ConcreteVisitor

