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
//       Tester.cpp
//
// Purpose-
//       Wilbur bringup utilities and unit tests.
//
// Last change date-
//       2010/01/01
//
// Content-
//        --test            (Run simpleTest)
//        --testApprox      (Test Approximately.h)
//        --listDB database (List database)
//                          DbAttr, DbHttp, DbText, DbWord
//        --loadDB database (Load database, inp/database.inp == database)
//                          DbWord
//        --testDB database (Test database)
//                          DbAttr, DbHttp, DbText, DbWord
//        --testDate        (Test DateParser.h)
//        --testHtml file   (Test HtmlParser.h)
//        --testObject      (Test Object.h)
//        --testRobots      (Test Robots.h)
//        --testUrl  item   (Test Url.h)
//        --verify filename (Verify words in filename)
//
//----------------------------------------------------------------------------
#define __STDC_FORMAT_MACROS        // For linux inttypes.h
#include <iostream>
#include <string>

#include <assert.h>
#include <inttypes.h>               // For PRI*64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//nclude "com/DebugObject.h"        // For debugging
#include <com/Debug.h>
#include <com/Dispatch.h>
#include <com/FileSource.h>
#include <com/Interval.h>
#include <com/Random.h>
#include <com/Signal.h>
#include <com/Thread.h>
#include <sys/stat.h>

#include <exception>
#include <string>

#include "Approximately.h"
#include "DateParser.h"
#include "DbAttr.h"
#include "DbHttp.h"
#include "DbWord.h"
#include "DbText.h"
#include "HtmlNode.h"
#include "HtmlParser.h"
#include "HtmlNodeVisitor.h"
#include "ObjectList.h"
#include "Properties.h"
#include "Robots.h"
#include "Url.h"

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
// Internal data areas
//----------------------------------------------------------------------------
static unsigned        testObjectCount= 0; // Number of allocated TestObjects

//----------------------------------------------------------------------------
// Internal classes (for testObject)
//----------------------------------------------------------------------------
class TestObject : public Object {  // Our TestObject
public:
char                   prefix[8];   // Prefix
ObjectList             list;        // Our ObjectList
char                   suffix[8];   // Suffix

public:
virtual
   ~TestObject( void )              // Destructor
{
   IFHCDM( debugf("TestObject(%p)::~TestObject()\n", this); )

   --testObjectCount;
}

   TestObject( void )               // Constructor
:  Object(), list()
{
   IFHCDM( debugf("TestObject(%p)::TestObject()\n", this); )

   ++testObjectCount;
   memset(prefix, 0, sizeof(prefix));
   memset(suffix, 0, sizeof(suffix));
   strcpy(prefix, "prefix");
   strcpy(suffix, "suffix");
}
}; // class TestObject

class TestObjectREF : public Object {
public:
Ref<TestObject>        ref;

virtual
   ~TestObjectREF( void )           // Destructor
{
   IFHCDM(
     debugf("TestObjectREF(%p)::~TestObjectREF() %p\n", this, ref.get());
   )

   --testObjectCount;
}

   TestObjectREF(                   // Constructor
     TestObject*       object)
:  Object(), ref(object)
{
   IFHCDM( debugf("TestObjectREF(%p)::TestObjectREF(%p)\n", this, object); )

   ++testObjectCount;
}
}; // class TestObjectREF

//----------------------------------------------------------------------------
//
// Class-
//       MetaVisitor
//
// Purpose-
//       Visit the HTML nodes, extracting META entries.
//
//----------------------------------------------------------------------------
class MetaVisitor : public HtmlNodeVisitor {
//----------------------------------------------------------------------------
// MetaVisitor::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (0 OK)
   visit(                           // Visit text HtmlNodes
     HtmlNode*         node)        // -> HtmlNode
{
   #if FALSE
     debugf("MetaVisitor::visit() ");
     debugf("type(%d) ", node->getType());
     debugf("name(%s) ", node->getName().c_str());
     debugf("data(%s) ", node->getData().c_str());
     debugf("\n");
   #endif

   // Look for META element nodes
   if( node->getType() == HtmlNode::TYPE_ELEM // If ELEMENT Node
       && stricmp("meta", node->getName().c_str()) == 0 ) // With META tag
   {
     ElemHtmlNode* elem= dynamic_cast<ElemHtmlNode*>(node);
     assert( elem != NULL );        // INTERNAL ERROR if triggered

     HtmlNode* child= elem->getChild();

     for(;;)                        // Get http-equiv/content value pairs
     {
       while( child != NULL && child->getType() != HtmlNode::TYPE_ATTR )
         child= child->getPeer();

       if( child == NULL )
         break;

       if( stricmp("http-equiv", child->getName().c_str()) != 0 )
       {
         debugf("META: expected(http-equiv), got(%s), \n",
                child->getName().c_str());
         break;
       }

       AttrHtmlNode* attr= dynamic_cast<AttrHtmlNode*>(child);
       assert( attr != NULL );      // INTERNAL ERROR if triggered

       std::string name= attr->getData(); // The http-equiv commmand

       child= child->getPeer();
       while( child != NULL && child->getType() != HtmlNode::TYPE_ATTR )
         child= child->getPeer();

       if( child == NULL )
       {
         debugf("META: missing(content) attribute\n");
         break;
       }

       if( stricmp("content", child->getName().c_str()) != 0 )
       {
         debugf("META: expected(content), got(%s), \n",
                child->getName().c_str());
         break;
       }

       attr= dynamic_cast<AttrHtmlNode*>(child);
       assert( attr != NULL );      // INTERNAL ERROR if triggered

       std::string value= attr->getData(); // The content value
       debugf("Found (%s:%s)\n", name.c_str(), value.c_str());

       child= child->getPeer();
     }
   }

   return 0;
}
}; // class MetaVisitor

//----------------------------------------------------------------------------
//
// Subroutine-
//       listDbAttr
//
// Purpose-
//       List DbAttr database.
//
//----------------------------------------------------------------------------
static void
   listDbAttr( void )               // List database
{
   struct Attribute {               // The Attribute key/value structure
   uint32_t            key;         // Key (attribute)
   uint64_t            value;       // Value (or link)
   }; // struct Attribute

   DbAttr              dbAttr;      // The database
   uint64_t            assoc;       // Assoc buffer
   uint64_t            x, y;        // Working indexes

   debugf("listDbAttr()\n");

   //-------------------------------------------------------------------------
   // Retrieve all indexes
   debugf("\n");
   x= 0;
   for(;;)
   {
     y= x;
     x= dbAttr.nextIndex(y);
     debugf("%.16" PRIx64 "= nextIndex(%.16" PRIx64 ")", x, y);
     if( x != 0 )
     {
       Attribute record[256];
       int count= dbAttr.getRecord(x, record, sizeof(record));
       count /= sizeof(Attribute);
       for(int i= 0; i<count; i++)
         debugf(" {%2d, %.16" PRIx64 "}", record[i].key, record[i].value);
     }
     debugf("\n");

     if( x == 0 )
       break;
   }

   //-------------------------------------------------------------------------
   // Retrieve all values
   debugf("\n");
   x= 0;
   for(;;)
   {
     y= x;
     x= dbAttr.nextValue(y);
     assoc= dbAttr.getValue(x);
     debugf("%.16" PRIx64 "= nextValue(%.16" PRIx64 ") %.16" PRIx64 "\n",
            x, y, assoc);

     if( x == 0 )
       break;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       listDbHttp
//
// Purpose-
//       List DbHttp database entry.
//
//----------------------------------------------------------------------------
static void
   listDbHttp(                      // List database entry
     uint64_t          httpIX,      // The database index
     DbHttp::Value*    value)       // The database entry
{
   debugf("[%.16" PRIx64 "] ", httpIX);
   if( value == NULL )
     debugf("<NULL>\n");
   else
     debugf("time(%16" PRId64 ") link(%.16" PRIx64 ") http(http://%s)\n",
            DbBase::fetch64(&value->time), DbBase::fetch64(&value->text),
            value->name);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       listDbHttp
//
// Purpose-
//       List DbHttp database.
//
//----------------------------------------------------------------------------
static void
   listDbHttp( void )               // List database
{
   char                buffer[DbHttp::MAX_VALUE_LENGTH+1]; // Name buffer
   DbHttp              dbHttp;      // The database
   uint64_t            time;        // Working expiration time
   DbHttp::Value*      value;       // (Normally buffer)
   uint64_t            x, y;        // Working indexes

   debugf("listDbHttp()\n");

   //-------------------------------------------------------------------------
   // Retrieve all indexes
   debugf("\n");
   debugf("listDbHttp by Index\n");
   x= 0;
   do
   {
     y= dbHttp.nextIndex(x);
     value= NULL;
     if( y != 0 )
       value= dbHttp.getValue(buffer, y);

     debugf("[%.16" PRIx64 "] -> ", x);
     listDbHttp(y, value);

     x= y;
   } while( x != 0 );

   //-------------------------------------------------------------------------
   // Retrieve all names
   debugf("\n");
   debugf("listDbHttp by Name\n");
   value= (DbHttp::Value*)buffer;
   value->name[0]= '\0';
   for(;;)
   {
     x= dbHttp.nextName(value->name);
     if( x == 0 )
       break;

     value= dbHttp.getValue(buffer, x);
     if( value == NULL )
       throwf("%4d value(NULL)", __LINE__);

     listDbHttp(x, value);
   }
   debugf("[%.16" PRIx64 "]\n", int64_t(0));

   //-------------------------------------------------------------------------
   // Retrieve all times
   debugf("\n");
   debugf("listDbHttp by Time\n");
   value= (DbHttp::Value*)buffer;
   x= 0;
   time= 0;
   for(;;)
   {
     y= dbHttp.nextTime(x, time);
     if( y == 0 )
       break;

     value= dbHttp.getValue(buffer, y);
     if( value == NULL )
       throwf("%4d value(NULL)", __LINE__);

     listDbHttp(y, value);

     x= y;
     time= dbHttp.fetch64(&value->time);
   }
   debugf("[%.16" PRIx64 "]\n", int64_t(0));
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       listDbText
//
// Purpose-
//       List DbText database.
//
//----------------------------------------------------------------------------
static void
   listDbText( void )               // List database
{
   DbText              dbText;      // The database
   uint64_t            x, y;        // Working indexes

   debugf("listDbText()\n");

   //-------------------------------------------------------------------------
   // Retrieve all indexes
   debugf("\n");
   x= 0;
   for(;;)
   {
     y= dbText.nextIndex(x);
     debugf("%.16" PRIx64 "= nextIndex(%.16" PRIx64 ")\n", y, x);
     if( y == 0 )
       break;

     char* C= dbText.getValue(y);
     if( C == NULL )
       debugf("<NULL TEXT>\n");
     else
     {
       debugf("%s\n", C);
       free(C);
     }

     x= y;
   }

   debugf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       listDbWord
//
// Purpose-
//       List DbWord database.
//
//----------------------------------------------------------------------------
static void
   listDbWord( void )               // List database
{
   DbWord              dbWord;      // The DbWord database

   char                value[DbWord::MAX_VALUE_LENGTH+1];  // Value buffer
   uint32_t            index;       // Index buffer

   // List DbWord file by value
   debugf("listDbWord()\n");
   strcpy(value, "");
   for(;;)
   {
     char* result= dbWord.nextValue(value, &index);
     if( result == NULL )
       break;

     tracef("%.8x (%s)\n", index, value);
   }

   // List DbWord file by index
   tracef("..By index\n");
   index= 0;
   for(;;)
   {
     index= dbWord.nextIndex(index, value);
     if( index == 0 )
       break;

     tracef("%.8x (%s)\n", index, value);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       listDB
//
// Purpose-
//       List database.
//
//----------------------------------------------------------------------------
static void
   listDB(                          // Load database
     const char*       dbName)      // The database name
{
   if( strcmp(dbName, "DbAttr") == 0 )
     listDbAttr();
   else if( strcmp(dbName, "DbHttp") == 0 )
     listDbHttp();
   else if( strcmp(dbName, "DbText") == 0 )
     listDbText();
   else if( strcmp(dbName, "DbWord") == 0 )
     listDbWord();
   else
     fprintf(stderr, "Unknown DB(%s)\n", dbName);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       loadDbWord
//
// Purpose-
//       Load DbWord database.
//
//----------------------------------------------------------------------------
static void
   loadDbWord( void )               // Load database
{
   DbWord*             dbWord= NULL;// The DbWord database
   int                 errorCount= 0; // Error counter
   char                value[DbWord::MAX_VALUE_LENGTH+1];  // Value buffer

   int                 rc;

   // Load database file
   debugf("loadDbWord()\n");
   FileSource source("inp/DbWord.inp");
   for(int count= 0;; count++)
   {
     int x= 0;
     int C= source.get();
     if( C == EOF )
       break;

     while( C != '\n' && C != EOF )
     {
       if( C != '\r' )
       {
         if( x < (sizeof(value)-1) )
           value[x++]= C;
       }

       C= source.get();
     }
     value[x]= '\0';

     if( value[0] == '#' )
       continue;

     if( value[0] == '_' )
     {
       if( dbWord != NULL )
       {
         delete dbWord;
         dbWord= NULL;
       }
     }

     if( dbWord == NULL )
     {
       dbWord= new DbWord(value);
       continue;
     }

     rc= dbWord->insert(value);
     if( rc == 0 )
     {
       debugf("%.8x= insert(%s)\n", rc, value);
       errorCount++;
       if( errorCount > 5 )
         break;
     }
     else
       tracef("%.8x= insert(%s)\n", rc, value);
   }

   if( dbWord != NULL )
     delete dbWord;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       loadDB
//
// Purpose-
//       Load database. (Currently only DbWord)
//
//----------------------------------------------------------------------------
static void
   loadDB(                          // Load database
     const char*       dbName)      // The database name
{
   if( strcmp(dbName, "DbWord") == 0 )
     loadDbWord();
   else
     fprintf(stderr, "Unknown DB(%s)\n", dbName);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testApprox
//
// Purpose-
//       Test Approximately object.
//
//----------------------------------------------------------------------------
static void
   testApprox( void )               // Test Approximately object
{
   unsigned int        ITERATIONS= 33554432; // Iteration counter

   Random::standard.randomize();    // Randomize the Random object
   srand(time(NULL));               // Randomize the system object

   Approximately a;
   Approximately b;
   Approximately c;
   Approximately d;

   for(unsigned int i= 0; i<ITERATIONS; i++)
   {
     a.event();

     if( (i & 1) == 0 )
       b.event();

     if( (i & 3) == 0 )
       c.event();

     if( (i & 7) == 0 )
       d.event();
   }

   debugf("a: Expected(%8u) Actual: %9u\n", ITERATIONS/1, a.getCount());
   debugf("b: Expected(%8u) Actual: %9u\n", ITERATIONS/2, b.getCount());
   debugf("c: Expected(%8u) Actual: %9u\n", ITERATIONS/4, c.getCount());
   debugf("d: Expected(%8u) Actual: %9u\n", ITERATIONS/8, d.getCount());

   #if FALSE
     debugf("\n");
     Approximately e(4095);         //    4095
     Approximately f(4096);         //    8091
     Approximately g(500000);       //  524287
     Approximately h(1000000);      // 1048575
     debugf("e: %u\n", e.getCount());
     debugf("f: %u\n", f.getCount());
     debugf("g: %u\n", g.getCount());
     debugf("h: %u\n", h.getCount());

     debugf("\n");
     debugf("%ld= sizeof(Approximately)\n", (long)sizeof(Approximately));
     debugf("%ld= sizeof(a)\n", (long)sizeof(a));
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testDateParser
//
// Purpose-
//       Test DateParser object.
//
//----------------------------------------------------------------------------
static void
   testDateParser( void )           // Test DateParser object
{
   std::string         s;           // Working std::string
   time_t              tod;         // Working time_t

   s= DateParser::generate(time(NULL));
   printf("%s\n", s.c_str());

   tod= DateParser::parse("Sun, 06 Nov 1994 08:49:37 GMT");
   s= DateParser::generate(tod);
   if( s != "Sun, 06 Nov 1994 08:49:37 GMT" )
     printf("%4d ShouldNotOccur(%s)\n", __LINE__, s.c_str());

   tod= DateParser::parse("Sunday, 06-Nov-94 08:49:37 GMT");
   s= DateParser::generate(tod);
   if( s != "Sun, 06 Nov 1994 08:49:37 GMT" )
     printf("%4d ShouldNotOccur(%s)\n", __LINE__, s.c_str());

   tod= DateParser::parse("Sun Nov  6 08:49:37 1994");
   s= DateParser::generate(tod);
   if( s != "Sun, 06 Nov 1994 08:49:37 GMT" )
     printf("%4d ShouldNotOccur(%s)\n", __LINE__, s.c_str());

   tod= DateParser::parse("This is bogus");
   s= DateParser::generate(tod);
   if( s == "Sun, 06 Nov 1994 08:49:37 GMT" )
     printf("%4d ShouldNotOccur(%s)\n", __LINE__, s.c_str());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testHtmlParser
//
// Purpose-
//       Test HtmlParser object.
//
//----------------------------------------------------------------------------
static void
   testHtmlParser(                  // Test HtmlParser object
     const char*       fileName)    // For this file name
{
   FileSource          source(fileName);
   HtmlParser          parser;
   MetaVisitor         visitor;

   int                 rc;

   rc= parser.parse(source);
   #if FALSE
     debugf("%d= HTMLparser.parse(%s)\n", rc, fileName);
     parser.debug();
   #endif

   if( rc == 0 )
     parser.getRoot()->visit(visitor);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testDbAttr
//
// Purpose-
//       Test DbAttr database.
//
//----------------------------------------------------------------------------
static void
   testDbAttr( void )               // Test DbAttr database
{
   uint64_t            index[8];    // Index buffer
   uint64_t            assoc;       // Assoc buffer
   uint64_t            x, y;        // Working indexes

   int                 rc;
   int                 i, j;

   debugf("testDbAttr()\n");

   //-------------------------------------------------------------------------
   // Insert test data
   DbAttr              dbAttr;      // The database
   index[0]= dbAttr.insert(0xfedcba9876543210ULL);
   dbAttr.setAssoc(index[0], 1, 0xfedcba9876543210ULL);

   index[1]= dbAttr.insert(0x0123456789abcdefULL);
   dbAttr.setAssoc(index[1], 2, 0x0123456789abcdefULL);

   index[2]= dbAttr.insert(0x00fe00000000fefeULL);
   dbAttr.setAssoc(index[2], 1, 0xfe11111111111111ULL);
   dbAttr.setAssoc(index[2], 2, 0xfe22222222222222ULL);
   dbAttr.setAssoc(index[2], 0, index[2]);

   index[3]= dbAttr.insert(0x00fe333333333333ULL);
   dbAttr.setAssoc(index[3], 1, 0xfe00000000000001ULL);
   dbAttr.setAssoc(index[3], 2, 0xfe00000000000002ULL);
   dbAttr.setAssoc(index[3], 1, 0xfe11111111111111ULL);
   dbAttr.setAssoc(index[3], 2, 0xfe22222222222222ULL);

   index[4]= dbAttr.insert(0x00fe444444444444ULL);

   index[5]= dbAttr.insert(0x00fe555555555555ULL);
   dbAttr.setAssoc(index[5], 0, index[5]);
   dbAttr.setAssoc(index[5], 1, 0xfe11111111111111ULL);

   index[6]= dbAttr.insert(0x00fe666666666666ULL);
   dbAttr.setAssoc(index[6], 1, 0xfe11111111111111ULL);

   index[7]= dbAttr.insert(0x00fe777777777777ULL);
   dbAttr.setAssoc(index[7], 1, 0xfe77777777777777ULL);

   //-------------------------------------------------------------------------
   // Test insert resultants
   for(i=0; i<8; i++)
   {
     debugf("[%2d] %.16" PRIx64 "\n", i, index[i]);
     if( index[i] == 0 )
     {
       debugf("!!ERROR!!\n");
       return;
     }
   }

   //-------------------------------------------------------------------------
   // Test remove
// rc= dbAttr.remAssoc(index[2], 0);
// if( rc != 0 )
//   debugf("%4d !!ERROR!!\n", __LINE__);

   rc= dbAttr.remove(index[1]);
   if( rc != 0 )
     debugf("%4d !!ERROR!!\n", __LINE__);

   rc= dbAttr.remove(index[1]);
   if( rc == 0 )
     debugf("%4d !!ERROR!!\n", __LINE__);

   rc= dbAttr.remove(0);
   if( rc == 0 )
     debugf("%4d !!ERROR!!\n", __LINE__);

   rc= dbAttr.remove(1);
   if( rc == 0 )
     debugf("%4d !!ERROR!!\n", __LINE__);

   //-------------------------------------------------------------------------
   // Retrieve test data
   debugf("\n");
   for(i=0; i<8; i++)
   {
     uint64_t result= dbAttr.getValue(index[i]);
     debugf("[%2d] %.16" PRIx64 "= getValue(%.16" PRIx64 ")\n", i,
            result, index[i]);
   }

   debugf("\n");
   for(i=0; i<8; i++)
   {
     for(j=0; j<3; j++)
     {
       assoc= dbAttr.getAssoc(index[i], j);
       debugf("[%2d] %.16" PRIx64 "= getAssoc(%.16" PRIx64 ",%d)\n",
              i, assoc, index[i], j);
     }
   }

   //-------------------------------------------------------------------------
   // Retrieve all indexes
   debugf("\n");
   x= 0;
   for(;;)
   {
     y= x;
     x= dbAttr.nextIndex(y);
     debugf("%.16" PRIx64 "= nextIndex(%.16" PRIx64 ")\n", x, y);

     if( x == 0 )
       break;
   }

   //-------------------------------------------------------------------------
   // Retrieve all values
   debugf("\n");
   x= 0;
   for(;;)
   {
     y= x;
     x= dbAttr.nextValue(y);
     assoc= dbAttr.getValue(x);
     debugf("%.16" PRIx64 "= nextValue(%.16" PRIx64 ") %.16" PRIx64 "\n",
            x, y, assoc);

     if( x == 0 )
       break;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testDbHttp
//
// Purpose-
//       Test DbHttp database.
//
//----------------------------------------------------------------------------
static void
   testDbHttp( void )               // Test DbHttp database
{
   DbHttp              dbHttp;      // The database
   char                buffer[DbHttp::MAX_VALUE_LENGTH+1]; // Value buffer
   uint64_t            httpIX;      // Working index
   uint64_t            index[8];    // Index buffer
   DbHttp::Value*      value;       // Formatted buffer

   int                 rc;
   int                 i;

   debugf("testDbHttp()\n");

   //-------------------------------------------------------------------------
   // Insert test data
   value= (DbHttp::Value*)buffer;

   DbHttp::setValue(buffer,
                    0xfedcba9811111111ULL, 0x0000000033333333ULL,
                    "www.nada.com/fe01");
   index[0]= dbHttp.insert(value);

   DbHttp::setValue(buffer,
                    0xfedcba9811111112ULL, 0x0000000033333333ULL,
                    "www.nada.com/01fe");
   index[1]= dbHttp.insert(value);

   DbHttp::setValue(buffer,
                    0xfedcba9822222221ULL, 0x0000000022222222ULL,
                    "www.nada.com/2221");
   index[2]= dbHttp.insert(value);

   DbHttp::setValue(buffer,
                    0xfedcba9822222222ULL, 0x0000000011111111ULL,
                    "www.nada.com/2222");
   index[3]= dbHttp.insert(value);

   strcpy(value->name, "2222");
   index[4]= dbHttp.insert(value);

   strcpy(value->name, "2222/1");
   index[5]= dbHttp.insert(value);

   strcpy(value->name, "2222/2");
   index[6]= dbHttp.insert(value);

   strcpy(value->name, "zzz.last");
   index[7]= dbHttp.insert(value);

   //-------------------------------------------------------------------------
   // Test insert resultants
   for(i=0; i<8; i++)
   {
     debugf("[%2d] %.16" PRIx64 "\n", i, index[i]);
     if( index[i] == 0 )
     {
       debugf("!!ERROR!!\n");
       return;
     }
   }

   //-------------------------------------------------------------------------
   // Test remove
   rc= dbHttp.remove(index[3]);
   if( rc != 0 )
     debugf("%4d !!ERROR!!\n", __LINE__);

   rc= dbHttp.remove(index[3]);
   if( rc == 0 )
     debugf("%4d !!ERROR!!\n", __LINE__);

   rc= dbHttp.remove(0);
   if( rc == 0 )
     debugf("%4d !!ERROR!!\n", __LINE__);

   //-------------------------------------------------------------------------
   // Test revise
   DbHttp::setValue(buffer,
                    0xfedcba9833333333ULL, 0x0000000033333333ULL,
                    "changed.from/2222");
   rc= dbHttp.revise(index[4], value);
   if( rc != 0 )
     debugf("%4d !!ERROR!!\n", __LINE__);

   DbHttp::setValue(buffer,
                    0xfedcba9833333333ULL, 0x0000000044444444ULL,
                    "changed.from/2222");
   rc= dbHttp.revise(index[4], value);
   if( rc != 0 )
     debugf("%4d !!ERROR!!\n", __LINE__);

   //-------------------------------------------------------------------------
   // Timing test
   DbHttp::setValue(buffer,
                    0xfedcba9844444444ULL, 0ULL,
                    "2222/2");

   Interval interval;
   debugf("Timer interval...\n");
   for(i= 1; i<=1000; i++)
   {
     dbHttp.store64(&value->time, i);
     rc= dbHttp.revise(index[6], value);
     if( rc != 0 )
     {
       debugf("!!ERROR!! revise(%d)\n", i);
       return;
     }
   }
   debugf("...Timer interval\n");

   interval.stop();
   debugf("%.3f seconds (%.3f per second)\n", interval.toDouble(),
          double(i) / interval.toDouble());

   //-------------------------------------------------------------------------
   // Test locate
   httpIX= dbHttp.locate("www.nada.com/01fe"); // Locate index[1]
   if( httpIX != index[1] )
     debugf("%4d !!ERROR!! locate(%s) expected(%.16" PRIx64 ") got(%.16" PRIx64 "\n",
            __LINE__, "www.nada.com/01fe", index[1], httpIX);

   httpIX= dbHttp.locate("www.nada.com/2221"); // Locate index[2]
   if( httpIX != index[2] )
     debugf("%4d !!ERROR!! locate(%s) expected(%.16" PRIx64 ") got(%.16" PRIx64 "\n",
            __LINE__, "www.nada.com/2221", index[2], httpIX);

   httpIX= dbHttp.locate("www.nada.com/2222"); // Locate index[3], removed
   if( httpIX != 0 )
     debugf("%4d !!ERROR!! locate(%s) expected(%.16" PRIx64 ") got(%.16" PRIx64 "\n",
            __LINE__, "www.nada.com/2222", int64_t(0), httpIX);

   httpIX= dbHttp.locate("changed.from/2222"); // Locate index[4], revised
   if( httpIX != index[4] )
     debugf("%4d !!ERROR!! locate(%s) expected(%.16" PRIx64 ") got(%.16" PRIx64 "\n",
            __LINE__, "changed.from/2222", index[4], httpIX);

   //-------------------------------------------------------------------------
   // Retrieve (all) data
   if( TRUE  )                      // Opt in/out
     listDbHttp();

   //-------------------------------------------------------------------------
   // Remove test data
   if( TRUE  )                      // Opt in/out
   {
     for(i=0; i<8; i++)
       dbHttp.remove(index[i]);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testDbText
//
// Purpose-
//       Test DbText database.
//
//----------------------------------------------------------------------------
static void
   testDbText( void )               // Test DbText database
{
   DbText              dbText;      // The database
   uint64_t            index[8];    // Index buffer

   int                 rc;
   int                 i;

   debugf("testDbText()\n");

   //-------------------------------------------------------------------------
   // Insert test data
   index[0]= dbText.insert("Text file 0");
   index[1]= dbText.insert("This is Text file 1");
   index[2]= dbText.insert("This is Text file 2");
   index[3]= dbText.insert("This is Text file 3");
   index[4]= dbText.insert("This is Text file 4");
   index[5]= dbText.insert("This is Text file 5");
   index[6]= dbText.insert("This is Text file 6");
   index[7]= dbText.insert("This is Text file 7");

   //-------------------------------------------------------------------------
   // Test insert resultants
   for(i=0; i<8; i++)
   {
     debugf("[%2d] %.16" PRIx64 "\n", i, index[i]);
     if( index[i] == 0 )
     {
       debugf("!!ERROR!!\n");
       return;
     }
   }

   //-------------------------------------------------------------------------
   // Test remove
   rc= dbText.remove(index[3]);
   if( rc != 0 )
     debugf("%4d !!ERROR!!\n", __LINE__);

   rc= dbText.remove(0);
   if( rc == 0 )
     debugf("%4d !!ERROR!!\n", __LINE__);

   rc= dbText.remove(1);
   if( rc == 0 )
     debugf("%4d !!ERROR!!\n", __LINE__);

   //-------------------------------------------------------------------------
   // Test revise
   char buffer[128];
   Interval interval;
   debugf("Timer interval...\n");
   for(i= 1; i<=1000; i++)
   {
     sprintf(buffer, "Replacement value %d", i);
     rc= dbText.revise(index[4], buffer);
     if( rc != 0 )
     {
       debugf("!!ERROR!! revise(%d)\n", i);
       return;
     }
   }
   debugf("...Timer interval\n");

   interval.stop();
   debugf("%.3f seconds (%.3f per second)\n", interval.toDouble(),
          double(i) / interval.toDouble());

   //-------------------------------------------------------------------------
   // Retrieve test data
   debugf("\n");
   for(i=0; i<8; i++)
   {
     char* result= dbText.getValue(index[i]);
     debugf("[%2d] getValue(%.16" PRIx64 ") %s\n",
            i, index[i], result);
     if( result != NULL )
       free(result);
   }

   //-------------------------------------------------------------------------
   // Retrieve (all) data
   if( TRUE  )                      // Opt in/out
     listDbText();

   //-------------------------------------------------------------------------
   // Remove test data
   if( TRUE  )                      // Opt in/out
   {
     for(i=0; i<8; i++)
       dbText.remove(index[i]);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       dbWordTest
//
// Purpose-
//       Test DbWord database word.
//
//----------------------------------------------------------------------------
static void
   dbWordTest(                      // Test database word
     DbWord&           dbWord,      // Database reference
     const char*       lang,        // Associated language
     const char*       word)        // Word
{
   char   value[DbWord::MAX_VALUE_LENGTH+1];

   int rc= dbWord.getIndex(word);
   if( rc != 0 )
   {
     char* drow= dbWord.getValue(rc, value);
     debugf("%.8x= %s.getIndex(%s), %s= getValue(%.8x)\n",
            rc, lang, word, drow, rc);
   }
   else
     debugf("%.8x= %s.getIndex(%s)\n", rc, lang, word);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testDbWord
//
// Purpose-
//       Test DbWord database. (Works best after loadDbWord.)
//
//----------------------------------------------------------------------------
static void
   testDbWord( void )               // Test DbWord database
{
   debugf("testDbWord()\n");

   DbWord _en("_en");
   DbWord _es("_es");
   DbWord _fr("_fr");

   //-------------------------------------------------------------------------
   // Test language differentiation
   dbWordTest(_en, "_en", "la");
   dbWordTest(_en, "_en", "xxyyz");
   dbWordTest(_en, "_en", "zymurgy");

   tracef("\n");
   dbWordTest(_es, "_es", "la");
   dbWordTest(_es, "_es", "xxyyz");
   dbWordTest(_es, "_es", "zymurgy");

   tracef("\n");
   dbWordTest(_fr, "_fr", "la");
   dbWordTest(_fr, "_fr", "xxyyz");
   dbWordTest(_fr, "_fr", "zymurgy");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testDB
//
// Purpose-
//       Test database.
//
//----------------------------------------------------------------------------
static void
   testDB(                          // Test database
     const char*       dbName)      // The database name
{
   if( dbName == NULL )
     fprintf(stderr, "Missing DB name\n");
   else if( strcmp(dbName, "DbAttr") == 0 )
     testDbAttr();
   else if( strcmp(dbName, "DbHttp") == 0 )
     testDbHttp();
   else if( strcmp(dbName, "DbText") == 0 )
     testDbText();
   else if( strcmp(dbName, "DbWord") == 0 )
     testDbWord();
   else
     fprintf(stderr, "Unknown DB(%s)\n", dbName);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testObject
//
// Purpose-
//       Test Object.h.
//
//----------------------------------------------------------------------------
static void
   testObject( void )               // Test Object.h
{
{{{{ // For cascade delete test
   Object o1;
   Object o2;
   Ref<String> r1= new String("SourceString");
   Ref<String> r2= new String("ObjectString");
   String& s1= *r1;
   String& s2= *r2;

   if( o1.compare(o2) == 0 )
     printf("%4d Should Not Occur\n", __LINE__);

   if( o1.compare(s1) == 0 )
     printf("%4d Should Not Occur\n", __LINE__);

// if( s1.compare(o1) == 0 )        // CompareCastException
//   printf("%4d Should Not Occur\n", __LINE__);

   if( s1 == s2 )
     printf("%4d Should Not Occur\n", __LINE__);

   if( s1.compare(s2) <= 0 )
     printf("%4d Should Not Occur\n", __LINE__);

   cout << "'Object.toString()'" << " : '" << o1.toString() << "'" << endl;
   cout << "'Source.toString()'" << " : '" << s1.toString() << "'" << endl;

   cout << "'SourceString'" << " : '" << s1 << "'" << endl;
   cout << "'ObjectString'" << " : '" << s2 << "'" << endl;

   o2= o1;
   s2= s1;
   if( s1 != s2 )
     printf("%4d Should Not Occur\n", __LINE__);

   if( s1.compare(s2) != 0 )
     printf("%4d Should Not Occur\n", __LINE__);

   if( strcmp("SourceString", s1.c_str()) != 0 )
     printf("%4d Should Not Occur\n", __LINE__);

   // Test String operator[]
   printf("String operator[]: '");
   for(int i= 0; i<s1.length(); i++)
     printf("%c", s1[i]);
   printf("'\n");

   //-------------------------------------------------------------------------
   // Test cascade delete
   printf("Cascade delete preparation...\n");
   ObjectList top;
   for(int i= 0; i<10; i++)
   {
     TestObject* ti= new TestObject();
     TestObjectREF* ri= new TestObjectREF(ti);
     top.lifo(ti);
     top.lifo(ri);
     for(int j=0; j<(i+1); j++)
     {
       TestObject* tj= new TestObject();
       TestObjectREF* rj= new TestObjectREF(tj);
       ti->list.fifo(tj);
       ti->list.fifo(rj);
     }
   }

   if( testObjectCount == 0 )
     printf("%4d ERROR, testObjectCount(%d)\n", __LINE__, testObjectCount);
   if( Object::getObjectCounter() == 0 )
     printf("%4d ERROR, Object::objectCount(%d)\n", __LINE__,
            Object::getObjectCounter());

   printf("Cascade delete...\n");
}}}} // Begin cascade delete

   if( testObjectCount != 0 )
     printf("%4d ERROR, testObjectCount(%d)\n", __LINE__, testObjectCount);
   if( Object::getObjectCounter() != 0 )
     printf("%4d ERROR, Object::objectCount(%d)\n", __LINE__,
            Object::getObjectCounter());
   printf("...Cascade delete\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       expectRobots
//
// Purpose-
//       Test Robots.h
//
//----------------------------------------------------------------------------
static int                          // Error count
   expectRobots(                    // Test Robots.h
     int               xallow,      // Expect allowed?
     Robots&           robots,      // Robots control
     const char*       url)         // Test Url
{
   int                 errorCount= 0; // Resultant

   if( xallow != robots.allowed(url) )
   {
     errorCount++;
     if( xallow == FALSE )
       debugf("%4d ERROR: allowed disallowed(%s) \n", __LINE__, url);
     else
       debugf("%4d ERROR: disallowed allowed(%s)\n", __LINE__, url);
   }

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testRobots
//
// Purpose-
//       Test Robots.h
//
//----------------------------------------------------------------------------
static void
   testRobots( void )               // Test Robots.h
{
   FileSource          fileSource("out/google.com-robots.txt");
   Robots              robots("Wilbur", fileSource);

   debugf("Request delay: %.3f\n", robots.getDelay());
   debugf("Request times: %.4d-%.4d\n",
          robots.getVisit()/10000, robots.getVisit()%10000);
   robots.debug();

   expectRobots( TRUE, robots, "/index.html");

   expectRobots(FALSE, robots, "/search");
   expectRobots(FALSE, robots, "/search/foo");

   expectRobots( TRUE, robots, "/toolkit/foobar.html");
   expectRobots(FALSE, robots, "/toolkit/foobar.htmlx");
   expectRobots(FALSE, robots, "/toolkit/foobar.htm");

   expectRobots(FALSE, robots, "/news");
   expectRobots(FALSE, robots, "/news/foo");
   expectRobots( TRUE, robots, "/news/directory");

   expectRobots(FALSE, robots, "/?");
   expectRobots(FALSE, robots, "/?A");
   expectRobots( TRUE, robots, "/pagead");
   expectRobots(FALSE, robots, "/pagead/");
   expectRobots(FALSE, robots, "/pagead/data");

   expectRobots(FALSE, robots, "/patents/mypatent.html");
   expectRobots( TRUE, robots, "/patents/about");
   expectRobots( TRUE, robots, "/patents/about.html");

   expectRobots( TRUE, robots, "/booksrightsholders");
   expectRobots( TRUE, robots, "/booksrightsholders/data");

   expectRobots(FALSE, robots, "/profiles/me/mystuff.htm");
   expectRobots( TRUE, robots, "/profiles/you/yourstuff.htm");
   expectRobots(FALSE, robots, "/s2/profiles/me/mystuff.htm");
   expectRobots( TRUE, robots, "/s2/profiles/you/yourstuff.htm");
   expectRobots( TRUE, robots, "/s2/photos/myphoto.gif");

   expectRobots(FALSE, robots, "/reader/data");
   expectRobots( TRUE, robots, "/reader/play");
   expectRobots( TRUE, robots, "/reader/plays");

   expectRobots( TRUE, robots, "/unknown/dir");

   //-------------------------------------------------------------------------
   // Validate our actual Robots.txt
   fileSource.open("html/robots.txt");
   robots.open("Brian", fileSource);

   debugf("Request delay: %.3f\n", robots.getDelay());
   debugf("Request times: %.4d-%.4d\n",
          robots.getVisit()/10000, robots.getVisit()%10000);
   robots.debug();

   expectRobots(FALSE, robots, "/forbidden.html");
   expectRobots( TRUE, robots, "/index.html");
   expectRobots( TRUE, robots, "/input.html");
   expectRobots( TRUE, robots, "/shutdown.html");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testUrl
//
// Purpose-
//       Test Url.h.
//
//----------------------------------------------------------------------------
static void
   testUrl(                         // Test Url object
     const char*       item)        // For this url
{
   Url                 url;         // The Url

   int                 rc;

   rc= url.setURI(item);
   printf("%d= Url.set(%s)\n", rc, item);
   if( rc == 0 )
   {
     printf("  getAuthority: %s\n", url.getAuthority().c_str());
     printf("getDefaultPort: %d\n", url.getDefaultPort());
     printf("   getFragment: %s\n", url.getFragment().c_str());
     printf("       getHost: %s\n", url.getHost().c_str());
     printf("       getPath: %s\n", url.getPath().c_str());
     printf("       getPort: %d\n", url.getPort());
     printf("   getProtocol: %s\n", url.getProtocol().c_str());
     printf("      getQuery: %s\n", url.getQuery().c_str());
     printf("   getUserInfo: %s\n", url.getUserInfo().c_str());
     printf("\n");
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verifyFile
//
// Purpose-
//       Verify words in gather output file.
//
//----------------------------------------------------------------------------
static void
   verifyFile(                      // Verify words in file
     const char*       fileName)    // The fileName
{
   FileSource          source(fileName);
   char                buff1[256];
   char                buff2[256];
   char                buff3[512];

   for(;;)
   {
     int x= 0;
     int C= source.get();
     if( C == EOF )
       break;

     while( C != '\n' && C != EOF )
     {
       if( C != '\r' )
       {
         if( x < (sizeof(buff1)-1) )
           buff1[x++]= C;
       }

       C= source.get();
     }
     buff1[x]= '\0';

     x= 0;
     for(int i= 13; buff1[i] != '\0'; i++)
     {
       if( buff1[i] == '\'' )
       {
         if( x < (sizeof(buff1)-1) )
           buff2[x++]= '\\';
       }

       if( x < (sizeof(buff1)-1) )
         buff2[x++]= buff1[i];
     }
     buff2[x]= '\0';

     if( x > 0 )
     {
       sprintf(buff3, "echo %s | aspell list >/tmp/aspell.out", buff2);
////   printf("%s\n", buff3);
       if( system(buff3) ) ;        // Avoids compiler complaint

       struct stat filestat;
       memset(&filestat, 0, sizeof(filestat));
       stat("/tmp/aspell.out", &filestat);
       if( filestat.st_size == 0 )
       {
         debugf("%s\n", buff1);
         Debug::get()->flush();
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       simpleTest
//
// Purpose-
//       Run a simple test.
//
//----------------------------------------------------------------------------
static void
   simpleTest( void )               // Simple test
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       usage
//
// Purpose-
//       Wrirte usage information
//
//----------------------------------------------------------------------------
static void
   usage( void )                    // Write usage information
{
   printf("Tester <option>\n"
          "--listDB database (List database)\n"
          "--loadDB database (Load database)\n"
          "--testDB database (Test database)\n"
          "--testDate        (Test DateParser.h)\n"
          "--testObject      (Test Object.h)\n"
          "--testRobots      (Test Robots.h)\n"
          "--testUrl  item   (Test Url.h)\n"
          "--verify filename (Verify words in filename)\n"
          );

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     const char*       argv[])      // Argument array
{
   debugf("Tester...\n");
   Signal handler;                  // Default Signal handler

   try {
     for(int argx= 1; argx<argc; argx++)
     {
       if( strcmp(argv[argx], "--help") == 0 )
         usage();
       else if( strcmp(argv[argx], "--listDB") == 0 )
         listDB(argv[++argx]);
       else if( strcmp(argv[argx], "--loadDB") == 0 )
         loadDB(argv[++argx]);
       else if( strcmp(argv[argx], "--testDB") == 0 )
         testDB(argv[++argx]);
       else if( strcmp(argv[argx], "--testApprox") == 0 )
         testApprox();
       else if( strcmp(argv[argx], "--testDate") == 0 )
         testDateParser();
       else if( strcmp(argv[argx], "--testHtml") == 0 )
         testHtmlParser(argv[++argx]);
       else if( strcmp(argv[argx], "--testObject") == 0 )
         testObject();
       else if( strcmp(argv[argx], "--testRobots") == 0 )
         testRobots();
       else if( strcmp(argv[argx], "--testUrl") == 0 )
         testUrl(argv[++argx]);
       else if( strcmp(argv[argx], "--test") == 0 )
         simpleTest();
       else if( strcmp(argv[argx], "--verify") == 0 )
         verifyFile(argv[++argx]);
       else
         usage();
     }
   } catch(const char* X) {
     debugf("EXCEPTION!(%s)\n", X);
   } catch(std::exception& X) {
     debugf("EXCEPTION!(%s)\n", X.what());
   } catch(...) {
     debugf("EXCEPTION!(...)\n");
   }

   return 0;
}

