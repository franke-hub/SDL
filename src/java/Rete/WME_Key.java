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
//       WME_Key.java
//
// Purpose-
//       Working Memory Element Hashtable key.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
import java.lang.*;
import java.util.*;
import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       WME_Key
//
// Purpose-
//       Working Memory Element Hashtable key.
//
// Reference-
//       WME.fields
//
//----------------------------------------------------------------------------
public class WME_Key extends MapDebugAdaptor { // Working WME Hashtable key
//----------------------------------------------------------------------------
// WME_Key.Attributes
//----------------------------------------------------------------------------
static final int       INDEX_ID= 0;    // Identifier index
static final int       INDEX_ATTR= 1;  // Attribute  index
static final int       INDEX_VALUE= 2; // Value      index
static final int       INDEX_COUNT= 3; // Number of index values

final String           id;          // Identifier
final String           attr;        // Attribute
final String           value;       // Value

//----------------------------------------------------------------------------
//
// Method-
//       WME_Key.WME_Key
//
// Purpose-
//       Constructor
//
// Usage notes-
//       Once constructed, a WME_Key *MUST NOT* be modified.
//
//----------------------------------------------------------------------------
public
   WME_Key(                         // Constructor
     String            id,          // Identifier
     String            attr,        // Attribute
     String            value)       // Value
{
   this.id= id;
   this.attr= attr;
   this.value= value;
}

public
   WME_Key(                         // Copy constructor
     WME_Key           copy)        // Source
{
   id=    copy.id;
   attr=  copy.attr;
   value= copy.value;
}

//----------------------------------------------------------------------------
//
// Method-
//       WME_Key.MapDebug
//
// Purpose-
//       Override MapDebugAdaptor methods
//
//----------------------------------------------------------------------------
public void
   debug(                           // Debugging display
     DebugMap          map)         // DebugMap extention
{
   super.debug(map);                // Display this entry
}

public String                       // Extended reference String
   getReference( )                  // Get reference String
{
   return toString();
}

//----------------------------------------------------------------------------
//
// Method-
//       WME_Key.Object
//
// Purpose-
//       Override Hashtable update methods
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff keys are equal
   equals(                          // Are keys equal?
     Object            obj)         // To this WKE_Key Object
{
   if( obj instanceof WME_Key )
   {
     WME_Key key= (WME_Key)obj;

     if( key.id == id && key.attr == attr && key.value == value )
       return true;
   }

   return false;
}

public int                          // The associated hash code
   hashCode( )                      // Get associated hash code
{
   int code= id.hashCode()
           + attr.hashCode()
           + value.hashCode();

   return code;
}

//----------------------------------------------------------------------------
//
// Method-
//       WME_Key.equalsDontCare
//
// Purpose-
//       Compare, ignoring "dont care" variables in THIS WME_Key
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff keys are equal
   equalsDontCare(                  // Are keys equal?
     WME_Key           key)         // To this WKE_Key Object
{
   if( id.equals("*") || id.equals(key.id) )
     if( attr.equals("*") || attr.equals(key.attr) )
       if( value.equals("*") || value.equals(key.value) )
         return true;

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       WME_Key.dontCare
//
// Purpose-
//       Convert symbols to "Don't care"
//
//----------------------------------------------------------------------------
public WME_Key                      // Copied key
   dontCare( )                      // Return associated "DontCare" key
{
   String i= this.id;
   String a= this.attr;
   String v= this.value;

   if( i.charAt(0) == '<' )
     i= "*";

   if( a.charAt(0) == '<' )
     a= "*";

   if( v.charAt(0) == '<' )
     v= "*";

   return new WME_Key(i, a, v);
}

//----------------------------------------------------------------------------
//
// Method-
//       WME_Key.index
//
// Purpose-
//       Return id, attribute, or value
//
//----------------------------------------------------------------------------
public String                       // The id, attribute, or value
   index(                           // Get id, attribute, or value
     int               x)           // INDEX_* control
{
   String result= null;             // Resultant

   switch(x)
   {
     case INDEX_ID:
       result= id;
       break;

     case INDEX_ATTR:
       result= attr;
       break;

     case INDEX_VALUE:
       result= value;
       break;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       WME_Key.size
//
// Purpose-
//       Return INDEX_COUNT
//
//----------------------------------------------------------------------------
public int                          // The number of index values
   size( )                          // Get number of index values
{  return INDEX_COUNT;
}

//----------------------------------------------------------------------------
//
// Method-
//       WME_Key.toString
//
// Purpose-
//       Convert to String
//
//----------------------------------------------------------------------------
public String                       // The display String
   toString( )                      // Get display String
{
   return "{" + id + "," + attr + "," + value + "}";
}
} // class WME_Key

