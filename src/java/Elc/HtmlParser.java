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
//       HtmlParser.java
//
// Purpose-
//       HTML Parser.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.util.*;

import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       HtmlParser
//
// Purpose-
//       HTML Parser.
//
//----------------------------------------------------------------------------
class HtmlParser extends Debug implements Parser // HTML Parser
{
//----------------------------------------------------------------------------
// HtmlParser.Attributes
//----------------------------------------------------------------------------
ElementNode            root;        // Root node

final boolean          HCDM= false; // Hard Core Debug Mode?
final boolean          IODM= false; // I/O Debug Mode?
final boolean          SCDM= false; // Soft Core Debug Mode?

final String[]         autoTerm=    // Auto-complete table
{  "dt"
};

final String[]         empty=       // List of empty names
{  "area"
,  "base"
,  "basefont"
,  "br"
,  "col"
,  "frame"
,  "hr"
,  "img"
,  "input"
,  "isindex"
,  "link"
,  "meta"
,  "param"
}; // empty

final String[][]       entity=      // Entity definition table
{  {"AElig",    "&#198;"}
,  {"Aacute",   "&#193;"}
,  {"Acirc",    "&#194;"}
,  {"Agrave",   "&#192;"}
,  {"Alpha",    "&#913;"}
,  {"Aring",    "&#197;"}
,  {"Atilde",   "&#195;"}
,  {"Auml",     "&#196;"}
,  {"Beta",     "&#914;"}
,  {"Ccedil",   "&#199;"}
,  {"Chi",      "&#935;"}
,  {"Dagger",   "&#8225;"}
,  {"Delta",    "&#916;"}
,  {"ETH",      "&#208;"}
,  {"Eacute",   "&#201;"}
,  {"Ecirc",    "&#202;"}
,  {"Egrave",   "&#200;"}
,  {"Epsilon",  "&#917;"}
,  {"Eta",      "&#919;"}
,  {"Euml",     "&#203;"}
,  {"Gamma",    "&#915;"}
,  {"Iacute",   "&#205;"}
,  {"Icirc",    "&#206;"}
,  {"Igrave",   "&#204;"}
,  {"Iota",     "&#921;"}
,  {"Iuml",     "&#207;"}
,  {"Kappa",    "&#922;"}
,  {"Lambda",   "&#923;"}
,  {"Mu",       "&#924;"}
,  {"Ntilde",   "&#209;"}
,  {"Nu",       "&#925;"}
,  {"OElig",    "&#338;"}
,  {"Oacute",   "&#211;"}
,  {"Ocirc",    "&#212;"}
,  {"Ograve",   "&#210;"}
,  {"Omega",    "&#937;"}
,  {"Omicron",  "&#927;"}
,  {"Oslash",   "&#216;"}
,  {"Otilde",   "&#213;"}
,  {"Ouml",     "&#214;"}
,  {"Phi",      "&#934;"}
,  {"Pi",       "&#928;"}
,  {"Prime",    "&#8243;"}
,  {"Psi",      "&#936;"}
,  {"Rho",      "&#929;"}
,  {"Scaron",   "&#352;"}
,  {"Sigma",    "&#931;"}
,  {"THORN",    "&#222;"}
,  {"Tau",      "&#932;"}
,  {"Theta",    "&#920;"}
,  {"Uacute",   "&#218;"}
,  {"Ucirc",    "&#219;"}
,  {"Ugrave",   "&#217;"}
,  {"Upsilon",  "&#933;"}
,  {"Uuml",     "&#220;"}
,  {"Xi",       "&#926;"}
,  {"Yacute",   "&#221;"}
,  {"Yuml",     "&#376;"}
,  {"Zeta",     "&#918;"}
,  {"aacute",   "&#225;"}
,  {"acirc",    "&#226;"}
,  {"acute",    "&#180;"}
,  {"aelig",    "&#230;"}
,  {"agrave",   "&#224;"}
,  {"alefsym",  "&#8501;"}
,  {"alpha",    "&#945;"}
,  {"amp",      "&#38;"}
,  {"and",      "&#8743;"}
,  {"ang",      "&#8736;"}
,  {"aring",    "&#229;"}
,  {"asymp",    "&#8776;"}
,  {"atilde",   "&#227;"}
,  {"auml",     "&#228;"}
,  {"bdquo",    "&#8222;"}
,  {"beta",     "&#946;"}
,  {"brvbar",   "&#166;"}
,  {"bull",     "&#8226;"}
,  {"cap",      "&#8745;"}
,  {"ccedil",   "&#231;"}
,  {"cedil",    "&#184;"}
,  {"cent",     "&#162;"}
,  {"chi",      "&#967;"}
,  {"circ",     "&#710;"}
,  {"clubs",    "&#9827;"}
,  {"cong",     "&#8773;"}
,  {"copy",     "&#169;"}
,  {"crarr",    "&#8629;"}
,  {"cup",      "&#8746;"}
,  {"curren",   "&#164;"}
,  {"dArr",     "&#8659;"}
,  {"dagger",   "&#8224;"}
,  {"darr",     "&#8595;"}
,  {"deg",      "&#176;"}
,  {"delta",    "&#948;"}
,  {"diams",    "&#9830;"}
,  {"divide",   "&#247;"}
,  {"eacute",   "&#233;"}
,  {"ecirc",    "&#234;"}
,  {"egrave",   "&#232;"}
,  {"empty",    "&#8709;"}
,  {"emsp",     "&#8195;"}
,  {"ensp",     "&#8194;"}
,  {"epsilon",  "&#949;"}
,  {"equiv",    "&#8801;"}
,  {"eta",      "&#951;"}
,  {"eth",      "&#240;"}
,  {"euml",     "&#235;"}
,  {"euro",     "&#8364;"}
,  {"exist",    "&#8707;"}
,  {"fnof",     "&#402;"}
,  {"forall",   "&#8704;"}
,  {"frac12",   "&#189;"}
,  {"frac14",   "&#188;"}
,  {"frac34",   "&#190;"}
,  {"frasl",    "&#8260;"}
,  {"gamma",    "&#947;"}
,  {"ge",       "&#8805;"}
,  {"gt",       "&#62;"}
,  {"hArr",     "&#8660;"}
,  {"harr",     "&#8596;"}
,  {"hearts",   "&#9829;"}
,  {"hellip",   "&#8230;"}
,  {"iacute",   "&#237;"}
,  {"icirc",    "&#238;"}
,  {"iexcl",    "&#161;"}
,  {"igrave",   "&#236;"}
,  {"image",    "&#8465;"}
,  {"infin",    "&#8734;"}
,  {"int",      "&#8747;"}
,  {"iota",     "&#953;"}
,  {"iquest",   "&#191;"}
,  {"isin",     "&#8712;"}
,  {"iuml",     "&#239;"}
,  {"kappa",    "&#954;"}
,  {"lArr",     "&#8656;"}
,  {"lambda",   "&#955;"}
,  {"lang",     "&#9001;"}
,  {"laquo",    "&#171;"}
,  {"larr",     "&#8592;"}
,  {"lceil",    "&#8968;"}
,  {"ldquo",    "&#8220;"}
,  {"le",       "&#8804;"}
,  {"lfloor",   "&#8970;"}
,  {"lowast",   "&#8727;"}
,  {"loz",      "&#9674;"}
,  {"lrm",      "&#8206;"}
,  {"lsaquo",   "&#8249;"}
,  {"lsquo",    "&#8216;"}
,  {"lt",       "&#60;"}
,  {"macr",     "&#175;"}
,  {"mdash",    "&#8212;"}
,  {"micro",    "&#181;"}
,  {"middot",   "&#183;"}
,  {"minus",    "&#8722;"}
,  {"mu",       "&#956;"}
,  {"nabla",    "&#8711;"}
,  {"nbsp",     "&#160;"}
,  {"ndash",    "&#8211;"}
,  {"ne",       "&#8800;"}
,  {"ni",       "&#8715;"}
,  {"not",      "&#172;"}
,  {"notin",    "&#8713;"}
,  {"nsub",     "&#8836;"}
,  {"ntilde",   "&#241;"}
,  {"nu",       "&#957;"}
,  {"oacute",   "&#243;"}
,  {"ocirc",    "&#244;"}
,  {"oelig",    "&#339;"}
,  {"ograve",   "&#242;"}
,  {"oline",    "&#8254;"}
,  {"omega",    "&#969;"}
,  {"omicron",  "&#959;"}
,  {"oplus",    "&#8853;"}
,  {"or",       "&#8744;"}
,  {"ordf",     "&#170;"}
,  {"ordm",     "&#186;"}
,  {"oslash",   "&#248;"}
,  {"otilde",   "&#245;"}
,  {"otimes",   "&#8855;"}
,  {"ouml",     "&#246;"}
,  {"para",     "&#182;"}
,  {"part",     "&#8706;"}
,  {"permil",   "&#8240;"}
,  {"perp",     "&#8869;"}
,  {"phi",      "&#966;"}
,  {"pi",       "&#960;"}
,  {"piv",      "&#982;"}
,  {"plusmn",   "&#177;"}
,  {"pound",    "&#163;"}
,  {"prime",    "&#8242;"}
,  {"prod",     "&#8719;"}
,  {"prop",     "&#8733;"}
,  {"psi",      "&#968;"}
,  {"quot",     "&#34;"}
,  {"rArr",     "&#8658;"}
,  {"radic",    "&#8730;"}
,  {"rang",     "&#9002;"}
,  {"raquo",    "&#187;"}
,  {"rarr",     "&#8594;"}
,  {"rceil",    "&#8969;"}
,  {"rdquo",    "&#8221;"}
,  {"real",     "&#8476;"}
,  {"reg",      "&#174;"}
,  {"rfloor",   "&#8971;"}
,  {"rho",      "&#961;"}
,  {"rlm",      "&#8207;"}
,  {"rsaquo",   "&#8250;"}
,  {"rsquo",    "&#8217;"}
,  {"sbquo",    "&#8218;"}
,  {"scaron",   "&#353;"}
,  {"sdot",     "&#8901;"}
,  {"sect",     "&#167;"}
,  {"shy",      "&#173;"}
,  {"sigma" ,   "&#963;"}
,  {"sigmaf",   "&#962;"}
,  {"sim",      "&#8764;"}
,  {"spades",   "&#9824;"}
,  {"sub",      "&#8834;"}
,  {"sube",     "&#8838;"}
,  {"sum",      "&#8721;"}
,  {"sup",      "&#8835;"}
,  {"sup1",     "&#185;"}
,  {"sup2",     "&#178;"}
,  {"sup3",     "&#179;"}
,  {"supe",     "&#8839;"}
,  {"szlig",    "&#223;"}
,  {"tau",      "&#964;"}
,  {"there4",   "&#8756;"}
,  {"theta",    "&#952;"}
,  {"thetasym", "&#977;"}
,  {"thinsp",   "&#8201;"}
,  {"thorn",    "&#254;"}
,  {"tilde",    "&#732;"}
,  {"times",    "&#215;"}
,  {"trade",    "&#8482;"}
,  {"uArr",     "&#8657;"}
,  {"uacute",   "&#250;"}
,  {"uarr",     "&#8593;"}
,  {"ucirc",    "&#251;"}
,  {"ugrave",   "&#249;"}
,  {"uml",      "&#168;"}
,  {"upsih",    "&#978;"}
,  {"upsilon",  "&#965;"}
,  {"uuml",     "&#252;"}
,  {"weierp",   "&#8472;"}
,  {"xi",       "&#958;"}
,  {"yacute",   "&#253;"}
,  {"yen",      "&#165;"}
,  {"yuml",     "&#255;"}
,  {"zeta",     "&#950;"}
,  {"zwj",      "&#8205;"}
,  {"zwnj",     "&#8204;"}
};

//----------------------------------------------------------------------------
// HtmlParser::Constructors
//----------------------------------------------------------------------------
public
   HtmlParser( )                    // Default constructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlParser.debugTree
//
// Purpose-
//       Debug a subtree.
//
//----------------------------------------------------------------------------
static void
   debugTree(                       // Bringup
     int               depth,       // Current depth
     int               index,       // Current index
     Node              node)        // Root node
{
   if( node.getType() == Node.TYPE_TEXT)
     System.out.println("[" + depth + ":" + index + "] text(" + node.getData() + ")");
   else if( node.getType() == Node.TYPE_ATTR)
     System.out.println("[" + depth + ":" + index + "] attr(" + node.getName() + ") text(" + node.getData() + ")");
   else
     System.out.println("[" + depth + ":" + index + "] elem(" + node.getName() + ")");

   Node child= node.getChild();
   if( child != null )
   {
     debugTree(depth+1, 0, child);
     while( child != null )
     {
       if( node != child.getParent() )
         System.out.println("HtmlParser.debugTree: parent != node.getParent");

       child= child.getPeer();
     }
   }

   node= node.getPeer();
   if( node != null )
     debugTree(depth, index+1, node);
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlParser.debug
//
// Purpose-
//       Debug a subtree.
//
//----------------------------------------------------------------------------
public void
   debug( )                         // Write debugging messages
{
   debugTree(0, 0, root);
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlParser.error
//
// Purpose-
//       Error handler.
//
//----------------------------------------------------------------------------
static protected void
   error(                           // Error handler
     LineReader        reader,      // Current Reader
     String            text)        // Error text
{
   System.err.println("ERROR:"
                      + reader.getLine() + ":"
                      + reader.getColumn() + ": "
                      + text);
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlParser.exitComment
//
// Purpose-
//       Complete a '--' comment.
//
//----------------------------------------------------------------------------
static protected int                // Next character
   exitComment(                     // Complete a "--" comment
     LineReader        reader,      // Current Reader
     int               C)           // Current source
   throws Exception
{
   while( C >= 0 )
   {
     if( C == '-' )
     {
       C= reader.read();
       if( C == '-' )
       {
         C= reader.read();
         break;
       }
     }

     C= reader.read();
   }

   return C;
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlParser.getRoot
//
// Purpose-
//       Return the root Node
//
//----------------------------------------------------------------------------
public ElementNode                  // The root Node
   getRoot( )                       // Get root Node
{
   return root;
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlParser.isalpha
//
// Purpose-
//       Determine whether a character is alphabetic
//
//----------------------------------------------------------------------------
static protected boolean            // TRUE iff character is alphabetic
   isalpha(                         // Is character alphabetic?
     int               C)           // The character
{
   return Character.isLetter((char)C);
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlParser.isspace
//
// Purpose-
//       Determine whether a character is a space.
//
//----------------------------------------------------------------------------
static protected boolean            // TRUE iff character is white space
   isspace(                         // Is character white space?
     int               C)           // The character
{
   return Character.isWhitespace((char)C);
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlParser.skipspace
//
// Purpose-
//       Skip spaces in character array.
//
//----------------------------------------------------------------------------
static protected int                // Resultant index
   skipspace(                       // Skip spaces in character array
     char[]            array,       // The array
     int               x)           // The current index
{
   while( x < array.length && isspace(array[x]) )
     x++;

   return x;
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlParser.parse
//
// Purpose-
//       Parse HTML.
//
//----------------------------------------------------------------------------
public int                          // Return code (0 OK)
   parse(                           // Parse
     LineReader        reader)      // Using this Reader
   throws Exception
{
   int                 angle;       // Angle indentation count
   int                 C;           // Current source character
   StringBuffer        data;        // Attribute data
   StringBuffer        name;        // Statement name
   ElementNode         node;        // Current ELEMENT node
   int                 prior;       // Previous source character
   char[]              ptrC;        // Working char[]
   char[]              ptrN;        // name.toString().toCharArray()
   int                 quote;       // Quote delimiter
   StringBuffer        stmt;        // Statement collector

   int                 i;

   if( HCDM )
     System.out.println("HtmlParser.parse()");

   reset();
   angle= prior= quote=0;
   node= new ElementNode("<root>");
   root= node;
   C= reader.read();
   if( C < 0 )
   {
     if( HCDM )
       error(reader, "File empty");
     return 0;
   }
   stmt= new StringBuffer();
   prior= ' ';
   for(;;)
   {
     // Open text
     if( C < 0 )
       break;

     if( C == '\r' )
     {
       C= reader.read();
       continue;
     }

     //-----------------------------------------------------------------------
     // Remove duplicate spaces
     if( isspace(C) || C == 0 )
     {
       while( isspace(C) || C == 0 )
         C= reader.read();

       if( prior != ' ' )
       {
         prior= ' ';
         stmt.append(' ');
       }
       continue;
     }

     //-----------------------------------------------------------------------
     // Handle statement
     if( C == '<' )
     {
       // Extract statement
       boolean isSTMT= true;
       name= new StringBuffer();
       prior= C;
       C= reader.read();
       if( C > 0 && C != '>' )
       {
         if( !isalpha(C) && C != '/' && C != '!' )
           isSTMT= false;

         name.append((char)C);
         prior= C;
         C= reader.read();
       }
       while( C >= 0 && !isspace(C) && C != '/' && C != '>' )
       {
         name.append((char)C);
         prior= C;
         C= reader.read();

         if( name.length() == 3 )
         {
           if( name.toString().equals("!--") )
             break;
         }
       }

       ptrN= name.toString().toCharArray();
       for(i= 0; i < ptrN.length; i++)
         ptrN[i]= Character.toLowerCase(ptrN[i]);
       name= new StringBuffer();
       name.append(ptrN);
       if( node.getName().equals("script") )
       {
         if( !name.toString().equals("/script") )
           isSTMT= false;
       }
       if( isSTMT != true )
       {
         stmt.append('<');
         stmt.append(name);
         continue;
       }

       if( stmt.length() > 0 )
       {
         node.insert(new TextNode(stmt.toString().trim()));
         stmt= new StringBuffer();
       }
       while( C >= 0 && isspace(C) )
         C= reader.read();

       if( name.toString().equals("!--") )
         C= exitComment(reader, C);
       while( C >= 0 )
       {
         if( C == '>' )
           break;

         if( C == '\'' || C == '\"' )
         {
           quote= C;
           while( C >= 0 && C != quote )
           {
             stmt.append((char)C);
             C= reader.read();
           }
           stmt.append((char)quote);
           if( C < 0 )
             break;

           prior= quote;
           C= reader.read();
           continue;
         }

         if( C == '-' && ptrN[0] == '!' )
         {
           C= reader.read();
           if( C == '-' )
             C= exitComment(reader, C);
           else
           {
             prior= '-';
             stmt.append('-');
           }

           continue;
         }

         stmt.append((char)C);
         if( isspace(C) )
         {
           while( isspace(C) )
           {
             C= reader.read();
             if( C < 0 )
               break;
           }
           continue;
         }

         prior= C;
         C= reader.read();
       }

       // Process statement
       if( C < 0 )
       {
         if( HCDM )
           error(reader, "Unexpected EOF");
         break;
       }

       if( ptrN[0] == '/' )
       {
         String term= new String(ptrN, 1, ptrN.length - 1);
         ElementNode n= node;
         while( n != null )
         {
           if( n.getName().equals(term) )
             break;

           n= (ElementNode)n.getParent();
         }

         if( n != null )
           node= (ElementNode)n.getParent();
       }
       else if( ptrN[0] != '!' )
       {
         ElementNode child= new ElementNode(new String(ptrN));

         // Handle empty terms
         prior= 0;
         ptrC= stmt.toString().toCharArray();
         if( ptrC.length > 0 )
           prior= ptrC[ptrC.length-1];

         for(i= 0; i < empty.length; i++)
         {
           if( name.toString().equals(empty[i]) )
           {
             prior= '/';
             break;
           }
         }

         // Handle autoTerm
         for(i= 0; i < autoTerm.length; i++ )
         {
           if( name.toString().equals(autoTerm[i]) )
           {
             ElementNode n= node;
             while( n != null )
             {
               if( n.getName().equals(autoTerm[i]) )
               {
                 node= n;
                 break;
               }

               n= (ElementNode)n.getParent();
             }

             break;
           }
         }

         if( node.getName().equals(child.getName()) )
           node= (ElementNode)node.getParent();
         node.insert(child);
         if( prior != '/' )
           node= child;

         // Extract attributes
         int x= 0;
         name= null;
         data= null;
         for(;;)
         {
           x= skipspace(ptrC, x);
           if( x >= ptrC.length )
             break;

           if( !isalpha(ptrC[x]) )
           {
             if( HCDM )
               error(reader, "Invalid attribute name");
             break;
           }

           name= new StringBuffer();
           data= new StringBuffer();
           while( x < ptrC.length && ptrC[x] != '=' )
             name.append(Character.toLowerCase(ptrC[x++]));
           x++;

           if( x >= ptrC.length )
           {
             if( HCDM )
               error(reader, "Missing '=' in attributes");
             break;
           }

           quote= ' ';
           if( x < ptrC.length && ptrC[x] == '\"' || ptrC[x] == '\'')
             quote= ptrC[x++];

           while( x < ptrC.length && ptrC[x] != quote )
             data.append(ptrC[x++]);
           x++;

           child.insert(new AttributeNode(name.toString(), data.toString()));
         }
       }
       // <! statements are ignored>

       stmt= new StringBuffer();
       name= data= null;
       prior= ' ';
       C= reader.read();
       continue;
     }

     //-----------------------------------------------------------------------
     // Handle ENTITY reference
     if( C == '&' && !node.getName().equals("script") && !node.getName().equals("style") )
     {
       prior= C;
       C= reader.read();

       // Accumulate the ENTITY
       StringBuffer token= new StringBuffer();
       while( C >= 0 && isalpha(C) )
       {
         token.append((char)C);
         C= reader.read();
       }

       // Substitute the reference
       String string= token.toString();
       for(i= 0; i < entity.length; i++)
       {
         if( string.equals(entity[i][0]) )
           break;
       }

       if( i >= entity.length )
       {
         stmt.append('&');
         stmt.append(token);
       }
       else
       {
         stmt.append(entity[i][1]);
         if( C == ';' )
           C= reader.read();
       }

       continue;
     }

     //-----------------------------------------------------------------------
     // Statement text character
     stmt.append((char)C);
     prior= C;
     C= reader.read();
   }

   if( HCDM )
   {
     if( stmt.length() > 0 )
       error(reader, "Missing </name> at EOF");

     debugTree(0, 0, root);
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlParser.reset
//
// Purpose-
//       Reset the HtmlParser.
//
//----------------------------------------------------------------------------
public void
   reset( )                         // Reset the HtmlParser
{
   root= null;
}
}; // class HtmlParser

