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
//       LinkInstruction.java
//
// Purpose-
//       Link Instruction descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       LinkInstruction
//
// Purpose-
//       Goto and Link Instruction.
//
//----------------------------------------------------------------------------
public class LinkInstruction extends Instruction
{
//----------------------------------------------------------------------------
//
// Method-
//       LinkInstruction.LinkInstruction
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   LinkInstruction( )               // Constructor
{
   super();
}

public
   LinkInstruction(                 // Constructor
     int               amod)        // Modifier
{
   super(amod);
}

public
   LinkInstruction(                 // Constructor
     int               amod,        // Modifier
     int               addr)        // Address
{
   super(amod, addr);
}

//----------------------------------------------------------------------------
//
// Method-
//       LinkInstruction.execute
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
   cpu.lr= cpu.iaddr;
   cpu.iaddr= getAddr(cpu);
}
} // Class LinkInstruction

