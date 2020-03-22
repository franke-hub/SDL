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
//       HaltInstruction.java
//
// Purpose-
//       Halt Instruction descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       HaltInstruction
//
// Purpose-
//       Halt the Cpu.
//
//----------------------------------------------------------------------------
public class HaltInstruction extends Instruction
{
//----------------------------------------------------------------------------
//
// Method-
//       HaltInstruction.HaltInstruction
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   HaltInstruction( )               // Constructor
{
   super();
}

public
   HaltInstruction(                 // Constructor
     int               amod)        // Modifier
{
   super(amod);
}

public
   HaltInstruction(                 // Constructor
     int               amod,        // Modifier
     int               addr)        // Address
{
   super(amod, addr);
}

//----------------------------------------------------------------------------
//
// Method-
//       HaltInstruction.execute
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
   throw new Exception("Halt: " + Debug.toHex(addr));
}
} // Class HaltInstruction

