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
//       Cpu.java
//
// Purpose-
//       Cpu descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       Cpu
//
// Purpose-
//       Cpu descriptor.
//
//----------------------------------------------------------------------------
public class Cpu extends Debug
{
//----------------------------------------------------------------------------
// Cpu.attributes
//----------------------------------------------------------------------------
static boolean         debugging;   // Debugging control
static boolean         hcdm;        // Hard-Core Debug Mode
static boolean         scdm;        // Soft-Core Debug Mode

   MainStorage         main;        // Main storage
   VirtStorage         virt;        // Virtual storage

   //-------------------------------------------------------------------------
   // Architected components
   //-------------------------------------------------------------------------
   Exception           exception;   // Current exception
   Interrupt           interrupt;   // Current interrupt

   int                 psw;         // Program Status Word
static final int       PSW_SUPERVISOR=  0x80000000; // Supervisor state
static final int       PSW_DVIRT=       0x00000001; // Data virtual
static final int       PSW_IVIRT=       0x00000002; // Instruction virtual

   int                 iaddr;       // Instruction Address
static final int       IVEC_SOFT_RESET= 0x00000000; // Interrupt vector
static final int       IVEC_PAGE_FAULT= 0x00000010; // Interrupt vector
static final int       IVEC_USER_FAULT= 0x00000070; // Interrupt vector

   int                 bp;          // Break pointer
   int                 sp;          // Stack pointer
   int                 lr;          // Link Register
   int                 ix;          // Index Register

//----------------------------------------------------------------------------
//
// Method-
//       Cpu.Cpu
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Cpu(                             // Constructor
     Hw                hw)          // Hardware object
{
   debugging= true;

   main= hw.main;
   por();
}

//----------------------------------------------------------------------------
//
// Method-
//       Cpu.checkStop
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
//       Cpu.tracef
//
// Purpose-
//       Tracing display
//
//----------------------------------------------------------------------------
public static void
   tracef(                          // Tracing display
     String            string)      // The String to display
{
   Trace.get().tracef(string);
   if( scdm )
     System.out.println(string);
}

//----------------------------------------------------------------------------
//
// Method-
//       Cpu.toHex
//
// Purpose-
//       Convert address to hexidecimal representation.
//
//----------------------------------------------------------------------------
public static String                // Resultant
   toHex(                           // Convert address to hexidecimal
     int               address)     // The address
{
   StringFormat        string= new StringFormat();

   return string.setRadix(16).append(address,8,8).toString();
}

//----------------------------------------------------------------------------
//
// Method-
//       Cpu.toString
//
// Purpose-
//       Convert an Object to a String
//
//----------------------------------------------------------------------------
public static String                // Resultant String
   toString(                        // Format an Object
     Object            object)      // The Object to format
{
   StringBuffer        resultant;   // Resultant

   if( object == null )
     resultant= new StringBuffer("<null>");
   else
   {
     resultant= new StringBuffer();
     resultant.append("{")
              .append(object.getClass().getName()).append(":")
              .append(object.toString())
              .append("}");
   }

   return resultant.toString();
}

//----------------------------------------------------------------------------
//
// Method-
//       Cpu.por
//
// Purpose-
//       Power-On Reset
//
//----------------------------------------------------------------------------
public void
   por( )                           // Power-On Reset
{
   bp=    0;
   sp=    main.getSize() * Page.size;
   lr=    0;
   ix=    0;

   psw=   0;
   iaddr= Const.rosOrigin;

   virt=  null;
   exception= null;
   interrupt= null;
}

//----------------------------------------------------------------------------
//
// Method-
//       Cpu.iFetch
//
// Purpose-
//       Fetch code from Storage.
//
//----------------------------------------------------------------------------
public CodeWord                     // Resultant
   iFetch(                          // Fetch code from Storage
     int               address)     // Real/Virtual address
   throws PagingException, Exception
{
   if( (psw&PSW_IVIRT) != 0 )       // If paging mode
     return (CodeWord)virt.fetch(this, address); // Fetch virtual

   return (CodeWord)main.fetch(address);
}

//----------------------------------------------------------------------------
//
// Method-
//       Cpu.dFetch
//
// Purpose-
//       Fetch word from Storage.
//
//----------------------------------------------------------------------------
public Object                       // Resultant
   dFetch(                          // Fetch word from Storage
     int               address)     // Real/Virtual address
   throws PagingException, Exception
{
   if( (psw&PSW_DVIRT) != 0 )       // If paging mode
     return virt.fetch(this, address); // Fetch virtual

   return main.fetch(address);
}

//----------------------------------------------------------------------------
//
// Method-
//       Cpu.bpFetch
//
// Purpose-
//       Fetch word from Storage.
//
//----------------------------------------------------------------------------
public Object                       // Resultant
   bpFetch(                         // Fetch word from Storage
     int               offset)      // Real/Virtual address, offset from bp
   throws PagingException, Exception
{
   return dFetch(bp + offset);      // Working address
}

//----------------------------------------------------------------------------
//
// Method-
//       Cpu.spFetch
//
// Purpose-
//       Fetch word from Storage.
//
//----------------------------------------------------------------------------
public Object                       // Resultant
   spFetch(                         // Fetch word from Storage
     int               offset)      // Real/Virtual address, offset from bp
   throws PagingException, Exception
{
   return dFetch(sp + offset);      // Working address
}

//----------------------------------------------------------------------------
//
// Method-
//       Cpu.dStore
//
// Purpose-
//       Store data into Storage.
//
//----------------------------------------------------------------------------
public void
   dStore(                          // Store word into MainStorage
     int               address,     // Real address
     Object            object)      // Data word
   throws PagingException, Exception
{
   if( (psw&PSW_DVIRT) != 0 )       // If paging mode
     virt.store(this, address, object); // Store virtual
   else
     main.store(address, object);   // Store real
}

//----------------------------------------------------------------------------
//
// Method-
//       Cpu.bpStore
//
// Purpose-
//       Store into Page.
//
//----------------------------------------------------------------------------
public void
   bpStore(                         // Store into Storage
     int               offset,      // Real/Virtual address, offset from bp
     Object            object)      // Word value
   throws PagingException, Exception
{
   dStore(bp + offset, object);
}

//----------------------------------------------------------------------------
//
// Method-
//       Cpu.spStore
//
// Purpose-
//       Store into Page.
//
//----------------------------------------------------------------------------
public void
   spStore(                         // Store into Storage
     int               offset,      // Real/Virtual address, offset from bp
     Object            object)      // Word value
   throws PagingException, Exception
{
   dStore(sp + offset, object);
}

//----------------------------------------------------------------------------
//
// Method-
//       Cpu.operate
//
// Purpose-
//       Run the cpu.
//
//----------------------------------------------------------------------------
public void
   operate( )                       // Run the cpu
{
   StringFormat        string= new StringFormat(); // Formatting Object
   Object              o;           // Generic Object
   Object[]            parm;        // Parameters
   CodeWord            codeWord;    // Current instruction

   try {
     for(;;)                        // Run the cpu
     {
       try {
         codeWord= iFetch(iaddr);   // Fetch next instruction
         if( debugging )
         {
           string.reset().setRadix(16)
                 .append("Exec: ")
                 .append("[").append(iaddr,8,8).append("] ")
                 .append(codeWord.toString());
           debugf(string.toString());
         }

         iaddr++;
         codeWord.execute(this);

       } catch(PagingException x10) {
         exception= x10;
         interrupt= new Interrupt(0, IVEC_PAGE_FAULT, Page.size-1);
         interrupt.execute(this);
       }
     }
   } catch(Exception e) {
     debugf("Cpu.operate: Exception: " + e);
     e.printStackTrace();
   }
}
} // Class Cpu

