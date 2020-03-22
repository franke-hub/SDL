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
//       PushInstruction.java
//
// Purpose-
//       Push instruction.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       PushInstruction
//
// Purpose-
//       Push value onto stack.
//
//----------------------------------------------------------------------------
public class PushInstruction extends Instruction
{
//----------------------------------------------------------------------------
//
// Method-
//       PushInstruction.PushInstruction
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   PushInstruction( )               // Constructor
{
   super();
}

public
   PushInstruction(                 // Constructor
     int               amod)        // Modifier
{
   super(amod);
}

public
   PushInstruction(                 // Constructor
     int               amod,        // Modifier
     int               addr)        // Address
{
   super(amod, addr);
}

//----------------------------------------------------------------------------
//
// Method-
//       PushInstruction.execute
//
// Purpose-
//       Execute this Instruction.
//
// Notes-
//       Object= fetch(addr)
//       ... - ... Object
//
//----------------------------------------------------------------------------
public void                         // Execute this Instruction
   execute(                         // Execute this Instruction
     Cpu               cpu)         // Associated Cpu
   throws Exception
{
   Object              o;

   o= cpu.dFetch(getAddr(cpu));
   cpu.spStore(-1, o);
   cpu.sp--;
}
} // Class PushInstruction

