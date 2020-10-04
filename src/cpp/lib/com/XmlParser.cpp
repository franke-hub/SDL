//----------------------------------------------------------------------------
//
//       Copyright (c) 2012-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       XmlParser.cpp
//
// Purpose-
//       XmlParser object methods.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <ctype.h>                  // For isspace()
#include <stdarg.h>                 // For botch()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/Debug.h>              // Used in debug method
#include <com/Buffer.h>
#include <com/Reader.h>
#include <com/Writer.h>

#include "com/XmlParser.h"
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

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifdef _OS_WIN
  #define vsnprintf _vsnprintf
#endif

//----------------------------------------------------------------------------
//
// Struct-
//       Entity
//
// Purpose-
//       Entity name/value pair
//
//----------------------------------------------------------------------------
struct Entity                       // Entity Name/Value pair
{  const char*         name;        // The entity name
   const char*         value;       // The entity value
};

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Entity          builtin[]=   // Built-in Entity name/value set
{  {"amp",     "&"}                 // Ampersand
,  {"apos",    "\'"}                // Apostrophe
,  {"gt",      ">"}                 // Greater Than symbol
,  {"lt",      "<"}                 // Less Than symbol
,  {"quot",    "\""}                // Quotation marks

,  {"cent",    "&#162"}             // US Cent
,  {"pound",   "&#163"}             // British Pound
,  {"yen",     "&#165"}             // Japanese Yen
,  {"sect",    "&#167"}             // Section
,  {"copy",    "&#169"}             // Copyright (C)
,  {"reg",     "&#174"}             // Registered Trade Mark (R)

,  {NULL,      NULL}                // Last entry delimiter
}; // builtin[]

//----------------------------------------------------------------------------
//
// Subroutine-
//       getReader
//
// Purpose-
//       Get next character from reader, exception if EOF
//
//----------------------------------------------------------------------------
static int                          // The next character
   getReader(                       // Get next character from Reader
     Reader&           reader,      // The Reader
     const string&     text)        // Accumulation text
{
   int C= reader.get();
   if( C < 0 )
     throwf("Unexpected EOF in %s", text.c_str());

   return C;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::~XmlParser
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   XmlParser::~XmlParser( void )    // Destructor
{
   reset();                         // Reset the XmlParser
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::XmlParser
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   XmlParser::XmlParser( void )     // Default constructor
:  desc(NULL), root(NULL), entity(), temp(pmet)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::debug
//
// Purpose-
//       Object method: Debugging display
//
//----------------------------------------------------------------------------
void
   XmlParser::debug(                // Debugging display
     int               level,       // For this nexting level
     const XmlNode*    here) const  // And this node
{
   int                 i;

   for(i= 0; i<level; i++)
     debugf("| ");

   int type= here->getType();
   if( type == XmlNode::TYPE_ELEM || type == XmlNode::TYPE_ROOT )
     debugf("[%s] name(%s) text(%s)\n", XmlNode::type2name(type),
            here->getName().c_str(), getText(here).c_str());
   else
   {
     debugf("[%s] name(%s) data(", XmlNode::type2name(type),
            here->getName().c_str());
     string data= here->getValue();
     int L= data.size();
     for(i= 0; i<L; i++)
     {
       int C= data[i];
       if( C == '\r' )
         debugf("\\r");
       else if( C == '\n' )
         debugf("\\n");
       else if( C == '\t' )
         debugf("\\t");
       else if( C == '\\' )
         debugf("\\\\");
       else if( C == '\0' )
         debugf("\\0");
       else
         debugf("%c", C);
     }
     debugf(")\n");
   }

   XmlNode* node= here->getAttrib();
   while( node != NULL )
   {
     for(i= 0; i<level; i++)
       debugf("| ");

     debugf("| %s='%s'\n", node->getName().c_str(), getValue(node).c_str());
     node= node->getNext();
   }

   node= here->getChild();
   while( node != NULL )
   {
     debug(level+1, node);
     node= node->getNext();
   }
}

void
   XmlParser::debug( void ) const   // Debugging display
{
   debugf("Descriptor(%p)\n", desc);
   if( desc != NULL )
     debug(1, desc);

   debugf("Root(%p)\n", root);
   if( root != NULL )
     debug(1, root);

   XmlNode* node= entity.getHead();
   debugf("Entities(%p)\n", node);
   while( node != NULL )
   {
     debugf("! %s='%s'\n", node->getName().c_str(), node->getValue().c_str());

     node= node->getNext();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::genAttr
//
// Purpose-
//       Generate attribute node.
//
// Implementation node-
//       Prior reader characters ' '
//
//----------------------------------------------------------------------------
XmlNode*                            // The new attribute node
   XmlParser::genAttr(              // Generate attribute node
     XmlNode*          parent,      // Parent node
     Reader&           reader)      // The Reader
{
   int C= reader.skipBlank();       // Current character
   if( C == '/' || C == '>' )       // If end of attributes
     return NULL;

   if( C == '=' )
     throwf("Missing name in <%s =", parent->getName().c_str());

   // Extract name (overly flexible)
   string name= "";
   while( C >= 0 && C != '=' )
   {
     if( isspace(C) )
       break;

     if( C == '\'' || C == '\"' )
       throwf("Quote in name in '<%s %s", parent->getName().c_str(), name.c_str());

     name += C;
     C= reader.get();
   }

   if( isspace(C) )
     C= reader.skipBlank();
   if( C != '=' )
     throwf("Missing '=' after '<%s %s'", parent->getName().c_str(), name.c_str());

   // Extract value
   int Q= reader.skipBlank();
   if( Q != '\'' && Q != '\"' )
     throwf("Missing quote in <%s %s=", parent->getName().c_str(), name.c_str());

   string data= "";
   data += Q;
   for(;;)
   {
     C= reader.get();
     if( C < 0 || C == '\r' || C == '\n' )
       throwf("Missing terminator in <%s %s=%s", parent->getName().c_str(),
              name.c_str(), data.c_str());

     data += C;
     if( C == Q )
     {
       C= reader.get();
       if( C != Q )
         break;
     }
   }

   if( C != '/' && C != '>' )
   {
     if( !isspace(C) )
       throwf("Malformed header after <%s %s=%s", parent->getName().c_str(),
              name.c_str(), data.c_str());
   }

   XmlNode* node= new XmlNode(XmlNode::TYPE_ATTR, name, data);
   return node;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::genData
//
// Purpose-
//       Generate comment or CDATA node.
//
// Implementation node-
//       Current reader characters '<!'
//
//----------------------------------------------------------------------------
XmlNode*                            // The new data node
   XmlParser::genData(              // Generate data nodes
     XmlNode*          parent,      // Parent node
     Reader&           reader)      // The Reader
{
   XmlNode*            node= NULL;  // Resultant
   int                 C;           // Current character
   char                term[8];     // Termination string accumulator

   (void)parent;                    // (Currently) unused
   memset(term, 0, sizeof(term));   // (Removes erroneous compiler warnings)
   string text= "<!";               // Resultant text
   for(;;)
   {
     C= getReader(reader, text);
     text += C;
     if( text == "<!--" )           // If comment
     {
       term[0]= '\0';
       for(;;)
       {
         C= getReader(reader, text);
         text += C;
         if( isspace(C) )
         {
           term[0]= ' ';            // strcpy(term, " ")
           term[1]= '\0';
         }
         else if( C == '>' )
         {
           if( term[0] == ' ' && term[1] == '-' && term[2] == '-' )
             break;

           term[0]= '\0';
         }
         else if( C == '-' )
         {
           if( term[0] == ' ' )     // It's either '\0' or ' '
           {
             if( term[1] == '\0' )
             {
               term[1]= '-';
               term[2]= '\0';
             }
             else if( term[2] == '\0' )
             {
               term[2]= '-';
               term[3]= '\0';
             }
             else
               term[0]= '\0';
           }
         }
         else
           term[0]= '\0';
       }

       node= new XmlNode(XmlNode::TYPE_COMMENT, "#comment", text);
       break;
     }

     if( text == "<![CDATA[" )      // If CDATA
     {
       term[0]= '\0';
       for(;;)
       {
         C= getReader(reader, text);
         text += C;
         if( C == ']' )
         {
           if( term[0] == '\0' )
           {
             term[0]= ']';
             term[1]= '\0';
           }
           else
           {
             term[1]= ']';
             term[2]= '\0';
           }
         }
         else if( C == '>' )
         {
           if( term[0] == ']' && term[1] == ']' )
             break;

           term[0]= '\0';
         }
         else
           term[0]= '\0';
       }

       node= new XmlNode(XmlNode::TYPE_CDATA, "#CDATA", text);
       break;
     }

     if( isspace(C) )               // Possibly <!ELEMENT or <!ENTITY
     {
       for(;;)
       {
         C= getReader(reader, text);
         text += C;
         if( C == '>' )
           break;
       }

       node= new XmlNode(XmlNode::TYPE_DECL, "#declare", text);
       break;
     }

     if( C == '>' )
       throwf("'%s' Malformed", text.c_str());
   }

   return node;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::genDesc
//
// Purpose-
//       Generate descriptive node.
//
// Implementation node-
//       Current reader characters '<?'
//
//----------------------------------------------------------------------------
XmlNode*                            // The new declarative node
   XmlParser::genDesc(              // Generate declarative node
     XmlNode*          parent,      // Parent node
     Reader&           reader)      // The Reader
{
   XmlNode*            node= NULL;  // Resultant
   int                 C;           // Current character

   (void)parent;                    // (Currently) unused
   string text= "<?";               // Resultant text
   char term[8];                    // Termination string accumulator
   term[0]= '\0';
   for(;;)
   {
     C= getReader(reader, text);
     text += C;
     if( C == '?' )
     {
       term[0]= '?';                // strcpy(term, "?")
       term[1]= '\0';
     }
     else if( C == '>' )
     {
       if( term[0] == '?' )
         break;

       term[0]= '\0';
     }
     else
       term[0]= '\0';
   }

   node= new XmlNode(XmlNode::TYPE_DESC, "#descriptor", text);

   return node;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::genNode
//
// Purpose-
//       Generate child nodes.
//
// Implementation node-
//       Current reader character '<'
//
//----------------------------------------------------------------------------
XmlNode*                            // The new child node
   XmlParser::genNode(              // Generate child nodes
     XmlNode*          parent,      // Parent node
     Reader&           reader)      // The Reader
{
   int                 C;           // Current character
   string              name;        // Current name string

   C= getReader(reader, "EOF in XML header <");
   if( C == '!' )
     return genData(parent, reader);

   if( C == '?' )
     return genDesc(parent, reader);

   if( isspace(C) )
     C= reader.skipBlank();
   if( C < 0 )
     throwf("EOF in XML header <");

   if( C == '/' )                   // If parent termination
   {
     C= reader.skipBlank();
     name= "";
     while( C != '>' && C >= 0 )
     {
       name += C;
       C= reader.get();
       if( isspace(C) )
       {
         C= reader.skipBlank();
         if( C != '>' )
           throwf("XML header '</%s' invalid whitespace", name.c_str());
       }
     }

     if( C < 0 )
       throwf("EOF in XML terminator '</%s'", name.c_str());

     if( parent == NULL )
       throwf("XML begins with '</%s>'", name.c_str());

     if( parent->getName() != name )
       throwf("<%s> ... </%s>", parent->getName().c_str(), name.c_str());

     return NULL;
   }

   if( C == '!' )                   // If comment or CDATA
     return genData(parent, reader);

   // Gather the node name
   name= C;
   for(;;)
   {
     C= reader.get();
     if( C < 0 )
       throwf("EOF in XML header '<%s'", name.c_str());

     if( isspace(C) || C == '>' )
       break;

     name += C;
   }

   int type= parent == NULL ? XmlNode::TYPE_ROOT : XmlNode::TYPE_ELEM;
   XmlNode* node= new XmlNode(type, name); // Resultant
   while( reader.prior() != '>' && reader.prior() != '/' )
   {
     XmlNode* next= genAttr(node, reader);
     if( next != NULL )
       node->insert(next);
   }

   if( reader.prior() == '/' )      // If self-contained header/trailer
   {
     C= reader.skipBlank();
     if( C != '>' )
       throwf("Malformed XML header '<%s /'", name.c_str());

     return node;
   }

   //-------------------------------------------------------------------------
   // Node itself is complete. Look for child nodes.
   C= reader.get();
   for(;;)
   {
     if( C < 0 )
       throwf("EOF in <%s>", name.c_str());

     if( C == '<' )
     {
       XmlNode* next= genNode(node, reader);
       if( next == NULL )
         break;

       node->insert(next);
       C= reader.get();
       continue;
     }

     string data;
     data= C;
     for(;;)
     {
       C= reader.get();
       if( C == '<' || C < 0 )
         break;

       data += C;
     }

     XmlNode* text= new XmlNode(XmlNode::TYPE_TEXT, "#text", data);
     node->insert(text);
   }

   return node;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::getDesc
//
// Purpose-
//       Extract the associated descriptor node.
//
//----------------------------------------------------------------------------
XmlNode*                            // The associated descriptor node
   XmlParser::getDesc( void ) const // Get associated descriptor node
{
   return desc;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::getEntity
//
// Purpose-
//       Extract entity from name
//
//----------------------------------------------------------------------------
const char*                         // The associated Entity value
   XmlParser::getEntity(            // Extract entity value
     const string&     name) const  // For this name
{
   int L= name.size();              // Entity string length
   if( L < 1 )                      // Minimum entity size
     return NULL;

   if( name[0] == '#' )             // Numeric reference
   {
     if( L < 2 )
       return NULL;

     int result= 0;                 // Resultant
     if( name[1] == 'x' )
     {
       if( L < 3 )
         return NULL;

       for(int i= 2; i<L; i++)
       {
         int C= name[i];
         if( C >= '0' && C <= '9' )
           C= C-'0';
         else if( C >= 'A' && C <= 'F' )
           C= 10 + C - 'A';
         else if( C >= 'a' && C <= 'f' )
           C= 10 + C - 'a';
         else
           return NULL;

         if( result >= 0x08000000 )
           return NULL;

         result <<= 4;
         result |=  C;
       }
     }
     else
     {
       for(int i= 1; i<L; i++)
       {
         int C= name[i];
         if( C >= '0' && C <= '9' )
           C= C-'0';
         else
           return NULL;

         result *= 10;
         result +=  C;

         if( result < 0 )
           return NULL;
       }
     }

     temp[0]= result;
     temp[1]= '\0';
     return temp;
   }

   XmlNode* node= entity.getHead();
   while( node != NULL )
   {
     if( node->getName() == name )
       return node->getValue().c_str();

     node= node->getNext();
   }

   for(int i= 0; builtin[i].name != NULL; i++)
   {
     if( name == builtin[i].name )
       return builtin[i].value;
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::setEntity
//
// Purpose-
//       Set associated entity value
//
//----------------------------------------------------------------------------
void
   XmlParser::setEntity(            // Update entity value
     const string&     name,        // For this entity name
     const char*       value)       // And this entity value
{
   int L= name.size();              // Entity string length
   if( L < 1 )                      // Minimum entity size
     throwf("XmlParser::setEntity(\"\")");

   if( name[0] == '#' )             // Numeric reference
     throwf("XmlParser::setEntity(%s)", name.c_str());

   XmlNode* node= entity.getHead();
   while( node != NULL )
   {
     if( node->getName() == name )
     {
       entity.remove(node, node);
       delete node;
       break;
     }
   }

   if( value != NULL )
   {
     XmlNode* node= new XmlNode(XmlNode::TYPE_ENTITY, name, value);
     entity.fifo(node);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::getRoot
//
// Purpose-
//       Extract the associated root node.
//
//----------------------------------------------------------------------------
XmlNode*                            // The associated root node
   XmlParser::getRoot( void ) const // Get associated root node
{
   return root;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::evaluate
//
// Purpose-
//       Object method: Replace entities
//
//----------------------------------------------------------------------------
string                              // Evaluated string
   XmlParser::evaluate(             // Replace entities
     const string&     data) const  // Within this string
{
   string              result= "";  // Resultant

   int                 C;           // Current character
   int                 x= 0;        // Current value index

   int L= data.size();              // String length
   if( L < 2 )                      // If not long enough to contain reference
     return data;

   // Extract data, converting &ref; entities
   while( x < L )
   {
     C= data[x++];
     if( C == '&' && x < L )        // If entity reference
     {
       string entity= "";
       while( x < L )
       {
         C= data[x++];
         if( C == ';' || isspace(C) )
           break;

         entity += C;
       }

       const char* REF= getEntity(entity);
       if( REF == NULL )
       {
         result += "&";
         result += entity;
         if( C == ';' || isspace(C) )
           result += C;
       }
       else
       {
         if( strlen(REF) > 2 && REF[0] == '&' && REF[1] == '#' )
           REF= getEntity(REF+1);

         result += REF;
       }
     }
     else
       result += C;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::getText
//
// Purpose-
//       Object method: Extract all text, normalizing it
//
//----------------------------------------------------------------------------
string                              // Resultant string
   XmlParser::getText(              // Get associated text
     const XmlNode*    node) const  // From this XmlNode
{
   int                 P;           // Prior character
   int                 C;           // Current character
   int                 L;           // Current node->getValue().size()
   int                 x;           // Current value index

   string result= "";               // Resultant

   // Locate first non-whitespace character
   node= node->getChild();          // Get first child
   while( node != NULL )
   {
     const string data= node->getValue();
     if( node->getType() == XmlNode::TYPE_TEXT ) // If text node
     {
       x= 0;
       L= data.size();
       while( x < L && isspace(data[x]) )
         x++;

       if( x < L )
         break;
     }

     node= node->getNext();
   }

   if( node == NULL )               // No text found
     return result;                 // No references possible

   // Extract data, combining whitespace
   P= 0;                            // Prior character not whitespace
   string data= node->getValue();   // Current node data
   for(;;)
   {
     C= data[x++];
     if( !isspace(C) )
     {
       if( isspace(P) )
         result += ' ';

       result += C;
     }

     P= C;
     if( x >= L )
     {
       node= node->getNext();
       while( node != NULL )
       {
         data= node->getValue();
         if( node->getType() == XmlNode::TYPE_TEXT && data != "" )
           break;

         node= node->getNext();
       }

       if( node == NULL )
         break;

       x= 0;
       L= data.size();
     }
   }

   return evaluate(result);
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::getValue
//
// Purpose-
//       Object method: Extract value, replacing entities if required
//
//----------------------------------------------------------------------------
string                              // Resultant string
   XmlParser::getValue(             // Get associated value
     const XmlNode*    node) const  // From this XmlNode
{
   string data= node->getValue();   // Get associated value
   int L= data.size();              // String length
   if( L < 2 )                      // If not long enough to be modifiable
     return data;

   int C= data[0];                  // Remove bounding quotes (if any)
   if( C == data[L-1] && ( C == '\'' || C == '\"' ) )
   {
     L -= 2;                        // New length
     data= data.substr(1, L);       // Remove the source quotes
   }

   return evaluate(data);
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::output
//
// Purpose-
//       Object method: Output the node tree
//
//----------------------------------------------------------------------------
void
   XmlParser::output(               // Output the node tree
     XmlNode*          here,        // Starting from here
     Writer&           writer)      // To this writer
{
   XmlNode*            node;        // Working -> node

   int    type= here->getType();
   string name= here->getName();
   string data= here->getValue();
   switch( type )
   {
     case XmlNode::TYPE_ATTR:
       writer.printf("%s=%s", name.c_str(), data.c_str());
       break;

     case XmlNode::TYPE_ELEM:
     case XmlNode::TYPE_ROOT:
       writer.printf("<%s", name.c_str());
       node= here->getAttrib();
       while( node != NULL )
       {
         writer.put(' ');
         output(node, writer);
         node= node->getNext();
       }

       node= here->getChild();
       if( node == NULL )
       {
         if( here->getAttrib() == NULL ) // If no attribute and no children
           writer.put(' ');         // Add space delimiter

         writer.printf("/>");
         break;
       }

       writer.printf(">");
       while( node != NULL )
       {
         output(node, writer);
         node= node->getNext();
       }

       writer.printf("</%s>", name.c_str());
       break;

     // These types are all unformatted data
     case XmlNode::TYPE_TEXT:
     case XmlNode::TYPE_COMMENT:
     case XmlNode::TYPE_CDATA:
     case XmlNode::TYPE_DECL:
     case XmlNode::TYPE_DESC:
       writer.printf("%s", data.c_str());

       node= here->getChild();      // (For descriptor text)
       while( node != NULL )
       {
         output(node, writer);
         node= node->getNext();
       }
       break;

     default:
       throwf("XmlParser(*)::output() invalid type");
       break;
   }
}

void
   XmlParser::output(               // Output the node tree
     Writer&           writer) const// To this writer
{
   if( desc != NULL )
     output(desc, writer);

   if( root != NULL )
     output(root, writer);
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::parse
//
// Purpose-
//       Static method: Generate node tree from string
//
//----------------------------------------------------------------------------
XmlNode*                            // The associated root node
   XmlParser::parse(                // Get associated node tree
     const std::string&input)       // From this string
{
   TempBuffer buffer;
   buffer.open("XmlParser.$$$", buffer.MODE_WRITE);
   buffer.write(input.c_str(), input.size());
   buffer.close();

   buffer.open("XmlParser.$$$", buffer.MODE_READ);
   XmlNode* node= parse(buffer);
   buffer.close();

   return node;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::parse
//
// Purpose-
//       Static method: Generate node tree from Reader.
//
//----------------------------------------------------------------------------
XmlNode*                            // Resultant (allocated) root node
   XmlParser::parse(                // Get next complete node tree
     Reader&           reader)      // From this Reader
{
   reset();                         // Reset the XmlParser

   int C= reader.get();
   while( C != '<' && C >= 0 )
     C= reader.get();

   XmlNode* node= NULL;
   if( C >= 0 )
     node= genNode(NULL, reader);

   while( node != NULL )
   {
     string name= node->getName();
     if( name[0] == '#' )           // If descriptor node
     {
       if( desc == NULL )
         desc= node;
       else
         desc->insert(node);

       string data;
       C= reader.get();
       while( C != '<' && C >= 0 )
       {
         data += C;
         C= reader.get();
       }

       if( data != "" )             // If data present
       {
         XmlNode* text= new XmlNode(XmlNode::TYPE_TEXT, "#text", data);
         node->insert(text);
       }

       node= genNode(NULL, reader);
     }
     else                           // If element node
     {
       root= node;
       node= NULL;
     }
   }

   return root;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::reset
//
// Purpose-
//       Reset the XmlParser
//
//----------------------------------------------------------------------------
void
   XmlParser::reset( void )         // Reset the XmlParser
{
   delete desc;
   desc= NULL;

   delete root;
   root= NULL;

   for(;;)
   {
     XmlNode* node= entity.remq();
     if( node == NULL )
       break;

     delete node;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlParser::toString
//
// Purpose-
//       Object method: Output node tree to string
//
//----------------------------------------------------------------------------
string                              // Resultant
   XmlParser::toString(             // Output the node tree
     XmlNode*          here)        // From this node
{
   string result= "";               // Resultant
   XmlNode*            node;        // Working -> node

   int    type= here->getType();
   string name= here->getName();
   string data= here->getValue();
   switch( type )
   {
     case XmlNode::TYPE_ATTR:
       result= name + "=" + data;
       break;

     case XmlNode::TYPE_ELEM:
     case XmlNode::TYPE_ROOT:
       result= "<" + name;
       node= here->getAttrib();
       while( node != NULL )
       {
         result += " " + toString(node);
         node= node->getNext();
       }

       node= here->getChild();
       if( node == NULL )
       {
         result += "/>";
         break;
       }

       result += ">";
       while( node != NULL )
       {
         result += toString(node);
         node= node->getNext();
       }

       result += "</" + name + ">";
       break;

     // These types are all unformatted data
     case XmlNode::TYPE_TEXT:
     case XmlNode::TYPE_COMMENT:
     case XmlNode::TYPE_CDATA:
     case XmlNode::TYPE_DECL:
     case XmlNode::TYPE_DESC:
       result= data;
       break;

     default:
       throwf("XmlParser(*)::toString() invalid type");
       break;
   }

   return result;
}

string                              // Resultant node tree
   XmlParser::toString( void ) const// Output the node tree
{
   string result;

   if( desc != NULL )
     result += toString(desc);

   if( root != NULL )
     result += toString(root);

   return result;
}

