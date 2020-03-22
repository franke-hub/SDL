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
//       StxlInstruction.java
//
// Purpose-
//       Stxl Instruction descriptor.
//
// Last change date-
//       2010/01/01
//
// Operation-
//       ... p2 p1 Addr(LR) Addr(IX) Addr(BP) - ... p2 p1
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       StxlInstruction
//
// Purpose-
//       Standard exit linkage.
//
//----------------------------------------------------------------------------
public class StxlInstruction extends Instruction
{
//----------------------------------------------------------------------------
//
// Method-
//       StxlInstruction.StxlInstruction
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   StxlInstruction( )               // Constructor
{
   super();
}

//----------------------------------------------------------------------------
//
// Method-
//       StxlInstruction.execute
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
   int                 bp;
   int                 ix;

           bp= ((Addr)cpu.spFetch(0)).getAddr();
           ix= ((Addr)cpu.spFetch(1)).getAddr();
   cpu.lr=     ((Addr)cpu.spFetch(2)).getAddr();
   cpu.bp= bp;
   cpu.ix= ix;

   cpu.sp += 3;
}
} // Class StxlInstruction

