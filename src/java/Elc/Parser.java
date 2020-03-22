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
//       Parser.java
//
// Purpose-
//       Parser interface.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
import java.io.*;

//----------------------------------------------------------------------------
//
// Interface-
//       Parser
//
// Purpose-
//       Parser interface.
//
//----------------------------------------------------------------------------
interface Parser                    // Parser interface
{
public void
   debug( );                        // Write debugging messages

public int                          // Return code (0 OK)
   parse(                           // Parse
     LineReader        reader)      // Using this LineReader.
   throws Exception;

public void
   reset( );                        // Reset the Parser
}; // interface Parser

