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
//       ReadModule.java
//
// Purpose-
//       ReadModule: Power-On System Test Module.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       ReadModule
//
// Purpose-
//       Power-On System Test.
//
//----------------------------------------------------------------------------
public class ReadModule extends LcModule
{
//----------------------------------------------------------------------------
//
// Method-
//       ReadModule.ReadModule
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   ReadModule( )                    // Constructor
{
   super("ROS", new Object[] {
new LcOrigin(Const.rosOrigin),
       new GotoInstruction(),
       new LcAbsolute("BOOT"),
       new String("ROS.SymbolTable"),
       new LcAlign(32),

new LcLabel("BOOT"),
       Const.showInstruction,
       new LinkInstruction(),
       new LcAbsolute(new String[] {"POST","0"}),

       Const.showInstruction,
       Const.dumpInstruction,
       new HaltInstruction(0, 0x00fe),

       // Embedded modules
       new PostModule(),

       new LcAlign(Page.size),
       new VirtModule(),

       // End of module
       new LcAlign(Page.size),
       new Addr(0),
       new LcAbsolute("SIZEOF(ROS)"),

       new Addr(0xfe000000),
       new String("ReadModule"),
new LcLabel("SIZEOF(ROS)")
       });
}
} // Class ReadModule

