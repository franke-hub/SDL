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
//       PostModule.java
//
// Purpose-
//       PostModule: Power-On System Test Module.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       PostModule
//
// Purpose-
//       Power-On System Test.
//
//----------------------------------------------------------------------------
public class PostModule extends LcModule
{
//----------------------------------------------------------------------------
//
// Method-
//       PostModule.PostModule
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   PostModule( )                    // Constructor
{
   super("POST", new Object[] {
       Const.showInstruction,       // First instruction
       Const.stelInstruction,
       new GotoInstruction(),
       new LcAbsolute("skip1"),
       Const.haltInstruction,

       //---------------------------------------------------------------------
       // This subroutine adds 1 to the LR before returning
       //---------------------------------------------------------------------
new LcLabel("skipSubroutine"),
////   Const.showInstruction,
       Const.stelInstruction,
////   new ShowInstruction(0, 3),
       new PushInstruction(I.BP, -1),
////   new ShowInstruction(0, 3),
       new IncrInstruction(I.SP),
////   new ShowInstruction(0, 3),
       new PullInstruction(I.BP, -1),
////   new ShowInstruction(0, 3),
       Const.stxlInstruction,
////   Const.showInstruction,
       Const.backInstruction,
       Const.haltInstruction,

new LcLabel("ShouldBranch"),
       new ShowInstruction(0, 1),
       new GoifInstruction(),
       new LcAbsolute("back"),

new LcLabel("WrongBranch"),
       new HaltInstruction(I.ABS, 0xfe88),

new LcLabel("ShouldMiss"),
       new ShowInstruction(0, 1),
       new GoifInstruction(),
       new LcAbsolute("WrongBranch"),

new LcLabel("back"),
       Const.backInstruction,
       Const.haltInstruction,

new LcLabel("skip1"),
       Const.showInstruction,
       new GotoInstruction(I.IA),
       new LcRelative("skip2"),
       Const.haltInstruction,

new LcLabel("skip2"),
       new LinkInstruction(),
       new LcAbsolute("skipSubroutine"),
       Const.haltInstruction,
       new LinkInstruction(I.IA),
       new LcRelative("skipSubroutine"),
       Const.haltInstruction,

       // Test the conditionals
       new PimmInstruction(0, 1),      // Compare (value (1))

       // ISGE
       new PimmInstruction(0, 0),
       new IsgeInstruction(I.SP, +1),  // 0 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldMiss"),

       new PimmInstruction(0, 1),
       new IsgeInstruction(I.SP, +1),  // 1 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldBranch"),

       new PimmInstruction(0, 2),
       new IsgeInstruction(I.SP, +1),  // 2 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldBranch"),

       // ISGT
       new PimmInstruction(0, 0),
       new IsgtInstruction(I.SP, +1),  // 0 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldMiss"),

       new PimmInstruction(0, 1),
       new IsgtInstruction(I.SP, +1),  // 1 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldMiss"),

       new PimmInstruction(0, 2),
       new IsgtInstruction(I.SP, +1),  // 2 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldBranch"),

       // ISLE
       new PimmInstruction(0, 0),
       new IsleInstruction(I.SP, +1),  // 0 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldBranch"),

       new PimmInstruction(0, 1),
       new IsleInstruction(I.SP, +1),  // 1 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldBranch"),

       new PimmInstruction(0, 2),
       new IsleInstruction(I.SP, +1),  // 2 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldMiss"),

       // ISLT
       new PimmInstruction(0, 0),
       new IsltInstruction(I.SP, +1),  // 0 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldBranch"),

       new PimmInstruction(0, 1),
       new IsltInstruction(I.SP, +1),  // 1 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldMiss"),

       new PimmInstruction(0, 2),
       new IsltInstruction(I.SP, +1),  // 2 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldMiss"),

       // ISEQ
       new PimmInstruction(0, 0),
       new IseqInstruction(I.SP, +1),  // 0 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldMiss"),

       new PimmInstruction(0, 1),
       new IseqInstruction(I.SP, +1),  // 1 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldBranch"),

       new PimmInstruction(0, 2),
       new IseqInstruction(I.SP, +1),  // 2 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldMiss"),

       // ISNE
       new PimmInstruction(0, 0),
       new IsneInstruction(I.SP, +1),  // 0 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldBranch"),

       new PimmInstruction(0, 1),
       new IsneInstruction(I.SP, +1),  // 1 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldMiss"),

       new PimmInstruction(0, 2),
       new IsneInstruction(I.SP, +1),  // 2 :: 1
       new LinkInstruction(),
       new LcAbsolute("ShouldBranch"),

       new PullInstruction(I.SP, 0),   // Discard the value (1)

       // End of module
       Const.stxlInstruction,
       Const.backInstruction,
       new Addr(0xfe000000),
       new String("PostModule")
       });
}
} // Class PostModule

