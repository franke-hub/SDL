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
//       MapDebugAdaptor.java
//
// Purpose-
//       Extend Debug with MapDebug interfaces.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
package user.util;

//----------------------------------------------------------------------------
//
// Classe-
//       MapDebugAdaptor
//
// Purpose-
//       Extend Debug, implementing MapDebug interfaces
//
//----------------------------------------------------------------------------
public class MapDebugAdaptor extends Debug // A MapDebug adaptor
             implements MapDebug {  // Implementing MapDebug interface
//----------------------------------------------------------------------------
//
// Method-
//       MapDebugAdaptor.MapDebugAdaptor
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   MapDebugAdaptor( )               // Default constructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       MapDebugAdaptor.debug
//
// Purpose-
//       Implement MapDebug.debug
//
// Sample usage-
//       super.debug(map)
//
//       ForEach contined field
//         debugln(".. fieldName: " + fieldName);
//
//       ForEach contined object
//         map.debug(".. fieldName: ", fieldName);
//
//----------------------------------------------------------------------------
public void
   debug(                           // Object debugging display
     DebugMap          map)         // With Memory reference analysis
{
   debugln(getReference());
}

//----------------------------------------------------------------------------
//
// Method-
//       MapDebugAdaptor.getObjectSN
//
// Purpose-
//       Get object serial number   (Hard core debug item)
//
// Usage notes-
//       This base method is usable, but not expcted to be used much.
//
//----------------------------------------------------------------------------
public int
   getObjectSN( )                   // Get object serial number
{
   char[]              c= new char[8];
   String              s= getObjectString(); // "ClassName@hex-addr"
   int                 L= s.length();

   for(int i= 0; i<c.length; i++)
     c[i]= '0';

   int j= c.length - 1;
   for(int i= L; L>0 ; i--)
   {
     char C= s.charAt(i-1);
     if( C == '@' )
       break;

     c[j--]= s.charAt(i-1);
   }

   return Integer.parseInt(new String(c), 16); // Return converted value
}

//----------------------------------------------------------------------------
//
// Method-
//       MapDebugAdaptor.getReference
//
// Purpose-
//       Get reference string, which MUST NOT change after construction
//
// Usage notes-
//       There is generally no need to override this method.
//
//----------------------------------------------------------------------------
public String                       // The Debug reference String
   getReference( )                  // Get Debug reference String
{
   return getClassName()
        + "[" + toHex(getObjectSN()) + "]";
}
} // class MapDebugAdaptor

