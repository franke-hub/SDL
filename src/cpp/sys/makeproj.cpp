//----------------------------------------------------------------------------
//
//       Copyright (C) 2006-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       makeproj.cpp
//
// Function-
//       Create a dependency file for C or C++ programs.
//
// Last change date-
//       2023/05/19
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "makeproj.hpp"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef DEBUGGING                   // Generic Debugging
#define DEBUGGING             FALSE // Generic Debugging
#endif

#ifndef DEBUGGING_OC                // Debugging open/close
#define DEBUGGING_OC          FALSE // Debugging open/close
#endif

#ifndef HCDM                        // Hard Core Debug Mode
#define HCDM                        // (TRUE if defined)
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define PATHPART_SIZE          256L // sizeof(path part of file name)
#define DESCPART_SIZE          256L // sizeof(desc part of file name)
#define FILENAME_SIZE          540L // sizeof(file name)

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
struct Depend;
struct Entity;
struct Path;

//----------------------------------------------------------------------------
//
// Struct-
//       Depend
//
// Description-
//       Dependency descriptor
//
//----------------------------------------------------------------------------
struct Depend
{
   // Depend::Constructor/Destructor
   Depend(                          // Constructor
     Entity*         entity);       // -> Associated Entity

   // Depend::Methods
static Depend*                      // -> Allocated Depend (Never NULL)
   allocate(                        // Allocate a new Entity
     Entity*         entity);       // -> Associated Entity

   // Depend::Attributes
   Depend*           next;          // -> Next Depend in linear chain
   Entity*           entity;        // -> Associated Entity
};

//----------------------------------------------------------------------------
//
// Struct-
//       Entity
//
// Description-
//       Entity descriptor
//
//----------------------------------------------------------------------------
struct Entity
{
   // Entity::Enumerations
public:
enum                                // Constants for parameterization
{
   MaxColumn=                   130,// The size of the include file array
   MaxGlobal=                 4096L,// Number of entities on global list
   MaxHashTable=              5000L // Hash table size
};

enum Type                           // Entity type
{
   TypeInclude=                'I', // Entity is an included file
   TypeProject=                'P', // Entity is a project file
   TypeSource=                 'S'  // Entity is a source file
};

   // Entity::Constructor/Destructor
public:
   Entity(                          // Constructor
     Type            type,          // Entity Type
     const char*     descName,      // Desc name
     const char*     pathName);     // Path name (may be NULL)

   // Entity::Methods
public:
void
   addDepend(                       // Add Depend
     Entity*         inpEntity);    // -> Dependent Entity

static Entity*                      // -> Allocated Entity (Never NULL)
   allocate(                        // Allocate a new Entity
     Type            type,          // Entity Type
     const char*     descName,      // Desc name
     const char*     pathName= NULL); // Path name

Depend*                             // -> Depend
   getDepend(                       // Get Depend of this
     const Entity*   entity);       // -> Dependent Entity

char*                               // resultant
   getFileName(                     // Extract the file name
     char*           resultant);    // resultant

char*                               // -> STATIC resultant
   getFileName( void );             // Extract the file name

static inline unsigned int          // HASH(fileName)
   hash(                            // Compute hash(fileName)
     const char*     fileName);     // File name

void
   insSourceList( void );           // Insert this Entity on the source list

int                                 // TRUE if entity is on global list
   isGlobal( void );                // Is entity in global list?

static Entity*                      // -> Entity
   locate(                          // Does entity exist?
     const char*     descName,      // Desc name
     const char*     pathName= NULL); // Path name

FILE*                               // -> FILE
   open( void );                    // Open file using search list

void
   popGlobal( void );               // Pop  entity from global list

void
   pushGlobal( void );              // Push entity onto global list

static Entity*                      // Removed Entity
   remSourceList( void );           // Remove first Entity from source list

void
   rename(                          // Rename (delete) this entity
     Entity*         newEntity);    // -> Rename value

void
   rename(                          // Rename dependencies
     Entity*         oldEntity,     // -> old value
     Entity*         newEntity);    // -> new value

static void
   resetGlobal( void );             // Reset the global list

void
   resetHandled( void );            // Reset the handled indicator

void
   resolveDepend( void );           // Resolve dependencies

void
   showEntityRelation(              // Display Entity relationship
     int             level);        // Relationship level

int                                 // TRUE if dependency found
   showEntityRelation(              // Display Entity relationship
     int             level,         // Relationship level
     const char*     name);         // For this dependency

void
   sortDepend( void );              // Sort the dependencies

static void
   sortEntityList( void );          // Sort entityList

void
   writeDepend( void );             // Write all dependencies

   // Entity::Attributes
public:
   unsigned int      isCompiled : 1;// TRUE iff file has been scanned
   unsigned int      isExistant : 1;// TRUE iff file is known to exist
   unsigned int      isHandled  : 1;// TRUE iff Entity has been handled
   unsigned int      isInclude  : 1;// TRUE iff file is included
   unsigned int      isSource   : 1;// TRUE iff file is named in project file
   unsigned int                 : 3;// Reserved for alignment
   unsigned int                 : 8;// Reserved for alignment
   unsigned int                 : 8;// Reserved for alignment
   unsigned int      type       : 8;// Entity type

   Entity*           nextHash;      // -> Next entry in hash chain
   Entity*           nextSource;    // -> Next entry in source chain
   Depend*           child;         // -> First child Depend

   char              fullName[FILENAME_SIZE]; // File name
   char              pathName[PATHPART_SIZE]; // Path used to load file
   char*             descName;      // -> Desc name (within fullName)

   // Entity::Static attributes
   static Entity*    hashTable[MaxHashTable]; // Hash table collision list

   static int        entityCount;   // Number of unique entities
   static Entity*    entityList[MaxGlobal];

   static char       getFileNameResultant[FILENAME_SIZE];
   static Entity*    sourceHead;    // First unread Entity
};

//----------------------------------------------------------------------------
//
// Struct-
//       Path
//
// Description-
//       Path descriptor
//
//----------------------------------------------------------------------------
struct Path
{
   // Path::Constructor/Destructor
   Path( void );                    // Constructor

   // Path::Methods
static Path*                        // -> Allocated Path (Never NULL)
   allocate(                        // Allocate a new Path
     const char*     pathName);     // Path name

   // Path::Attributes
   Path*             next;          // -> Next Path in linear chain
   char              name[PATHPART_SIZE]; // Path name (including trailing '/')
};

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*   projName;      // -> Project filename  parameter

static Entity*       sysEntity;     // The root project file Entity

static int           globalErrorCount; // Number of global errors
static char          swAngle;       // Follow <angle> files?
static char          swBom;         // List Bill of Materials?
static char          swList;        // List dependencies?
static const char*   swName;        // List specific dependency?

static FILE*         bomFile;       // Output (makeproj.bom)  file
static FILE*         outFile;       // Output (makeproj.incl) file

// Entity::open Controls
static Path*         pathHead;      // Head(Directory search list)
static Path*         pathTail;      // Tail(Directory search list)

// Entity::open Returns
static char          openFile[FILENAME_SIZE];// Full name of file
static char          openPath[PATHPART_SIZE];// Path name of file
static char*         openDesc;               // Desc name of file

//----------------------------------------------------------------------------
//
// Subroutine-
//       hcdm
//
// Purpose-
//       Hard Core Debug Mode
//
//----------------------------------------------------------------------------
static inline void
   hcdm(                            // Hard Core Debug Mode
     int             line)          // File line number
{
#ifdef HCDM
   printf("%s %d: HCDM\n", __FILE__, line);
#endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       fputsName
//
// Purpose-
//       Output a name which may include blanks.
//       (Blanks must be prefixed by backslash to avoid confusion.)
//
//----------------------------------------------------------------------------
static int                          // 0 iff successful, otherwise EOF
   fputsName(                       // Write a name which may include blanks
     const char*     name,          // The name of the file
     FILE*           handle)        // The associated file
{
   int               result= 0;

   while( *name != '\0' )
   {
     if( *name == ' ' )
     {
       result= fputc('\\', handle);
       if( result == EOF )
         break;
     }

     result= fputc(*name, handle);
     if( result == EOF )
       break;

     name++;
   }

   if( *name == '\0' )
     result= 0;

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       isFileReadable
//
// Purpose-
//       Determine, roughly, whether a file can be read.
//       For our purposes, a file can be read if it can be opened for reading.
//
//----------------------------------------------------------------------------
static int                          // TRUE if file can be read
   isFileReadable(                  // Determine whether a file can be read
     const char*     fileName)      // The name of the file
{
   FILE*             ptrFile;       // -> FILE descriptor

   ptrFile= fopen(fileName, "rb");  // Open the file for reading
   if( ptrFile == NULL )            // If the open failed
     return FALSE;

   fclose(ptrFile);                 // We're done with the file
   return TRUE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       extractDesc
//
// Purpose-
//       Extract the DescName (filename.ext) from a file descriptor string.
//
//----------------------------------------------------------------------------
static const char*                  // -> DescName (within inpName)
   extractDesc(                     // Extract DescName
     const char*     inpName)       // -> Input name
{
   const char*       ptrC;          // -> Generic character
   const char*       ptrLastC;      // -> Generic character

   // Locate the file name
   ptrC= inpName;                   // Default, no path component
   ptrLastC= inpName;
   while( ptrC != NULL )            // Locate the file name
   {
     ptrC= strchr(ptrLastC, '/');   // Look for separator
     if( ptrC == NULL )             // If none
       break;

     ptrLastC= ptrC+1;              // Last found file name
   }

   return ptrLastC;
}

static char*                        // -> DescName (within inpName)
   extractDesc(                     // Extract DescName
     char*           inpName)       // -> Input name
{
   return (char*)extractDesc((const char*)inpName);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       extractPath
//
// Purpose-
//       Extract the PathName from a file descriptor string.
//
//----------------------------------------------------------------------------
static void
   extractPath(                     // Extract PathName
     const char*     inpName,       // -> Input name
     char*           outPath)       // -> Output pathName
{
   char*             ptrC;          // -> Generic character
   char*             ptrLastC;      // -> Generic character

   // Extract the pathname component
   ptrC= (char*)strchr(inpName, '/'); // Look for delimiter
   if( ptrC == NULL )               // If no path qualifier
     outPath[0]= '\0';
   else                             // If path qualifier
   {
     if( strlen(inpName) >= PATHPART_SIZE )
     {
       fprintf(stderr, "Invalid filename(%s)\n", inpName);
       exit(EXIT_FAILURE);
     }

     strcpy(outPath, inpName);      // Path/Name specified
     ptrC= strchr(outPath, '/');    // Find the first delimiter
     do                             // Look for last delimiter
     {
       ptrLastC= ptrC;              // Last found delimiter
       ptrC= strchr(ptrC+1, '/');   // Look for another
     }
     while( ptrC != NULL );         // Look for last delimiter

     *(ptrLastC+1)= '\0';
   }
}
#if 0
//----------------------------------------------------------------------------
//
// Subroutine-
//       extractPart
//
// Purpose-
//       Extract the PathName and the DescName from a string.
//
//----------------------------------------------------------------------------
static void
   extractPart(                     // Extract PathName and DescName
     const char*     inpName,       // -> Input name
     char*           outPath,       // -> Output PathName
     char*           outDesc)       // -> Output DescName
{
   char*             ptrC;          // -> Generic character
   char*             ptrLastC;      // -> Generic character

   // Split the name into path and name components
   ptrC= strchr(inpName, '/');      // Look for delimiter
   if( ptrC == NULL )               // If no path qualifier
   {
     strcpy(outDesc, inpName);      // Only DescName specified
     outPath[0]= '\0';
   }
   else                             // If path qualifier
   {
     strcpy(outPath, inpName);      // Path/Name specified
     ptrC= strchr(outPath, '/');    // Find the first delimiter
     do                             // Look for last delimiter
     {
       ptrLastC= ptrC;              // Last found delimiter
       ptrC= strchr(ptrC+1, '/');   // Look for another
     }
     while( ptrC != NULL );         // Look for last delimiter

     strcpy(outDesc, ptrLastC+1);
     *(ptrLastC+1)= '\0';
   }
}
#endif
//----------------------------------------------------------------------------
//
// Subroutine-
//       extractType
//
// Purpose-
//       Extract the FileType from a DescName string.
//
//----------------------------------------------------------------------------
static char*                        // -> FileType (within inpName)
   extractType(                     // Extract FileType
     char*           inpDesc)       // -> Input DescName
{
   char*             ptrC;          // -> Generic character
   char*             ptrLastC;      // -> Generic character

   // Locate the file type
   ptrC= inpDesc;                   // Default, no file type
   ptrLastC= NULL;
   for(;;)                          // Locate the file type
   {
     ptrC= strchr(ptrC, '.');       // Look for separator
     if( ptrC == NULL )             // If none
       break;

     ptrC++;
     ptrLastC= ptrC;                // Last found file type
   }

   return ptrLastC;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       extractWord
//
// Purpose-
//       Extract a word from a line.
//
//----------------------------------------------------------------------------
static const char*                  // -> Delimiter (within line)
   extractWord(                     // Extract word from a line
     const char*     inpLine,       // -> Input line
     char*           outWord)       // -> Output word
{
   int               inpX= 0;       // Input line index
   int               outX= 0;       // Output word index

   for(; inpLine[inpX] == ' '; inpX++) // Skip blanks
     ;

   while( inpLine[inpX] != '\0' && inpLine[inpX] != ' ' ) // Find delimiter
     outWord[outX++]= inpLine[inpX++];

   outWord[outX]= '\0';             // Set delimiter in outWord
   return &inpLine[inpX];
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Depend::Depend
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Depend::Depend(                  // Constructor
     Entity*         entity)        // -> Associated Entity
:  next(NULL)
,  entity(entity)
{
#if DEBUGGING
   // TRACE
   printf("Depend(%p)::Depend(%p=%s)\n", this, entity, entity->getFileName());
#endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Depend::allocate
//
// Purpose-
//       Allocate a new Depend
//
//----------------------------------------------------------------------------
Depend*                             // -> Allocated Depend
   Depend::allocate(                // Allocate a new Depend
     Entity*         entity)        // -> Associated Entity
{
   Depend*           ptrDepend;     // -> Allocated Depend

   ptrDepend= new Depend(entity);
   if( ptrDepend == NULL )
   {
     fprintf(stderr, "No storage\n");
     exit(EXIT_FAILURE);
   }

   return ptrDepend;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::Static Attributes
//
// Purpose-
//       Static Attributes.
//
//----------------------------------------------------------------------------
   Entity*           Entity::hashTable[MaxHashTable]; // Static hash table

   int               Entity::entityCount; // Number of unique entities
   Entity*           Entity::entityList[MaxGlobal]; // Global Entity list

   Entity*           Entity::sourceHead= NULL; // Source Entity list

   // Resultant for getFileName -- inherently single threaded, non-recursive
   char              Entity::getFileNameResultant[FILENAME_SIZE];

//----------------------------------------------------------------------------
//
// INLINE Subroutine-
//       Entity::hash
//
// Purpose-
//       Compute HASH(fileName)
//
//----------------------------------------------------------------------------
unsigned int                        // HASH(fileName)
   Entity::hash(                    // Compute HASH(fileName)
     const char*     fileName)      // File name
{
   const char*       fileDesc;
   unsigned int      resultant= 0;  // Resultant
   int               i;

   fileDesc= extractDesc(fileName); // Extract the file descriptor
   for(i=0; fileDesc[i] != '\0'; i++)
     resultant += fileDesc[i] + resultant + (resultant << 6);

   resultant %= MaxHashTable;

#if DEBUGGING
   // TRACE
   printf("0x%.8X= Entity::hash(%s)\n", resultant, fileName);
#endif

   return resultant;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::Entity
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Entity::Entity(                  // Constructor
     Type            type,          // Entity Type
     const char*     descName,      // Desc name (may be qualified)
     const char*     pathName)      // Path name (may be NULL)
:  isCompiled(FALSE)
,  isExistant(FALSE)
,  isHandled(FALSE)
,  isInclude(FALSE)
,  isSource(FALSE)
,  type(type)
,  nextHash(NULL)
,  nextSource(NULL)
,  child(NULL)
{
   unsigned int      hashX;         // Hash index

#if DEBUGGING
   // TRACE
   printf("Entity(%p)::Entity(%c,%s,%s)\n", this,
          type, descName, pathName == NULL ? "NULL" : pathName);
#endif

   // Set the file descriptor
   if( pathName != NULL )
   {
     while( *pathName == ' ' )
       pathName++;
   }
   else
     pathName= "";

   while( *descName == ' ' )
     descName++;

   if( (strlen(pathName) + strlen(descName)) >= sizeof(this->fullName) )
   {
     fprintf(stderr, "FileName(%s%s) too long\n", pathName, descName);
     exit(EXIT_FAILURE);
   }

   strcpy(fullName, pathName);
   strcat(fullName, descName);
   extractPath(fullName, this->pathName);
   this->descName= extractDesc(fullName);

   // Insert entry onto static hash table
   hashX= hash(this->descName);
   this->nextHash= hashTable[hashX];
   hashTable[hashX]= this;

   // Insert entry onto source list
   this->nextSource= sourceHead;
   sourceHead= this;

#if 0
   Entity*           ptrEntity= sourceHead;

   printf("Entity(%p)::Entity(%s,%c) path(%s) desc(%s)\n",
          this, getFileName(), type, this->pathName, this->descName);
   printf("sourceHead:");
   while( ptrEntity != NULL )
   {
     printf("%s->", ptrEntity->getFileName());
     ptrEntity= ptrEntity->nextSource;
   }
   printf("0\n");
#endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::addDepend
//
// Purpose-
//       Add Depend to list.
//
//----------------------------------------------------------------------------
void
   Entity::addDepend(               // Add Depend
     Entity*         inpEntity)     // -> Dependent Entity
{
   Depend*           ptrDepend;     // -> Depend

#if DEBUGGING
   // TRACE
   printf("Entity(%p=%s)::", this, getFileName());
   printf("addDepend(%s)\n", inpEntity->getFileName());
#endif

   // If *inpEntity is already our dependent, no need to add it again
   ptrDepend= getDepend(inpEntity);
   if( ptrDepend == NULL )
   {
     ptrDepend= Depend::allocate(inpEntity);
     ptrDepend->next= this->child;
     this->child= ptrDepend;
   }

#if 0
   // Debugging
   ptrDepend= child;
   printf("..child(%p):", ptrDepend);
   while( ptrDepend != NULL )
   {
     printf("%s->", ptrDepend->entity->getFileName());
     ptrDepend= ptrDepend->next;
   }
   printf("0\n");
#endif

   return;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::allocate
//
// Purpose-
//       Allocate a new Entity
//
//----------------------------------------------------------------------------
Entity*                             // -> Allocated Entity
   Entity::allocate(                // Allocate a new Entity
     Type            type,          // Entity Type
     const char*     descName,      // Desc name
     const char*     pathName)      // Path name
{
   Entity*           ptrEntity;     // -> Allocated Entity

   ptrEntity= new Entity(type, descName, pathName);
   if( ptrEntity == NULL )
   {
     fprintf(stderr, "No storage\n");
     exit(EXIT_FAILURE);
   }

   return ptrEntity;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::getDepend
//
// Purpose-
//       Does this entity have this (primary) dependent?
//
//----------------------------------------------------------------------------
Depend*                             // -> Depend
   Entity::getDepend(               // Get Depend of this
     const Entity*   entity)        // -> Dependent Entity
{
   Depend*           ptrDepend;     // -> Depend

   ptrDepend= child;
   while( ptrDepend != NULL )
   {
     if( ptrDepend->entity == entity )
       break;

     ptrDepend= ptrDepend->next;
   }

   return ptrDepend;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::getFileName
//
// Purpose-
//       Extract the file name.
//
//----------------------------------------------------------------------------
char*                               // resultant
   Entity::getFileName(             // Extract the file name
     char*           resultant)     // resultant
{
   strcpy(resultant, pathName);
   strcat(resultant, descName);
   return resultant;
}

char*                               // STATIC resultant
   Entity::getFileName( void )      // Extract the file name
{
   return getFileName(getFileNameResultant);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::insSourceList
//
// Purpose-
//       Insert Entity on source list.
//
//----------------------------------------------------------------------------
void
   Entity::insSourceList( void )    // Insert Entity on source list
{
   Entity*           ptrEntity;     // -> Entity

   ptrEntity= sourceHead;
   while( ptrEntity != NULL )
   {
     if( ptrEntity == this )
       return;

     ptrEntity= ptrEntity->nextSource;
   }

   this->nextSource= sourceHead;
   sourceHead= this;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::isGlobal
//
// Purpose-
//       Is entity in the global push list?
//
//----------------------------------------------------------------------------
int                                 // TRUE if entity is on global list
   Entity::isGlobal( void )         // Is entity in global list?
{
   int               i;

   for(i=0; i<entityCount; i++)
   {
     if( this == entityList[i] )
       return TRUE;
   }

   return FALSE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::locate
//
// Purpose-
//       Does entity exist?
//
//----------------------------------------------------------------------------
Entity*                             // -> Entity
   Entity::locate(                  // Does Entity exist?
     const char*     descName,      // Desc name
     const char*     pathName)      // Path name
{
   char              tempPath[PATHPART_SIZE];
   char              tempName[FILENAME_SIZE];

   Entity*           ptrEntity;     // -> Entity

   unsigned int      hashX;         // Hash index

#if DEBUGGING
   // TRACE (return)
#endif

   if( pathName == NULL )
     pathName= "";

   if( (strlen(pathName) + strlen(descName)) >= sizeof(tempName) )
   {
     fprintf(stderr, "FileName(%s%s) too long\n", pathName, descName);
     exit(EXIT_FAILURE);
   }

   strcpy(tempName, pathName);
   strcat(tempName, descName);
   extractPath(tempName, tempPath);
   descName= extractDesc(tempName);

   hashX= hash(descName);
   for(ptrEntity= hashTable[hashX];
       ptrEntity != NULL;
       ptrEntity= ptrEntity->nextHash)
   {
     if( strcmp(tempPath, ptrEntity->pathName) == 0
         &&strcmp(descName, ptrEntity->descName) == 0 )
       break;
   }

#if DEBUGGING
   // TRACE (return)
   printf("%p= Entity::locate(%s,%s)\n",
          ptrEntity, descName,
          pathName == NULL ? "NULL" : pathName);
#endif

   return ptrEntity;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::open
//
// Purpose-
//       Open a file using the list of available Paths.
//
//----------------------------------------------------------------------------
FILE*                               // -> FILE, NULL if not found
   Entity::open( void )             // Open file using search list
{
   FILE*             ptrFile;       // -> Opened file
   Path*             ptrPath;       // -> Current path

#if DEBUGGING_OC
   // TRACE
   printf("Entity(%p=%s)::open()\n", this, getFileName());
#endif

   // Look in current directory
   strcpy(openFile, getFileName());
   strcpy(openPath, pathName);
   openDesc= extractDesc(openFile);

#if DEBUGGING_OC
   printf("..File(%s) Path(%s) Name(%s)\n", openFile, openPath, openDesc);
#endif

   ptrFile= fopen(openFile, "rb");
   if( ptrFile != NULL )
     return ptrFile;

   // Search the Path list
   ptrPath= pathHead;
   while( ptrPath != NULL )
   {
     // Create the name components
     strcpy(openFile, ptrPath->name);
     strcat(openFile, pathName);
     strcat(openFile, descName);

     extractPath(openFile, openPath);
     openDesc= extractDesc(openFile);

     // Attempt to open the file
#if DEBUGGING_OC
     printf("..File(%s) Path(%s) Name(%s)\n", openFile, openPath, openDesc);
#endif

     ptrFile= fopen(openFile, "rb");
     if( ptrFile != NULL )
       return ptrFile;

     ptrPath= ptrPath->next;
   }

   // File not found
#if DEBUGGING_OC
   printf("..(NULL)\n");
#endif
   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::popGlobal
//
// Purpose-
//       Pop entity from global queue.
//
//----------------------------------------------------------------------------
void
   Entity::popGlobal( void )        // Pop  entity from global list
{
#if DEBUGGING
   // TRACE
   printf("Entity::popGlobal(%s)\n", getFileName());
#endif

   if( entityCount <= 0 )
   {
     fprintf(stderr, "%s %d: internal logic error\n", __FILE__, __LINE__);
     exit(EXIT_FAILURE);
   }

   entityCount--;
   if( entityList[entityCount] != this )
   {
     fprintf(stderr, "%s %d: internal logic error\n", __FILE__, __LINE__);
     exit(EXIT_FAILURE);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::pushGlobal
//
// Purpose-
//       Push entity onto global queue.
//
//----------------------------------------------------------------------------
void
   Entity::pushGlobal( void )       // Push entity onto global list
{
#if DEBUGGING
   // TRACE
   printf("Entity::pushGlobal(%s)\n", getFileName());
#endif

   if( entityCount >= MaxGlobal )
   {
     fprintf(stderr, "Too many dependencies\n");
     exit(EXIT_FAILURE);
   }

   entityList[entityCount++]= this;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::remSourceList
//
// Purpose-
//       Remove top Entity on source list.
//
//----------------------------------------------------------------------------
Entity*                             // -> Entity
   Entity::remSourceList( void )    // Remove Entity from source list
{
   Entity*           ptrEntity;     // -> Entity

   ptrEntity= sourceHead;
   if( ptrEntity != NULL )
     sourceHead= ptrEntity->nextSource;

   return ptrEntity;
}
#if 0
//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::removeDuplicateDepend
//
// Purpose-
//       Remove duplicate dependencies
//
// Notes-
//       Not needed because renames occur as needed in loadIncludeFile().
//
//----------------------------------------------------------------------------
void
   Entity::removeDuplicateDepend( void ) // Remove duplicate dependencies
{
   char              tempName[FILENAME_SIZE];

   Entity*           oldEntity;     // -> Entity
   Entity*           prvEntity;     // -> Entity
   Entity*           ptrEntity;     // -> Entity

#if DEBUGGING
   // TRACE
   printf("Entity::removeDuplicateDepend()\n");
#endif

   int               i;

   for(i=0; i<MaxHashTable; i++)
   {
     oldEntity= hashTable[i];
     while( oldEntity != NULL )
     {
       strcpy(tempName, oldEntity->getFileName());
       prvEntity= oldEntity;
       ptrEntity= prvEntity->nextHash;
       while( ptrEntity != NULL )
       {
         if( strcmp(tempName, ptrEntity->getFileName()) == 0 )
         {
           ptrEntity->rename(oldEntity); // Rename ptrEntity's depends
         }
         else
           prvEntity= ptrEntity;

         ptrEntity= prvEntity->nextHash;
       }

       oldEntity= oldEntity->nextHash;
     }
   }
}
#endif
//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::rename
//
// Purpose-
//       Rename a duplicate dependency
//
//----------------------------------------------------------------------------
void
   Entity::rename(                  // Rename this entity
     Entity*         newEntity)     // -> Rename value
{
   Depend*           ptrDepend;     // -> Depend
   Entity*           prvEntity;     // -> Entity
   Entity*           ptrEntity;     // -> Entity

   int               hashX;         // Hash index
   int               i;

#if DEBUGGING
   // TRACE
   printf("Entity(%p=%s)::", this, getFileName());
   printf("rename(%p=%s)\n", newEntity, newEntity->getFileName());
#endif

   // Remove this Entity from the static hash table
   prvEntity= NULL;
   hashX= hash(descName);
   ptrEntity= hashTable[hashX];
   while( ptrEntity != NULL )
   {
     if( this == ptrEntity )
     {
       if( prvEntity == NULL )
         hashTable[hashX]= nextHash;
       else
         prvEntity->nextHash= nextHash;
       break;
     }

     prvEntity= ptrEntity;
     ptrEntity= prvEntity->nextHash;
   }

   // Remove this Entity from the sourceHead list
   prvEntity= NULL;
   ptrEntity= sourceHead;
   while( ptrEntity != NULL )
   {
     if( this == ptrEntity )
     {
       if( prvEntity == NULL )
         sourceHead= this->nextSource;
       else
         prvEntity->nextSource= this->nextSource;

       break;
     }

     prvEntity= ptrEntity;
     ptrEntity= ptrEntity->nextSource;
   }

   // Take our dependencies and add them to the new entity
   while( child != NULL )
   {
     ptrEntity= child->entity;      // Cache the child entity
     if( this == ptrEntity )
     {
       child->entity= newEntity;
       ptrEntity= newEntity;
     }

     ptrDepend= newEntity->getDepend(ptrEntity);//marker
     if( ptrDepend == NULL )        // If newEntity does not have this dependency
     {
       ptrDepend= child;
       child= child->next;

       ptrDepend->next= newEntity->child;
       newEntity->child= ptrDepend;
     }
     else                           // If the dependency exists
       child= child->next;          // Just remove it
   }

   // Rename all dependencies everywhere
   for(i=0; i<MaxHashTable; i++)
   {
     ptrEntity= hashTable[i];
     while( ptrEntity != NULL )
     {
       ptrEntity->rename(this, newEntity);
       ptrEntity= ptrEntity->nextHash;
     }
   }
}

void
   Entity::rename(                  // Rename dependencies
     Entity*         oldEntity,     // -> Old value
     Entity*         newEntity)     // -> New value
{
   Depend*           ptrDepend;     // -> Entity

#if DEBUGGING
   // TRACE
   printf("Entity(%p)::rename(%p,%p)\n", this, oldEntity, newEntity);
#endif

   // Rename all Depend elements
   ptrDepend= child;
   while( ptrDepend != NULL )
   {
     if( ptrDepend->entity == oldEntity )
       ptrDepend->entity= newEntity;

     ptrDepend= ptrDepend->next;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::resetGlobal
//
// Purpose-
//       Reset the global queue.
//
//----------------------------------------------------------------------------
void
   Entity::resetGlobal( void )      // Reset the global list
{
#if DEBUGGING
   printf("Entity::resetGlobal()\n");
#endif

   entityCount= 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::resetHandled
//
// Purpose-
//       Reset the Handled indicator.
//
//----------------------------------------------------------------------------
void
   Entity::resetHandled( void )     // Reset the handled indicator
{
   Depend*           ptrDepend;

#if DEBUGGING
   // TRACE
   printf("Entity(%p)::resetHandled()\n", this);
#endif

   // Check for duplicate Entity
   if( isGlobal() )
     return;

   // Only reset the indicator once
   if( isHandled == FALSE )
     return;
   isHandled= FALSE;

   // Add Entity to Entity list
   pushGlobal();

   // Reset the next lower level
   ptrDepend= child;
   while( ptrDepend != NULL )
   {
     ptrDepend->entity->resetHandled();
     ptrDepend= ptrDepend->next;
   }

   // Remove Entity from global list
   popGlobal();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::resolveDepend
//
// Purpose-
//       Resolve the child dependencies
//
//----------------------------------------------------------------------------
void
   Entity::resolveDepend( void )    // Resolve child dependencies
{
   Depend*           ptrDepend;     // -> Depend

   // Check for duplicate Entity
   if( isGlobal() )
     return;

   // Check for existant Entity
   if( !isExistant )
     return;

   // Add Entity to Entity list (it stays on)
   pushGlobal();

   // Resolve lower-level dependencies
   ptrDepend= child;
   while( ptrDepend != NULL )
   {
     ptrDepend->entity->resolveDepend();
     ptrDepend= ptrDepend->next;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::showEntityRelation
//
// Purpose-
//       Display the entity and its relations
//
//----------------------------------------------------------------------------
void
   Entity::showEntityRelation(      // Display Entity relationship
     int             level)         // SubEntity level
{
   Depend*           ptrDepend;
   int               i;

#if DEBUGGING
   // TRACE
   printf("Entity(%p=%s)::showEntityRelation(%2d)\n",
          this, getFileName(), level);
#endif

   // Display this level
   for(i=0; i<level; i++)
     printf(" |");

   if( isHandled )
   {
     printf("(*)%s\n", getFileName());
     return;
   }
   isHandled= TRUE;

   printf("(%c)%s\n", isExistant ? type : 'X' , getFileName());

   // Check for duplicate Entity
   if( isGlobal() )
     return;

   // Add Entity to Entity list
   pushGlobal();

   // Display next lower level
   ptrDepend= child;
   while( ptrDepend != NULL )
   {
#if 0
     printf("Depend(%p)::entity(%s) child(%p=%s)\n",
            ptrDepend, ptrDepend->entity->getFileName(),
            ptrDepend->entity->child,
            ptrDepend->entity->child == NULL ? "NULL" : ptrDepend->entity->child->entity->getFileName());
#endif

     ptrDepend->entity->showEntityRelation(level+1);
     ptrDepend= ptrDepend->next;
   }

   // Remove Entity from global list
   popGlobal();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::showEntityRelation
//
// Purpose-
//       Display the entity and its relations
//
//----------------------------------------------------------------------------
int                                 // TRUE if dependency found
   Entity::showEntityRelation(      // Display Entity relationship
     int             level,         // SubEntity level
     const char*     name)          // For this dependency
{
   Depend*           ptrDepend;
   int               found= FALSE;
   int               i;

#if DEBUGGING
   // TRACE
   printf("Entity(%p=%s)::showEntityRelation(%2d,'%s')\n",
          this, getFileName(), level, name);
#endif

   // Is this the name we're looking for?
   if( strcmp(name, descName) == 0 )
   {
     for(i=0; i<level; i++)
       printf(" |");

     printf("(%c)%s\n", isExistant ? type : 'X' , getFileName());

     return TRUE;
   }

   // Check for duplicate Entity
   if( isGlobal() )
     return FALSE;

   if( isHandled )
   {
//// printf("isHandled!\n");
     return FALSE;
   }
   isHandled= TRUE;

   // Add Entity to Entity list
   pushGlobal();

   // Display next lower level
   ptrDepend= child;
   while( ptrDepend != NULL )
   {
#if 0
     printf("Depend(%p)::entity(%s) child(%p=%s)\n",
            ptrDepend, ptrDepend->entity->getFileName(),
            ptrDepend->entity->child,
            ptrDepend->entity->child == NULL ? "NULL" : ptrDepend->entity->child->entity->getFileName());
#endif

     found |= ptrDepend->entity->showEntityRelation(level+1,name);
     ptrDepend= ptrDepend->next;
   }

   // Have we found the name we're looking for?
   if( found )
   {
     for(i=0; i<level; i++)
       printf(" |");

     printf("(%c)%s\n", type, getFileName());
   }

   // Remove Entity from global list
   popGlobal();

   return found;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::sortDepend
//
// Purpose-
//       Sort the dependencies
//
//----------------------------------------------------------------------------
void
   Entity::sortDepend( void )       // Sort the dependencies
{
   char              workName[FILENAME_SIZE]; // Working filename

   Depend*           prvLowDepend;  // -> ptrLowDepend (last sorted dependency)
   Depend*           ptrLowDepend;  // -> Bubble sort low dependency
   Depend*           prvDepend;     // -> ptrDepend (previous entry)
   Depend*           ptrDepend;     // -> Depend (current entry)

#if DEBUGGING
   // TRACE
   printf("Entity(%p=%s)::sortDepend()\n", this, getFileName());
#endif

   // Check for duplicate Entity
   if( isGlobal() )
     return;

   // Only sort dependencies once
   if( isHandled )
     return;
   isHandled= TRUE;

   // Add Entity to Entity list
   pushGlobal();

   // Sort the dependencies at this level
   prvLowDepend= NULL;
   ptrLowDepend= child;
   while( ptrLowDepend != NULL )
   {
     strcpy(workName, ptrLowDepend->entity->getFileName());
     prvDepend= ptrLowDepend;
     ptrDepend= ptrLowDepend->next;
     while( ptrDepend != NULL )
     {
       if( strcmp(workName, ptrDepend->entity->getFileName()) > 0 )
       {
         prvDepend->next= ptrDepend->next; // Remove *ptrDepend from the list

         if( prvLowDepend == NULL )
         {
           child= ptrLowDepend->next; // Remove *ptrLowDepend from the list
           ptrDepend->next= child;
           child= ptrDepend;
         }
         else
         {
           prvLowDepend->next= ptrLowDepend->next; // Remove *ptrLowDepend from the list
           ptrDepend->next= prvLowDepend->next;
           prvLowDepend->next= ptrDepend;
         }

         ptrLowDepend->next= ptrDepend->next;
         ptrDepend->next= ptrLowDepend;

         ptrLowDepend= ptrDepend;
         strcpy(workName, ptrLowDepend->entity->getFileName());
       }

       prvDepend= ptrDepend;
       ptrDepend= prvDepend->next;
     }

     prvLowDepend= ptrLowDepend;
     ptrLowDepend= ptrLowDepend->next;
   }

   // Sort the dependencies at the next lower level
   ptrDepend= child;
   while( ptrDepend != NULL )
   {
     ptrDepend->entity->sortDepend();
     ptrDepend= ptrDepend->next;
   }

   // Remove Entity to Entity list
   popGlobal();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::sortEntityList
//
// Purpose-
//       Sort entityList[1::entityCount]
//
//----------------------------------------------------------------------------
void
   Entity::sortEntityList( void )   // Sort entityList
{
   char              workName[FILENAME_SIZE]; // Working filename
   Entity*           ptrEntity;     // -> Entity

   int               i, j;

   // Sort entityList
   for(i=1; i<entityCount; i++)
   {
     strcpy(workName, entityList[i]->getFileName());

     for(j=i+1; j<entityCount; j++)
     {
       if( strcmp(workName, entityList[j]->getFileName()) > 0 )
       {
         ptrEntity= entityList[i];
         entityList[i]= entityList[j];
         entityList[j]= ptrEntity;

         strcpy(workName, entityList[i]->getFileName());
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Entity::writeDepend
//
// Purpose-
//       Write the dependencies
//
//----------------------------------------------------------------------------
void
   Entity::writeDepend( void )      // Write the dependencies
{
   Depend*           ptrDepend;     // -> Depend
   Entity*           ptrEntity;     // -> Entity

   char              workName[FILENAME_SIZE]; // Working filename
   char*             ptrC;          // -> Generic character
   char*             ptrD;          // -> Generic character

   int               column;        // Current column number
   int               lenFileName;   // strlen(ptrEntity->getFileName())
   int               i;

#if DEBUGGING
   // TRACE
   printf("Entity(%p=%s)::writeDepend()\n", this, getFileName());
#endif

   // Only process an entry once
   if( isHandled )
     return;
   isHandled= TRUE;

   // Only process source files
   if( isSource )
   {
     // Resolve the dependencies
     resetGlobal();
     resolveDepend();
     sortEntityList();

     // Write the dependency entry
     strcpy(workName, getFileName());
     ptrD= extractDesc(workName);   // Extract the file name
     ptrC= extractType(ptrD);       // Extract the file type
     ptrC--;                        // Address the '.' delimiter

     *ptrC= '\0';                   // End it
     fputsName(ptrD, outFile);      // The base file name
     fputsName(TEXT_FILETYPE, outFile); // The file name extension
     fputs(":", outFile);           // Set qualifier delimiter

     column= strlen(workName) + strlen(TEXT_FILETYPE) + 1;

     // Write the dependency list
     for(i=0; i<entityCount; i++)
     {
       ptrEntity= entityList[i];
       lenFileName= strlen(ptrEntity->getFileName());
       if( (column + lenFileName) >= MaxColumn )
       {
         fputs(" \\\n", outFile);   // End of line delimiter
         column= 1;
       }
       fputs(" ", outFile);
       fputsName(ptrEntity->getFileName(), outFile);
       column += lenFileName + 1;
     }

     fputs("\n", outFile);          // Complete the last line
   }

   // For children which are source, write their dependencies
   ptrDepend= child;
   while( ptrDepend != NULL )
   {
     ptrDepend->entity->writeDepend();
     ptrDepend= ptrDepend->next;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Path::Path
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Path::Path( void )               // Constructor
:  next(NULL)
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Path::allocate
//
// Purpose-
//       Allocate a new Path
//
//----------------------------------------------------------------------------
Path*                               // -> Allocated Path (Never NULL)
   Path::allocate(                  // Allocate a new Path
     const char*     pathName)      // Path name
{
   Path*             ptrPath;       // -> Allocated Path

   ptrPath= new Path();
   if( ptrPath == NULL )
   {
     fprintf(stderr, "No storage\n");
     exit(EXIT_FAILURE);
   }

   // Initialize
   strcpy(ptrPath->name, pathName);
   if( strlen(ptrPath->name) > 0
       &&ptrPath->name[strlen(ptrPath->name)-1] != '/')
     strcat(ptrPath->name, "/");

   if( pathHead == NULL )
     pathHead= ptrPath;
   else
     pathTail->next= ptrPath;
   pathTail= ptrPath;

   return ptrPath;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       infoExit
//
// Purpose-
//       Display the parameters, then exit.
//
//----------------------------------------------------------------------------
static void
   infoExit( void )                 // Informational exit
{
   printf("Proper syntax: makeproj <options> project-file\n");
   printf("\n");
   printf("Reads project-file and locates includes and other programs");
   printf(" on which it depends.\n");
   printf("It produces a file named project-file.incl to be");
   printf(" included from a makefile.\n");
   printf("\n");
   printf("Options:\n");
   printf(" -A  Treat #include <file> as #include \"file\".\n");
   printf(" -L  List dependencies.\n");
   printf(" -S: List specific dependency.\n");
   printf("\n");
   printf("Project file commands:\n");
   printf(" ## any text\n");
   printf("   Comment line.\n");
   printf(" #include \"file-name\"\n");
   printf("   Project file \"file-name\" is read.\n");
   printf(" #make whatever-you-want-to-go-right-into-the-.incl-file\n");
   printf("   Includes the directive in project_file.incl\n");
   printf(" #path path-name\n");
   printf("   Include path-name in the default file search path.\n");

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize.
//
//----------------------------------------------------------------------------
static void
   init( void )                    // Initialize
{
   int               i;

   globalErrorCount= 0;

   for(i=0; i<Entity::MaxHashTable; i++)
     Entity::hashTable[i]= NULL;

   Entity::sourceHead= NULL;

   pathHead= NULL;
   pathTail= NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       loadIncludeFile
//
// Purpose-
//       Process a new include file.
//
//----------------------------------------------------------------------------
static void
   loadIncludeFile(                 // Load an include file
     Entity*         inpEntity)     // -> Entity
{
   char              inpBuff[2048]; // Input buffer
   char              outWord[2048]; // Generic word
   char              tempName[FILENAME_SIZE];

   char*             ptrC;          // Generic character
   char*             pathSpec;      // -> Path specifier

   Entity*           ptrEntity;     // -> Entity
   FILE*             inpFile;       // Input file

   int               outX;

   // Skip if not compileable
   if( !inpEntity->isSource && !inpEntity->isInclude )
     return;

   // Only compile once
   if( inpEntity->isCompiled )
     return;
   inpEntity->isCompiled= TRUE;

   // Open the file
   inpFile= inpEntity->open();
   if( inpFile == NULL )
   {
     if( inpEntity->isSource )
       globalErrorCount++;

     fprintf(stderr, "Cannot open(%s)\n", inpEntity->getFileName());
     return;
   }
   inpEntity->isExistant= TRUE;

   // Look for a rename
   if( strcmp(openPath, inpEntity->pathName) != 0 )
   {
     ptrEntity= Entity::locate(openDesc, openPath);
     if( ptrEntity != NULL )
     {
       fclose(inpFile);
       inpEntity->rename(ptrEntity); // Rename the entity
       return;
     }

     strcpy(inpEntity->fullName, openFile);
     strcpy(inpEntity->pathName, openPath);
     inpEntity->descName= extractDesc(inpEntity->fullName);
   }

#if DEBUGGING
   printf("Reading(%s)\n", inpEntity->getFileName());
#endif

   // Read the file
   inpBuff[sizeof(inpBuff)-1]= '\0';
   while( fgets(inpBuff, sizeof(inpBuff)-1, inpFile) != NULL ) // Read a line
   {
#if 0
     printf(">>'%s'\n", inpBuff);
#endif

     ptrC= inpBuff;                 // -> First character in line
     while( isspace(*ptrC) )        // Skip leading white space
       ptrC++;

//   if( memcmp(ptrC, "#include", 8) == 0 ) // If include statement
     if( *ptrC == '#' )            // If control statement
     {
       ++ptrC;                     // Skip the '#'
       while( isspace(*ptrC) )     // Skip over white space
         ptrC++;

       if( memcmp(ptrC, "include", 7) != 0 ) // If not an include statement
         continue;

       ptrC += 7;                  // Skip over the "include"
       if( !isspace(*ptrC) )       // If missing white space after "include"
         continue;                 // (Invalid directive, skip it)

       while( isspace(*ptrC) )     // Skip over white space
         ptrC++;

       outX= 0;
       if( *ptrC == '<')           // If system include
       {
         if( !swAngle )            // If system includes ignored
           continue;               // Ignore it

         ptrC++;                   // Skip past the '<'
         while( *ptrC != '>' && *ptrC != '\n' && *ptrC != '\r')
         {
           outWord[outX++]= *ptrC;
           ptrC++;
         }
       }
       else if( *ptrC == '"' )     // A regular include
       {
         ptrC++;                   // Skip past the '"'
         while( *ptrC != '"' && *ptrC != '\n' && *ptrC != '\r')
         {
           outWord[outX++]= *ptrC;
           ptrC++;
         }
       }
       else                        // A malformed include
         continue;                 // Silently ignore it. User knows best.

       if( *ptrC == '\n' || *ptrC == '\r' ) // If invalid close quote
       {
         globalErrorCount++;
         inpBuff[strlen(inpBuff)-1]= '\0'; // Delete the trailing "\n"
         fprintf(stderr, "File(%s) contains(%s)\n",
                         inpEntity->getFileName(), inpBuff);
         fputs(inpBuff, stderr);
         fputs("\n", stderr);
         continue;
       }
       outWord[outX]= '\0';        // Terminate name

       pathSpec= NULL;             // Default, no path specifier
       if( *ptrC == '"' )          // If this is a direct include
       {
         strcpy(tempName, inpEntity->pathName);
         strcat(tempName, outWord);
         if( isFileReadable(tempName) )
           pathSpec= inpEntity->pathName;
       }

       ptrEntity= Entity::locate(outWord, pathSpec);
       if( ptrEntity == NULL )
         ptrEntity= Entity::allocate(Entity::TypeInclude, outWord, pathSpec);
       ptrEntity->insSourceList();
       ptrEntity->isInclude= TRUE;
       inpEntity->addDepend(ptrEntity);
     }
     else if (memcmp(ptrC,"extern ",7)==0) // If external procedure
     {
       // Right now we don't care
     }
   }

#if DEBUGGING
   printf("...read(%s)\n", inpEntity->getFileName());
#endif
   fclose(inpFile);                 // Close file being searched
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       loadProjectFile
//
// Purpose-
//       Process a new project file.
//
//----------------------------------------------------------------------------
static void
   loadProjectFile(                 // Load a project file
     Entity*         inpEntity,     // -> Entity
     FILE*           inpFile)       // -> FILE, may be NULL
{
   const char*       const inpName=   inpEntity->fullName;

   char              inpBuff[2048]; // Input buffer
   char              outWord[2048]; // Input word
   const char*       ptrC;          // Generic character

   Entity*           priEntity;     // -> Primary Entity
   Entity*           ptrEntity;     // -> Entity
   FILE*             ptrFile;       // -> FILE

   int               i;

   // Insure that the file isn't already open
   if( inpEntity->isGlobal() )
   {
     globalErrorCount++;
     fprintf(stderr, "Circular project file:\n");
     for(i=1; i<Entity::entityCount; i++)
     {
       fprintf(stderr, "From(%s) include(%s)\n",
               Entity::entityList[i-1]->fullName,
               Entity::entityList[i]->fullName);
     }
     fprintf(stderr, "From(%s) include(%s)\n\n",
                     Entity::entityList[Entity::entityCount-1]->fullName,
                     inpEntity->fullName);
     return;
   }

   ptrFile= inpFile;                // Use the specified file
   if( ptrFile == NULL )            // If the file hasn't been opened
   {
     ptrFile= fopen(inpName, "rb"); // Open the file
     if( ptrFile == NULL )          // If open failure
     {
       globalErrorCount++;
       if( Entity::entityCount == 0 )
         fprintf(stderr, "Cannot open(%s)\n", inpEntity->fullName);
       else
         fprintf(stderr, "From(%s) cannot open(%s)\n",
                         Entity::entityList[Entity::entityCount-1]->fullName,
                         inpEntity->fullName);
       return;
     }
   }

   // The file is now open
   inpEntity->pushGlobal();
   inpEntity->isExistant= TRUE;

   // Read the file
#if DEBUGGING
   printf("Project(%s)\n", inpName);
#endif

   inpBuff[sizeof(inpBuff)-1]= '\0';
   while( fgets(inpBuff, sizeof(inpBuff)-10, ptrFile) != NULL ) // Read a line
   {
     while( strlen(inpBuff) > 0 )        // Remove trailing '\r' or '\n'
     {
       if( inpBuff[strlen(inpBuff)-1] != '\r'
           && inpBuff[strlen(inpBuff)-1] != '\n' )
         break;
       inpBuff[strlen(inpBuff)-1]= '\0'; // Remove end of line delimiter
     }
#if 0
     printf(">>'%s'\n", inpBuff);
#endif
     for(i=0; inpBuff[i] == ' '; i++)    // Skip leading blanks
       ;

     if( inpBuff[i] == '\0' )            // If empty line
       continue;
     if( inpBuff[i] == '*' )             // If comment line
       continue;
     if( inpBuff[i] == '#' )             // If directive
     {
       if( inpBuff[i+1] == '#' )         // If comment
         continue;
       else if( memcmp(&inpBuff[i], "#include",8) == 0 )
       {
         ptrC= strchr(inpBuff,'"');      // -> " before name
         if( ptrC == NULL )              // " is missing
         {
           globalErrorCount++;
           fprintf(stderr, "In(%s), no start quote(%s)\n",
                           inpName, &inpBuff[i]);
           continue;
         }
         ptrC++;                         // -> Project file name
         if( strchr(ptrC,'"') == NULL )  // -> Closing "
         {
           globalErrorCount++;
           fprintf(stderr, "In(%s), no final quote(%s)\n",
                           inpName, &inpBuff[i]);
           continue;
         }
         *strchr((char*)ptrC,'"')= '\0'; // Terminate the filename string

         ptrEntity= Entity::locate(ptrC);
         if( ptrEntity == NULL )
           ptrEntity= Entity::allocate(Entity::TypeProject, ptrC);
         inpEntity->addDepend(ptrEntity);

         loadProjectFile(ptrEntity, NULL); // Load the project file
       }
       else if( memcmp(&inpBuff[i], "#make ",  6) == 0 )
       {
         fputs(&inpBuff[i+6], outFile);
         fputs("\n", outFile);
       }
       else if( strcmp(&inpBuff[i], "#make") == 0 )
         fputs("\n", outFile);
       else if( memcmp(&inpBuff[i], "#path ",  6) == 0 )
       {
         for(i= i+5; inpBuff[i] == ' '; i++) // Skip blanks
           ;
         Path::allocate( &inpBuff[i] );
       }
       else
       {
         globalErrorCount++;
         fprintf(stderr, "In(%s), what's(%s)\?\n",
                         inpName, &inpBuff[i]);
         continue;
       }
     }

     // Not a directive -- must be input line
     else
     {
       // Insert the primary dependency
       ptrC= &inpBuff[i];           // First non-blank
       ptrC= extractWord(ptrC, outWord);

       priEntity= Entity::locate(outWord);
       if( priEntity == NULL )
         priEntity= Entity::allocate(Entity::TypeSource, outWord);
       inpEntity->addDepend(priEntity);

       // Extract further dependencies
       for(;;)
       {
         ptrC= extractWord(ptrC, outWord);
         if( outWord[0] == '\0' )
           break;

         ptrEntity= Entity::locate(outWord);
         if( ptrEntity == NULL )
           ptrEntity= Entity::allocate(Entity::TypeInclude, outWord);
         priEntity->addDepend(ptrEntity);
       }

       // Determine whether the primary dependency is for a source file
       ptrC= extractType(priEntity->descName);
       if( ptrC != NULL             // If a filetype exists
           &&toupper(*ptrC) == 'C' )// and it's a C or CPP file
         priEntity->isSource= TRUE;
     }
   }

   // Close the file
   inpEntity->popGlobal();
   fclose(ptrFile);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       openOutput
//
// Purpose-
//       Open an output file.
//
//----------------------------------------------------------------------------
static FILE*                        // -> FILE
   openOutput(                      // Open an output file
     const char*     inpName,       // The input file name
     const char*     extension)     // The required extension
{
   FILE*             outFile;       // Output FILE
   char              outName[FILENAME_SIZE]; // The output file name
   char*             ptrC;          // -> Generic character

   // Open the output file
   strcpy(outName, inpName);
   ptrC= extractDesc(outName);
   ptrC= extractType(ptrC);
   if( ptrC != NULL )
   {
     ptrC--;
     *ptrC= '\0';
   }
   strcat(outName, extension);
   outFile= fopen(outName, "w");
   if( outFile == NULL )
     fprintf(stderr, "Cannot open(%s)\n", outName);

   return outFile;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
static int                          // Return code
   parm(                            // Parameter analysis
     int             argc,          // Argument count
     const char*     argv[])        // Argument list
{
   const char*       ptrArg;        // -> Current argument
   int               argn;          // Argument being looked at
   int               errorCount=0;  // Number of errors encountered

   projName= NULL;                  // No project name
   swAngle= FALSE;                  // Do not follow <angle> files
   swBom=   FALSE;                  // Do not list bill of materials
   swList=  FALSE;                  // Do not list dependencies
   swName=  NULL;                   // No specific dependency name
   for (argn=1; argn<argc; argn++)  // Examine the arguments
   {
     ptrArg= argv[argn];            // Address the argument
     if( argv[argn][0] == '-' )     // This argument is an option
     {
       ptrArg++;                    // Skip the '-'
       if( toupper(*ptrArg) == 'A' )// If <angle> files are to be included
         swAngle= TRUE;
       else if( toupper(*ptrArg) == 'B' ) // If bill of materials required
         swBom= TRUE;
       else if( toupper(*ptrArg) == 'L' ) // If dependencies are to be listed
         swList= TRUE;
       else if( toupper(*ptrArg) == 'S' ) // If specific dependency
       {
         if( *(ptrArg+1) != ':' )
         {
           errorCount++;
           fprintf(stderr, "Invalid parameter(-%s)\n", ptrArg);
         }
         else
           swName= ptrArg+2;
       }
       else                         // Invalid switch
       {
         errorCount++;
         fprintf(stderr, "Invalid parameter(-%s)\n", ptrArg);
       }
     }
     else                           // This argument is a name
       if( projName == NULL )       // Don't have a file name yet
       {
         projName= ptrArg;
         continue;
       }
       else                         // Already have a name
       {
         errorCount++;
         fprintf(stderr, "Unexpected parameter(%s)\n", argv[argn]);
       }
   }

   if( errorCount != 0 )
     infoExit();
   return 0;
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
int                                 // Return code
   main(                            // Mainline code
     int             argc,          // Argument count
     const char*     argv[])        // Argument list
{
   char              inpName[FILENAME_SIZE]; // The input file name

   Entity*           ptrEntity;     // -> Entity
   FILE*             inpFile;       // Input file

   int               i;

   // Initialize
   init();

   // Analyze the parameters
   parm(argc, argv);

   // Open the input file
   if( projName == NULL )           // If default project name
   {
     projName= "makeproj.project";  // Default fileName
     inpFile= fopen(projName, "rb");// Open the input file
     if( inpFile == NULL )
     {
       projName= "makeproj.proj";   // Default fileName
       inpFile= fopen(projName, "rb"); // Open the input file
       if( inpFile == NULL )
       {
         projName= "makeproj";      // Default fileName
         inpFile= fopen(projName, "rb"); // Open the input file
         if( inpFile == NULL )
         {
           projName= "makeproj.project"; // Default fileName
           fprintf(stderr, "Cannot open(%s)\n", projName);
           exit(EXIT_FAILURE);
         }
       }
     }

     strcpy(inpName, projName);
   }
   else                             // Project name was specified
   {
     if( (strlen(projName) + strlen(".project")) >= sizeof(inpName) )
     {
       fprintf(stderr, "Name(%s) too long\n", projName);
       exit(EXIT_FAILURE);
     }

     strcpy(inpName, projName);
     inpFile= fopen(inpName, "rb"); // Open the input file
     if( inpFile == NULL )          // If the default failed
     {
       strcat(inpName, ".proj");
       inpFile= fopen(inpName, "rb"); // Open the input file
       if( inpFile == NULL )
       {
         strcat(inpName, "ect");

         inpFile= fopen(inpName, "rb"); // Open the input file
         if( inpFile == NULL )
         {
           fprintf(stderr, "Cannot open(%s)\n", inpName);
           exit(EXIT_FAILURE);
         }
       }
     }
   }

   // Create the root Entity
   sysEntity= Entity::allocate(Entity::TypeProject, inpName);

   // Open the output file
   outFile= openOutput(inpName, ".incl");
   if( outFile == NULL )
     exit(EXIT_FAILURE);

   // Load the input file
   Entity::resetGlobal();
   loadProjectFile(sysEntity, inpFile);
   if( globalErrorCount != 0 )
   {
     fprintf(stderr, "%d Error%s\n",
                     globalErrorCount,
                     (globalErrorCount > 1) ? "s" : "");
     exit(EXIT_FAILURE);
   }

   // Load the include files
   for(;;)
   {
     ptrEntity= Entity::remSourceList();
     if( ptrEntity == NULL )
       break;

     loadIncludeFile(ptrEntity);
   }

   if( globalErrorCount != 0 )
   {
     fprintf(stderr, "%d Error%s\n",
                     globalErrorCount,
                     (globalErrorCount > 1) ? "s" : "");
     exit(EXIT_FAILURE);
   }

   // Sort the dependencies
   Entity::resetGlobal();
   sysEntity->sortDepend();

   // Create the dependency file
   Entity::resetGlobal();
   sysEntity->resetHandled();
   sysEntity->writeDepend();

   // List the bill of materials
   if( swBom )
   {
     bomFile= openOutput(inpName, ".bom");
     if( bomFile != NULL )
     {
       Entity::resetGlobal();
       sysEntity->resetHandled();
       sysEntity->resolveDepend();
       Entity::sortEntityList();
       for(i=1; i<Entity::entityCount; i++)
       {
         fputs(Entity::entityList[i]->getFileName(), bomFile);
         fputs("\n", bomFile);
       }
     }
   }

   // List the dependencies
   if( swList )
   {
     Entity::resetGlobal();
     sysEntity->resetHandled();
     sysEntity->showEntityRelation(0);
   }

   if( swName != NULL )
   {
     printf("\n");
     printf("Dependency(%s)\n", swName);
     Entity::resetGlobal();
     sysEntity->resetHandled();
     sysEntity->showEntityRelation(0, swName);
   }

   // Function complete
   return 0;
}

