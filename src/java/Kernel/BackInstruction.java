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
//       BackInstruction.java
//
// Purpose-
//       Back Instruction descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       BackInstruction
//
// Purpose-
//       Return from subroutine.
//
//----------------------------------------------------------------------------
public class BackInstruction extends Instruction
{
//----------------------------------------------------------------------------
//
// Method-
//       BackInstruction.BackInstruction
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   BackInstruction( )               // Constructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       BackInstruction.execute
//
// Purpose-
//       Execute this Instruction.
//
//----------------------------------------------------------------------------
public void                         // Execute this Instruction
   execute(                         // Execute this Instruction
     Cpu           cpu)             // Associated Cpu
   throws Exception
{
   cpu.iaddr= cpu.lr;
}
} // Class BackInstruction

