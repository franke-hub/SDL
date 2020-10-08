//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Gather.cpp
//
// Purpose-
//       Bringup data gatherer.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include <com/Signal.h>             // Prevent system defines from botching

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <exception>
#include <string>
#include <map>

#include <com/AutoDelete.h>
#include <com/Buffer.h>
#include <com/Debug.h>
#include <com/FileInfo.h>
#include <com/FileList.h>
#include <com/FileName.h>
#include <com/FileSource.h>
#include <com/istring.h>
#include <com/Media.h>
#include <com/Reader.h>
#include <com/Signal.h>
#include <com/Unconditional.h>

#include "Common.h"
#include "DtdParser.h"
#include "HtmlParser.h"
#include "HtmlNode.h"
#include "HtmlNodeVisitor.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#define GATHER_TEMP "/tmp/gather.tmp"

#include <com/ifmacro.h>

#define USE_DTD_PARSER  FALSE
#define USE_HTML_PARSER TRUE
#define USE_PDF_PARSER  TRUE

#define USE_STOPERROR  FALSE
#define USE_WRITEDEBUG FALSE

//----------------------------------------------------------------------------
//
// Class-
//       MySignal
//
// Purpose-
//       My signal handler.
//
//----------------------------------------------------------------------------
class MySignal : public Signal
{
public:
virtual
   ~MySignal( void );               // Destructor
   MySignal( void );                // Constructor

virtual int                         // Return code (0 iff handled)
   handle(                          // Signal handler
     SignalCode        signal);     // Signal code
}; // class MySignal

//----------------------------------------------------------------------------
//
// Class-
//       StringSource
//
// Purpose-
//       Convert a String into a DataSource.
//
//----------------------------------------------------------------------------
class StringSource : public DataSource { // String DataSource
protected:
std::string            string;      // The source std::string

public:
inline virtual
   ~StringSource( void ) {}
inline
   StringSource(
     std::string       string)
:  DataSource()
,  string(string)
{
   reset();

   length= string.length();
}

virtual inline unsigned int         // The number of bytes read
   read(                            // Read from StringSource
     void*             addr,        // Data address
     unsigned int      size)        // Data length (in bytes)
{
   if( (offset + size) > length )
     size= length - offset;

   char* target= (char*)addr;
   unsigned int count= size;
   while( count > 0 )
   {
     *target= string[offset++];
     target++;
     count--;
   }

   return size;
}
}; // class StringSource

//----------------------------------------------------------------------------
//
// Struct-
//       ltSTR
//
// Purpose-
//       Define less than operator for C-strings.
//
//----------------------------------------------------------------------------
struct ltSTR
{
   bool operator()(const char* l, const char* r) const
   {
     return strcmp(l, r) < 0;
   }
};

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static FILE*           word_ok= nullptr; // Verified word list
static FILE*           word_ng= nullptr; // Rejected word list

typedef std::map<const char*, int, ltSTR>
                       WordMap;
typedef std::map<const char*, int, ltSTR>::iterator
                       WordMapIter;
static WordMap         wordMap;

static const char*     argList[]=
{  NULL
,  "."
};

static const int       argC= sizeof(argList) / sizeof(argList[0]);

static const char*     typeList[]=
{  NULL
#if( USE_DTD_PARSER )
,  ".dtd"
#endif

#if( USE_HTML_PARSER )
,  ".htm"
,  ".html"
#endif

#if( USE_PDF_PARSER )
,  ".pdf"
#endif
};

static const int       typeC= sizeof(typeList) / sizeof(typeList[0]);

static const char*     ignore[]=    // List of ignored nodes
{  "div"
,  "span"
,  "style"
,  "tt"
,  NULL
};

//----------------------------------------------------------------------------
// Diagnostic/recovery area
static const char*     action= NULL;// Current action
static MySignal        handler;     // Signal handler
static void*           spare= NULL; // Spare storage (for cleanup)

//----------------------------------------------------------------------------
//
// Class-
//       TextVisitor
//
// Purpose-
//       Visit text files, extracting words.
//
//----------------------------------------------------------------------------
class TextVisitor : public HtmlNodeVisitor
{
//----------------------------------------------------------------------------
// TextVisitor::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (0 OK)
   visit(                           // Visit text HtmlNodes
     HtmlNode*         node);       // -> HtmlNode
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       getWord
//
// Purpose-
//       Extract next word from source.
//
// Notes-
//       Splits hyphenated words.
//
//----------------------------------------------------------------------------
static const char*                  // buffer (or NULL)
   getWord(                         // Extract next word from source
     DataSource&       source,      // The DataSource
     char*             buffer,      // Result buffer
     unsigned          length)      // Result buffer length
{
   int C= source.get();
   while( C != EOF && !isalpha(C) )
     C= source.get();
   if( C == EOF )
     return NULL;

   unsigned x= 0;
   while( C != EOF && isalpha(C) && x < (length-2) )
   {
     buffer[x++]= tolower(C);
     C= source.get();
     if( C == '\'' )
     {
       C= source.get();
       if( C == EOF || !isalpha(C) )
         break;

       if( tolower(C) == 's' )      // Ignore possessives and 'is' contractions
       {
         C= source.get();
         if( isspace(C) )
           break;

         buffer[x++]= '\'';         // Not followed by space, accept
         buffer[x++]= 's';
         continue;
       }

       buffer[x++]= '\'';           // Accept all others
     }
   }
   buffer[x]= '\0';

   return buffer;
}

//----------------------------------------------------------------------------
//
// Method-
//       TextVisitor::visit
//
// Purpose-
//       Extract words from text.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 to decend)
   TextVisitor::visit(              // Visit a HtmlNode
     HtmlNode*         node)        // -> HtmlNode
{
   char                buffer[256]; // Working buffer
   int                 i;

   IFHCDM(
     if( node->getType() == HtmlNode::TYPE_TEXT)
       printf("Visit text(%s)\n", node->getData().c_str());
     else if( node->getType() == HtmlNode::TYPE_ATTR)
       printf("Visit attr(%s) data(%s)\n",
              node->getName().c_str(), node->getData().c_str());
     else
       printf("Visit elem(%s)\n", node->getName().c_str());
     )

   if( node->getType() != HtmlNode::TYPE_TEXT )
     return 0;

   HtmlNode* parent= node->getParent();
   for(i= 0; ignore[i] != NULL; i++)
   {
     if( parent->getName() == ignore[i] )
       return 0;
   }

   StringSource source(node->getData());
   for(;;)
   {
     if( getWord(source, buffer, sizeof(buffer)) == NULL )
       break;

     IFHCDM( debugf(".word(%s)\n", buffer); )

     WordMapIter mi= wordMap.find(buffer);
     if( mi != wordMap.end() )
       mi->second++;
     else
       wordMap[Unconditional::strdup(buffer)]= 1;
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       dtdReader
//
// Purpose-
//       Read local DTD file.
//
//----------------------------------------------------------------------------
static void
   dtdReader(                       // Read local HTML file
     const char*       fileName)    // File name
{
   FileSource          source(fileName);
   DtdParser           parser;

   int                 rc;

   rc= parser.parse(source);
   #if( USE_WRITEDEBUG )
     parser.debug();
   #endif

   debugf("%d=  DTDparser.parse(%s)\n", rc, fileName);
   if( rc != 0 )
   {
     debugf("%s\n", parser.getREPORT());

     if( USE_STOPERROR )
       exit(1);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       htmlReader
//
// Purpose-
//       Read local HTML file.
//
//----------------------------------------------------------------------------
static void
   htmlReader(                      // Read local HTML file
     const char*       fileName)    // File name
{
   FileSource          source(fileName);
   HtmlParser          parser;
   TextVisitor         visitor;

   int                 rc;

   rc= parser.parse(source);
   #if( USE_WRITEDEBUG )
     parser.debug();
   #endif

   debugf("%d= HTMLparser.parse(%s)\n", rc, fileName);
   if( rc != 0 )
   {
     if( USE_STOPERROR )
       exit(1);
   }

   action= "Visiting";
   if( rc == 0 )
     parser.getRoot()->visit(visitor);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       pdfReader
//
// Purpose-
//       Read local PDF file.
//
//----------------------------------------------------------------------------
static void
   pdfReader(                       // Read local PDF file
     const char*       fileName)    // File name
{
   char                buffer[4096];// Working buffer
   TextBuffer          text;        // Expand fileName

   const char* C= fileName;
   while( *C != '\0' )
   {
     if( *C == ' ' || *C == '~' || *C == '#' || *C == '$' || *C == '%'
         || *C == '\\' || *C == '\'' || *C == '\"' || *C == ':' || *C == ';'
         || *C == '?' || *C == '*'
         || *C == '(' || *C == ')'
         || *C == '{' || *C == '}'
         || *C == '[' || *C == ']'
         || *C == '&' || *C == '|' || *C == '<' || *C == '>' )
       text.put('\\');
     text.put(*C);
     C++;
   }

   fflush(stdout); fflush(stderr);  // For error coherence
   Debug::get()->flush();           // (likewise)
   sprintf(buffer, "pdftotext %s " GATHER_TEMP, text.toChar());
   int rc= system(buffer);
   debugf("%d= system(%s)\n", rc, buffer);
   if( rc != 0 )
     return;

   action= "reading " GATHER_TEMP;
   FileSource source(GATHER_TEMP);
   for(;;)
   {
     if( getWord(source, buffer, 256) == NULL )
       break;

     IFHCDM( debugf(".word(%s)\n", buffer); )

     WordMapIter mi= wordMap.find(buffer);
     if( mi != wordMap.end() )
       mi->second++;
     else
       wordMap[Unconditional::strdup(buffer)]= 1;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       fileReader
//
// Purpose-
//       Read local file.
//
//----------------------------------------------------------------------------
static void
   fileReader(                      // Read local file
     const char*       fileName)    // File name
{
   FileName            source(fileName);

   action= fileName;
   const char* extension= source.getExtension();
   if( stricmp(extension, ".dtd") == 0 )
     dtdReader(fileName);

   else if( stricmp(extension, ".htm") == 0 || stricmp(extension, ".html") == 0 )
     htmlReader(fileName);

   else if( stricmp(extension, ".pdf") == 0 )
     pdfReader(fileName);

   action= NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       pathReader
//
// Purpose-
//       Read local files.
//
//----------------------------------------------------------------------------
static void
   pathReader(                      // Read local files
     const char*       pathName)    // From this directory
{
   tracef("======== pathReader(%s)\n", pathName);

   for(FileList fileList(pathName, "*");;fileList.getNext())
   {
     const char* ptrName= fileList.getCurrent();
     if( ptrName == NULL )
       break;

     FileInfo fileInfo(pathName, ptrName);
     if( fileInfo.exists() )
     {
       FileName fileName(pathName, ptrName);
       if( !fileInfo.isLink() )
       {
         if( fileInfo.isPath() )
         {
           if( fileName.resolve() == NULL )
             pathReader(fileName.getFileName());
         }
         else
         {
           const char* T= fileName.getExtension();
           for(int i= 1; i<typeC; i++)
           {
             if( stricmp(typeList[i], T) == 0 )
             {
               fileReader(fileName.getFileName());
               break;
             }
           }
         }
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verifyWord
//
// Purpose-
//       Verify a word.
//
//----------------------------------------------------------------------------
static int                          // TRUE iff word is valid
   verifyWord(                      // Verify a word
     const char*       word)        // The word to verify
{
   char                buffer[256];
   char                copied[256];

   if( strchr(word, '\'') )         // If word contains a quote
   {
     size_t j= 0;
     for(int i= 0; word[i] != '\0'; i++)
     {
       if( j >= (sizeof(copied) - 2) )
         return false;

       int C= word[i];
       if( C == '\'' )
         copied[j++]= '\\';
       copied[j++]= C;
     }

     copied[j]= '\0';
     word= copied;
   }

   // Roman numeral check (I,V,X,L,C,M)
   // Quick check: I, V, X
   if( word[0] == 'i' )
   {
     if( word[1] == 'v' )
     {
       if( word[2] == '\0' )
         return false;
     }
     else if( word[1] == 'i' || word[1] == 'x' )
       return false;
   }
   else if( word[0] == 'v' )
   {
     if( word[1] == 'i' )
     {
       if( word[2] == '\0' || word[2] == 'i' )
         return false;
     }
   }
   else if( word[0] == 'x' )
   {
     if( word[1] == 'v' || word[1] == 'x' || word[1] == 'l' )
       return false;
     if( word[1] == 'i' )
     {
       // xi and xis: The 16th letter of the greek alphabet
       if( word[2] != '\0' && word[2] != 's' )
         return false;
     }
   }

   int L= snprintf(buffer, sizeof(buffer),
                   "echo %s | aspell list >" GATHER_TEMP, word);
   if( size_t(L) < (sizeof(buffer) - 2) )
   {
     int rc= system(buffer);
     if( rc != 0 )
     {
       debugf("%d= system(%s)\n", rc, buffer);
       return false;
     }

     struct stat filestat;
     memset(&filestat, 0, sizeof(filestat));
     stat(GATHER_TEMP, &filestat);
     if( filestat.st_size == 0 )
       return true;
   }

   return false;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       wordList
//
// Purpose-
//       Display the word list.
//
//----------------------------------------------------------------------------
static void
   wordList( void )                 // Display the word list
{
   if( spare != NULL )
   {
     free(spare);
     spare= NULL;

     int flush_count= 256;
     for(WordMapIter i= wordMap.begin(); i != wordMap.end(); ++i)
     {
       const char* text= "";
       if( verifyWord(i->first) )
         fprintf(word_ok, "%12d %s\n", i->second, i->first);
       else
       {
         fprintf(word_ng, "%12d %s\n", i->second, i->first);
         text= " (*REJ*)";
       }

       debugf("%12d %s%s\n", i->second, i->first, text);
       flush_count--;
       if( flush_count <= 0 )
       {
         flush_count= 256;
         fflush(word_ng);
         fflush(word_ok);
       }
     }

     fclose(word_ng);
     fclose(word_ok);
   }

   // Delete temporary file
   remove(GATHER_TEMP);
}

//----------------------------------------------------------------------------
//
// Class-
//       MySignal
//
// Purpose-
//       Try to insure that the wordList is displayed.
//
//----------------------------------------------------------------------------
#undef SIGCHLD                      // Remove nasty define
   MySignal::~MySignal( void )      // Destructor
{
   debugf("MySignal::~MySignal()\n");
   if( spare != NULL )
   {
     free(spare);
     spare= malloc(1);
     wordList();
   }
}

   MySignal::MySignal( void )       // Constructor
:  Signal()
{
}

int                                 // Return code
   MySignal::handle(                // Signal handler
     SignalCode        signal)      // Signal code
{
   debugf("MySignal::handle(%d)\n", (int)signal);
   if( spare != NULL )
   {
     free(spare);
     spare= malloc(1);
   }
   if( action != NULL )
     debugf("Action(%s)\n", action);

   wordList();

   return 1;
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
   Debug debug("gather.log");
   Debug::set(&debug);
   debugf("Starting Gather...\n");
   spare= Unconditional::malloc(0x00100000);

   word_ng= fopen("gather.NG", "wb");
   word_ok= fopen("gather.OK", "wb");

   if( word_ng == nullptr || word_ok == nullptr )
   {
     if( word_ng ) fclose(word_ng);
     else errorf("Unable to open(gather.NG)\n");
     if( word_ok ) fclose(word_ok);
     else errorf("Unable to open(gather.OK)\n");
     exit(EXIT_FAILURE);
   }

   atexit(wordList);

   try {
     if( argc < 2 )
     {
       argc= argC;
       argv= argList;
     }

     for(int i= 1; i<argc; i++)
       pathReader(argv[i]);

     debugf("\n");
   } catch(const char* X) {
     free(spare);
     spare= NULL;
     debugf("EXCEPTION!(%s)\n", X);
     spare= malloc(1);
   } catch(std::exception& X) {
     free(spare);
     spare= NULL;
     debugf("EXCEPTION!(%s)\n", X.what());
     spare= malloc(1);
   } catch(...) {
     free(spare);
     spare= NULL;
     debugf("EXCEPTION!(...)\n");
     spare= malloc(1);
   }

   wordList();

   return 0;
}

