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
//       LcReference.java
//
// Purpose-
//       LoaderControl: Reference.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       LcReference
//
// Purpose-
//       LoaderControl: Define reference.
//
//----------------------------------------------------------------------------
public abstract class LcReference implements LoaderControl
{
//----------------------------------------------------------------------------
// LcReference.attributes
//----------------------------------------------------------------------------
   Object              target;      // Target name
   Object              origin;      // Origin name

//----------------------------------------------------------------------------
//
// Method-
//       LcReference.LcReference
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   LcReference(                     // Constructor
     String            target)      // Target name
{
   this.target= target;
   this.origin= null;
}

public
   LcReference(                     // Constructor
     String[]          target)      // Qualified target name
{
   this.target= target;
   this.origin= null;
}

public
   LcReference(                     // Constructor
     String            target,      // Target name
     String            origin)      // Origin name
{
   this.target= target;
   this.origin= origin;
}

public
   LcReference(                     // Constructor
     String[]          target,      // Qualified target name
     String[]          origin)      // Qualified origin name
{
   this.target= target;
   this.origin= origin;
}

//----------------------------------------------------------------------------
//
// Method-
//       LcReference.install
//
// Purpose-
//       In pass II, install the reference.
//
//----------------------------------------------------------------------------
public abstract void
   install(                         // Install the reference
     Loader            loader,      // The associated Loader
     Object            o)           // The target Object
   throws Exception;
} // Class LcReference

