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
//       DtdParser.cpp
//
// Purpose-
//       DtdParser implementation methods.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <com/DataSource.h>

#include "Common.h"
#include "TextBuffer.h"
#include "TextSource.h"

#include "DtdParser.h"

using namespace std;

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
// Return codes for DtdParser::parse
//----------------------------------------------------------------------------
enum RC_PARSE                       // DtdParser::parse return codes
{  RC_OK                            // Normal, no errors found
,  RC_SHOULD_NOT_OCCUR=           1 // Internal error 1

,  RC_UNEXPECTED_EOF=           100 // EOF not expected
,  RC_UNEXPECTED_ANGLE=         101 // Unexpected '<' or '>'
,  RC_UNEXPECTED_BRACE=         102 // Unexpected '[' or ']'
,  RC_MISSING_BRACE=            103 // Found '>' before ']'

,  RC_MALFORMED_ATTLIST=        110 // Malformed ATTLIST
,  RC_MALFORMED_DOCTYPE=        111 // Malformed DOCTYPE
,  RC_MALFORMED_ELEMENT=        112 // Malformed ELEMENT
,  RC_MALFORMED_ENTITY=         113 // Malformed ENTITY
,  RC_MALFORMED_NOTATION=       114 // Malformed NOTATION
,  RC_MALFORMED_SWITCH=         115 // Malformed SWITCH
,  RC_DUPLICATE_ELEMENT=        116 // Duplicate ELEMENT

,  RC_SYNTAX_ERROR=             120 // General syntax error
,  RC_UNDEFINED_STATEMENT=      121 // Statement type not defined
,  RC_UNDEFINED_ELEMENT=        122 // Element not defined

,  RC_UNKNOWN_PUBLIC=           130 // Unknown PUBLIC declaration
,  RC_UNKNOWN_SYSTEM=           131 // Unknown SYSTEM declaration
}; // PARSE_RC

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     publicDTD[]=
{  "-//W3C//DTD HTML 4.01 Frameset//EN"
,  "-//W3C//DTD HTML 4.01 Transitional//EN"
,  "-//W3C//DTD HTML 4.01//EN"
,  NULL
};

static const char*     publicURI[]=
{  "http://www.w3.org/TR/html4/frameset.dtd"
,  "http://www.w3.org/TR/html4/loose.dtd"
,  "http://www.w3.org/TR/html4/strict.dtd"
,  NULL
};

static const char*     systemURI[]=
{  "frameset.dtd"
,  "loose.dtd"
,  "strict.dtd"
,  NULL
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       getToken
//
// Purpose-
//       Extract next token from string.
//
//----------------------------------------------------------------------------
static inline char*                 // -> TOKEN
   getToken(                        // Extract token
     char*&            source)      // Source string (modified)
{
   char*               result;      // Resultant
   int                 delim;       // Delimiter

   if( *source == ' ' )
     source++;

   if( *source == '\0' )
     return NULL;

   result= source;
   delim= ' ';
   if( *result == '\'' || *result == '\"' )
   {
     delim= *result;
     result= ++source;
   }
   else if( *result == '(' )
     delim= ')';

   while( *source != delim )
   {
     if( *source == '\0' )
     {
       if( delim != ' ' )
         return NULL;

       break;
     }

     source++;
   }

   if( *source != '\0' )
   {
     if( *source == ')' )
       source++;

     *(source++)= '\0';
     if( isspace(*source) )
       source++;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       isNameChar
//
// Purpose-
//       Determine validity of name character
//
//----------------------------------------------------------------------------
static inline bool                  // TRUE iff valid name character
   isNameChar(                      // Is character a valid name character?
     int               C)           // The character to test
{
   if( C <= 0 )
     return false;

   if( isalnum(C) || C == '.' || C == '-' || C == '_' || C == ':' )
     return true;

   return false;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       isValidName
//
// Purpose-
//       Determine validity of name
//
//----------------------------------------------------------------------------
static inline bool                  // TRUE iff valid name
   isValidName(                     // Is name valid?
     const char*       C)           // The name to test
{
   if( !isNameChar(*C) )
     return false;

   if( !isalnum(*C) && *C != '_' )
     return false;

   while( *(++C) != '\0' )
   {
     if( !isNameChar(*C) )
       return false;
   }

   return true;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       stripString
//
// Purpose-
//       Strip comments and excess white space from string
//
//----------------------------------------------------------------------------
static char*                        // Stripped string
   stripString(                     // Strip string
     char*             source)      // From string (string is modified)
{
   char*               from;        // Working source
   char*               into;        // Working target
   int                 inSpace;     // TRUE iff last character was space
   int                 quote;       // Quote delimiter character

   from= into= source;
   while( isspace(*from) )
     from++;
   inSpace= FALSE;
   while( *from != '\0' )
   {
     switch( *from )
     {
       // Handle comment
       case '-':
         if( *(from+1) != '-' )
         {
           inSpace= FALSE;
           *(into++)= *(from++);
         }
         else
         {
           from += 2;
           for(;;)
           {
             if( *from == '\0' )
               return NULL;

             if( *from == '-' && *(from+1) == '-' )
             {
               from += 2;
               break;
             }

             from++;
           }
         }
         break;

       // Remove surrounding spaces
       case ',':
       case '|':
         if( inSpace )
           into--;
         *(into++)= *(from++);
         inSpace= TRUE;
         break;

       // Remove trailing spaces
       case '(':
         *(into++)= *(from++);
         inSpace= TRUE;
         break;

       // Remove leading spaces
       case ')':
       case '+':
       case '*':
       case '?':
         if( inSpace )
           into--;
         *(into++)= *(from++);
         inSpace= FALSE;
         break;

       // Convert whitespace to single blank
       case ' ':
       case '\t':
       case '\n':
       case '\r':
       case '\v':
       case '\f':
         from++;

         if( inSpace == FALSE )
         {
           *(into++)= ' ';
           inSpace= TRUE;
         }
         break;

       // Handle quoted string
       case '\"':
       case '\'':
         quote= *from;

         for(;;)
         {
           *(into++)= *(from++);

           if( *from == '\0' )
             return NULL;

           if( *from == quote )
             break;
         }
         ;;

       default:
         inSpace= FALSE;
         *(into++)= *(from++);
         break;
     }
   }

   if( inSpace )
     into--;
   *into= '\0';

   return source;
}

//----------------------------------------------------------------------------
//
// Method-
//       DtdParser::~DtdParser
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DtdParser::~DtdParser( void )    // Destructor
{
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       DtdParser::DtdParser
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DtdParser::DtdParser( void )     // Constructor
:  attlistMap()
,  elementMap()
,  entityMap()
,  publicMap()
,  errorReport()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       DtdParser::debug
//
// Purpose-
//       Write debugging messages.
//
//----------------------------------------------------------------------------
void
   DtdParser::debug( void ) const   // Write debugging message
{
   NvPair::const_iterator
                       mi;          // Map iterator

   printf("\nENTITY MAP:\n");
   for(mi= entityMap.begin(); mi != entityMap.end(); mi++)
     printf(" ENTITY(%s) text(%s)\n", mi->first.c_str(), mi->second.c_str());

   printf("\nELEMENT MAP:\n");
   for(mi= elementMap.begin(); mi != elementMap.end(); mi++)
     printf("ELEMENT(%s) text(%s)\n", mi->first.c_str(), mi->second.c_str());

   printf("\nATTLIST MAP:\n");
   for(mi= attlistMap.begin(); mi != attlistMap.end(); mi++)
   {
     printf("ATTLIST(%s) text(%s)\n", mi->first.c_str(), mi->second.c_str());
     if( elementMap.find(mi->first) == elementMap.end() )
       printf(" ORPHAN(%s)\n", mi->first.c_str());
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DtdParser::errorText
//
// Purpose-
//       Convert error code to error string
//
//----------------------------------------------------------------------------
const char*                         // Error string
   DtdParser::errorText(            // Convert into error string
     int                code)       // From this error code
{
   const char*          result= "UNDEFINED";

   switch( code )
   {
     case RC_OK:
       result= "No error";
       break;

     case RC_SHOULD_NOT_OCCUR:
       result= "SHOULD_NOT_OCCUR internal error";
       break;

     case RC_UNEXPECTED_EOF:
       result= "Unexpected EOF";
       break;

     case RC_UNEXPECTED_ANGLE:
       result= "Unexpected '<' or '>'";
       break;

     case RC_UNEXPECTED_BRACE:
       result= "Found '>' before ']'";
       break;

     case RC_MISSING_BRACE:
       result= "Missing '[' or ']'";
       break;

     case RC_MALFORMED_ATTLIST:
       result= "Malformed ATTLIST statement";
       break;

     case RC_MALFORMED_DOCTYPE:
       result= "Malformed DOCTYPE statement";
       break;

     case RC_MALFORMED_ELEMENT:
       result= "Malformed ELEMENT statement";
       break;

     case RC_MALFORMED_ENTITY:
       result= "Malformed ENTITY statement";
       break;

     case RC_MALFORMED_NOTATION:
       result= "Malformed NOTATION statement";
       break;

     case RC_MALFORMED_SWITCH:
       result= "Malformed SWITCH statement";
       break;

     case RC_DUPLICATE_ELEMENT:
       result= "Duplicated ELEMENT name";
       break;

     case RC_SYNTAX_ERROR:
       result= "Syntax error";
       break;

     case RC_UNDEFINED_STATEMENT:
       result= "Undefined statement name";
       break;

     case RC_UNDEFINED_ELEMENT:
       result= "Undefined ELEMENT";
       break;

     case RC_UNKNOWN_PUBLIC:
       result= "PUBLIC not defined";
       break;

     case RC_UNKNOWN_SYSTEM:
       result= "Unable to include";
       break;

     default:
       break;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DtdParser::parse
//
// Purpose-
//       Parse from DataSource
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   DtdParser::parse(                // Parse
     DataSource&       data)        // Using this DataSource
{
   IFHCDM( printf("  PARSE(%s)\n", data.getName().c_str()); )

   int rc= includeSOURCE(data, data);
   if( rc != 0 )
     error(rc, data);

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       DtdParser::reset
//
// Purpose-
//       Reset the DtdParser
//
//----------------------------------------------------------------------------
void
   DtdParser::reset( void )         // Reset the DtdParser
{
   attlistMap.clear();
   elementMap.clear();
   entityMap.clear();
   publicMap.clear();
   errorReport.reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       DtdParser::includeSOURCE
//
// Purpose-
//       Parse from DataSource
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   DtdParser::includeSOURCE(        // Parse
     DataSource&       file,        // Using this DataSource
     DataSource&       data)        // Using this data
{
   int                 angle;       // Angle indentation count
   int                 brace;       // Brace indentation count
   int                 C;           // Current source character
   NvIter              mi;          // Map iterator
   int                 prior;       // Previous source character
   int                 quote;       // Quote type
   TextBuffer          stmt;        // Statement collector

   int                 rc;

   IFHCDM( printf(" SOURCE(%s)\n", file.getName().c_str()); )
   ELHCDM( (void)file; )            // (Unused) TODO: INVESTIGATE

   angle= brace= quote= 0;
   prior= ' ';
   C= data.get();
   if( C < 0 )
     return RC_UNEXPECTED_EOF;
   for(;;)
   {
     // Open text
     if( C < 0 )
       break;
     if( C == '\r' )
     {
       C= data.get();
       continue;
     }

     //-----------------------------------------------------------------------
     // Char '%'
     if( C == '%' )
     {
       prior= C;
       C= data.get();
       if( isspace(C) )
       {
         stmt.put('%');
         continue;
       }

       // Load token
       TextBuffer token;            // Token accumulator
       token.put('%');
       while( isNameChar(C) )
       {
         token.put(C);
         C= data.get();
       }

       char* ptrT= token.toChar();
       mi= entityMap.find(ptrT);
       if( mi == entityMap.end() )
         stmt.put(ptrT);
       else
       {
         token.reset();
         token.put(mi->second);
         ptrT= token.toChar();
         if( stmt.size() == 0 && *ptrT != '\0' && *ptrT != '<' )
         {
           int result= includeSTMT(data, ptrT);
           if( result != 0 )
             return result;
         }
         else
           stmt.put(ptrT);
       }

       if( isspace(C) || C == ';' )
         C= data.get();
       continue;
     }

     //-----------------------------------------------------------------------
     // Handle special characters
     if( quote == 0 )
     {
       if( isspace(C) )
       {
         while( isspace(C) )
           C= data.get();

         if( angle == 0 )
           continue;

         if( prior != ' ' )
         {
           prior= ' ';
           stmt.put(' ');
         }
         continue;
       }

       switch( C )
       {
         case '-':
           C= data.get();
           if( C != '-' )
           {
             prior= '-';
             stmt.put('-');
             continue;
           }

           // In COMMENT
           for(;;)
           {
             C= data.get();
             if( C < 0 )
               return RC_UNEXPECTED_EOF;

             if( C == '-' )
             {
               C= data.get();
               if( C == '-' )
                 break;
             }
           }

           C= data.get();
           continue;

         case '[':
           if( angle == 0 )
             return RC_UNEXPECTED_BRACE;

           brace++;
           break;

         case ']':
           if( brace == 0 )
             return RC_UNEXPECTED_BRACE;

           brace--;
           break;

         case '<':
           if( brace > 0 )
             angle++;
           else
           {
             if( angle != 0 )
               return RC_UNEXPECTED_ANGLE;

             if( stmt.size() > 0 )
             {
               rc= includeSTMT(data, stmt.toChar());
               if( rc != 0 )
                 return rc;
               stmt.reset();
             }

             angle= 1;
           }
           break;

         case '>':
           if( angle == 0 )
             return RC_UNEXPECTED_ANGLE;

           angle--;
           if( angle == 0 )
           {
             if( brace > 0 )
               return RC_MISSING_BRACE;

             stmt.put('>');
             rc= includeSTMT(data, stmt.toChar());
             if( rc != 0 )
               return rc;

             stmt.reset();
             C= data.get();
             prior= ' ';
             continue;
           }
           break;

         case '\"':
         case '\'':
           quote= C;
           break;

         default:
           break;
       }
       prior= C;
     }
     else if( quote == C )
     {
       quote= 0;
       prior= C;
     }

     //-----------------------------------------------------------------------
     // Insert character into statement
     stmt.put(C);
     C= data.get();
   }

   rc= 0;
   if( stmt.size() > 0 )
   {
     if( angle > 0 )
       return RC_UNEXPECTED_EOF;

     rc= includeSTMT(data, stmt.toChar());
   }

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       DtdParser::includeSTMT
//
// Purpose-
//       Parse statement
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   DtdParser::includeSTMT(          // Parse statement
     DataSource&       data,        // From this DataSource
     char*             stmt)        // This statement
{
   DataSource*         clone;       // The DataSource clone
   NvIter              mi;          // Map iterator
   string              name;        // Name string
   char*               ptrC;        // Collect string pointer
   char*               ptrN;        // Name token pointer
   char*               ptrT;        // Generic token pointer
   string              text;        // Text string

   int                 i;

   IFHCDM( printf("   STMT(%s)\n", stmt); )

   if( stmt[0] != '<' )
   {
     if( memcmp(stmt, "PUBLIC ", 7) == 0 )
     {
       ptrC= stmt+ 7;

       if( *ptrC != '\'' && *ptrC != '\"' )
         return RC_SYNTAX_ERROR;
       ptrN= getToken(ptrC);
       ptrN= stripString(ptrN);
       name= ptrN;

       mi= publicMap.find(name);
       if( mi != publicMap.end() )
       {
         text= mi->second;
         clone= data.clone(text.c_str());
         if( clone == NULL )
         {
           errorReport.put("PUBLIC(");
           errorReport.put(name);
           errorReport.put(") Cannot load(");
           errorReport.put(text);
           errorReport.put(")");
           return RC_UNKNOWN_SYSTEM;
         }
       }
       else
       {
         for(i= 0; publicDTD[i] != NULL; i++)
         {
           if( strcmp(ptrN, publicDTD[i]) == 0 )
             break;
         }

         if( publicDTD[i] == NULL )
         {
           errorReport.put("PUBLIC(");
           errorReport.put(name);
           errorReport.put(") Not defined");
           return RC_UNKNOWN_PUBLIC;
         }

         clone= data.clone(systemURI[i]);
         if( clone == NULL )
         {
////////   clone= data.make(publicURI[i]);
////////   if( clone == NULL )
           {
             errorReport.put("PUBLIC(");
             errorReport.put(name);
             errorReport.put(") Cannot load(");
             errorReport.put(systemURI[i]);
             errorReport.put(")\n");

             errorReport.put("PUBLIC(");
             errorReport.put(name);
             errorReport.put(") Cannot load(");
             errorReport.put(publicURI[i]);
             errorReport.put(")");
             return RC_UNKNOWN_SYSTEM;
           }
         }
       }
     }
     else if( memcmp(stmt, "SYSTEM ", 7) == 0 )
     {
       ptrC= stmt+ 7;

       if( *ptrC != '\'' && *ptrC != '\"' )
         return RC_SYNTAX_ERROR;
       ptrN= getToken(ptrC);
       if( *ptrC != '\0' )
         return RC_SYNTAX_ERROR;

       clone= data.clone(ptrN);
       if( clone == NULL )
       {
         errorReport.put("SYSTEM(");
         errorReport.put(name);
         errorReport.put(") Cannot load");
         return RC_UNKNOWN_SYSTEM;
       }
     }
     else
       return RC_SYNTAX_ERROR;

     int result= includeSOURCE(*clone, *clone);
     if( result != 0 )
       error(result, *clone);
     delete clone;

     return result;
   }

   if( stmt[1] == '?' )
   {
     IFHCDM( printf("PROCESS(%s)\n", stmt); )

     // TODO: NOT CODED YET
     return 0;
   }

   if( stmt[1] != '!' )
   {
     IFHCDM( printf(" SYNTAX(%s)\n", stmt); )
     return RC_SYNTAX_ERROR;
   }

   if( strcmp(stmt,"<!>") == 0 )
   {
     IFHCDM( printf("COMMENT(%s)\n", stmt); )
     return 0;
   }

   // Remove statment trailer
   int L= strlen(stmt);
   stmt[--L]= '\0';                 // Remove closing '>'
   if( stmt[L-1] == ' ' )           // Remove trailing blank, if any
     stmt[--L]= '\0';

   stmt += 2;
   if( memcmp(stmt, "ENTITY ", 7) == 0 )
   {
     ptrC= stmt+7;

     IFHCDM( printf(" ENTITY(%s)\n", ptrC); )

     ptrN= getToken(ptrC);
     if( ptrN == NULL )
       return RC_MALFORMED_ENTITY;
     if( *ptrN == '%' )
     {
       if( strcmp(ptrN, "%") != 0 )
         return RC_MALFORMED_ENTITY;
       ptrN= getToken(ptrC);
       if( ptrN == NULL )
         return RC_MALFORMED_ENTITY;
       if( !isValidName(ptrN) )
         return RC_MALFORMED_ENTITY;

       // The character before the name must exist, so we use it to contain
       // the '%' prefix character, simplifying the name string creation.
       ptrN--;
       *ptrN= '%';
     }
     else if( !isValidName(ptrN) )
       return RC_MALFORMED_ENTITY;
     name= ptrN;

     int quote= 0;
     if( *ptrC == '\"' || *ptrC == '\'' ) // If quoted string
     {
       quote= *ptrC;
       L= strlen(ptrC);
       if( ptrC[L-1] != quote )
         return RC_MALFORMED_ENTITY;
       ptrC[L-1]= '\0';
       ptrC++;
       if( strchr(ptrC, quote) != NULL )
         return RC_MALFORMED_ENTITY;
     }

     text= ptrC;
     if( entityMap.find(name) == entityMap.end() )
     {
       entityMap[name]= text;
       IFHCDM( printf("..name(%s) text(%s)\n", name.c_str(), text.c_str()); )
     }

     if( quote == 0 )               // If PUBLIC entity
     {
       ptrT= getToken(ptrC);
       if( ptrT == NULL )
         return RC_MALFORMED_ENTITY;
       else if( strcmp(ptrT, "CDATA") == 0 || strcmp(ptrT, "SYSTEM") == 0 )
       {
         if( *ptrC != '\'' && *ptrC != '\"' )
           return RC_MALFORMED_ENTITY;

         return 0;
       }
       else if( strcmp(ptrT, "PUBLIC") == 0 )
       {
         if( *ptrC != '\'' && *ptrC != '\"' )
           return RC_MALFORMED_ENTITY;
         ptrN= getToken(ptrC);
         if( ptrN == NULL )
           return RC_MALFORMED_ENTITY;
         if( *ptrC == '\0' )          // If target not specified
           return 0;
         name= ptrN;

         if( *ptrC != '\'' && *ptrC != '\"' )
           return RC_MALFORMED_ENTITY;
         ptrT= getToken(ptrC);
         if( ptrT == NULL )
           return RC_MALFORMED_ENTITY;
         if( *ptrC != '\0' )
           return RC_MALFORMED_ENTITY;
         text= ptrT;

         if( publicMap.find(name) == publicMap.end() )
         {
           publicMap[name]= text;
           IFHCDM( printf("..PUBLIC name(%s) text(%s)\n",
                          name.c_str(), text.c_str()); )
         }
       }
       else
         return RC_MALFORMED_ENTITY;
     }
   }

   else if( memcmp(stmt, "ELEMENT ", 8) == 0 )
   {
     ptrC= stmt+8;
     ptrC= stripString(ptrC);
     if( ptrC == NULL )
       return RC_MALFORMED_ELEMENT;

     IFHCDM( printf("ELEMENT(%s)\n", ptrC); )

     char* ptrN= getToken(ptrC);
     if( ptrN == NULL )
       return RC_MALFORMED_ELEMENT;
     name= ptrN;
     text= ptrC;

     if( *ptrN != '(' )
     {
       mi= elementMap.find(ptrN);
       if( mi != elementMap.end() )
         return RC_DUPLICATE_ELEMENT;

       elementMap[name]= text;
       IFHCDM( printf("..name(%s) text(%s)\n", name.c_str(), text.c_str()); )
       return 0;
     }

     TextBuffer symbol;             // Symbol accumulator
     ptrN++;
     while( *ptrN != 0 )
     {
       if( *ptrN == '|' || *ptrN == ')' )
       {
         char* ptrS= symbol.toChar();
         if( *ptrS == '\0' )
           return RC_MALFORMED_ELEMENT;

         mi= elementMap.find(ptrS);
         if( mi != elementMap.end() )
           return RC_DUPLICATE_ELEMENT;

         elementMap[ptrS]= text;
         IFHCDM( printf("..name(%s) text(%s)\n", ptrS, text.c_str()); )
         if( *ptrN == ')' )
         {
           if( *(ptrN+1) != '\0' )
             return RC_MALFORMED_ELEMENT;

           return 0;
         }

         symbol.reset();
       }
       else
         symbol.put(*ptrN);

       ptrN++;
     }

     return RC_MALFORMED_ENTITY;    // No closing ')'
   }

   else if( memcmp(stmt, "ATTLIST ", 8) == 0 )
   {
     ptrC= stmt+8;
     ptrC= stripString(ptrC);
     if( ptrC == NULL )
       return RC_MALFORMED_ATTLIST;

     IFHCDM( printf("ATTLIST(%s)\n", ptrC); )

     ptrN= getToken(ptrC);
     if( ptrN == NULL )
       return RC_MALFORMED_ATTLIST;
     name= ptrN;
     text= ptrC;

     if( *ptrN != '(' )
     {
       mi= attlistMap.find(name);
       if( mi != attlistMap.end() )
       {
         mi->second += ' ';
         mi->second += text;
         IFHCDM( printf("..name(%s) text(%s)\n",
                        name.c_str(), mi->second.c_str()); )
       }
       else
       {
         attlistMap[name]= text;
         IFHCDM( printf("..name(%s) text(%s)\n", name.c_str(), text.c_str()); )
       }

       return 0;
     }

     TextBuffer symbol;             // Symbol accumulator
     ptrN++;
     while( *ptrN != 0 )
     {
       if( *ptrN == '|' || *ptrN == ')' )
       {
         char* ptrS= symbol.toChar();
         if( *ptrS == '\0' )
           return RC_MALFORMED_ELEMENT;

         mi= attlistMap.find(ptrS);
         if( mi != attlistMap.end() )
         {
           mi->second += ' ';
           mi->second += text;
           IFHCDM( printf("..name(%s) text(%s)\n", ptrS, mi->second.c_str()); )
         }
         else
         {
           attlistMap[ptrS]= text;
           IFHCDM( printf("..name(%s) text(%s)\n", ptrS, text.c_str()); )
         }

         if( *ptrN == ')' )
         {
           if( *(ptrN+1) != '\0' )
             return RC_MALFORMED_ATTLIST;

           return 0;
         }

         symbol.reset();
       }
       else
         symbol.put(*ptrN);

       ptrN++;
     }

     return RC_MALFORMED_ATTLIST;
   }

   else if( stmt[0] == '[' )
   {
     ptrC= stmt;
     IFHCDM( printf(" SWITCH(%s)\n", ptrC); )

     ptrC++;
     while( *ptrC == ' ' )
       ptrC++;

     ptrN= ptrC;
     while( *ptrC != ' ' && *ptrC != '[' )
       ptrC++;

     if( *ptrC == '[' )
       *ptrC= '\0';
     else
     {
       *ptrC= '\0';
       ptrC++;
       while( *ptrC == ' ' )
         ptrC++;

       if( *ptrC != '[' )
         return RC_MALFORMED_SWITCH;
     }

     ptrC++;
     while( *ptrC == ' ' )
       ptrC++;
     L= strlen(ptrC);
     if( ptrC[L-1] != ']' || ptrC[L-2] != ']' )
       return RC_MALFORMED_SWITCH;
     ptrC[L-2]= '\0';

     if( strcmp(ptrN, "INCLUDE") != 0 )
       return 0;

     TextSource more(ptrC);
     return includeSOURCE(data, more);
   }

   else if( memcmp(stmt, "NOTATION ", 9) == 0 )
   {
     ptrC= stmt+9;
     IFHCDM( printf("NOTATION(%s)\n", ptrC); )

     // TODO: NOT CODED YET
   }

   else if( memcmp(stmt, "SGML ", 5) == 0 )
   {
     ptrC= stmt+5;
     IFHCDM( printf("   SGML(%s)\n", ptrC); )

     // TODO: NOT CODED YET
   }

   else if( memcmp(stmt, "DOCTYPE ", 8) == 0 )
   {
     ptrC= stmt+8;
     IFHCDM( printf("DOCTYPE(%s)\n", ptrC); )

     while( *ptrC != ' ' )
     {
       if( *ptrC == '\0' )
         return RC_MALFORMED_DOCTYPE;

       ptrC++;
     }
     ptrC++;

     if( ptrC[0] != '[' && ptrC[1] != ' ' )
       return RC_MALFORMED_DOCTYPE;

     L= strlen(ptrC);
     if( ptrC[L-1] != ']' )
       return RC_MALFORMED_DOCTYPE;

     ptrC[L-1]= '\0';
     ptrC += 2;

     TextSource more(ptrC);
     return includeSOURCE(data, more);
   }

   else
   {
     fprintf(stderr, "%4d DtdParser UNKNOWN(%s)\n", __LINE__, stmt);
     return RC_UNDEFINED_STATEMENT;
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       DtdParser::error
//
// Purpose-
//       Append error text
//
//----------------------------------------------------------------------------
void
   DtdParser::error(                // Parse
     int               rc,          // Error code
     DataSource&       data)        // For this DataSource
{
   char                string[128]; // Message buffer
   int                 L;           // Length

   if( errorReport.size() == 0 )
   {
     L= sprintf(string, "RC(%d):", rc);
     string[L]= '\0';
     errorReport.put(string);
     errorReport.put(errorText(rc));
     errorReport.put(':');
   }
   else
     errorReport.put("\nincluded from ");

   L= sprintf(string, "(%ld:%d):", data.getLine(), data.getColumn());
   string[L]= '\0';
   errorReport.put(string);
   errorReport.put(data.getName());
}

