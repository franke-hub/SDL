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
//       LcRelative.java
//
// Purpose-
//       LoaderControl: Relative reference
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       LcRelative
//
// Purpose-
//       LoaderControl: Relative reference
//
//----------------------------------------------------------------------------
public class LcRelative extends LcReference
{
//----------------------------------------------------------------------------
//
// Method-
//       LcRelative.LcRelative
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   LcRelative(                      // Constructor
     String            target)      // Target name
{
   super(target);
}

public
   LcRelative(                      // Constructor
     String[]          target)      // Qualified target name
{
   super(target);
}

//----------------------------------------------------------------------------
//
// Method-
//       LcRelative.install
//
// Purpose-
//       In pass II, install the reference.
//
//----------------------------------------------------------------------------
public void
   install(                         // Install the reference
     Loader            loader,      // The associated Loader
     Object            o)           // The target Object
   throws Exception
{
   int                 offset;

   offset= loader.resolve(target) - (loader.lOrigin + loader.offset);
   if( o instanceof Instruction )
   {
     ((Instruction)o).addr= offset;
     return;
   }

   if( o instanceof Addr )
   {
     ((Addr)o).setAddr(((Addr)o).getAddr() + offset + 1);
     return;
   }

   throw new Exception("LcRelative for class: " + o.getClass().getName());
}
} // Class LcRelative

