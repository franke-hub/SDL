//----------------------------------------------------------------------------
//
//       Copyright (C) 2007-2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Bringup.java
//
// Purpose-
//       Bringup data gatherer.
//
// Last change date-
//       2013/01/01
//
// Parameter options-
//       --hcdm         Hard Core Debug Mode
//       --iodm         I/O Debug Mode
//       --scdm         Soft Core Debug Mode
//
//       --USE_LOADER= {OBJ || TXT}
//                      Test {object || text} word loader
//       --USE_OSTORE   Test ObjectStore
//       --USE_PARSER= { {DTD || HTM}, {PATH || FULL}, {STOP}, {DEBUG} }
//                      Test {DTD || HTM} Parser, {PATH || FULL} reader
//                      {STOP} on error, {DEBUG} parser
//       --USE_SCHED    Test Scheduler
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.util.*;

import user.util.*;
import user.util.logging.*;

//----------------------------------------------------------------------------
//
// Class-
//       Bringup
//
// Purpose-
//       Bringup tests.
//
//----------------------------------------------------------------------------
class Bringup extends Debug {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
boolean                HCDM= false; // Hard Core Debug Mode?
boolean                IODM= false; // I/O Debug Mode?
boolean                SCDM= false; // Soft Core Debug Mode?

String                 CHECKWORD= "additionalsysinfo"; // Debugging check word
boolean                USE_LOADER=     false;
boolean                USE_LOADER_OBJ= false;
boolean                USE_LOADER_TXT= false;
boolean                USE_OSTORE=     false;
boolean                USE_PARSER=     false;
boolean                USE_PARSE_DTD=  false;
boolean                USE_PARSE_HTM=  false;
boolean                USE_PARSE_FULL= false;
boolean                USE_PARSE_PATH= false;
boolean                USE_PARSE_INFO= false;
boolean                USE_PARSE_STOP= false;
boolean                USE_SCHED=      false;

static final int       OSTORE_LENGTH= 4096;
static final String[]  dtdList= {"dtd"};
static final String[]  htmList= {"htm", "html"};
static final String[]  ignore=      // List of ignored nodes
{  "div"
,  "span"
,  "style"
,  "tt"
};

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
Vector<String>         pathVector= new Vector<String>();
String[]               typeList= htmList; // File types {dtdList | htmList}
StreamLogger           logger= Common.get().logger;

//----------------------------------------------------------------------------
//
// Class-
//       TextVisitor
//
// Purpose-
//       Visit text files, extracting words.
//
//----------------------------------------------------------------------------
class TextVisitor extends NodeVisitor
{
//----------------------------------------------------------------------------
// TextVisitor::Attributes
//----------------------------------------------------------------------------
String                 string;      // Node string
int                    index;       // String index
char[]                 word= new char[256];   // Word extractor

//----------------------------------------------------------------------------
//
// Method-
//       TextVisitor.visit
//
// Purpose-
//       Extract words from text.
//
//----------------------------------------------------------------------------
public int                          // Return code (0 OK)
   visit(                           // Visit text Nodes
     Node              node)        // -> Node
{
   int                 i;

   if( HCDM )
   {
     if( node.getType() == Node.TYPE_TEXT)
       System.out.println("Visit text(" + node.getData() + ")");
     else if( node.getType() == Node.TYPE_ATTR)
       System.out.println("Visit attr(" + node.getName() + ") data(" + node.getData() + ")");
     else
       System.out.println("Visit elem(" + node.getName() + ")");
   }

   if( node.getType() != Node.TYPE_TEXT )
     return 0;

   String parent= node.getParent().getName();
   for(i= 0; i < ignore.length; i++)
   {
     if( parent.equals(ignore[i]) )
     {
       if( CHECKWORD != null && word.equals(CHECKWORD) )
         System.out.println("skip(" + CHECKWORD + ") parent(" + parent + ")");
       return 0;
     }
   }
   if( CHECKWORD != null && word.equals(CHECKWORD) )
     System.out.println("word(" + CHECKWORD + ") parent(" + parent + ")");

   index= 0;
   string= node.getData();
   for(;;)
   {
     String word= nextWord();
     if( word == null )
       break;

     if( HCDM )
       System.out.println(".word(" + word + ")");

     Word.insert(word);
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       TextVisitor.nextWord
//
// Purpose-
//       Get next word.
//
//----------------------------------------------------------------------------
String                              // The next word
   nextWord( )                      // Get next word
{
   int                 C;           // Current character
   int                 x;           // Word index

   x= 0;
   while( index < string.length() )
   {
     C= string.charAt(index++);
     if( Character.isLetter(C) && C < 127 )
     {
       if( x < word.length )
         word[x++]= Character.toLowerCase((char)C);
     }
     else if( C == ' ' )
     {
       if( x > 0 && x < 64 )
         break;

       x= 0;
     }
     else if( C == '\'' )
     {
       if( x > 0 && x < word.length && word[x-1] != '\'' )
         word[x++]= (char)C;
       else
       {
         while( index < string.length() && !Character.isLetter(string.charAt(index)) )
           index++;

         x= 0;
       }
     }
     else if( C != '@' && C != '.' && C != '-' )
     {
       if( x > 0 && x < 64 )
         break;

       x= 0;
     }
     else if( index >= string.length() || !Character.isLetter(string.charAt(index)) )
     {
       if( x > 0 && x < 64 )
         break;

       x= 0;
     }
     else
     {
       if( x > 0 && x < word.length )
         word[x++]= (char)C;
     }
   }

   String result= null;
   if( x > 0 && x < 64 )
     result= new String(word, 0, x);

   return result;
}
} // class TextVisitor

//----------------------------------------------------------------------------
//
// Method-
//       Bringup.fileReader
//
// Purpose-
//       Read local file.
//
//----------------------------------------------------------------------------
void
   fileReader(                      // Read local HTML file
     String            fileName)    // File name
   throws Exception
{
   if( HCDM )
     printf("fileReader(" + fileName + ")\n");

   File                file= new File(fileName);
   LineReader          reader;

   DtdParser           dtdParser= null;
   HtmlParser          htmParser= null;
   Parser              parser;

   int                 rc;

   if( !file.exists() )
     return;

   reader= new LineReader(fileName);
   if( USE_PARSE_DTD )
     parser= dtdParser= new DtdParser();
   else
     parser= htmParser= new HtmlParser();

   rc= parser.parse(reader);
   if( USE_PARSE_INFO )
     parser.debug();

   if( rc != 0 )
   {
     if( USE_PARSE_STOP )
     {
       System.out.println(rc + "= parser.parse(" + fileName + ")\n");
       System.exit(1);
     }
   }
   System.out.println(rc + "= parser.parse(" + fileName + ")\n");

   if( USE_PARSE_HTM && rc == 0 )
     htmParser.getRoot().visit(new TextVisitor());
}

//----------------------------------------------------------------------------
//
// Method-
//       Bringup.fullReader
//
// Purpose-
//       Read files.
//
//----------------------------------------------------------------------------
void
   fullReader( )                    // Read local files
   throws Exception
{
   BufferedReader      reader= new BufferedReader(new FileReader("data/names.all"));

   for(;;)
   {
     String s= reader.readLine();
     if( s == null )
       break;

     try {
       FileInfo file= new FileInfo(s);
       String ext= file.getExtension();
       for(int i= 0; i<typeList.length; i++)
       {
         if( ext.equalsIgnoreCase(typeList[i]) )
         {
           fileReader(s);
           break;
         }
       }
     } catch(Exception X) {
       debugException(X);
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Bringup.pathReader
//
// Purpose-
//       Read directory.
//
//----------------------------------------------------------------------------
void
   pathReader(                      // Read files
     String            pathName)    // From this directory
   throws Exception
{
   if( HCDM )
     printf("pathReader(" + pathName + ")\n");

   String[]            list= new File(pathName).list();
   int                 i;

   if( list == null )
   {
     System.err.println("pathReader(" + pathName + ") NOT A DIRECTORY");
     return;
   }

   for(i= 0; i<list.length; i++)
   {
     FileInfo file= new FileInfo(pathName, list[i]);
     if( file.exists() )
     {
       if( file.isFile() )
       {
         String ext= file.getExtension();
         for(int j= 0; j<typeList.length; j++)
         {
           if( ext.equalsIgnoreCase(typeList[j]) )
           {
             fileReader(file.getCanonicalPath());
             break;
           }
         }
       }

       else if( file.isDirectory()
                && !list[i].equals(".") && !list[i].equals("..") )
         pathReader(file.getCanonicalPath());

       else if( HCDM )
         printf("IGNORED(" + list[i] + ")\n");
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Bringup.wordList
//
// Purpose-
//       Display the word list.
//
//----------------------------------------------------------------------------
void
   wordList( )                      // Display the word list
   throws Exception
{
   Word.display();

   ObjectOutputStream oos= new ObjectOutputStream(new FileOutputStream("word.obj"));
   Word.store(oos);
   oos.close();

   FileWriter writer= new FileWriter("word.txt");
   Word.store(writer);
   writer.close();
}

//----------------------------------------------------------------------------
//
// Method-
//       Bringup.testLoader
//
// Purpose-
//       Test the word loader.
//
//----------------------------------------------------------------------------
public void
   testLoader( )                    // Load the word list.
   throws Exception
{
   if( USE_LOADER_OBJ )
   {
     System.out.println("Bringup --USE_LOADER=OBJ");
     ObjectInputStream ois= new ObjectInputStream(new FileInputStream("word.obj"));
     Word.load(ois);
     ois.close();
   }
   else
   {
     System.out.println("Bringup --USE_LOADER=TXT");
     FileReader reader= new FileReader("word.txt");
     Word.load(reader);
     reader.close();
   }

   Word.check();
   Word.display();
}

//----------------------------------------------------------------------------
//
// Method-
//       Bringup.testParser
//
// Purpose-
//       Test a Parser.
//
//----------------------------------------------------------------------------
public void
   testParser( )                    // Test a Parser
   throws Exception
{
   if( USE_PARSE_FULL )
   {
     System.out.println("Bringup --USE_PARSER=FULL");

     fullReader();
     wordList();
   }

   if( USE_PARSE_PATH )
   {
     System.out.println("Bringup --USE_PARSER=PATH");

     for(int i= 0; i<pathVector.size(); i++)
     {
       System.out.println("PATH(" + pathVector.get(i).toString() + ")");
       pathReader(pathVector.get(i).toString());
     }

     System.out.println("");
     wordList();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Bringup.testObjectStore
//
// Purpose-
//       Test the ObjectStore.
//
//----------------------------------------------------------------------------
void
   testObjectStore()                // Test the ObjectStore
   throws Exception
{
   final long          baseIdent= 0xffff0000fe000001L;
   final int           length= OSTORE_LENGTH;

   ObjectStore         os= Common.get().db;
   Random              random= new Random(0x0123456789abcdefL);

   System.out.println("Bringup --USE_OSTORE");

   Long[] identArray= (Long[])os.fetchRD(baseIdent);
   if( identArray == null )
   {
     logger.log("Generating ObjectStore test data[" + length + "]...");
     identArray= new Long[length];
     for(int i= 0; i<identArray.length; i++)
     {
       identArray[i]= random.nextLong() | 0xffff000000000000L;
       os.change(identArray[i], new Long(identArray[i]));
     }

     os.change(baseIdent, identArray);
     logger.log("...ObjectStore test data generated");
     return;
   }

   os.store(baseIdent);
   logger.log("Validating ObjectStore test data[" + length + "]...");
   Debug.verify( identArray.length == length );
   for(int i= 0; i<identArray.length; i++)
   {
     Long ident= (Long)os.fetchRD(identArray[i]);
     Debug.verify( identArray[i] == ident.longValue() );
   }

   for(int i= identArray.length-1; i>=0; i--)
   {
     Long ident= (Long)os.fetchRD(identArray[i]);
     Debug.verify( identArray[i] == ident.longValue() );
     os.store(identArray[i]);
     os.store(identArray[i]);
   }
   logger.log("...ObjectStore test data validated");
}

//----------------------------------------------------------------------------
//
// Method-
//       Bringup.testScheduler
//
// Purpose-
//       Test the Scheduler.
//
//----------------------------------------------------------------------------
void
   testScheduler()                  // Test the Scheduler
   throws Exception
{
   class MyWorkItem implements WorkItem {
     long              value;

     MyWorkItem(
       long            value)
     {
       this.value= value;
     }

     public String
       toString( )
     {
       return "WorkItem(" + value + ")";
     }
   }; // class MyWorkItem

   class MyWorkList extends WorkList {
   //-------------------------------------------------------------------------
   //
   // Method-
   //    MyWorkList.next
   //
   // Purpose-
   //    Handle a WorkItem
   //
   //-------------------------------------------------------------------------
   protected void
      next(                         // Process a WorkItem
         WorkItem      item)        // The item
      {
         Common.get().logger.log("MyWorkList(" + toString() +
                                 ").next(" + item.toString() + ")");
      }
   }; // class MyWorkList

   System.out.println("Bringup --USE_SCHED");

   MyWorkList          list1= new MyWorkList();
   MyWorkList          list2= new MyWorkList();

   for(int i= 1; i<=100; i++)
   {
     MyWorkItem item1= new MyWorkItem(+i);
     MyWorkItem item2= new MyWorkItem(-i);

     list1.todo(item1);
     list2.todo(item2);
     logger.log("Item[" + i + "] inserted");
   }

   Scheduler.get().close();
}

//----------------------------------------------------------------------------
//
// Method-
//       Bringup.usage
//
// Purpose-
//       Display usage information.
//
//----------------------------------------------------------------------------
public void
   usage( )                         // Display usage information
{
   System.out.print("Bringup.usage()\n"
       + "--help         This help message\n"
       + "--hcdm         Hard Core Debug Mode\n"
       + "--iodm         I/O Debug Mode\n"
       + "--scdm         Soft Core Debug Mode\n"

       + "--USE_LOADER= {OBJ || TXT}\n"
       + "               Test {object || text} word loader\n"
       + "--USE_OSTORE   Test ObjectStore\n"
       + "--USE_PARSER= { {DTD || HTM}, {PATH || FULL}, {STOP}, {DEBUG} }\n"
       + "               Test {DTD || HTM} Parser, {PATH || FULL} reader\n"
       + "               {STOP} on error, {DEBUG} parser\n"
       + "--USE_SCHED    Test Scheduler\n"
       );
}

//----------------------------------------------------------------------------
//
// Method-
//       Bringup.initLoader
//
// Purpose-
//       Initialize USE_LOADER options.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff valid
   initLoader(                      // Initialize USE_LOADER options
     String            arg)         // USE_LOADER option
   throws Exception
{
   if( USE_LOADER )
   {
     System.err.println("Multiple --USE_LOADER options");
     return false;
   }

   String options= "";
   if( arg.length() > 12 )
   {
     if( arg.charAt(12) != '=' )
     {
       System.err.println("Invalid option: '" + arg + "'");
       return false;
     }

     String thisOpt= arg.substring(13);
     if( thisOpt.equalsIgnoreCase("OBJ") )
       USE_LOADER_OBJ= true;
     else if( thisOpt.equalsIgnoreCase("TXT") )
       USE_LOADER_TXT= true;
     else
     {
       System.err.println("Invalid --USE_LOADER options");
       return false;
     }
   }

   if( !USE_LOADER_OBJ && !USE_LOADER_TXT )
     USE_LOADER_OBJ= true;

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       Bringup.initParser
//
// Purpose-
//       Initialize USE_PARSER options.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff valid
   initParser(                      // Initialize USE_PARSER options
     String            arg)         // USE_PARSER option
   throws Exception
{
   if( USE_PARSER )
   {
     System.err.println("Multiple --USE_PARSER options");
     return false;
   }

   if( arg.length() > 12 )
   {
     if( arg.charAt(12) != '=' )
     {
       System.err.println("Invalid option: '" + arg + "'");
       return false;
     }

     String options= arg.substring(13);
     while( !options.equals("") )
     {
       String thisOpt;
       if( options.indexOf(',') >= 0 )
       {
         thisOpt= options.substring(0, options.indexOf(','));
         options= options.substring(options.indexOf(',') + 1);
       }
       else
       {
         thisOpt= options;
         options= "";
       }

       if( thisOpt.equalsIgnoreCase("DTD") )
       {
         USE_PARSE_DTD= true;
         typeList= dtdList;
       }
       else if( thisOpt.equalsIgnoreCase("HTM")
                  ||thisOpt.equalsIgnoreCase("HTML") )
       {
         USE_PARSE_HTM= true;
         typeList= htmList;
       }
       else if( thisOpt.equalsIgnoreCase("PATH") )
         USE_PARSE_PATH= true;
       else if( thisOpt.equalsIgnoreCase("FULL") )
         USE_PARSE_FULL= true;
       else if( thisOpt.equalsIgnoreCase("STOP") )
         USE_PARSE_STOP= true;
       else if( thisOpt.equalsIgnoreCase("DEBUG") )
         USE_PARSE_INFO= true;
       else
       {
         System.err.println("Invalid --USE_PARSER option");
         return false;
       }
     }
   }

   if( USE_PARSE_HTM && USE_PARSE_DTD )
   {
     System.err.println("Cannot select both DTD and HTM parser");
     return false;
   }
   if( !USE_PARSE_HTM && !USE_PARSE_DTD )
   {
     typeList= htmList;
     USE_PARSE_HTM= true;
   }

   if( USE_PARSE_PATH && USE_PARSE_FULL )
   {
     System.err.println("Cannot select both PATH and FULL reader");
     return false;
   }
   if( !USE_PARSE_PATH && !USE_PARSE_FULL )
     USE_PARSE_PATH= true;

   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       Bringup.init
//
// Purpose-
//       Initialize parameters.
//
//----------------------------------------------------------------------------
public boolean                      // TRUE iff initialized
   init(                            // Initialize parameters
     String[]          args)        // Argument array
   throws Exception
{
   boolean             result= true;// No errors yet

   for(int i= 0; i<args.length; i++)
   {
     if( args[i].equalsIgnoreCase("--help")
         || args[i].equalsIgnoreCase("--h") )
       result= false;
     else if( args[i].equalsIgnoreCase("--hcdm") )
       HCDM= true;
     else if( args[i].equalsIgnoreCase("--iodm") )
       IODM= true;
     else if( args[i].equalsIgnoreCase("--scdm") )
       SCDM= true;
     else if( args[i].equalsIgnoreCase("--USE_OSTORE") )
       USE_OSTORE= true;
     else if( args[i].equalsIgnoreCase("--USE_SCHED") )
       USE_SCHED= true;
     else if( args[i].length() >= 12 )
     {
       if( args[i].substring(0,12).equalsIgnoreCase("--USE_LOADER") )
       {
         USE_LOADER= initLoader(args[i]);
         if( !USE_LOADER )
           result= false;
       }
       else if( args[i].substring(0,12).equalsIgnoreCase("--USE_PARSER") )
       {
         USE_PARSER= initParser(args[i]);
         if( !USE_PARSER )
           result= false;
       }
     }
     else if( args[i].length() > 0 && args[i].charAt(0) != '-' )
       pathVector.add(args[i]);
     else
       throw new Exception("Invalid argument(" + args[i] + ")");
   }

   if( USE_PARSER && USE_PARSE_PATH )
   {
     if( pathVector.size() == 0 )
       pathVector.add(new String("."));
   }
   else
   {
     if( pathVector.size() != 0 )
     {
       result= false;
       System.err.println("Unexpected path arguments");
     }
   }

   if( !USE_LOADER && !USE_OSTORE && !USE_PARSER && !USE_SCHED )
   {
     result= false;
     System.err.println("No function selected");
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Bringup.bringup
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
public void
   bringup(                         // Mainline code
     String[]          args)        // Argument array
   throws Exception
{
   if( !init(args) )
   {
     usage();
     return;
   }

   if( USE_LOADER )
     testLoader();

   if( USE_OSTORE )
     testObjectStore();

   if( USE_PARSER )
     testParser();

   if( USE_SCHED )
     testScheduler();
}

//----------------------------------------------------------------------------
//
// Method-
//       Bringup.main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
{
   Bringup bringup= new Bringup();
   Common.get();

   try {
     bringup.bringup(args);
   } catch(Exception X) {
     debugException(X);
   }

   Common.get().close();
   System.gc();
   try {
     Thread.sleep(1500);
   } catch(Throwable t) {
   }
}
} // class Bringup

