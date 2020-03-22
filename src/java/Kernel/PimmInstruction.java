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
//       PimmInstruction.java
//
// Purpose-
//       Push immediate instruction.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       PimmInstruction
//
// Purpose-
//       Pimm value onto stack.
//
//----------------------------------------------------------------------------
public class PimmInstruction extends Instruction
{
//----------------------------------------------------------------------------
//
// Method-
//       PimmInstruction.PimmInstruction
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   PimmInstruction( )               // Constructor
{
   super();
}

public
   PimmInstruction(                 // Constructor
     int               amod)        // Modifier
{
   super(amod);
}

public
   PimmInstruction(                 // Constructor
     int               amod,        // Modifier
     int               addr)        // Address
{
   super(amod, addr);
}

//----------------------------------------------------------------------------
//
// Method-
//       PimmInstruction.execute
//
// Purpose-
//       Execute this Instruction.
//
// Notes-
//       Object= new Addr(addr)
//       ... - ... Object
//
//----------------------------------------------------------------------------
public void                         // Execute this Instruction
   execute(                         // Execute this Instruction
     Cpu               cpu)         // Associated Cpu
   throws Exception
{
   Object              o;

   o= new Addr(getAddr(cpu));
   cpu.spStore(-1, o);
   cpu.sp--;
}
} // Class PimmInstruction

