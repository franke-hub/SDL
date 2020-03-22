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
//       LcAbsolute.java
//
// Purpose-
//       LoaderControl: Absolute reference
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       LcAbsolute
//
// Purpose-
//       LoaderControl: Absolute reference
//
//----------------------------------------------------------------------------
public class LcAbsolute extends LcReference
{
//----------------------------------------------------------------------------
//
// Method-
//       LcAbsolute.LcAbsolute
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   LcAbsolute(                      // Constructor
     String            target)      // Target name
{
   super(target);
}

public
   LcAbsolute(                      // Constructor
     String[]          target)      // Qualified target name
{
   super(target);
}

public
   LcAbsolute(                      // Constructor
     String            target,      // Target name
     String            origin)      // Origin name
{
   super(target, origin);
}

public
   LcAbsolute(                      // Constructor
     String[]          target,      // Qualified target name
     String[]          origin)      // Qualified origin name
{
   super(target, origin);
}

//----------------------------------------------------------------------------
//
// Method-
//       LcAbsolute.install
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
   int                 address;

   address= loader.resolve(target);
   if( origin != null )
     address -= loader.resolve(origin);

   if( o instanceof Instruction )
   {
     ((Instruction)o).addr= address;
     return;
   }

   if( o instanceof Addr )
   {
     ((Addr)o).setAddr(((Addr)o).getAddr() + address);
     return;
   }

   throw new Exception("LcAbsolute for class: " + o.getClass().getName());
}
} // Class LcAbsolute

