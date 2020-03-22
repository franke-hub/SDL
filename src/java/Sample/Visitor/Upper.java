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
//       Upper.java
//
// Purpose-
//       Upper level function -- do real stuff.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Upper
//
// Purpose-
//       Do real stuff.
//
//----------------------------------------------------------------------------
public class Upper extends Lower
{
//----------------------------------------------------------------------------
//
// Method-
//       Upper.Upper
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Upper(                           // Constructor
     String            name,        // The name of the Object
     Upper             parent)      // The parent Object
{
   super(name, parent);

   System.out.println("Upper.Upper(" + name + ")");
}

//----------------------------------------------------------------------------
//
// Method-
//       Upper.visit
//
// Purpose-
//       Visit the remaining subtree.
//
//----------------------------------------------------------------------------
public void
   visit( )                         // Visit an Object
{
   System.out.println("Upper.visit(" + name + ")");
   super.visit();
}

//----------------------------------------------------------------------------
//
// Method-
//       Upper.toString
//
// Purpose-
//       Displayable
//
//----------------------------------------------------------------------------
public String                       // Resultant
   toString( )                      // Convert to String
{
   return "Upper: " + name;
}
} // class Upper

