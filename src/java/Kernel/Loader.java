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
//       Loader.java
//
// Purpose-
//       Module loader.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
import java.util.*;
import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       Loader
//
// Purpose-
//       Module loader.
//
//----------------------------------------------------------------------------
public class Loader
{
//----------------------------------------------------------------------------
// Loader.attributes
//----------------------------------------------------------------------------
static boolean         hcdm= false; // Hard-Core Debug Mode

   Module              module;      // The associated (base) Module
   int                 lOrigin;     // Logical origin
   int                 pOrigin;     // Physical origin
   int                 offset;      // The current offset

   SymbolTable         gTable;      // The global symbol table
   SymbolTable         xTable;      // The extended global symbol table
   SymbolTable         lTable;      // The local  symbol table
   StoreInterface      storeInterface; // The StoreInterface

//----------------------------------------------------------------------------
//
// Method-
//       Loader.Loader
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Loader(                          // Constructor
     Module            module)      // The Module to load
{
   this.module= module;
   this.lOrigin= 0;
   this.pOrigin= 0;
   offset=       0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Loader.debug
//
// Purpose-
//       Display the current loader item.
//
//----------------------------------------------------------------------------
public String                       // Debugging string
   debug(                           // Debug loader item
     Object            o)           // The item
{
   StringFormat        string= new StringFormat();

   string.setRadix(16);
   string.append("[").append(lOrigin,8,8).append("]")
         .append("[").append(pOrigin,8,8).append("]")
         .append("[").append(offset ,8,8).append("]")
         .append(": ").append(Debug.toString(o));

   return string.toString();
}

//----------------------------------------------------------------------------
//
// Method-
//       Loader.exception
//
// Purpose-
//       Throw an exception.
//
//----------------------------------------------------------------------------
public void
   exception(                       // Throw an Exception
     String            description) // Exception description
   throws Exception
{
   StringFormat        string= new StringFormat();

   string.setRadix(16);
   string.append("Loader: ")
         .append("Module(").append(module.getName()).append(") ")
         .append("Offset(").append(offset,8,8).append(") ");
   string.append(description);

   throw new Exception(string.toString());
}

//----------------------------------------------------------------------------
//
// Method-
//       Loader.pass1
//
// Purpose-
//       Look at the Module, adding Labels to the SymbolTable.
//
//----------------------------------------------------------------------------
public int                          // Resultant offset
   pass1( )                         // Load the Module
   throws Exception
{
   Loader              loader;      // Recursive loader
   Object              o;           // Working object

   int                 i;

   if( hcdm )
     Debug.debugf("PASS1: " + module.getName());

   lTable= new SymbolTable();

   // Define the origin physical entry point "0"
   lTable.insert("0", new Addr(lOrigin));

   // Load the module
   offset= 0;
   for(i=0; i<module.code.length; i++)
   {
     o= module.code[i];
     if( hcdm )
       Debug.debugf("pass1: " + debug(o));
     if( o instanceof LoaderControl )
     {
       if( o instanceof LcModule )
       {
         // Define a recursive Loader
         loader= new Loader((LcModule)o);

         // Preserve the origins
         loader.lOrigin= lOrigin+offset;
         loader.pOrigin= pOrigin+offset;

         // Preserve the lower level symbol tables
         loader.storeInterface= storeInterface;
         loader.gTable= gTable;
         loader.xTable= xTable;
         offset += loader.pass1();

         // Define a local label for the module
         lTable.insert(((LcModule)o).getName(), loader.lTable);
       }
       else if( o instanceof LcLabel )
         lTable.insert(((LcLabel)o).name, new Addr(lOrigin + offset));
       else if( o instanceof LcOrigin )
         ((LcOrigin)o).update(this);
     }
     else
       offset++;
   }

   // Return the cumulative Module size
   return offset;
}

//----------------------------------------------------------------------------
//
// Method-
//       Loader.pass2
//
// Purpose-
//       Load the Module, processing LoaderControls as they occur.
//
//----------------------------------------------------------------------------
public int                          // Resultant offset
   pass2( )                         // Load the Module
   throws Exception
{
   Loader              loader;      // Recursive loader
   Object              p;           // Prior object
   Object              o;           // Working object

   int                 i;

   if( hcdm )
     Debug.debugf("PASS2: " + module.getName());
   offset= 0;
   p= null;
   for(i=0; i<module.code.length; i++)
   {
     o= module.code[i];
     if( hcdm )
       Debug.debugf("pass2: " + debug(o));
     if( o instanceof LoaderControl )
     {
       if( o instanceof LcModule )
       {
         loader= new Loader((Module)o);

         // Preserve the origins
         loader.lOrigin= lOrigin+offset;
         loader.pOrigin= pOrigin+offset;

         // Preserve the lower level symbol tables
         loader.storeInterface= storeInterface;
         loader.gTable= gTable;
         loader.xTable= xTable;
         loader.lTable= (SymbolTable)lTable.locate(((Module)o).getName());

         // Load the inner Module
         offset += loader.pass2();
       }
       else if( o instanceof LcReference )
       {
         ((LcReference)o).install(this, p);
       }
       else if( o instanceof LcOrigin )
         ((LcOrigin)o).update(this);
     }
     else
     {
       p= o;
       storeInterface.store(pOrigin + offset, o);
       offset++;
     }
   }

   return offset;
}

//----------------------------------------------------------------------------
//
// Method-
//       Loader.load
//
// Purpose-
//       Load the Module.
//
//----------------------------------------------------------------------------
public int                          // The module size
   load(                            // Load the Module
     StoreInterface    storeInterface, // The StoreInterface Object
     SymbolTable       gTable)      // The global symbol table
{
   int                 size;
   int                 rc;

   this.storeInterface= storeInterface;
   this.gTable= gTable;
   xTable= new SymbolTable();

   size= 0;
   try {
     lOrigin= 0;
     pOrigin= 0;
     size= pass1();

     lOrigin= 0;
     pOrigin= 0;
     rc=   pass2();
     if( rc != size )
       exception("pass1: " + size + ", pass2: " + rc);

     // Add this Module to the global symbol table
     gTable.insert(module.getName(), lTable);

   } catch(Exception e) {
     e.printStackTrace();
     Cpu.checkStop("Loader.load(" + module.getName() + ") " + e);
   }

   displaySymbolTable(gTable);

   return size;
}

//----------------------------------------------------------------------------
//
// Method-
//       Loader.resolve
//
// Purpose-
//       Extract a symbol from the combined table.
//
//----------------------------------------------------------------------------
public int                          // Resultant address
   resolve(                         // Resolve symbol
     Object            name)        // The symbol name
   throws Exception
{
   String              s;           // Working String
   Object              o;           // Resultant Object
   int                 i;

   if( name instanceof String[] )   // If qualified name
   {
     o= lTable.locate((String[])name);
     if( o == null )
     {
       o= xTable.locate((String[])name);
       if( o == null )
       {
         o= gTable.locate((String[])name);
         if( o == null )
         {
           s= ((String[])name)[0];
           for(i= 1; i<((String[])name).length; i++)
             s= s + "::" + ((String[])name)[i];

           exception("Unresolved: " + s);
         }
       }
     }

     if( !(o instanceof Addr) )
       exception("Unexpected class: " +
                 o.getClass().getName() + "= resolve(" + name + ")");

     return ((Addr)o).getAddr();
   }

   if( name instanceof String )     // If normal name
   {
     o= lTable.locate((String)name);
     if( o == null )
       exception("Unresolved: " + name);

     if( !(o instanceof Addr) )
       exception("Unexpected class: " +
                 o.getClass().getName() + "= resolve(" + name + ")");

     return ((Addr)o).getAddr();
   }

   exception("Unexpected class: " + Cpu.toString(name));
   return (-1);
}

//----------------------------------------------------------------------------
//
// Method-
//       Loader.displaySymbol
//
// Purpose-
//       Display a symbol.
//
//----------------------------------------------------------------------------
public static void
   displaySymbol(                   // Display symbol
     String            name,        // The symbol's name
     Object            value)       // The symbol's value
{
   if( name.length() >= 32 )
   {
     Debug.debugf(name);
     name= "";
   }

   name= name + new String("                                ")
                    .substring(name.length());

   Debug.debugf(name + Debug.toString(value));
}

//----------------------------------------------------------------------------
//
// Method-
//       Loader.displaySymbolTable
//
// Purpose-
//       Display a symbol table.
//
//----------------------------------------------------------------------------
public static void
   displaySymbolTable(              // Display SymbolTable
     SymbolTable       table)       // The SymbolTable
{
   displaySymbolTable("", table);
}

public static void
   displaySymbolTable(              // Display SymbolTable
     String            prefix,      // Prefix qualifier
     SymbolTable       table)       // The SymbolTable
{
   Enumeration         e;
   Object              o;

   String              lastSymbol;
   String              thisSymbol;
   String              s;

   lastSymbol= null;
   for(;;)
   {
     thisSymbol= null;
     for(e= table.elements(); e.hasMoreElements(); )
     {
       s= (String)e.nextElement();
       if( lastSymbol == null
           || s.compareTo(lastSymbol) > 0 )
       {
         if( thisSymbol == null
             || s.compareTo(thisSymbol) < 0 )
           thisSymbol= s;
       }
     }

     if( thisSymbol == null )
       break;

     o= table.locate(thisSymbol);
     if( o instanceof SymbolTable )
       displaySymbolTable(prefix + thisSymbol + "::", (SymbolTable)o);
     else
       displaySymbol(prefix + thisSymbol, o);

     lastSymbol= thisSymbol;
  }
}
} // Class Loader

