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
//       CodeWord.java
//
// Purpose-
//       CodeWord descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       CodeWord
//
// Purpose-
//       A CodeWord contains executable code.
//
//----------------------------------------------------------------------------
interface CodeWord
{
//----------------------------------------------------------------------------
//
// Method-
//       CodeWord.execute
//
// Purpose-
//       Execute this CodeWord.
//
//----------------------------------------------------------------------------
public abstract void                // Execute this CodeWord
   execute(                         // Execute this CodeWord
     Cpu               cpu)         // Associated Cpu
   throws Exception;
} // Class CodeWord

