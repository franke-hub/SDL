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
//       MainStorage.java
//
// Purpose-
//       Storage base class.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
import user.util.StringFormat;

//----------------------------------------------------------------------------
//
// Class-
//       MainStorage
//
// Purpose-
//       MainStorage class.
//
//----------------------------------------------------------------------------
public class MainStorage extends BaseStorage
{
//----------------------------------------------------------------------------
// MainStorage.attributes
//----------------------------------------------------------------------------
static final int       ROS_REGION= 0x7FFF0000; // Read/only storage region
static final int       ROS_OFFSET= 0x0000FFFF; // Read/only storage region

static final int       ROS_FRAME=  ROS_REGION / Page.size;
static final int       ROS_FRAMEO= ROS_OFFSET / Page.size;

   RealStorage         real;        // Real Storage region
   ReadStorage         read;        // Read-only storage region

//----------------------------------------------------------------------------
//
// Method-
//       MainStorage.MainStorage
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   MainStorage(                     // Constructor
     RealStorage       realStorage, // RealStorage region
     ReadStorage       readStorage) // ReadStorage region
{
   super();

   real= realStorage;
   read= readStorage;
}

//----------------------------------------------------------------------------
//
// Method-
//       MainStorage.getSize
//
// Purpose-
//       Extract size.
//
//----------------------------------------------------------------------------
public int                          // Resultant
   getSize( )                       // Exttract size
{
   return real.getSize();
}

//----------------------------------------------------------------------------
//
// Method-
//       MainStorage.frame
//
// Purpose-
//       Return frame at index.
//
//----------------------------------------------------------------------------
public Page                         // Resultant
   frame(                           // Return Page
     int               inpIndex)    // Frame index
{
   int                 index= inpIndex;

   if( index < 0 )
     Cpu.checkStop("MainStorage.frame, Addr: " + Cpu.toHex(inpIndex));

   if( index >= ROS_FRAME )
   {
     index &= ROS_FRAMEO;
     return read.frame(index);
   }

   return real.frame(index);
}

//----------------------------------------------------------------------------
//
// Method-
//       MainStorage.fetch
//
// Purpose-
//       Fetch word from MainStorage.
//
//----------------------------------------------------------------------------
public Object                       // Resultant
   fetch(                           // Fetch word from MainStorage
     int               address)     // Real address
{
   Object              result;      // Resultant
   Page                page;        // Page

   int                 pIndex;      // Page index
   int                 wIndex;      // Word index

   wIndex= address & Page.mask;
   pIndex= address / Page.size;

   page= frame(pIndex);
   result= page.fetch(wIndex);

   if( Cpu.hcdm )
   {
     StringFormat      s= new StringFormat();

     s.append("Fetch [").append(Cpu.toHex(address)).append("] ")
      .append(Cpu.toString(result));
     Cpu.debugf(s.toString());
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       MainStorage.store
//
// Purpose-
//       Store word into MainStorage.
//
//----------------------------------------------------------------------------
public void
   store(                           // Store word into MainStorage
     int               address,     // Real address
     Object            object)      // Data word
{
   Page                page;        // Page

   int                 pIndex;      // Page index
   int                 wIndex;      // Word index

   wIndex= address & Page.mask;
   pIndex= address / Page.size;

   page= frame(pIndex);
   page.store(wIndex, object);

   if( Cpu.hcdm )
   {
     StringFormat      s= new StringFormat();

     s.append("Store [").append(Cpu.toHex(address)).append("] ")
      .append(Cpu.toString(object));
     Cpu.debugf(s.toString());
   }
}
} // Class MainStorage

