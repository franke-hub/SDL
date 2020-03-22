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
//       LcModule.java
//
// Purpose-
//       LcModule descriptor, defines an embedded Module.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       LcModule
//
// Purpose-
//       LoaderControl: Module descriptor.
//
//----------------------------------------------------------------------------
public class LcModule extends Module implements LoaderControl
{
//----------------------------------------------------------------------------
//
// Method-
//       LcModule.LcModule
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   LcModule(                        // Constructor
     String            name,        // The module name
     Object[]          code)        // The associated code
{
   super(name, code);
}
} // Class LcModule

