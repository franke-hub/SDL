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
//       BootInstruction.java
//
// Purpose-
//       Boot Instruction descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       BootInstruction
//
// Purpose-
//       Boot the Cpu.
//
//----------------------------------------------------------------------------
public class BootInstruction extends Instruction
{
//----------------------------------------------------------------------------
//
// Method-
//       BootInstruction.BootInstruction
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   BootInstruction( )               // Constructor
{
   super();
}

//----------------------------------------------------------------------------
//
// Method-
//       BootInstruction.execute
//
// Purpose-
//       Execute this Instruction.
//
//----------------------------------------------------------------------------
public void                         // Execute this Instruction
   execute(                         // Execute this Instruction
     Cpu               cpu)         // Associated Cpu
   throws Exception
{
   Object[]            args;        // Argument array
   HaltInstruction     haltInstruction= new HaltInstruction();
   VirtStorage         virtStorage;

   Integer             zero= new Integer(0);

   int                 i;

   cpu.dStore(Cpu.IVEC_SOFT_RESET, haltInstruction);
   cpu.dStore(Cpu.IVEC_PAGE_FAULT, haltInstruction);
   cpu.dStore(Cpu.IVEC_USER_FAULT, haltInstruction);

   // Boot loader
   cpu.dStore(0x0100, new LinkInstruction(I.IA));

   cpu.dStore(0x0101, new ShowInstruction());
   cpu.dStore(0x0102, new DumpInstruction());
   cpu.dStore(0x0103, new HaltInstruction());

   // Set Virtual environment
   cpu.sp= Page.size*Page.size*Page.size;
   cpu.bp= cpu.sp;

   cpu.virt= new VirtStorage(cpu.main, cpu.main.read.frame(1));
   cpu.psw=  Cpu.PSW_DVIRT | Cpu.PSW_IVIRT;
   cpu.iaddr= Page.size;

   // Set Real environment
   if( false )
   {
     cpu.sp= cpu.main.getSize()*Page.size;
     cpu.bp= cpu.sp;

     cpu.psw=   0;
     cpu.iaddr= 0x0100;
   }
}
} // Class BootInstruction

