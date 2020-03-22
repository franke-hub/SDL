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
//       VirtModule.java
//
// Purpose-
//       VirtModule: Virtual storage Power-On System Test Module.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       VirtModule
//
// Purpose-
//       Virtual storage Power-On System Test.
//
//----------------------------------------------------------------------------
public class VirtModule extends LcModule
{
//----------------------------------------------------------------------------
//
// Method-
//       VirtModule.VirtModule
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   VirtModule( )                    // Constructor
{
   super("VIRT", new Object[] {
new LcLabel("CR0"),
       new VmapAddr(0, VmapAddr.VALID),
       new LcAbsolute("VirtualSegment[0000]"),

       new LcSpace(Page.size - 2),
       new VmapAddr(0, VmapAddr.VALID),
       new LcAbsolute("VirtualSegment[ MAX]"),

       new LcAlign(Page.size),
new LcLabel("VirtualSegment[0000]"),
       new VmapAddr(0, VmapAddr.VALID),
       new LcAbsolute("VirtualPage[00000000]"),
       new VmapAddr(0, VmapAddr.VALID),
       new LcAbsolute("VirtualPage[00000400]"),

       new LcAlign(Page.size),
new LcLabel("VirtualSegment[ MAX]"),  // Maps ROS
       new VmapAddr(0*Page.size, VmapAddr.VALID),
       new LcAbsolute("CR0"),
       new VmapAddr(1*Page.size, VmapAddr.VALID),
       new LcAbsolute("CR0"),
       new VmapAddr(2*Page.size, VmapAddr.VALID),
       new LcAbsolute("CR0"),
       new VmapAddr(3*Page.size, VmapAddr.VALID),
       new LcAbsolute("CR0"),
       new VmapAddr(4*Page.size, VmapAddr.VALID),
       new LcAbsolute("CR0"),
       new VmapAddr(5*Page.size, VmapAddr.VALID),
       new LcAbsolute("CR0"),
       new VmapAddr(6*Page.size, VmapAddr.VALID),
       new LcAbsolute("CR0"),
       new VmapAddr(7*Page.size, VmapAddr.VALID),
       new LcAbsolute("CR0"),

       new LcAlign(Page.size),
       new LcSpace(-1),
       new VmapAddr(1*Page.size, VmapAddr.VALID),

       new LcAlign(Page.size),
new LcLabel("VirtualPage[00000000]"),
new LcLabel("VP0"),
       new HaltInstruction(0, 0xffff),
       new LcSpace(15),
       new HaltInstruction(0, 0x0010),
       new LcSpace(15),
       new HaltInstruction(0, 0x0020),
       new LcSpace(15),
       new HaltInstruction(0, 0x0030),
       new LcSpace(15),
       new HaltInstruction(0, 0x0040),
       new LcSpace(15),
       new HaltInstruction(0, 0x0050),
       new LcSpace(15),
       new HaltInstruction(0, 0x0060),
       new LcSpace(15),
       new HaltInstruction(0, 0x0070),
       new LcSpace(15),
       new HaltInstruction(0, 0x0080),
       new LcSpace(15),
       new HaltInstruction(0, 0x0090),
       new LcSpace(15),
       new HaltInstruction(0, 0x00a0),
       new LcSpace(15),
       new HaltInstruction(0, 0x00b0),
       new LcSpace(15),
       new HaltInstruction(0, 0x00c0),
       new LcSpace(15),
       new HaltInstruction(0, 0x00d0),
       new LcSpace(15),
       new HaltInstruction(0, 0x00e0),
       new LcSpace(15),
       new HaltInstruction(0, 0x00f0),

       new LcAlign(Page.size),
new LcLabel("VirtualPage[00000400]"),
       Const.showInstruction,
       Const.dumpInstruction,
       Const.haltInstruction,

       // These are just for testing LcAbsolute and LcRelative
       new Addr(0xfefefefe),
       new Addr(0xfefefefe),
       new Addr(0xfefefefe),
       new Addr(0xfefefefe),

       new LinkInstruction(I.IA),
       new LcRelative("Here"),
       new LinkInstruction(),
       new LcAbsolute("Here", "VP0"),

       new Addr(0),
       new LcAbsolute("Here", "VP0"),
       new Addr(0),
       new LcAbsolute("Here"),
       new Addr(0),
       new LcRelative("Here"),
       new Addr(0),
       new LcRelative("Here"),

new LcLabel("Here"),
       new Addr(0),
       new LcRelative("Here"),
       new Addr(0),
       new LcRelative("Here"),
       new Addr(0),
       new LcRelative("Here"),
       new Addr(0),
       new LcAbsolute("Here"),

       new LinkInstruction(I.IA),
       new LcRelative("Here"),
       new LinkInstruction(),
       new LcAbsolute("Here", "VP0"),

       new Addr(0xfefe0000),
       new String("VirtStorage")
       });
}
} // Class VirtModule

