//----------------------------------------------------------------------------
//
//       Copyright (C) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       AME.java
//
// Purpose-
//       Alpha Memory Element.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
import java.lang.*;
import java.util.*;
import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       AME
//
// Purpose-
//       Alpha Memory Element.
//
// Reference-
//       item-in-alpha-memory
//
//----------------------------------------------------------------------------
class AME extends MapDebugAdaptor { // Alpha Memory Element
//----------------------------------------------------------------------------
// AME.Attributes
//----------------------------------------------------------------------------
static int             globalSN= 0; // The GLOBAL serial number
int                    objectSN;    // The OBJECT serial number

AM_Node                amem;        // Associated Alpha Memory
WME                    wme;         // Associated Working Memory Element

//----------------------------------------------------------------------------
//
// Method-
//       AME.AME
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
protected
   AME(                             // Constructor
     AM_Node           amem,        // Associated AM_Node
     WME               wme)         // Associated WME
{
   this.amem= amem;                 // Initialize
   this.wme= wme;

   objectSN= globalSN++;            // FIRST AME is DUMMY
}

//----------------------------------------------------------------------------
//
// Method-
//       AME.delete
//
// Purpose-
//       Delete this object
//
//----------------------------------------------------------------------------
public void
   delete( )                        // Delete this AME
{
   amem.removeAME(this);

   amem= null;
   wme=  null;
}

//----------------------------------------------------------------------------
//
// Method-
//       AME.Accessor Methods
//
// Purpose-
//       Accessor methods
//
//----------------------------------------------------------------------------
public AM_Node
   getAM_Node( )
{  return amem; }

public String
   getReferWME( )
{  return getReference()+( wme==null ? "{<NULL>}" : wme); }

public WME
   getWME( )
{  return wme; }

//----------------------------------------------------------------------------
//
// Method-
//       AME.MapDebug
//
// Purpose-
//       Override MapDebugAdaptor methods
//
//----------------------------------------------------------------------------
public void
   debug(                           // Debugging display
     DebugMap          map)         // DebugMap extention
{
   super.debug(map);                // Display this entry

   map.debug(".. amem: ", amem);
   map.debug(".. wme: ", wme, wme == null ? "<NULL>" : wme.fields.toString());
}

public int                          // The object serial number
   getObjectSN( )                   // Get object serial number
{  return objectSN; }
} // class AME

