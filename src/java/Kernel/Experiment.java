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
//       Experiment.java
//
// Purpose-
//       Experimental code.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
import java.lang.*;
import java.util.*;
import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       Experiment
//
// Purpose-
//       Bringup.
//
//----------------------------------------------------------------------------
public class Experiment
{
//----------------------------------------------------------------------------
// Experiment.attributes
//----------------------------------------------------------------------------
   LcModule            inner;       // Inner module
   Module              code;        // Sample module
   Loader              loader;
   ExperimentStore     store;

//----------------------------------------------------------------------------
//
// Method-
//       Experiment.Experiment
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Experiment( )                    // Constructor
{
   inner= new LcModule("Inner", new Object[] {
new LcLabel("origin"),
         new LinkInstruction(0),
         new LcAbsolute("target"),
         new LinkInstruction(I.IA),
         new LcRelative("target"),
         Const.haltInstruction,
         Const.haltInstruction,
         Const.haltInstruction,
         Const.haltInstruction,
         new Addr(0),
         new LcAbsolute("origin"),
         new Addr(0),
         new LcAbsolute("target"),

new LcLabel("target"),
         Const.showInstruction,
         Const.backInstruction,
         new Addr(0xffffffff),
         new Addr(0xffffffff)
         });

   code= new Module("Experiment", new Object[] {
new LcLabel("origin"),
         new LcOrigin(0x00fe0000),
         new LinkInstruction(),
         new LcAbsolute("target"),
         new LinkInstruction(I.IA),
         new LcRelative("target"),
         new LinkInstruction(),
         new LcAbsolute(new String[] {"Inner", "target"}),
         new LinkInstruction(I.IA),
         new LcRelative(new String[] {"Inner", "target"}),
         Const.haltInstruction,
         Const.haltInstruction,
         Const.haltInstruction,
         Const.haltInstruction,
         new LcSpace(16),
         new Addr(0),
         new LcAbsolute("origin"),
         new Addr(0),
         new LcAbsolute(new String[] {"Inner", "target"}),

         inner,                     // Embedded module

new LcLabel("target"),
         Const.showInstruction,
         Const.backInstruction,

new LcAlign(Page.size),
new LcLabel("virtual"),
         new VirtModule()
         });

   store= new ExperimentStore();
   loader= new Loader(code);
   loader.load(store, new SymbolTable());

   store.debug();
}

//----------------------------------------------------------------------------
//
// Class-
//       ExperimentStore
//
// Purpose-
//       Bringup.
//
//----------------------------------------------------------------------------
class ExperimentStore implements StoreInterface
{
//----------------------------------------------------------------------------
// ExperimentStore:attributes
//----------------------------------------------------------------------------
   Object[]            data= new Object[8192];

//----------------------------------------------------------------------------
// ExperimentStore:method
//----------------------------------------------------------------------------
public void
   store(                           // Store a Word
     int               address,     // Address
     Object            word)        // Word to store
{
   data[address]= word;
}

//----------------------------------------------------------------------------
// ExperimentStore:debug
//----------------------------------------------------------------------------
public void
   debug( )                         // Debug the Module
{
   StringFormat        string;
   Object              o;
   int                 i;

   string= new StringFormat();
   string.setRadix(16);
   for(i= 0; i<data.length; i++)
   {
     o= data[i];
     if( o == null )
       continue;

     string.reset();
     string.append("[").append(i,4,4).append("]: ")
           .append(Debug.toString(o));

     Debug.printf(string);
   }
}
} // class ExperimentStore

//----------------------------------------------------------------------------
//
// Method-
//       Experiment.main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
   throws Exception
{
   try {
     Experiment main= new Experiment(); // Instantitaion object

   } catch(Exception e) {
     System.out.println("Main: Exception: " + e);
     e.printStackTrace();
   }
}
} // class Experiment

