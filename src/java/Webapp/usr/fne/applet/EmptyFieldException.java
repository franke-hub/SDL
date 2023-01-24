//----------------------------------------------------------------------------
//
//       Copyright (C) 2008-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EmptyFieldException.java
//
// Purpose-
//       This exception is thrown when a required field is empty.
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import java.lang.RuntimeException;

//----------------------------------------------------------------------------
//
// Class-
//       EmptyFieldException
//
// Purpose-
//       This exception is thrown when a required field is empty.
//
//----------------------------------------------------------------------------
public class EmptyFieldException extends RuntimeException {
   EmptyFieldException() { super(); }
   EmptyFieldException(String s) { super(s); }
} // class EmptyFieldException
