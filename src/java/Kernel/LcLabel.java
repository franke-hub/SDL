//----------------------------------------------------------------------------
//
//       Copyright (C) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       LcLabel.java
//
// Purpose-
//       LoaderControl: Define label.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       LcLabel
//
// Purpose-
//       LoaderControl: Define label.
//
//----------------------------------------------------------------------------
public class LcLabel implements LoaderControl
{
//----------------------------------------------------------------------------
// LcLabel.attributes
//----------------------------------------------------------------------------
   String              name;        // The label name

//----------------------------------------------------------------------------
//
// Method-
//       LcLabel.LcLabel
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   LcLabel(                         // Constructor
     String            name)        // The label name
{
   this.name= name;
}

//----------------------------------------------------------------------------
//
// Method-
//       LcLabel.equals
//
// Purpose-
//       Test for equality.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE if equal
   equals(                          // Test for equality
     Object            o)           // The label name
{
   return name.equals(o);
}

//----------------------------------------------------------------------------
//
// Method-
//       LcLabel.hashCode
//
// Purpose-
//       Return hash code
//
//----------------------------------------------------------------------------
public int                          // The hash code
   hashCode( )                      // Return hash code
{
   return name.hashCode();
}
} // Class LcLabel

