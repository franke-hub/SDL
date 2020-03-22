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
//       RealStorage.java
//
// Purpose-
//       Real storage descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       RealStorage
//
// Purpose-
//       Real storage descriptor.
//
//----------------------------------------------------------------------------
public class RealStorage extends BaseStorage
{
//----------------------------------------------------------------------------
//
// Method-
//       RealStorage.RealStorage
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   RealStorage(                     // Constructor
     int               size)        // Size (in frames)
{
   int                 i;

   page= new RealPage[size];
   for(i= 0; i<size; i++)
     page[i]= new RealPage();
}
} // Class RealStorage

