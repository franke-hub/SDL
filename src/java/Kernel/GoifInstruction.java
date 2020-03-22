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
//       GoifInstruction.java
//
// Purpose-
//       Goif Instruction descriptor.
//
// Last change date-
//       2010/01/01
//
// Operation-
//       ... Boolean() - ...
//       if( Boolean() ) machine.iaddr= addr
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       GoifInstruction
//
// Purpose-
//       Goif Instruction.
//
//----------------------------------------------------------------------------
public class GoifInstruction extends Instruction
{
//----------------------------------------------------------------------------
//
// Method-
//       GoifInstruction.GoifInstruction
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   GoifInstruction( )               // Constructor
{
   super();
}

public
   GoifInstruction(                 // Constructor
     int               amod)        // Modifier
{
   super(amod);
}

public
   GoifInstruction(                 // Constructor
     int               amod,        // Modifier
     int               addr)        // Address
{
   super(amod, addr);
}

//----------------------------------------------------------------------------
//
// Method-
//       GoifInstruction.execute
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
   Boolean             o;

   o= (Boolean)cpu.spFetch(0);

   if( o.booleanValue() )
     cpu.iaddr= getAddr(cpu);

   cpu.sp++;
}
} // Class GoifInstruction

