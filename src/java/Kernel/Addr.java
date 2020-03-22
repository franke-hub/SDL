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
//       Addr.java
//
// Purpose-
//       Address descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
import user.util.StringFormat;

//----------------------------------------------------------------------------
//
// Class-
//       Addr
//
// Purpose-
//       Address descriptor.
//
//----------------------------------------------------------------------------
public class Addr
{
//----------------------------------------------------------------------------
// Addr.attributes
//----------------------------------------------------------------------------
   int                 addr;        // The address

//----------------------------------------------------------------------------
//
// Method-
//       Addr.Addr
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Addr(                            // Constructor
     int               address)     // Address
{
   addr= address;
}

//----------------------------------------------------------------------------
//
// Method-
//       Addr.getAddr
//
// Purpose-
//       Get address.
//
//----------------------------------------------------------------------------
public int                          // The Address
   getAddr( )                       // Get Address
{
   return addr;
}

//----------------------------------------------------------------------------
//
// Method-
//       Addr.setAddr
//
// Purpose-
//       Set address.
//
//----------------------------------------------------------------------------
public void
   setAddr(                         // Set Address
     int               addr)        // To this value
{
   this.addr= addr;
}

//----------------------------------------------------------------------------
//
// Method-
//       Addr.toString
//
// Purpose-
//       Convert to String.
//
//----------------------------------------------------------------------------
public String                       // String representation
   toString( )                      // Get String representation
{
   return toString(addr);
}

public static String                // String representation
   toString(                        // Get String representation
     int               addr)        // Of this address
{
   StringFormat        string= new StringFormat();

   string.setRadix(16);
   string.append("[").append(addr,8,8).append("]");
   return string.toString();
}
} // Class Addr

