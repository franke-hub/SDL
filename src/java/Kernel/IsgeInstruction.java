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
//       IsgeInstruction.java
//
// Purpose-
//       Isge Instruction descriptor.
//
// Last change date-
//       2010/01/01
//
// Operation-
//       ... Object() - ... Boolean()
//
//       pull Object()
//       if( Object() >= target ) push Boolean(true)
//       if( Object() <  target ) push Boolean(false)
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       IsgeInstruction
//
// Purpose-
//       Isge Instruction.
//
//----------------------------------------------------------------------------
public class IsgeInstruction extends Instruction
{
//----------------------------------------------------------------------------
//
// Method-
//       IsgeInstruction.IsgeInstruction
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   IsgeInstruction( )               // Constructor
{
   super();
}

public
   IsgeInstruction(                 // Constructor
     int               amod)        // Modifier
{
   super(amod);
}

public
   IsgeInstruction(                 // Constructor
     int               amod,        // Modifier
     int               addr)        // Address
{
   super(amod, addr);
}

//----------------------------------------------------------------------------
//
// Method-
//       IsgeInstruction.execute
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
   Boolean             result;
   Object              source, target;
   int                 cvalue;

   source= cpu.spFetch(0);
   target= cpu.dFetch(getAddr(cpu));

   if( source instanceof Addr )
   {
     if( !(target instanceof Addr) )
       throw new ProgramException("Class mismatch");

     cvalue= ((Addr)source).getAddr() - ((Addr)target).getAddr();
   }

   else if( source instanceof IntValue )
   {
     if( !(target instanceof IntValue) )
       throw new ProgramException("Class mismatch");

     cvalue= ((IntValue)source).get() - ((IntValue)target).get();
   }

   else if( source instanceof String )
   {
     if( !(target instanceof String) )
       throw new ProgramException("Class mismatch");

     cvalue= ((String)source).compareTo((String)target);
   }

   // Not a comparable class
   else
     throw new ProgramException("CompareClass: " +
                                source.getClass().getName());

   if( cvalue >= 0 )
     cpu.spStore(0, Boolean.TRUE);
   else
     cpu.spStore(0, Boolean.FALSE);
}
} // Class IsgeInstruction

