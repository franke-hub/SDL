//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HtmlParser.cpp
//
// Purpose-
//       HtmlParser implementation methods.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <com/DataSource.h>
#include <com/istring.h>
#include <com/SafeParser.h>

#include "Common.h"
#include "HtmlNode.h"
#include "TextBuffer.h"

#include "HtmlParser.h"

using std::string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Struct-
//       NvPair
//
// Purpose-
//       NvPair <name,value> correspondence entry
//
//----------------------------------------------------------------------------
struct NvPair {                     // <Name,Value> entry
   const char*         name;        // Name
   const char*         data;        // Substitution data
}; // struct NvPair

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     autoTerm[]=  // Auto-complete table
{  "dt"
,  NULL
};

static const char*     empty[]=     // List of empty names
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
,  NULL
}; // empty

static NvPair          entity[]=    // Entity definition table
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
,  {NULL, NULL}
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugTree
//
// Purpose-
//       Debug a subtree.
//
//----------------------------------------------------------------------------
static inline void
   debugTree(                       // Bringup
     int               depth,       // Current depth
     int               index,       // Current index
     HtmlNode*         node)        // Root node
{
   if( node->getType() == HtmlNode::TYPE_TEXT)
     printf("[%4d:%4d] text(%s)\n", depth, index, node->getData().c_str());
   else if( node->getType() == HtmlNode::TYPE_ATTR)
     printf("[%4d:%4d] attr(%s) text(%s)\n", depth, index,
            node->getName().c_str(), node->getData().c_str());
   else
     printf("[%4d:%4d] elem(%s) data(%s)\n", depth, index,
            node->getName().c_str(), node->getData().c_str());

   HtmlNode* child= node->getChild();
   if( child != NULL )
   {
     debugTree(depth+1, 0, child);
     while( child != NULL )
     {
       if( node != child->getParent() )
         printf("%4d: parent(%p), but %p= child(%p).getParent()\n", __LINE__,
                node, child->getParent(), child);

       child= child->getPeer();
     }
   }

   node= node->getPeer();
   if( node != NULL )
     debugTree(depth, index+1, node);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       error
//
// Purpose-
//       Error handler.
//
//----------------------------------------------------------------------------
static inline void
   error(                           // Error handler
     const DataSource& data,        // Current DataSource
     const char*       text)        // Error text
{
   fprintf(stderr, "ERROR:%ld:%d:%s:%s\n", data.getLine(),
           data.getColumn(), text, data.getName().c_str());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       exitComment
//
// Purpose-
//       Complete a '--' comment.
//
//----------------------------------------------------------------------------
static inline int                   // Next character
   exitComment(                     // Complete a "--" comment
     DataSource&       data,        // Current DataSource
     int               C)           // Current source
{
   while( C >= 0 )
   {
     if( C == '-' )
     {
       C= data.get();
       if( C == '-' )
       {
         C= data.get();
         break;
       }
     }

     C= data.get();
   }

   return C;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parseAttributes
//
// Purpose-
//       Parse attributes.
//
//----------------------------------------------------------------------------
static void
   parseAttributes(                 // Parse the attribute string
     ElemHtmlNode*     elem,        // For this ElemHtmlNode
     Parser&           parser)      // And this statement source
{
   int                 C;           // Current character
   int                 L;           // Current length
   const char*         S;           // Working string pointer (in parser)
   int                 quote;       // Current quote character
   std::string         name;        // Current name
   std::string         value;       // Current value

   for(;;)                          // Extract name/value pairs
   {
     S= parser.skipSpace();         // Skip initial whitespace
     C= *S;
     for(L= 0;; L++) // Locate delimiter
     {
       if( C == '=' ||  C == ':' || C == ' ' || C == '\0' ) // If delimiter
         break;

       if( C == '\'' || C == '\"' ) // If begin quote
       {
         if( L != 0 )               // If malformed name
           return;

         quote= C;                  // Save delimiter
         S++;                       // Skip over quote
         C= parser.next();
         while( C != quote && C != '\0' )
         {
           C= parser.next();
           L++;
         }

         if( C != quote )           // If end of string
           return;

         C= parser.next();
         if( C != '=' && C != ':' && C != ' ' ) // If invalid delimiter
           return;                  // Ignore remainder

         break;
       }

       C= parser.next();            // The next character
     }

     // Validity checks
     if( C == '\0' )                // End of string
       return;

     if( L == 0 )                   // If form "=..." or ":..."
       return;                      // Ignore remainder

     name= std::string(S,L);        // Valid name found

     if( C == ' ' )                 // If space delimiter
     {
       S= parser.findSpace();       // Get value or delimiter
       C= *S;
     }

     if( C == '=' || C == ':' )     // If delimiter located
       C= parser.next();            // Skip the delimiter

     S= parser.skipSpace();         // Skip whitespace
     C= *S;

     if( C == '\0' )                // If form "name " without value
       return;

     for(L= 0;; L++) // Locate delimiter
     {
       if( C == '=' ||  C == ':' || C == ' ' || C == '\0' ) // If delimiter
         break;

       if( C == '\'' || C == '\"' ) // If begin quote
       {
         if( L != 0 )               // If malformed value
           return;

         quote= C;                  // Save delimiter
         S++;                       // Skip over quote
         C= parser.next();
         while( C != quote && C != '\0' )
         {
           C= parser.next();
           L++;
         }

         if( C != quote )           // If end of string (missing quote)
           return;

         C= parser.next();
         break;
       }

       C= parser.next();            // The next character
     }

     if( C != ' ' && C != '\0' )    // If invalid delimiter
       return;                      // Ignore remainder

     value= std::string(S,L);       // Valid value found

     HtmlNode* child= new AttrHtmlNode(name, value);
     elem->insertChild(child);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlParser::~HtmlParser
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   HtmlParser::~HtmlParser( void )  // Destructor
{
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlParser::HtmlParser
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   HtmlParser::HtmlParser( void )   // Constructor
:  root(NULL)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlParser::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   HtmlParser::debug( void ) const  // Debugging display
{
   fprintf(stderr, "HtmlParser(%p)::debug()\n", this);
   fprintf(stderr, ".. root(%p)\n", root);

   if( root != NULL )
     debugTree(0, 0, root);
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlParser::parse
//
// Purpose-
//       Parse from DataSource
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   HtmlParser::parse(               // Parse
     DataSource&       data)        // Using this DataSource
{
   int                 C;           // Current source character
   TextBuffer          name;        // Statement name
   ElemHtmlNode*       node;        // Current ELEMENT HtmlNode
   char*               ptrC;        // Working char*
   char*               ptrN;        // name.toChar()
   int                 prior;       // Previous source character
   int                 quote;       // Quote delimiter
   TextBuffer          stmt;        // Statement collector

   int                 i;

   IFHCDM( printf("  PARSE(%s)\n", data.getName().c_str()); )

   reset();
   prior= quote=0;
   C= data.get();
   if( C < 0 )
   {
     IFHCDM( error(data, "File empty"); )
     return (-1);
   }
   node= new ElemHtmlNode("<root>");
   root= node;
   prior= ' ';
   for(;;)
   {
     // Open text
     if( C < 0 )
       break;

     while( C == '\r' )
       C= data.get();

     //-----------------------------------------------------------------------
     // Remove duplicate spaces
     if( isspace(C) || C == '\0' )
     {
       while( isspace(C) )
         C= data.get();

       if( prior != ' ' )
       {
         prior= ' ';
         stmt.put(' ');
       }
       continue;
     }

     //-----------------------------------------------------------------------
     // Handle statement
     if( C == '<' )
     {
       // Extract statement
       name.reset();
       prior= C;
       C= data.get();
       if( C > 0 && C != '>' )
       {
         name.put(C);
         prior= C;
         C= data.get();
       }
       while( C >= 0 && !isspace(C) && C != '/' && C != '>' )
       {
         name.put(C);
         prior= C;
         C= data.get();

         if( name.size() == 3
             && name[0] == '!' && name[1] == '-' && name[2] == '-' )
           break;
       }
       ptrN= name.toChar();

       int isSTMT= TRUE;
       if( !isalpha(*ptrN) && *ptrN != '/' && *ptrN != '!' )
         isSTMT= FALSE;
       else if( node->getName() == "script" && stricmp(ptrN, "/script") != 0 )
         isSTMT= FALSE;
       if( isSTMT != TRUE )
       {
         stmt.put('<');
         stmt.put(name);
         continue;
       }

       if( stmt.size() > 0 )
       {
         SafeParser parser(stmt.toChar());
         const char* constC= parser.trim();
         node->insertChild(new TextHtmlNode(constC));
         stmt.reset();
       }
       while( C >= 0 && isspace(C) )
         C= data.get();

       if( strcmp(ptrN, "!--") == 0 )
         C= exitComment(data, C);
       while( C >= 0 )
       {
         if( C >= 0 && isspace(C) )
         {
           while( C >= 0 && isspace(C) )
             C= data.get();

           if( prior != ' ' )
             stmt.put(' ');

           prior= ' ';
           continue;
         }

         if( C == '>' )
           break;

         if( C == '\'' || C == '\"' )
         {
           quote= C;
           do {
             stmt.put(C);
             C= data.get();
           } while( C >= 0 && C != quote );
           stmt.put(quote);
           if( C < 0 )
             break;

           prior= quote;
           C= data.get();
           continue;
         }

         if( C == '-' && ptrN[0] == '!' )
         {
           C= data.get();
           if( C == '-' )
             C= exitComment(data, C);
           else
           {
             prior= '-';
             stmt.put('-');
           }

           continue;
         }

         stmt.put(C);
         prior= C;
         C= data.get();
       }

       // Process statement
       if( C < 0 )
       {
         IFHCDM( error(data, "Unexpected EOF"); )
         break;
       }

       for(ptrC= ptrN; *ptrC != '\0'; ptrC++)
         *ptrC= tolower(*ptrC);

       if( *ptrN == '/' )
       {
         ptrN++;
         ElemHtmlNode* n= node;
         while( n != NULL )
         {
           if( n->getName() == ptrN )
             break;

           n= (ElemHtmlNode*)n->getParent();
         }

         if( n != NULL )
           node= (ElemHtmlNode*)n->getParent();
       }
       else if( *ptrN != '!' )
       {
         ElemHtmlNode* child= new ElemHtmlNode(ptrN);

         // Extract attributes
         SafeParser parser(stmt.toChar());
         parseAttributes(child, parser);

         // Insert child node
         ptrC= stmt.toChar();
         prior= 0;
         while( *ptrC != '\0' )
         {
           prior= *ptrC;
           ptrC++;
         }

         for(i= 0; empty[i] != NULL; i++)
         {
           if( strcmp(ptrN, empty[i]) == 0 )
           {
             prior= '/';
             break;
           }
         }

         for(i= 0; autoTerm[i] != NULL; i++ )
         {
           if( strcmp(ptrN, autoTerm[i]) == 0 )
           {
             ElemHtmlNode* n= node;
             while( n != NULL )
             {
               if( n->getName() == ptrN )
               {
                 node= n;
                 break;
               }

               n= (ElemHtmlNode*)n->getParent();
             }

             break;
           }
         }

         if( node->getName() == child->getName() )
           node= (ElemHtmlNode*)node->getParent();
         node->insertChild(child);
         if( prior != '/' )
           node= child;
       }
       // <! statements are ignored>

       stmt.reset();
       prior= ' ';
       C= data.get();
       continue;
     }

     //-----------------------------------------------------------------------
     // Handle ENTITY reference
     if( C == '&' && node->getName() != "script" && node->getName() != "style" )
     {
       prior= C;
       C= data.get();

       // Accumulate the ENTITY
       TextBuffer token;
       while( C >= 0 && isalpha(C) )
       {
         token.put(C);
         C= data.get();
       }

       // Substitute the reference
       ptrC= token.toChar();
       for(i= 0; entity[i].name != NULL; i++)
       {
         if( strcmp(ptrC, entity[i].name) == 0 )
           break;
       }

       if( entity[i].name == NULL )
       {
         stmt.put('&');
         stmt.put(token);
       }
       else
       {
         stmt.put(entity[i].data);
         if( C == ';' )
           C= data.get();
       }

       continue;
     }

     //-----------------------------------------------------------------------
     // Statement text character
     stmt.put(C);
     prior= C;
     C= data.get();
   }

   IFHCDM(
     if( stmt.size() > 0 )
       error(data, "Missing </name> at EOF");

     debugTree(0, 0, root);
   )

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlParser::reset
//
// Purpose-
//       Reset the HtmlParser
//
//----------------------------------------------------------------------------
void
   HtmlParser::reset( void )        // Reset the HtmlParser
{
   delete root;
   root= NULL;
}

