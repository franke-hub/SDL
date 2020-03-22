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
//       Line reader skeleton.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
import java.io.*;
import java.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       LineReader
//
// Purpose-
//       Sample line reader.
//
//----------------------------------------------------------------------------
public class LineReader
{
//----------------------------------------------------------------------------
// LineReader.Attributes
//----------------------------------------------------------------------------
String                fileName= "LineReader.java"; // LineReader file name
Vector                line;         // The lines

final private boolean HCDM = true;  // Hard Core Debug Mode

//----------------------------------------------------------------------------
//
// Method-
//       LineReader.debugf
//
// Purpose-
//       Debugging
//
//----------------------------------------------------------------------------
protected void
   debugf(                          // Debugging printf
     String            string)      // String to print
{
  System.err.print(string);
}

//----------------------------------------------------------------------------
//
// Method-
//       LineReader.LineReader
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
protected void
   constructor( )                   // Constructor
{
   line= new Vector();              // Default (empty) Vector
}

   LineReader( )                    // Constructor
{
   constructor();
}

   LineReader(                      // Constructor
     String          fileName)      // File name (base)
{
   this.fileName= fileName;
   constructor();
}

//----------------------------------------------------------------------------
//
// Method-
//       LineReader.read
//
// Purpose-
//       Read the data file.
//
//----------------------------------------------------------------------------
void
   read( )                          // Read the data file
   throws Exception
{
   BufferedReader    reader;        // LineReader reader
   String            string;        // Working String

   // Initialize
   if( HCDM )
   {
     debugf("\n");
     debugf("LineReader.read('" + fileName + "')...\n");
   }
   this.line= new Vector();

   // Open the sample file
   reader= new BufferedReader(
           new FileReader(fileName)
           );

   try {
     for(;;)                        // Read the input data
     {
       try {
         string= reader.readLine(); // Read a line
       } catch(EOFException x) {    // If end of file
         break;                     // We're done
       }

       if( string == null )         // If end of file (alternatively)
         break;                     // We're still done

       // Add the data line into the local Vector
       line.add(string);            // Line collector
     }

     if( HCDM )
       debugf("...OK\n");
   } catch(Exception e) {
     System.err.println("Exception: " + e);
   }

   // Close the data file
   try {
     if( reader != null )
       reader.close();
   } catch(Exception e) {
     System.err.println("Exception: " + e);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       LineReader.write
//
// Purpose-
//       Read the data lines.
//
//----------------------------------------------------------------------------
void
   write( )                         // Write the data lines
   throws Exception
{
   for(int i= 0; i<line.size(); i++)
     System.out.println(line.get(i).toString());
}
} // class LineReader

