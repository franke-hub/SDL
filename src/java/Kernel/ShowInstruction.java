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
//       ShowInstruction.java
//
// Purpose-
//       Show Instruction descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
import user.util.StringFormat;

//----------------------------------------------------------------------------
//
// Class-
//       ShowInstruction
//
// Purpose-
//       Display registers.
//
//----------------------------------------------------------------------------
public class ShowInstruction extends Instruction
{
//----------------------------------------------------------------------------
//
// Method-
//       ShowInstruction.ShowInstruction
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   ShowInstruction()                // Constructor
{
   super();
}

public
   ShowInstruction(                 // Constructor
     int               amod)        // Modifier
{
   super(amod);
}

public
   ShowInstruction(                 // Constructor
     int               amod,        // Modifier
     int               addr)        // Address
{
   super(amod, addr);
}

//----------------------------------------------------------------------------
//
// Method-
//       ShowInstruction.execute
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
   StringFormat        string= new StringFormat();

   int                 i;

   Cpu.debugf("PSW: " +
              Cpu.toHex(cpu.psw) + "." +
              Cpu.toHex(cpu.iaddr) );
   Cpu.debugf(" BP: " + Cpu.toHex(cpu.bp) +
              " SP: " + Cpu.toHex(cpu.sp) );
   Cpu.debugf(" LR: " + Cpu.toHex(cpu.lr) +
              " IX: " + Cpu.toHex(cpu.ix) );

   if( addr > 0 )
   {
     for(i= addr-1; i>=0; i--)
     {
       Cpu.debugf("[" + Cpu.toHex(cpu.sp + i) + "] " +
                  Cpu.toString(cpu.spFetch(i)));
     }
   }
   Cpu.debugf("");
}
} // Class ShowInstruction

