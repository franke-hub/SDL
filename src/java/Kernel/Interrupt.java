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
//       Interrupt.java
//
// Purpose-
//       Interrupt descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Interrupt
//
// Purpose-
//       A Interrupt contains executable code.
//
//----------------------------------------------------------------------------
public class Interrupt implements CodeWord
{
//----------------------------------------------------------------------------
// Interrupt.attributes
//----------------------------------------------------------------------------
   int                 oldPsw;      // The old Psw
   int                 oldIaddr;    // The old Iaddr
   int                 oldSP;       // The old SP

   int                 newPsw;      // The new Psw
   int                 newIaddr;    // The new Iaddr
   int                 newSP;       // The new SP

//----------------------------------------------------------------------------
//
// Method-
//       Interrupt.Interrupt
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Interrupt(                       // Constructor
     int               psw,         // The new PSW
     int               iaddr,       // The new Iaddr
     int               sp)          // The new SP
{
   newPsw=   psw;
   newIaddr= iaddr;
   newSP=    sp;
}

//----------------------------------------------------------------------------
//
// Method-
//       Interrupt.execute
//
// Purpose-
//       Execute this Interrupt.
//
//----------------------------------------------------------------------------
public void                         // Execute this Interrupt
   execute(                         // Execute this Interrupt
     Cpu               cpu)         // Associated Cpu
   throws Exception
{
   oldPsw=    cpu.psw;
   oldIaddr=  cpu.iaddr;
   oldSP=     cpu.sp;

   cpu.psw=   newPsw;
   cpu.iaddr= newIaddr;
   cpu.sp=    newSP;
}

//----------------------------------------------------------------------------
//
// Method-
//       Interrupt.toString
//
// Purpose-
//       Convert to String.
//
//----------------------------------------------------------------------------
public String                       // String representation
   toString( )                      // Convert to String
{
   return getClass().getName() + ": " + Cpu.toHex(newIaddr);
}
} // Class Interrupt

