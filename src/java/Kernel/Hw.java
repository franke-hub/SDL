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
//       Hw.java
//
// Purpose-
//       Hardware descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
import user.util.*;

class PorReadInitializer implements StoreInterface
{
//----------------------------------------------------------------------------
// PorReadInitializer.attributes
//----------------------------------------------------------------------------
   ReadStorage         unit;        // The unit to initialize

//----------------------------------------------------------------------------
//
// Method-
//       PorReadInitializer.PorReadInitializer
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   PorReadInitializer(              // ReadStorage Initializer
     ReadStorage       unit)        // The unit to initialize
{
   this.unit= unit;
}

//----------------------------------------------------------------------------
//
// Method-
//       PorReadInitializer.store
//
// Purpose-
//       Store word into ReadStorage.
//
//----------------------------------------------------------------------------
public void
   store(                           // Store a Word
     int               address,     // Address
     Object            word)        // Word to store
{
   int                 pIndex;      // The Page index
   int                 wIndex;      // The Word index

   pIndex= address / Page.size;
   wIndex= address % Page.size;
   unit.frame(pIndex).word[wIndex]= word;
}
} // Class PorReadInitializer

//----------------------------------------------------------------------------
//
// Class-
//       Hw
//
// Purpose-
//       Hardware descriptor.
//
//----------------------------------------------------------------------------
public class Hw extends Debug
{
//----------------------------------------------------------------------------
// Hw.attributes
//----------------------------------------------------------------------------
static public boolean  debugging= false; // Debugging control
static public boolean  hcdm= true;  // Hard-Core Debug Mode
static public boolean  scdm= false; // Soft-Core Debug Mode

   //-------------------------------------------------------------------------
   // Architected components
   //-------------------------------------------------------------------------
   MainStorage         main;        // Main storage
   Cpu[]               cpu;         // Processor array
   KDev[]              dev;         // Device array

//----------------------------------------------------------------------------
//
// Method-
//       Hw.Hw
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Hw( )                            // Constructor
{
   ReadStorage         read;
   RealStorage         real;
   KDevPage            unit;

   // Define storage
   read= new ReadStorage(16);       // Ros Region
   real= new RealStorage(8192);     // R/W Region
   main= new MainStorage(real, read); // MainStorage

   // Define the processor array
   cpu= new Cpu[1];
   cpu[0]= new Cpu(this);

   // Define the device array
   dev= new KDev[16];
   dev[0]= new KDevPage(512);

   // Power-On Reset
   por();
}

//----------------------------------------------------------------------------
//
// Method-
//       Hw.checkStop
//
// Purpose-
//       Internal processor fault.
//
//----------------------------------------------------------------------------
public static void
   checkStop(                       // Internal processor fault
     String            message)     // Fault descriptor
{
   System.out.println("Checkstop");
   try {
     throw new Exception("Checkstop: " + message);
   } catch(Exception e) {
     e.printStackTrace();
   }

   System.exit(3);
}

//----------------------------------------------------------------------------
//
// Method-
//       Hw.por
//
// Purpose-
//       Power-On Reset
//
//----------------------------------------------------------------------------
public void
   por( )                           // Power-On Reset
{
   PorReadInitializer  initRead;    // The ReadStorage initializer
   Module              module;      // The ReadStorage Module
   Loader              loader;      // The Module Loader
   SymbolTable         symtab;      // The Symbol table

   int                 i;

   // POR: Storage
   initRead= new PorReadInitializer(main.read);
   module=   new ReadModule();
   loader=   new Loader(module);
   symtab=   new SymbolTable();
   loader.load(initRead, symtab);
   initRead.store(1, symtab);

   if( hcdm )
     main.read.dump();

   // POR: Processor
   cpu[0].por();                    // UniProcessor
}

//----------------------------------------------------------------------------
//
// Method-
//       Hw.operate
//
// Purpose-
//       Run the hardware.
//
//----------------------------------------------------------------------------
public void
   operate( )                       // Run the hardware
{
   cpu[0].operate();                // UniProcessor
}
} // Class Hw

