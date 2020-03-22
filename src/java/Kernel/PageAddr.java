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
//       PageAddr.java
//
// Purpose-
//       PageAddr descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
import user.util.StringFormat;

//----------------------------------------------------------------------------
//
// Class-
//       PageAddr
//
// Purpose-
//       PageAddr descriptor.
//
//----------------------------------------------------------------------------
public class PageAddr extends Addr
{
//----------------------------------------------------------------------------
// PageAddr.attributes
//----------------------------------------------------------------------------
static final int       REF= 0x00000001; // Reference attribute
static final int       CHG= 0x00000002; // Change    attribute

//----------------------------------------------------------------------------
//
// Method-
//       PageAddr.PageAddr
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   PageAddr(                        // Constructor
     int               address)     // Page address
{
   super(address);

   if( (address&(Page.mask)) != 0 )
     Cpu.checkStop("PageAddr: " + Addr.toString(address));
}

public
   PageAddr(                        // Constructor
     int               address,     // Page address
     int               attributes)  // Page attributes
{
   super(address);

   if( (address&(Page.mask)) != 0 )
     Cpu.checkStop("PageAddr: " + Addr.toString(address));

   setAttr(attributes);
}

//----------------------------------------------------------------------------
//
// Method-
//       PageAddr.getAddr
//
// Purpose-
//       Get address.
//
//----------------------------------------------------------------------------
public int                          // The Address
   getAddr( )                       // Get Address
{
   return (addr&(~Page.mask));
}

//----------------------------------------------------------------------------
//
// Method-
//       PageAddr.setAddr
//
// Purpose-
//       Get address.
//
//----------------------------------------------------------------------------
public void
   setAddr(                         // Set Address
     int               addr)        // To this value
{
   this.addr= (addr&(~Page.mask)) | (this.addr&Page.mask);
}

//----------------------------------------------------------------------------
//
// Method-
//       PageAddr.getFrame
//
// Purpose-
//       Get frame index
//
//----------------------------------------------------------------------------
public int                          // The frame index
   getFrame( )                      // Get frame index
{
   return addr / Page.size;
}

//----------------------------------------------------------------------------
//
// Method-
//       PageAddr.getAttr
//
// Purpose-
//       Get attributes.
//
//----------------------------------------------------------------------------
public int                          // The Attributes
   getAttr( )                       // Get Attributes
{
   return (addr&Page.mask);
}

//----------------------------------------------------------------------------
//
// Method-
//       PageAddr.clrAttr
//
// Purpose-
//       Clear attributes.
//
//----------------------------------------------------------------------------
public void
   clrAttr(                         // Clear Attributes
     int               attr)        // Attributes to clear
{
   attr &= Page.mask;
   attr &= addr;
   addr ^= attr;
}

//----------------------------------------------------------------------------
//
// Method-
//       PageAddr.setAttr
//
// Purpose-
//       Set attributes.
//
//----------------------------------------------------------------------------
public void
   setAttr(                         // Set Attributes
     int               attr)        // Attributes to set
{
   attr &= Page.mask;
   addr |= attr;
}

//----------------------------------------------------------------------------
//
// Method-
//       PageAddr.toString
//
// Purpose-
//       Convert to String.
//
//----------------------------------------------------------------------------
public String                       // String representation
   toString( )                      // Get String representation
{
   StringFormat        string= new StringFormat();

   string.setRadix(16);
   string.append("[").append(addr&(~Page.mask),8,8).append("]");

   if( (addr&REF) != 0 )
     string.append("R");
   else
     string.append("r");

   if( (addr&CHG) != 0 )
     string.append("C");
   else
     string.append("c");

   return string.toString();
}
} // Class PageAddr

