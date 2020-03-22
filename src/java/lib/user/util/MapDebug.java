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
//       MapDebug.java
//
// Purpose-
//       Define MapDebug interface.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
package user.util;

//----------------------------------------------------------------------------
//
// Interface-
//       MapDebug
//
// Purpose-
//       Debug MapDebug interface.
//
//----------------------------------------------------------------------------
public interface MapDebug {         // The MapDebug interface
//-------------------------------------------------------------------------
//
// Method-
//       MapDebug.debug
//
// Purpose-
//       Add element to the DebugMap
//
// Sample usage-
//       super.debug(map)
//
//       ForEach contined builtin field
//         debugln(".. fieldName: " + field);
//
//       ForEach contined object
//         map.debug(".. objectName: ", object);
//
//-------------------------------------------------------------------------
public void
   debug(                           // Object debugging display
     DebugMap          map);        // With Memory reference analysis

//-------------------------------------------------------------------------
//
// Method-
//       MapDebug.getReference
//
// Purpose-
//       Get reference string, which MUST NOT change after construction
//
//-------------------------------------------------------------------------
public String                       // The Debug reference String
   getReference( );                 // Get Debug reference String

//----------------------------------------------------------------------------
//
// Method-
//       MapDebug.getObjectSN
//
// Purpose-
//       Get object serial number   (Hard core debug item)
//
//----------------------------------------------------------------------------
public int                          // The object serial number
   getObjectSN( );                  // Get object serial number
} // class MapDebug

