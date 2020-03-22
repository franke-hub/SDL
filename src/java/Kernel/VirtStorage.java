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
//       VirtStorage.java
//
// Purpose-
//       Virtual storage descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
import user.util.StringFormat;

//----------------------------------------------------------------------------
//
// Class-
//       VirtStorage
//
// Purpose-
//       Virtual storage descriptor.
//
//----------------------------------------------------------------------------
public class VirtStorage extends BaseStorage
{
//----------------------------------------------------------------------------
// VirtStorage.attributes
//----------------------------------------------------------------------------
static final int       VALID=  VmapAddr.VALID;  // Valid indicator
static final int       WRONLY= VmapAddr.WRONLY; // Write inhibited
static final int       RDONLY= VmapAddr.RDONLY; // Read inhibited
static final int       SUPER=  VmapAddr.SUPER;  // Supervisor page

static final int       mask= Page.mask;
static final int       size= Page.size;
static final int       segSize=  size    * size;
static final int       memSize=  segSize * size;

   Page                cr0;         // The Segment Page
   MainStorage         main;        // The associated MainStorage

//----------------------------------------------------------------------------
//
// Method-
//       VirtStorage.VirtStorage
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   VirtStorage(                     // Constructor
     MainStorage       mainStorage, // The associated MainStorage
     Page              segment0)    // The Segment Page
{
   super(null);

   main= mainStorage;
   cr0=  segment0;
}

//----------------------------------------------------------------------------
//
// Method-
//       VirtStorage.getSize
//
// Purpose-
//       Get storage size (in frames)
//
//----------------------------------------------------------------------------
public int                          // The storage size
   getSize( )                       // Get storage size
{
   return segSize;
}

//----------------------------------------------------------------------------
//
// Method-
//       VirtStorage.getCr0
//
// Purpose-
//       Get CR0
//
//----------------------------------------------------------------------------
public Page                         // Control Register[0] content
   getCr0( )                        // Get control register[0]
{
   return cr0;
}

//----------------------------------------------------------------------------
//
// Method-
//       VirtStorage.lra
//
// Purpose-
//       Load real address.
//
//----------------------------------------------------------------------------
public int                          // The associated real address
   lra(                             // Convert virtual address to real address
     int               virt)        // Virtual address
{
   VmapAddr            pageAddr;    // Associated VmapAddr Object
   Page                page;        // Associated Page Object

   int                 pIndex;      // Page index
   int                 sIndex;      // Segment index
   int                 wIndex;      // Word index

   if( virt < 0 || virt >= memSize )
     return 0;

   // Address the Region page
   sIndex= virt / segSize;
   pageAddr= (VmapAddr)cr0.fetch(sIndex);
   if( pageAddr == null )
     return 0;

   if( (pageAddr.getAttr()&VALID) == 0 )
     return 0;

   page= main.frame(pageAddr.getFrame());

   // Address the Page
   pIndex= (virt / size) & mask;
   pageAddr= (VmapAddr)page.fetch(pIndex);
   if( pageAddr == null )
     return 0;

   if( (pageAddr.getAttr()&VALID) == 0 )
     return 0;

   // Return the real address
   wIndex= virt & mask;
   return pageAddr.getAddr() + wIndex;
}

//----------------------------------------------------------------------------
//
// Method-
//       VirtStorage.fetch
//
// Purpose-
//       Fetch word from VirtStorage.
//
//----------------------------------------------------------------------------
public Object                       // Resultant
   fetch(                           // Fetch word from VirtStorage
     Cpu               cpu,         // Cpu
     int               virt)        // Virtual address
   throws Exception
{
   VmapAddr            pageAddr;    // VmapAddr Object
   Page                page;        // Page Object

   int                 attr;        // Working attributes
   int                 real;        // Real address
   int                 pIndex;      // Page index
   int                 sIndex;      // Segment index
   int                 wIndex;      // Word index

   if( virt < 0 || virt >= memSize )
     Cpu.checkStop("VirtStorage.fetch: " + Cpu.toHex(virt));

   // Address the Region
   sIndex= virt / segSize;
   pageAddr= (VmapAddr)cr0.fetch(sIndex);
   if( pageAddr == null )
     throw new SegmentException(virt);
   if( (pageAddr.getAttr()&VALID) == 0 )
     throw new SegmentException(virt);

   page= main.frame(pageAddr.getFrame());

   // Address the Page
   pIndex= (virt / size) & mask;
   pageAddr= (VmapAddr)page.fetch(pIndex);
   if( pageAddr == null )
     throw new PageException(virt);
   if( (pageAddr.getAttr()&VALID) == 0 )
     throw new PageException(virt);

   // Check permissions
   if( (cpu.psw&Cpu.PSW_SUPERVISOR) == 0 )
   {
     attr= pageAddr.getAttr();
     if( (attr&SUPER) != 0
         || (attr&WRONLY) != 0 )
       throw new ProgramException("Access: " + Cpu.toHex(virt));
   }

   // Return the data word
   wIndex= virt & mask;
   real= pageAddr.getAddr() + wIndex;
   if( Cpu.hcdm )
   {
     StringFormat      s= new StringFormat();

     s.setRadix(16)
      .append("vAddr [").append(Cpu.toHex(virt)).append("] ")
         .append("=> [").append(Cpu.toHex(real)).append("] ");
     Cpu.debugf(s.toString());
   }

   return main.fetch(real);
}

//----------------------------------------------------------------------------
//
// Method-
//       VirtStorage.store
//
// Purpose-
//       Store word into VirtStorage
//
//----------------------------------------------------------------------------
public void
   store(                           // Store word into VirtStorage
     Cpu               cpu,         // Cpu
     int               virt,        // Virtual address
     Object            object)      // Data word
   throws Exception
{
   VmapAddr            pageAddr;    // VmapAddr Object
   Page                page;        // Page Object

   int                 attr;        // Working attributes
   int                 real;        // Real address
   int                 pIndex;      // Page index
   int                 sIndex;      // Segment index
   int                 wIndex;      // Word index

   if( virt < 0 || virt >= memSize )
     Cpu.checkStop("VirtStorage.fetch: " + Cpu.toHex(virt));

   // Address the Region
   sIndex= virt / segSize;
   pageAddr= (VmapAddr)cr0.fetch(sIndex);
   if( pageAddr == null )
     throw new SegmentException(virt);
   if( (pageAddr.getAttr()&VALID) == 0 )
     throw new SegmentException(virt);

   page= main.frame(pageAddr.getFrame());

   // Address the Page
   pIndex= (virt / size) & mask;
   pageAddr= (VmapAddr)page.fetch(pIndex);
   if( pageAddr == null )
     throw new PageException(virt);
   if( (pageAddr.getAttr()&VALID) == 0 )
     throw new PageException(virt);

   // Check permissions
   attr= pageAddr.getAttr();
   if( (attr&RDONLY) != 0 )
     throw new ProgramException("Access: " + Cpu.toHex(virt));

   if( (cpu.psw&Cpu.PSW_SUPERVISOR) == 0
       && (attr&SUPER) != 0 )
     throw new ProgramException("Access: " + Cpu.toHex(virt));

   // Store the data word
   wIndex= virt & mask;
   real= pageAddr.getAddr() + wIndex;
   if( Cpu.hcdm )
   {
     StringFormat      s= new StringFormat();

     s.setRadix(16)
      .append("vAddr [").append(Cpu.toHex(virt)).append("] ")
         .append("=> [").append(Cpu.toHex(real)).append("] ");
     Cpu.debugf(s.toString());
   }

   main.store(real, object);
}

//----------------------------------------------------------------------------
//
// Method-
//       VirtStorage.mapPage
//
// Purpose-
//       Map or unmap Page.
//
//----------------------------------------------------------------------------
public void
   mapPage(                         // Add Page to Map
     int               virt,        // Virtual address
     VmapAddr          pageAddr)    // Page to map (null to unmap)
   throws PagingException
{
   VmapAddr            workAddr;    // Working VmapAddr
   Page                page;        // Working Page
   int                 sIndex;      // Segment index
   int                 pIndex;      // Page index

   if( virt < 0 || virt >= memSize )
     Cpu.checkStop("VirtStorage.mapPage: " +
                   "Addr[" + Cpu.toHex(virt) + "] range");

   sIndex= virt / segSize;
   workAddr= (VmapAddr)cr0.fetch(sIndex);
   if( workAddr == null )
     throw new SegmentException(virt);

   page= main.frame(workAddr.getFrame());
   pIndex= (virt / size) & mask;
   page.store(pIndex, pageAddr);    // Map (or unmap) page
}

//----------------------------------------------------------------------------
//
// Method-
//       VirtStorage.mapRegion
//
// Purpose-
//       Map or unmap Region.
//
//----------------------------------------------------------------------------
public void
   mapRegion(                       // Add Region to Map
     int               virt,        // Virtual address
     VmapAddr          pageAddr)    // Region to map (null to unmap)
   throws PagingException
{
   int                 sIndex;      // Segment index

   if( virt < 0 || virt >= memSize )
     Cpu.checkStop("VirtStorage.mapRegion: " +
                   "Addr[" + Cpu.toHex(virt) + "] range");

   if( (virt&(segSize-1)) != 0 )
     Cpu.checkStop("VirtStorage.mapRegion: " +
                   "Addr[" + Cpu.toHex(virt) + "] boundary");

   sIndex= virt / segSize;
   cr0.store(sIndex, pageAddr);     // Map (or unmap) region
}

//----------------------------------------------------------------------------
//
// Method-
//       VirtStorage.getSegment
//
// Purpose-
//       Extract the Segment
//
//----------------------------------------------------------------------------
public Page                         // The associated segment Page
   segment(                         // Extract segment Page
     int               index)       // Frame index
{
   VmapAddr            pageAddr;    // Target page
   Page                segPage;     // Segment page
   int                 sIndex;      // Segment index;
   int                 pIndex;      // Page index

   if( index < 0 || index >= segSize )
     Cpu.checkStop("VirtStorage.segment: Range: " +
                   Cpu.toHex(index));

   sIndex= index / size;
   pageAddr= (VmapAddr)cr0.fetch(sIndex);
   if( pageAddr == null )
     return null;
   if( (pageAddr.getAttr()&VALID) == 0 )
     return null;

   segPage= main.frame(pageAddr.getFrame());
   if( segPage == null )
     Cpu.checkStop("VirtStorage.segment: Error: " +
                   pageAddr.toString());

   return segPage;
}

//----------------------------------------------------------------------------
//
// Method-
//       VirtStorage.frame
//
// Purpose-
//       Return frame at index.
//
//----------------------------------------------------------------------------
public Page                         // Resultant
   frame(                           // Return Page
     int               index)       // Frame index
{
   VmapAddr            pageAddr;    // Target page
   Page                segPage;     // Segment page
   int                 pIndex;      // Page index

   segPage= segment(index);
   if( segPage == null )
     return null;

   pIndex= index % size;
   pageAddr= (VmapAddr)segPage.fetch(pIndex);
   if( pageAddr == null )
     return null;
   if( (pageAddr.getAttr()&VALID) == 0 )
     return null;

   return main.frame(pageAddr.getFrame());
}
} // Class VirtStorage

