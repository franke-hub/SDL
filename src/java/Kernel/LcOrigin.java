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
//       LcOrigin.java
//
// Purpose-
//       LoaderControl: Update the origin
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       LcOrigin
//
// Purpose-
//       LoaderControl: Update the origin
//
//----------------------------------------------------------------------------
public class LcOrigin implements LoaderControl
{
//----------------------------------------------------------------------------
// LcOrigin.attributes
//----------------------------------------------------------------------------
   int                 origin;      // Offset modifier

//----------------------------------------------------------------------------
//
// Method-
//       LcOrigin.LcOrigin
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   LcOrigin(                        // Constructor
     int               origin)      // Offset modifier
{
   this.origin= origin;
}

//----------------------------------------------------------------------------
//
// Method-
//       LcOrigin.update
//
// Purpose-
//       Update the origin.
//
//----------------------------------------------------------------------------
public void
   update(                          // Update the origin
     Loader            loader)      // The associated Loader
   throws Exception
{
   loader.lOrigin= origin - loader.offset;

   if( loader.offset == 0 )         // If changing the origin
   {
     loader.lTable.remove("0");     // Update the associated symbol
     loader.lTable.insert("0", new Addr(origin));
   }
}
} // Class LcOrigin

