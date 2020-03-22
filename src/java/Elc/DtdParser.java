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
//       DtdParser.java
//
// Purpose-
//       DTD (Document Type Definition) Parser.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
import java.io.*;

//----------------------------------------------------------------------------
//
// Class-
//       DtdParser
//
// Purpose-
//       DTD Parser.
//
//----------------------------------------------------------------------------
class DtdParser implements Parser   // DTD Parser
{
//----------------------------------------------------------------------------
// DtdParser::Constructors
//----------------------------------------------------------------------------
public
   DtdParser( )                     // Default constructor
{
}

//----------------------------------------------------------------------------
// DtdParser::Accessors
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// DtdParser::Methods
//----------------------------------------------------------------------------
public void
   debug( )                         // Write debugging messages
{
}

public int                          // Return code (0 OK)
   parse(                           // Parse
     LineReader        reader)      // Using this LineReader
{
   return 0;
}

public void
   reset( )                         // Reset the DtdParser
{
}
}; // class DtdParser
