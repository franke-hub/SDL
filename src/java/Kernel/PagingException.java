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
//       PagingException.java
//
// Purpose-
//       Generic paging Exception.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       PagingException
//
// Purpose-
//       Paging Exception descriptor.
//
//----------------------------------------------------------------------------
public class PagingException extends Exception
{
//----------------------------------------------------------------------------
// PagingException.attributes
//----------------------------------------------------------------------------
   int                 address;     // Failing address

//----------------------------------------------------------------------------
// PagingException.constructor
//----------------------------------------------------------------------------
protected
   PagingException(
     int               address)
{
   super();
   this.address= address;
}

//----------------------------------------------------------------------------
//
// Method-
//       PagingException.toString
//
// Purpose-
//       Convert to String.
//
//----------------------------------------------------------------------------
public String                       // String representation
   toString( )                      // Get String representation
{
   return getClass().getName() + ": " + Cpu.toHex(address);
}
} // Class PagingException

