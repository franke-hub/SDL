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
//       Instruction.java
//
// Purpose-
//       (NOP) Instruction descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Instruction
//
// Purpose-
//       A Instruction contains executable code.
//
//----------------------------------------------------------------------------
public class Instruction implements CodeWord
{
//----------------------------------------------------------------------------
// Instruction.attributes
//----------------------------------------------------------------------------
   int                 addr;        // Address
   int                 amod;        // Address modifier
static final int       ABS= 0x0000; // Absolute address
static final int        IA= 0x0001; // Relative (to IA)
static final int        BP= 0x0002; // Relative (to BP)
static final int        SP= 0x0003; // Relative (to SP)
static final int        IX= 0x0004; // + IX
static final int       MEM= 0x0008; // + Indirect

//----------------------------------------------------------------------------
//
// Method-
//       Instruction.Instruction
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Instruction( )                   // Constructor
{
   amod= 0;
   addr= 0;
}

public
   Instruction(                     // Constructor
     int               amod)        // Modifier
{
   this.amod= amod;
   addr= 0;
}

public
   Instruction(                     // Constructor
     int               amod,        // Modifier
     int               addr)        // Address
{
   this.amod= amod;
   this.addr= addr;
}

//----------------------------------------------------------------------------
//
// Method-
//       Instruction.execute
//
// Purpose-
//       Execute this Instruction.
//
//----------------------------------------------------------------------------
public void
   execute(                         // Execute this Instruction
     Cpu               cpu)         // Associated Cpu
   throws Exception
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Instruction.getAddr
//
// Purpose-
//       Extract the (modified) address.
//
//----------------------------------------------------------------------------
public int                          // Working address
   getAddr(                         // Extract address
     Cpu               cpu)         // Associated Cpu
   throws Exception
{
   int                 address;     // Target address

   address= addr;                   // Base address
   switch( amod & 3 )               // Address type
   {
     case IA:
       address += cpu.iaddr;
       break;

     case BP:
       address += cpu.bp;
       break;

     case SP:
       address += cpu.sp;
       break;
   }

   if( (amod&IX) != 0 )
     address += cpu.ix;

   if( (amod&MEM) != 0 )
     address= ((Addr)cpu.dFetch(address)).getAddr();

   return address;
}

//----------------------------------------------------------------------------
//
// Method-
//       Instruction.toString
//
// Purpose-
//       Convert Instruction to String.
//
//----------------------------------------------------------------------------
public String                       // String representation
   toString( )                      // Convert to String
{
   String              string;      // Working String

   string= null;
   switch( amod & 3 )               // Address type
   {
     case ABS:
       string= "ABS";
       break;

     case IA:
       string= "IA";
       break;

     case BP:
       string= "BP";
       break;

     case SP:
       string= "SP";
       break;
   }
   if( (amod&IX) != 0 )
     string= string + "+IX";
   if( (amod&MEM) != 0 )
     string= "[" + string + "]";

   return getClass().getName() +
          "{" + string + "," + Cpu.toHex(addr) + "}";
}
} // Class Instruction

