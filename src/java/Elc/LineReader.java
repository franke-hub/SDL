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
//       LineReader.java
//
// Purpose-
//       Reader with line and column information.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
import java.io.*;

//----------------------------------------------------------------------------
//
// Class-
//       LineReader
//
// Purpose-
//       Reader with line and column information.
//
//----------------------------------------------------------------------------
class LineReader                    // Reader with line and column information
{
//----------------------------------------------------------------------------
// LineReader::Attributes
//----------------------------------------------------------------------------
Reader                 reader;      // The actual Reader

String                 name;        // The file name
int                    line;        // Line number
int                    column;      // Column number

//----------------------------------------------------------------------------
// LineReader::Constructors
//----------------------------------------------------------------------------
public
   LineReader(                      // Constructor
     String            name)        // The file name
   throws java.io.FileNotFoundException
{
   this.name= name;
   reader= new FileReader(name);

   line= 0;
   column= 0;
}

//----------------------------------------------------------------------------
// LineReader::Accessors
//----------------------------------------------------------------------------
public String                       // The file name
   getName( )                       // Get file name
{
   return name;
}

public int                          // The column number
   getColumn( )                     // Get column number
{
   return column + 1;
}

public int                          // The line number
   getLine( )                       // Get line number
{
   return line + 1;
}

//----------------------------------------------------------------------------
// LineReader::Methods
//----------------------------------------------------------------------------
public int                          // The next character
   read( )                          // Get next character
   throws java.io.IOException
{
   int result= reader.read();       // The next character

   if( result > 0 )
     column++;
   if( result == '\n' )
   {
     line++;
     column= 0;
   }

   return result;
}
}; // class LineReader

