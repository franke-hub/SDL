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
//       ReadPage.java
//
// Purpose-
//       Read-only memory Page descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       ReadPage
//
// Purpose-
//       Read-only memory Page descriptor.
//
//----------------------------------------------------------------------------
public class ReadPage extends Page
{
//----------------------------------------------------------------------------
// Page.attributes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       ReadPage.ReadPage
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   ReadPage( )                      // Constructor
{
   super();
}

//----------------------------------------------------------------------------
//
// Method-
//       ReadPage.copy
//
// Purpose-
//       Copy from source Page into this ReadPage.
//
//----------------------------------------------------------------------------
public void
   copy(                            // Copy Page
     Page              source)      // Source Page
{
   Cpu.checkStop("ReadPage.copy");
}

//----------------------------------------------------------------------------
//
// Method-
//       ReadPage.zero
//
// Purpose-
//       Zero this ReadPage.
//
//----------------------------------------------------------------------------
public void
   zero( )                          // Zero ReadPage
{
   Cpu.checkStop("ReadPage.zero");
}

//----------------------------------------------------------------------------
//
// Method-
//       ReadPage.fetch
//
// Purpose-
//       Fetch from ReadPage.
//
//----------------------------------------------------------------------------
public Object                       // Resultant
   fetch(                           // Fetch from ReadPage
     int               index)       // Word index
{
   return word[index];
}

//----------------------------------------------------------------------------
//
// Method-
//       ReadPage.store
//
// Purpose-
//       Store into ReadPage.
//
//----------------------------------------------------------------------------
public void
   store(                           // Store into ReadPage
     int               index,       // Word index
     Object            object)      // Word value
{
}
} // Class ReadPage

