//----------------------------------------------------------------------------
//
//       Copyright (C) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       NewsGroup.java
//
// Purpose-
//       Java News Reader: Group descriptor
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.net.*;
import java.util.*;

import user.util.Debug;

public class NewsGroup extends Debug {
//----------------------------------------------------------------------------
// NewsGroup.Configuration parameters
//----------------------------------------------------------------------------
static final String    fileName= "newsgroup.out"; // NewsGroup data file

//----------------------------------------------------------------------------
// NewsGroup.Attributes
//----------------------------------------------------------------------------
String                 name;        // The name of the NewsGroup
int                    first;       // First Article number
int                    last;        // Last  Article number
int                    type;        // Posting type

//----------------------------------------------------------------------------
//
// Method-
//       NewsGroup.NewsGroup
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   NewsGroup(                       // Constructor
     String            group)       // Group descriptor
   throws Exception
{
   String              string;      // Working string
   int                 left;        // Working string index
   int                 next;        // Working string index

   try{
     left= 0;
     while( group.charAt(left) == ' ' )
       left++;
     next= group.indexOf(' ', left);
     name= group.substring(left, next);

     left= next+1;
     while( group.charAt(left) == ' ' )
       left++;
     next= group.indexOf(' ', left);
     string= group.substring(left, next);
     last= Integer.parseInt(group.substring(left, next));

     left= next+1;
     while( group.charAt(left) == ' ' )
       left++;
     next= group.indexOf(' ', left);
     string= group.substring(left, next);
     first= Integer.parseInt(group.substring(left, next));

     left= next+1;
     while( group.charAt(left) == ' ' )
       left++;
     type= group.charAt(left);
   } catch( Exception e ) {
     if( HCDM )
       debugException(e);
     throw new Exception("Malformed Group: '" + group + "'");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsGroup.read
//
// Purpose-
//       Read the list of NewsGroups from a file.
//
//----------------------------------------------------------------------------
public static Vector                // List of NewsGroups
   read( )                          // Read NewsGroup list
   throws Exception
{
   NewsGroup           group;       // Working NewsGroup
   BufferedReader      reader;      // Input Reader
   String              string;      // Working string
   Vector              vector;      // Resultant Vector

   try {
     reader= new BufferedReader(
             new FileReader(fileName)
             );
   } catch(Exception e) {
     debugf("...Cannot open: " + fileName + "\n");
     return null;
   }

   vector= new Vector();
   for(;;)
   {
     try {
       string= reader.readLine();
     } catch(EOFException x) {  // If end of file
       break;                   // We're done
     }
     if( string == null )
       break;

     group= new NewsGroup(string);
     vector.add(group);
   }

   reader.close();

   return vector;
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsGroup.write
//
// Purpose-
//       Write the list of NewsGroups into a file.
//
//----------------------------------------------------------------------------
public static void                  // List of NewsGroups
   write(                           // Write NewsGroup list
     Vector            vector)
   throws Exception
{
   NewsGroup           group;       // Working NewsGroup
   BufferedWriter      writer;      // Output Writer

   try {
     writer= new BufferedWriter(
                 new FileWriter(fileName)
                 );
   } catch(Exception e) {
     debugf("...Cannot open: " + fileName + "\n");
     return;
   }

   for(Enumeration i= vector.elements(); i.hasMoreElements(); )
   {
     group= (NewsGroup)i.nextElement();

     writer.write(group.name + " " +
                  group.last + " " +
                  group.first + " " +
                  (char)group.type +
                  "\n");
   }

   writer.close();
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsGroup.toString
//
// Purpose-
//       Display String.
//
//----------------------------------------------------------------------------
public String
   toString( )                      // Display String
{
   return "{Group(" + name  + ")" +
          ",first(" + first + ")" +
          ",last("  + last  + ")" +
          ",type("  + (char)type  + ")" +
          "}";
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsGroup.main
//
// Purpose-
//       Self test.
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
   throws Exception
{
   shouldNotOccur();
}
} // Class NewsGroup

