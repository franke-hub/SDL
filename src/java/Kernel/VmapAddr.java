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
//       VmapAddr.java
//
// Purpose-
//       VmapAddr descriptor.
//
// Last change date-
//       2010/01/01
//
// Implementation notes-
//       A VmapAddr Object is used to confirm the validity of virtual address
//       mappings.  All Virtual to Real mapping objects are VmapAddr Objects.
//
//----------------------------------------------------------------------------
import user.util.StringFormat;

//----------------------------------------------------------------------------
//
// Class-
//       VmapAddr
//
// Purpose-
//       VmapAddr descriptor.
//
//----------------------------------------------------------------------------
public class VmapAddr extends PageAddr
{
//----------------------------------------------------------------------------
// VmapAddr.attributes
//----------------------------------------------------------------------------
static final int       VALID=  0x00000001; // Valid indicator
static final int       WRONLY= 0x00000002; // Write inhibited
static final int       RDONLY= 0x00000004; // Read inhibited
static final int       SUPER=  0x00000008; // Supervisor page

//----------------------------------------------------------------------------
//
// Method-
//       VmapAddr.VmapAddr
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   VmapAddr(                        // Constructor
     int               address)     // Vmap address
{
   super(address);
}

public
   VmapAddr(                        // Constructor
     int               address,     // Vmap address
     int               attributes)  // Vmap attributes
{
   super(address, attributes);
}

//----------------------------------------------------------------------------
//
// Method-
//       VmapAddr.toString
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
   string.append(".").append(addr&Page.mask,2,2);

   return string.toString();
}
} // Class VmapAddr

