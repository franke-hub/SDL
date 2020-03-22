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
//       PageException.java
//
// Purpose-
//       PageException descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       PageException
//
// Purpose-
//       Page fault descriptor.
//
//----------------------------------------------------------------------------
public class PageException extends PagingException
{
//----------------------------------------------------------------------------
// PageException.constructor
//----------------------------------------------------------------------------
   PageException(
     int               address)
{
   super(address);
   Cpu.tracef(toString());
}
} // Class PageException

