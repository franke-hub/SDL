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
//       StelInstruction.java
//
// Purpose-
//       Stel Instruction descriptor.
//
// Last change date-
//       2010/01/01
//
// Operation-
//       ... p2 p1 - ... p2 p1 | Addr(LR) Addr(IX) Addr(BP)
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       StelInstruction
//
// Purpose-
//       Standard entry linkage.
//
//----------------------------------------------------------------------------
public class StelInstruction extends Instruction
{
//----------------------------------------------------------------------------
//
// Method-
//       StelInstruction.StelInstruction
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   StelInstruction( )               // Constructor
{
   super();
}

//----------------------------------------------------------------------------
//
// Method-
//       StelInstruction.execute
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
   cpu.spStore(-1, new Addr(cpu.lr));
   cpu.spStore(-2, new Addr(cpu.ix));
   cpu.spStore(-3, new Addr(cpu.bp));

   cpu.bp= cpu.sp;
   cpu.sp -= 3;
}
} // Class StelInstruction

