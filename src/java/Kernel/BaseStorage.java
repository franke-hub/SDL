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
//       BaseStorage.java
//
// Purpose-
//       Storage base class.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
import user.util.Debug;

//----------------------------------------------------------------------------
//
// Class-
//       BaseStorage
//
// Purpose-
//       Storage base class.
//
//----------------------------------------------------------------------------
public class BaseStorage extends Debug
{
//----------------------------------------------------------------------------
// BaseStorage.attributes
//----------------------------------------------------------------------------
   Page[]              page;        // Page frame array

//----------------------------------------------------------------------------
//
// Method-
//       BaseStorage.BaseStorage
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   BaseStorage( )                   // Constructor
{
   this.page= null;
}

public
   BaseStorage(                     // Constructor
     Page[]            page)        // Page frame array
{
   this.page= page;
}

//----------------------------------------------------------------------------
//
// Method-
//       BaseStorage.dump
//
// Purpose-
//       Dump Storage (without modifying R/C indicators)
//
//----------------------------------------------------------------------------
public void
   dump( )                          // Dump this Storage
{
   Object              o;
   Page                page;
   boolean             isNull;

   int                 i, j;

   tracef(getClass().getName() + ".dump()");

   isNull= true;
   for(i= 0; i<getSize(); i++)
   {
     page= frame(i);
     if( page != null )
     {
       for(j= 0; j<page.size; j++)
       {
         o= page.word[j];
         if( o != null )
         {
           tracef("[" + toHex(i*page.size + j) + "] " + toString(o));
           isNull= false;
         }
       }
     }
   }

   if( isNull )
     tracef("..<null>..");

   tracef("");
}

//----------------------------------------------------------------------------
//
// Method-
//       BaseStorage.getSize
//
// Purpose-
//       Extract size.
//
//----------------------------------------------------------------------------
public int                          // Resultant
   getSize( )                       // Extract size
{
   if( page == null )
     return 0;

   return page.length;
}

//----------------------------------------------------------------------------
//
// Method-
//       BaseStorage.frame
//
// Purpose-
//       Return frame at index.
//
//----------------------------------------------------------------------------
public Page                         // Resultant
   frame(                           // Return Page
     int               index)       // Frame index
{
   return page[index];
}
} // Class BaseStorage

