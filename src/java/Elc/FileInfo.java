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
//       FileInfo.java
//
// Purpose-
//       File information.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
import java.io.*;

//----------------------------------------------------------------------------
//
// Class-
//       FileInfo
//
// Purpose-
//       File information.
//
//----------------------------------------------------------------------------
class FileInfo extends File         // File information
{
//----------------------------------------------------------------------------
// FileInfo::Constructors
//----------------------------------------------------------------------------
public
   FileInfo(                        // Constructor
     String            fileName)    // From file name
{
   super(fileName);
}

   FileInfo(                        // Constructor
     String            pathName,    // From path name
     String            fileName)    // From file name
{
   super(pathName, fileName);
}

//----------------------------------------------------------------------------
// FileInfo::Accessors
//----------------------------------------------------------------------------
public String                       // The file name extension
   getExtension( )                  // Get file name extension
{
   String              result= "";  // Resultant

   try {
     String name= getCanonicalPath();
     int L= name.length() - 1;

     int C= 0;                      // Current character
     while( L >= 0 )
     {
       C= name.charAt(L);
       if( C == separatorChar || C == '.' )
         break;

       L--;
     }

     if( L >= 0 && C != separatorChar )
       result= name.substring(L+1);
   } catch(Exception X) {
   }

   return result;
}
}; // class FileInfo

