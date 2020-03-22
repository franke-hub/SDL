//----------------------------------------------------------------------------
//
//       Copyright (C) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestCase.java
//
// Purpose-
//       Define TestCase interface.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Interface-
//       TestCase
//
// Purpose-
//       Define TestCase interface.
//
//----------------------------------------------------------------------------
public interface TestCase
{
//----------------------------------------------------------------------------
//
// Method-
//       TestCase.driver
//
// Purpose-
//       Test case driver.
//
//----------------------------------------------------------------------------
public abstract int                 // Number of errors encountered
   driver( )                        // Test case driver
   throws Exception;
} // Class TestCase

