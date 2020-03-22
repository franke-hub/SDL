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
//       ProgramException.java
//
// Purpose-
//       Generic program Exception.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       ProgramException
//
// Purpose-
//       Program Exception descriptor.
//
//----------------------------------------------------------------------------
public class ProgramException extends Exception
{
//----------------------------------------------------------------------------
// ProgramException.constructor
//----------------------------------------------------------------------------
   ProgramException(
     String            message)
{
   super(message);
   Cpu.tracef(toString());
}
} // Class ProgramException

