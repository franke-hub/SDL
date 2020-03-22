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
//       ReadStorage.java
//
// Purpose-
//       Read-only storage descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       ReadStorage
//
// Purpose-
//       Read-only storage descriptor.
//
//----------------------------------------------------------------------------
public class ReadStorage extends BaseStorage
{
//----------------------------------------------------------------------------
//
// Method-
//       ReadStorage.ReadStorage
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   ReadStorage(                     // Constructor
     int               size)        // Size (in frames)
{
   int                 i;

   page= new ReadPage[size];
   for(i= 0; i<size; i++)
     page[i]= new ReadPage();
}
} // Class ReadStorage

