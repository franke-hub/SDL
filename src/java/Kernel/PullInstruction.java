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
//       PullInstruction.java
//
// Purpose-
//       Pull instruction.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       PullInstruction
//
// Purpose-
//       Pull value from stack.
//
//----------------------------------------------------------------------------
public class PullInstruction extends Instruction
{
//----------------------------------------------------------------------------
//
// Method-
//       PullInstruction.PullInstruction
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   PullInstruction( )               // Constructor
{
   super();
}

public
   PullInstruction(                 // Constructor
     int               amod)        // Modifier
{
   super(amod);
}

public
   PullInstruction(                 // Constructor
     int               amod,        // Modifier
     int               addr)        // Address
{
   super(amod, addr);
}

//----------------------------------------------------------------------------
//
// Method-
//       PullInstruction.execute
//
// Purpose-
//       Execute this Instruction.
//
// Notes-
//       ... Object - ...
//                    store(addr, Object)
//
//----------------------------------------------------------------------------
public void                         // Execute this Instruction
   execute(                         // Execute this Instruction
     Cpu               cpu)         // Associated Cpu
   throws Exception
{
   Object              o;

   o= cpu.spFetch(0);
   cpu.dStore(getAddr(cpu), o);
   cpu.sp++;
}
} // Class PullInstruction

