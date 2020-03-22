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
//       LcAlign.java
//
// Purpose-
//       LoaderControl: Align the offset
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       LcAlign
//
// Purpose-
//       LoaderControl: Align the offset
//
//----------------------------------------------------------------------------
public class LcAlign extends LcOrigin
{
//----------------------------------------------------------------------------
//
// Method-
//       LcAlign.LcAlign
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   LcAlign(                         // Constructor
     int               origin)      // Alignment requirement
{
   super(origin);
}

//----------------------------------------------------------------------------
//
// Method-
//       LcAlign.update
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
   loader.offset += origin-1;       // Round up
   loader.offset /= origin;         // Truncate
   loader.offset *= origin;
}
} // Class LcAlign

