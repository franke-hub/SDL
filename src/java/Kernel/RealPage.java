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
//       RealPage.java
//
// Purpose-
//       RealPage descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
import user.util.StringFormat;

//----------------------------------------------------------------------------
//
// Class-
//       RealPage
//
// Purpose-
//       RealPage descriptor.
//
//----------------------------------------------------------------------------
public class RealPage extends Page
{
//----------------------------------------------------------------------------
// RealPage.attributes
//----------------------------------------------------------------------------
static int             serial= 0;   // Current address
   PageAddr            addr;        // Physical address

//----------------------------------------------------------------------------
//
// Method-
//       RealPage.RealPage
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   RealPage( )                      // Constructor
{
   super();

   addr= new PageAddr(serial);
   serial += size;
}

//----------------------------------------------------------------------------
//
// Method-
//       RealPage.getAttr
//
// Purpose-
//       Get attributes.
//
//----------------------------------------------------------------------------
public int                          // The Attributes
   getAttr( )                       // Get Attributes
{
   return addr.getAttr();
}

//----------------------------------------------------------------------------
//
// Method-
//       RealPage.clrAttr
//
// Purpose-
//       Clear attributes.
//
//----------------------------------------------------------------------------
public void
   clrAttr(                         // Clear Attributes
     int               attr)        // Attributes to clear
{
   addr.clrAttr(attr);
}

//----------------------------------------------------------------------------
//
// Method-
//       RealPage.setAttr
//
// Purpose-
//       Set attributes.
//
//----------------------------------------------------------------------------
public void
   setAttr(                         // Set Attributes
     int               attr)        // Attributes to set
{
   addr.setAttr(attr);
}

//----------------------------------------------------------------------------
//
// Method-
//       RealPage.copy
//
// Purpose-
//       Copy from source Page into this Page.
//
//----------------------------------------------------------------------------
public void
   copy(                            // Copy Page
     Page              source)      // Source Page
{
   super.copy(source);
   setAttr(PageAddr.CHG);
}

//----------------------------------------------------------------------------
//
// Method-
//       RealPage.zero
//
// Purpose-
//       Zero this Page.
//
//----------------------------------------------------------------------------
public void
   zero( )                          // Zero Page
{
   super.zero();
   setAttr(PageAddr.CHG);
}

//----------------------------------------------------------------------------
//
// Method-
//       RealPage.fetch
//
// Purpose-
//       Fetch from Page.
//
//----------------------------------------------------------------------------
public Object                       // Resultant
   fetch(                           // Fetch from Page
     int               index)       // Word index
{
   setAttr(PageAddr.REF);
   return word[index];
}

//----------------------------------------------------------------------------
//
// Method-
//       RealPage.store
//
// Purpose-
//       Store into Page.
//
//----------------------------------------------------------------------------
public void
   store(                           // Store into Page
     int               index,       // Word index
     Object            object)      // Word value
{
   setAttr(PageAddr.CHG);
   word[index]= object;
}

//----------------------------------------------------------------------------
//
// Method-
//       RealPage.toString
//
// Purpose-
//       Convert to String.
//
//----------------------------------------------------------------------------
public String                       // String representation
   toString( )                      // Get String representation
{
   return addr.toString();
}
} // Class RealPage

