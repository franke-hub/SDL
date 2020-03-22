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
//       KDevPage.java
//
// Purpose-
//       PAGE device driver.
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
//       KDevPage
//
// Purpose-
//       Page device driver.
//
//----------------------------------------------------------------------------
public class KDevPage extends KDevBase
{
//----------------------------------------------------------------------------
// KDevPage.attributes
//----------------------------------------------------------------------------
   int                 blocks;      // Number of blocks
   Page[]              page;        // Page array

//----------------------------------------------------------------------------
// KDevPage.contructor
//----------------------------------------------------------------------------
public
   KDevPage(                        // Constructor
     int               blocks)      // Number of blocks
{
   super("PAGE");

   int                 i;

   this.blocks= blocks;
   page= new Page[blocks];
   for(i=0; i<blocks; i++)
     page[i]= new Page();
}

private void
   validateOffset(
     int               offset)
   throws KioException
{
   if( offset >= 0
       && offset < blocks )
     return;

   reject("Offset(" + offset + "), Blocks(" + blocks + ")");
}

//----------------------------------------------------------------------------
// KDevPage.methods
//----------------------------------------------------------------------------
public void
   nop( )                           // 03 Operation
   throws KioException
{
}

public void
   sense(                           // 04 Operation
     int               offset,      // Offset
     Object[]          data,        // Data (implied length)
     IntValue          size)        // Size (transferred)
   throws KioException
{
   size.set(0);
}

public void
   pageOut(                         // 05 Operation
     int               offset,      // Offset
     RealPage          page)        // Data (implied length)
   throws KioException
{
   StringFormat        string;

   if( debugging )
   {
     string= new StringFormat();
     string.append(Cpu.toString(this)).append(": ")
           .append("pageOut(")
             .append(offset,4).append(",")
             .setRadix(16).append(Cpu.toString(page)).append(")");
     Trace.get().tracef(string.toString());
   }

   validateOffset(offset);
   this.page[offset].copy(page);
}

public void
   pageInp(                         // 06 Operation
     int               offset,      // Offset
     RealPage          page)        // Data (implied length)
   throws KioException
{
   StringFormat        string;

   if( debugging )
   {
     string= new StringFormat();
     string.append(Cpu.toString(this)).append(": ")
           .append("pageInp(")
             .append(offset,4).append(",")
             .setRadix(16).append(Cpu.toString(page)).append(")");
     Trace.get().tracef(string.toString());
   }

   validateOffset(offset);
   page.copy(this.page[offset]);
}

public void
   chase( )                         // 07 Operation
   throws KioException
{
}

public void
   reset( )                         // 0F Operation
   throws KioException
{
}
} // class KDevPage

