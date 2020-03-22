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
//       LdixInstruction.java
//
// Purpose-
//       Ldix instruction.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       LdixInstruction
//
// Purpose-
//       Load index.
//
//----------------------------------------------------------------------------
public class LdixInstruction extends Instruction
{
//----------------------------------------------------------------------------
//
// Method-
//       LdixInstruction.LdixInstruction
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   LdixInstruction( )               // Constructor
{
   super();
}

public
   LdixInstruction(                 // Constructor
     int               amod)        // Modifier
{
   super(amod);
}

public
   LdixInstruction(                 // Constructor
     int               amod,        // Modifier
     int               addr)        // Address
{
   super(amod, addr);
}

//----------------------------------------------------------------------------
//
// Method-
//       LdixInstruction.execute
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
   cpu.ix= getAddr(cpu);
}
} // Class LdixInstruction

