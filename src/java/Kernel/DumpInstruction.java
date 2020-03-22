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
//       DumpInstruction.java
//
// Purpose-
//       Dump Instruction descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
import user.util.StringFormat;
import user.util.Trace;

//----------------------------------------------------------------------------
//
// Class-
//       DumpInstruction
//
// Purpose-
//       Display registers.
//
//----------------------------------------------------------------------------
public class DumpInstruction extends Instruction
{
//----------------------------------------------------------------------------
//
// Method-
//       DumpInstruction.DumpInstruction
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   DumpInstruction()                // Constructor
{
   super();
}

//----------------------------------------------------------------------------
//
// Method-
//       DumpInstruction.execute
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
   final int           size= Page.size;
   VirtStorage         virt;
   Page                page;
   Object              word;

   StringFormat        string= new StringFormat();

   int                 i, j, k;

   Cpu.debugf("RealStorage");
   for(i=0; i<cpu.main.getSize(); i++)
   {
     page= cpu.main.frame(i);
     if( page != null )
     {
       for(j=0; j<size; j++)
       {
         word= page.word[j];
         if( word != null )
         {
           string.reset()
                 .append("[")
                   .append(Cpu.toHex(i*size + j))
                 .append("] ")
                 .append(Cpu.toString(word));
           Cpu.debugf(string.toString());
         }
       }
     }
   }

   Cpu.debugf("");
   Cpu.debugf("VirtStorage");
   if( cpu.virt == null )
     Cpu.debugf(" <null>");
   else
   {
     virt= cpu.virt;
     for(i=0; i<size; i++)
     {
       if( virt.segment(i) != null )
       {
         for(j=0; j<size; j++)
         {
           page= virt.frame(i*size + j);
           if( page != null )
           {
             if( page instanceof RealPage )
             {
               string.reset()
                     .append("[")
                       .append(Cpu.toHex((i*size + j)*size))
                     .append("] => ")
                     .append(((RealPage)page).addr.toString());
               Cpu.debugf(string.toString());
             }

             for(k=0; k<size; k++)
             {
               word= page.word[k];
               if( word != null )
               {
                 string.reset()
                       .append("[")
                         .append(Cpu.toHex((i*size + j)*size + k))
                       .append("] ")
                       .append(Cpu.toString(word));
                 Cpu.debugf(string.toString());
               }
             }
           }
         }
       }
     }
   }
   Cpu.debugf("");
}
} // Class DumpInstruction

