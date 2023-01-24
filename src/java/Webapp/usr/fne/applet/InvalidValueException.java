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
//       InvalidValueException.java
//
// Purpose-
//       This exception is thrown when a value is invalid.
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import java.lang.RuntimeException;

//----------------------------------------------------------------------------
//
// Class-
//       InvalidValueException
//
// Purpose-
//       This exception is thrown when a value is invalid.
//
//----------------------------------------------------------------------------
public class InvalidValueException extends RuntimeException {
   InvalidValueException() { super(); }
   InvalidValueException(String s) { super(s); }
   InvalidValueException(int v) { super("" + v); }
} // class InvalidValueException
