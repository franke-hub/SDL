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
//       AbstractVisitor.java
//
// Purpose-
//       Abstract visitor.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Interface-
//       AbstractVisitor
//
// Purpose-
//       Do something when an Object is visited.
//
//----------------------------------------------------------------------------
public interface AbstractVisitor
{
//----------------------------------------------------------------------------
//
// Method-
//       AbstractVisitor.visit
//
// Purpose-
//       Do something when an Object is visited.
//
//----------------------------------------------------------------------------
public void
   visit(                           // Visit an Object
     Object            obj);        // The visited Object
} // Interface AbstractVisitor

