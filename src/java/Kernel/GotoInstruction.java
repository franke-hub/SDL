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
//       GotoInstruction.java
//
// Purpose-
//       Goto Instruction descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       GotoInstruction
//
// Purpose-
//       Goto Instruction.
//
//----------------------------------------------------------------------------
public class GotoInstruction extends Instruction
{
//----------------------------------------------------------------------------
//
// Method-
//       GotoInstruction.GotoInstruction
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   GotoInstruction( )               // Constructor
{
   super();
}

public
   GotoInstruction(                 // Constructor
     int               amod)        // Modifier
{
   super(amod);
}

public
   GotoInstruction(                 // Constructor
     int               amod,        // Modifier
     int               addr)        // Address
{
   super(amod, addr);
}

//----------------------------------------------------------------------------
//
// Method-
//       GotoInstruction.execute
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
   cpu.iaddr= getAddr(cpu);
}
} // Class GotoInstruction

