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
//       Module.java
//
// Purpose-
//       Module descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Module
//
// Purpose-
//       Module descriptor.
//
//----------------------------------------------------------------------------
public class Module
{
//----------------------------------------------------------------------------
// Module.attributes
//----------------------------------------------------------------------------
   String              name;        // The name of the Module
   Object[]            code;        // The module data

//----------------------------------------------------------------------------
//
// Method-
//       Module.Module
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Module(                          // Constructor
     String            name,        // The module name
     Object[]          code)        // The associated code
{
   this.name= name;
   this.code= code;
}

//----------------------------------------------------------------------------
//
// Method-
//       Module.getSize
//
// Purpose-
//       Extract the size of this Module.
//
//----------------------------------------------------------------------------
public int                          // The size of the Module
   getSize( )                       // Get the size of the module
{
   int                 size;        // The code size
   int                 i;

   size= 0;
   for(i=0; i<code.length; i++)
   {
     if( !(code[i] instanceof LoaderControl) )
     {
       if( code[i] instanceof Module )
         size += ((Module)code[i]).getSize();
       else
         size++;
     }
   }

   return size;
}

//----------------------------------------------------------------------------
//
// Method-
//       Module.getName
//
// Purpose-
//       Get the module name.
//
//----------------------------------------------------------------------------
public String                       // The module name
   getName( )                       // Get module name
{
   return name;
}
} // Class Module

