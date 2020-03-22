//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Robots.cpp
//
// Purpose-
//       Robots implementation methods.
//
// Last change date-
//       2012/01/01
//
// Implementation notes-
//       Supports googlebot wildcards:
//         * Any character
//         $ (End of line character)
//       Googlebot examples:
//         Disallow: /foo           Disallow any request beginning with "foo"
//         Disallow: /foo*          Disallow any request beginning with "foo"
//         Disallow: /*foo          Disallow any request with "foo" in it
//         Disallow: /*.foo$        Disallow any request ending with ".foo"
//         Disallow: /*?            Disallow any request with a ? in it
//
//----------------------------------------------------------------------------
#include <ctype.h>                  // For tolower
#include <stdio.h>
#include <string>

#include <stdlib.h>
#include <string.h>

#include <com/DataSource.h>
#include <com/Debug.h>
#include <com/istring.h>
#include <com/List.h>
#include <com/Parser.h>

#include "Common.h"
#include "Robots.h"

using std::string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define USE_HCDM_BRINGUPOPEN FALSE  // Display ordered list of items?
#define USE_MOST_RESTRICTIVE  TRUE  // Use most restrictive definition?

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Subroutine-
//       strip
//
// Purpose-
//       Remove leading and trailing blanks from a string.
//
//----------------------------------------------------------------------------
static char*                        // -> Stripped string
   strip(                           // Strip a string
     char*             text)        // -> string (!MODIFIED!)
{
   int                 i;

   while( *text == ' ' )            // Remove leading blanks
     text++;

   i= strlen(text);
   for(i= strlen(text); i>0 && text[i-1] == ' '; i--)
     ;

   text[i]= '\0';
   return text;
}

//----------------------------------------------------------------------------
//
// Class-
//       Item
//
// Purpose-
//       An Item on one of our Lists
//
//----------------------------------------------------------------------------
class Item : public List<Item>::Link { // An Item on one of our lists
public:
string                 data;        // Associated string
}; // class Item

//----------------------------------------------------------------------------
//
// Subroutine-
//       oldItem
//
// Purpose-
//       Release an old Item
//
//----------------------------------------------------------------------------
static void
   oldItem(                         // Release old Item
     Item*             item)        // -> Item
{
   delete item;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       newItem
//
// Purpose-
//       Allocate a new Item
//
//----------------------------------------------------------------------------
static Item*                        // -> Item
   newItem( void )                  // Allocate new Item
{
   Item* item= new Item();

   return item;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parseDec
//
// Purpose-
//       Parse a string, extracting a decimal value.
//       Leading blanks are skipped.
//
// Returns-
//       Return (decimal value)
//       String (The value delimiter)
//
//----------------------------------------------------------------------------
static int                          // Return value
   parseDec(                        // Extract decimal value from string
     const char*&      C)           // -> String (updated)
{
   Parser parser(C);
   int result= parser.toDec();
   C= parser.getString();
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parseDouble
//
// Purpose-
//       Parse a string, extracting a double value.
//       Leading blanks are skipped.
//
// Returns-
//       Return (double value)
//       String (The value delimiter)
//
//----------------------------------------------------------------------------
static double                       // Return value
   parseDouble(                     // Extract decimal value from string
     const char*&      C)           // -> String (updated)
{
   Parser parser(C);
   double result= parser.toDouble();
   C= parser.getString();
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       convertList
//
// Purpose-
//       Convert list to an array of names
//
//----------------------------------------------------------------------------
static char**                       // The array of names
   convertList(                     // Convert list to array of names
     List<Item>&       list)        // The list of items
{
   char**              result;      // Resultant
   unsigned            count= 0;    // Number of names on list
   Item*               link;        // Working -> Item

   link= list.getHead();
   while( link != NULL )            // Count the list items
   {
     count++;
     link= link->getNext();
   }

   result= (char**)malloc((count+1)*sizeof(char*)); // Allocate array
   for(count= 0; ; count++)         // Move items from list into array
   {
     link= list.remq();
     if( link == NULL )
       break;

     if( result != NULL )
       result[count]= strdup(link->data.c_str());

     oldItem(link);
   }

   if( result != NULL )
     result[count]= NULL;

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       getUrlPath
//
// Purpose-
//       Get PATH portion of URL
//
//----------------------------------------------------------------------------
static const char*                  // The PATH portion of URL
   getUrlPath(                      // Get PATH portion of URL
     const char*       url)         // The URL string
{
   const char*         C;           // Current character

   C= strchr(url, ':');             // Protocol separator
   if( C == NULL )                  // If missing
     C= url;                        // Just assume omitted
   else
   {
     if( C[1] != '/' || C[2] != '/' )
       return NULL;

     C += 3;
   }

   //-------------------------------------------------------------------------
   // Skip over authority
   C= strchr(C, '/');               // Authority separator

   return C;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       fromHex
//
// Purpose-
//       Convert %xx string into character
//
//----------------------------------------------------------------------------
static int                          // Representative character (256 if invalid)
   fromHex(                         // Do these strings match
     const char*       source)      // Source string ('%' character)
{
   int                 result;      // Resultant

   if( source[1] >= '0' && source[1] <= '9' )
     result= source[1] - '0';
   else if( tolower(source[1]) >= 'a' && tolower(source[1]) <= 'f' )
     result= 10 + tolower(source[1]) - 'a';
   else
     return 256;

   result <<= 4;
   if( source[2] >= '0' && source[2] <= '9' )
     result += source[2] - '0';
   else if( tolower(source[2]) >= 'a' && tolower(source[2]) <= 'f' )
     result += 10 + tolower(source[2]) - 'a';
   else
     return 256;

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       isWildMatch
//
// Purpose-
//       String match, with wildcards
//
//----------------------------------------------------------------------------
static int                          // TRUE if names match
   isWildMatch(                     // Do these strings match
     const char*       ptrQual,     // -> Qualifier name (with wildcards)
     const char*       ptrName)     // -> Objective name
{
   for(;;)
   {
     if( *ptrQual == '\0' )
       break;

     if( *ptrQual == '*' )          // Mutiple character match
     {
       while( *ptrQual == '*' )
         ptrQual++;

       if( *ptrQual == '\0' )
         return TRUE;

       for(;;)
       {
         int altName= *ptrName;
         if( *ptrName == '%' && fromHex(ptrName) < 256 )
           altName= fromHex(ptrName);
         int altQual= *ptrQual;
         if( *ptrQual == '%' && fromHex(ptrQual) < 256 )
           altQual= fromHex(ptrQual);
         while( altQual != altName
             || (altQual == '/' && *ptrQual == '%' && *ptrName == '/' ) )
         {
           if( *ptrName == '\0' )
             return FALSE;

           if( *ptrName == '%' && fromHex(ptrName) < 256 )
             ptrName += 3;
           else
             ptrName++;

           altName= *ptrName;
           if( *ptrName == '%' && fromHex(ptrName) < 256 )
             altName= fromHex(ptrName);
         }

         if( isWildMatch(ptrQual, ptrName) )
           return TRUE;

         if( *ptrName == '%' && fromHex(ptrName) < 256 )
           ptrName += 3;
         else
           ptrName++;
       }
     }
     else if( ptrQual[0] == '$' && ptrQual[1] == '\0' ) // If end of line match
     {
       if( *ptrName == '\0' )
         break;

       return FALSE;
     }

     int altQual= *ptrQual;
     if( *ptrQual == '%' && fromHex(ptrQual) < 256 )
       altQual= fromHex(ptrQual);

     int altName= *ptrName;
     if( *ptrName == '%' && fromHex(ptrName) < 256 )
       altName= fromHex(ptrName);

     if( tolower(altQual) != tolower(altName) ) // If not character match
       return FALSE;

     if( *ptrQual == '%' && fromHex(ptrQual) < 256 )
       ptrQual += 3;
     else
       ptrQual++;

     if( *ptrName == '%' && fromHex(ptrName) < 256 )
       ptrName += 3;
     else
       ptrName++;
   }

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       Robots::~Robots
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Robots::~Robots( void )
{
   IFHCDM( logf("Robots(%p)::~Robots()\n", this); )
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       Robots::Robots
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Robots::Robots( void )           // Default constructor
:  match(NULL), delay(0.0), visit(0)
{
   IFHCDM( logf("Robots(%p)::Robots()\n", this); )
}

   Robots::Robots(                  // Constructor
     const char*       client,      // Client (agent) name
     DataSource&       source)      // ROBOTS.TXT file
:  match(NULL), delay(0.0), visit(0)
{
   IFHCDM( logf("Robots(%p)::Robots(%s,%s)\n", this,
                client, source.getName().c_str()); )
   open(client, source);
}

//----------------------------------------------------------------------------
//
// Method-
//       Robots::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   Robots::debug( void ) const      // Debugging display
{
   debugf("Robots(%p)::debug()\n", this);
   debugf(".. delay: %8.3f\n", delay);
   debugf(".. visit: %8d\n", visit);
   debugf(".. match: %p\n", match);
   if( match != NULL )
   {
     for(int i= 0; match[i] != NULL; i++)
     {
       debugf(".... [%3d] %s: '%s'\n", i,
              match[i][0] == 'A' ? "*ALLOW" : "FORBID", match[i] + 1);
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Robots::allowed
//
// Purpose-
//       Determine whether access is allowed.
//
//----------------------------------------------------------------------------
int                                 // TRUE iff allowed
   Robots::allowed(                 // Is access allowed
     const char*       url) const   // For this URL?
{
   if( match != NULL )
   {
     const char* C= getUrlPath(url);
     if( C == NULL )                // Disallow malformed URL
       return FALSE;

     for(unsigned i= 0; match[i] != NULL; i++)
     {
       char* T= match[i];
       if( isWildMatch(T+1,C) )
       {
         if( *T == 'A' )
           return TRUE;
         return FALSE;
       }
     }
   }

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       Robots::open
//
// Purpose-
//       Load the ROBOTS.TXT file.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Robots::open(                    // Open this Robots
     const char*       client,      // Client (agent) name
     DataSource&       source)      // ROBOTS.TXT file
{
   char*               C;           // Current character
   char                buffer[4096];// Line buffer
   int                 result= 0;   // Resultant
   int                 inAgent= 0;  // Does agent name apply?
   List<Item>          list;        // Our list

   IFHCDM( logf("%d= Robots::open(%s,%s)\n", result, client,
                source.getName().c_str()); )

   // Load the file
   reset();                         // Remove old content
   for(;;)
   {
     int ld= source.getLine(buffer, sizeof(buffer));
     if( ld < 0 && buffer[0] == '\0' )
       break;

     if( ld < DataSource::CC_EOF )
     {
       if( result == 0 )
         logf("Robots::open(%s) Line(%s) ERROR\n",
              source.getName().c_str(), buffer);
       result= 2;

       continue;
     }

     C= strchr(buffer, '#');
     if( C != NULL )
       *C= '\0';

     C= strchr(buffer, ':');
     if( C == NULL )
       continue;

     *C= '\0';
     C++;                           // Skip past inserted NULL
     C= strip(C);                   // Skip blanks, trim trailing blanks
     if( stricmp(buffer, "User-agent") == 0 )
     {
       inAgent= FALSE;
       if( isWildMatch(C, client) )
         inAgent= TRUE;
     }
     else if( stricmp(buffer,"Allow")==0 || stricmp(buffer,"Disallow")==0 )
     {
       if( !inAgent )
         continue;

       if( *C != '/' && *C != '\0' )
       {
         if( result == 0 )
           logf("Robots::open(%s) Line(%s) invalid allow/disallow\n",
                source.getName().c_str(), C);

         result= 1;
         continue;
       }

       Item* item= newItem();
       if (stricmp(buffer,"Allow") == 0 )
         item->data= "A";           // Allowed
       else
         item->data= "F";           // Forbidden
       item->data += C;

       Item* link= list.getHead();
       while( link != NULL )
       {
         if( isWildMatch(link->data.substr(1).c_str(), C) )
           break;

         link= link->getNext();
       }

       if( link != NULL )
       {
         if( isWildMatch(C, ((Item*)link)->data.substr(1).c_str()) )
         {
           //-----------------------------------------------------------------
           // Log this condition in case there's something that can occur
           logf("Robots::open(%s) '%s' equals '%s'\n", source.getName().c_str(),
                ((Item*)link)->data.substr(1).c_str(), C);

           oldItem(item);
         }
         else
         {
           #if( USE_MOST_RESTRICTIVE )
             list.insert(link->getPrev(), item, item);
           #else
             logf("Robots::open(%s) '%s' within '%s'\n", source.getName().c_str(),
                  C, ((Item*)link)->data.substr(1).c_str());
             oldItem(item);
           #endif
         }

         continue;
       }

       list.fifo(item);
     }
     else if( stricmp(buffer, "Crawl-delay") == 0 )
     {
       if( !inAgent )
         continue;

       const char* CC= C;
       delay= parseDouble(CC);
       if( delay < 0.0 )
         delay= 0.0;
     }
     else if( stricmp(buffer, "Request-rate") == 0 )
     {
       if( !inAgent )
         continue;

       const char* CC= C;
       delay= 0.0;
       int d= parseDec(CC);
       if( *CC == '/' )
       {
         CC++;
         int n= parseDec(CC);
         if( *CC == '\0'
             && d > 0 && n > 0 )
           delay= double(n)/double(d);
       }
     }
     else if( stricmp(buffer, "Visit-time") == 0 )
     {
       if( !inAgent )
         continue;

       const char* CC= C;
       visit= 0;
       int from= parseDec(CC);
       if( *CC == '-' )
       {
         CC++;
         int till= parseDec(CC);
         if( *CC == '\0'
             && from >= 0 && from <= 2359
             && till >= 0 && till <= 2359 )
           visit= from * 10000 + till;
       }
     }
     else if( stricmp(buffer, "Sitemap") == 0 )
     {
       continue;                    // IGNORED
     }
     else
     {
       if( result == 0 )
         logf("Robots::open(%s) Line(%s) Unknown directive\n",
              source.getName().c_str(), buffer);
       result= 1;

       continue;
     }
   }

   //-------------------------------------------------------------------------
   // Convert the list into the array of names
   match= convertList(list);

   #if( USE_HCDM_BRINGUPOPEN )
     debug();
   #endif

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Robots::reset
//
// Purpose-
//       Reset this object.
//
//----------------------------------------------------------------------------
void
   Robots::reset( void )            // Reset this object
{
   IFHCDM( logf("Robots(%p)::reset()\n", this); )

   if( match != NULL )
   {
     for(unsigned i= 0; match[i] != NULL; i++)
     {
       free(match[i]);
     }

     free(match);
     match= NULL;
   }

   delay= 0.0;
   visit= 0;
}

