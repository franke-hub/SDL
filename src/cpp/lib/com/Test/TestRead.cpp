//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestRead.cpp
//
// Purpose-
//       Test the Reader and Writer objects.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/Verify.h>

#include "com/Media.h"
#include "com/Reader.h"
#include "com/Writer.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#define ITERATIONS 10000

//----------------------------------------------------------------------------
//
// Subroutine-
//       testConstructors
//
// Purpose-
//       Test constructors.
//
//----------------------------------------------------------------------------
void
   testConstructors( void )         // Test constructors
{
   verify_info(); debugf("testConstructors()\n");

   MediaReader         reader;
   MediaReader         sizedReader(8192);
   MediaWriter         writer;
   MediaWriter         sizedWriter(8192);
   FileMedia           fileMedia;
   TempMedia           tempMedia;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testFileMedia
//
// Purpose-
//       Test the FileMedia methods.
//
//----------------------------------------------------------------------------
void
   testFileMedia( void )            // Test FileMedia object
{
   verify_info(); debugf("testFileMedia()\n");

   Media::Byte         string[128];
   Media::Byte         inpstr[128];

   FileMedia           media;       // The FileMedia
   unsigned long       L;           // Actual length
   unsigned long       length;      // Desired length
   int                 i;

   memset(string, 0, sizeof(string));
   memset(inpstr, 0, sizeof(string));
   sprintf(string, "This is line %6d of %6d\n", 0, ITERATIONS);
   length= strlen(string);

   media.open("MediaTest.out", Media::MODE_WRITE);
   for(i= 1; i<=ITERATIONS; i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= media.write(string, length);
     if( !verify(L == length) )
       break;
   }
   media.close();

   media.open("MediaTest.out", Media::MODE_READ);
   for(i=1;;i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= media.read(inpstr, length);
     if( L == 0 )
       break;

     verify( L == length );
     if( !verify(strcmp(string,inpstr) == 0) )
     {
       debugf("Expected(%s) Got(%s)\n", string, inpstr);
       break;
     }
   }
   media.close();

   if( !verify(i == (ITERATIONS+1)) )
     debugf("i(%d) inpstr(%s)\n", i, inpstr);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testTempMedia
//
// Purpose-
//       Test the TempMedia methods.
//
//----------------------------------------------------------------------------
void
   testTempMedia( void )            // Test TempMedia object
{
   verify_info(); debugf("testTempMedia()\n");

   Media::Byte         string[128];
   Media::Byte         inpstr[128];

   TempMedia           media;       // The FileMedia
   unsigned long       L;           // Actual length
   unsigned long       length;      // Desired length
   int                 i;

   memset(string, 0, sizeof(string));
   memset(inpstr, 0, sizeof(string));
   sprintf(string, "This is line %6d of %6d\n", 0, ITERATIONS);
   length= strlen(string);

   media.open("MediaTest.out", Media::MODE_WRITE);
   for(i= 1; i<=ITERATIONS; i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= media.write(string, length);
     if( !verify(L == length) )
       break;
   }
   media.close();

   media.open("MediaTest.out", Media::MODE_READ);
   for(i=1;;i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= media.read(inpstr, length);
     if( L == 0 )
       break;

     verify( L == length );
     if( !verify(strcmp(string,inpstr) == 0) )
     {
       debugf("Expected(%s) Got(%s)\n", string, inpstr);
       break;
     }
   }
   media.close();

   if( !verify(i == (ITERATIONS+1)) )
     debugf("i(%d) inpstr(%s)\n", i, inpstr);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testPushPull
//
// Purpose-
//       Test the Writer push and Reader pull methods.
//
//----------------------------------------------------------------------------
void
   testPushPull( void )             // Test Writer push, Reader pull
{
   verify_info(); debugf("testPushPull()\n");

   Media::Byte         string[128];
   Media::Byte         inpstr[128];

   Media::Byte*        pushData;    // Push data
   const Media::Byte*  pullData;    // Pull data
   TempMedia           media;       // The Media
   MediaReader         reader(4096);// The MediaReader
   MediaWriter         writer(4096);// The MediaWriter
   unsigned long       length;      // Desired length
   int                 i;

   reader.attach(media);
   writer.attach(media);
   memset(string, 0, sizeof(string));
   memset(inpstr, 0, sizeof(string));
   sprintf(string, "This is line %6d of %6d\n", 0, ITERATIONS);
   length= strlen(string);

   writer.open("Media.out");
   for(i= 1; i<=ITERATIONS; i++)
   {
     pushData= writer.push(length);
     if( !verify(pushData != NULL) )
       break;

     sprintf(pushData, "This is line %6d of %6d\n", i, ITERATIONS);
   }
   writer.close();

   reader.open("Media.out");
   for(i=1;;i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);

     pullData= reader.pull(length);
     if( pullData == NULL )
       break;

     memcpy(inpstr, pullData, length);

     if( !verify(strcmp(string,inpstr) == 0) )
     {
       debugf("Expected(%s) Got(%s)\n", string, inpstr);
       break;
     }
   }
   reader.close();

   if( !verify(i == (ITERATIONS+1)) )
     debugf("i(%d) inpstr(%s)\n", i, inpstr);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testPutGet
//
// Purpose-
//       Test the Writer put and Reader get methods.
//
//----------------------------------------------------------------------------
void
   testPutGet( void )               // Test Writer put, Reader get
{
   verify_info(); debugf("testPutGet()\n");

   Media::Byte         string[128];
   Media::Byte         inpstr[128];

   TempMedia           media;       // The Media
   MediaReader         reader(4096);// The MediaReader
   MediaWriter         writer(4096);// The MediaWriter
   int                 C= EOF;      // Input character
   unsigned long       length;      // Desired length
   int                 i, j;

   reader.attach(media);
   writer.attach(media);
   memset(string, 0, sizeof(string));
   memset(inpstr, 0, sizeof(string));
   sprintf(string, "This is line %6d of %6d\n", 0, ITERATIONS);
   length= strlen(string);

   writer.open("Media.out");
   for(i= 1; i<=ITERATIONS; i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     for(j= 0; string[j] != '\0'; j++)
       writer.put(string[j]);
   }
   writer.close();

   reader.open("Media.out");
   for(i=1;;i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     for(j= 0; j<length; j++)
     {
       C= reader.get();
       if( C == EOF )
       {
         verify( j == 0 );
         break;
       }
       inpstr[j]= C;
     }
     if( C == EOF )
       break;

     if( !verify(strcmp(string,inpstr) == 0) )
     {
       debugf("Expected(%s) Got(%s)\n", string, inpstr);
       break;
     }
   }
   reader.close();

   if( !verify(i == (ITERATIONS+1)) )
     debugf("i(%d) inpstr(%s)\n", i, inpstr);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testWriteRead
//
// Purpose-
//       Test the Writer write and Reader read methods.
//
//----------------------------------------------------------------------------
void
   testWriteRead( void )            // Test Writer write, Reader read
{
   verify_info(); debugf("testWriteRead()\n");

   Media::Byte         string[128];
   Media::Byte         inpstr[128];

   TempMedia           media;       // The Media
   MediaReader         reader(4096);// The MediaReader
   MediaWriter         writer(4096);// The MediaWriter
   unsigned long       L;           // Actual length
   unsigned long       length;      // Desired length
   int                 i;

   reader.attach(media);
   writer.attach(media);
   memset(string, 0, sizeof(string));
   memset(inpstr, 0, sizeof(string));
   sprintf(string, "This is line %6d of %6d\n", 0, ITERATIONS);
   length= strlen(string);

   writer.open("Media.out");
   for(i= 1; i<=ITERATIONS; i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= writer.write(string, length);
     if( !verify(L == length) )
       break;
   }
   writer.close();

   reader.open("Media.out");
   for(i=1;;i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= reader.read(inpstr, length);
     if( L == 0 )
       break;

     verify( L == length );
     if( !verify(strcmp(string,inpstr) == 0) )
     {
       debugf("Expected(%s) Got(%s)\n", string, inpstr);
       break;
     }
   }
   reader.close();

   if( !verify(i == (ITERATIONS+1)) )
     debugf("i(%d) inpstr(%s)\n", i, inpstr);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testPrintf
//
// Purpose-
//       Test the Writer printf method.
//
//----------------------------------------------------------------------------
void
   testPrintf( void )               // Test Writer printf, Reader read
{
   verify_info(); debugf("testPrintf()\n");

   Media::Byte         string[128];
   Media::Byte         inpstr[128];

   TempMedia           media;       // The Media
   MediaReader         reader(4096);// The MediaReader
   MediaWriter         writer(4096);// The MediaWriter
   unsigned long       L;           // Actual input length
   unsigned long       length;      // Desired input length
   int                 i;

   reader.attach(media);
   writer.attach(media);
   memset(string, 0, sizeof(string));
   memset(inpstr, 0, sizeof(string));
   sprintf(string, "This is line %6d of %6d\n", 0, ITERATIONS);
   length= strlen(string);

   writer.open("Media.out");
   for(i= 1; i<=ITERATIONS; i++)
     writer.printf("This is line %6d of %6d\n", i, ITERATIONS);
   writer.close();

   reader.open("Media.out");
   for(i=1;;i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= reader.read(inpstr, length);
     if( L == 0 )
       break;

     verify( L == length );
     if( !verify(strcmp(string,inpstr) == 0) )
     {
       debugf("Expected(%s) Got(%s)\n", string, inpstr);
       break;
     }
   }
   reader.close();

   if( !verify(i == (ITERATIONS+1)) )
     debugf("i(%d) inpstr(%s)\n", i, inpstr);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testReadline
//
// Purpose-
//       Test the Writer write and Reader readLine methods.
//
//----------------------------------------------------------------------------
void
   testReadline( void )             // Test Writer write, Reader readLine
{
   verify_info(); debugf("testReadline()\n");

   Media::Byte         string[128];
   Media::Byte         inpstr[128];

   TempMedia           media;       // The Media
   MediaReader         reader(4096);// The MediaReader
   MediaWriter         writer(4096);// The MediaWriter
   int                 C;           // Delimiter character
   unsigned long       L;           // Actual length
   unsigned long       length;      // Desired length
   int                 i;

   reader.attach(media);
   writer.attach(media);
   memset(string, 0, sizeof(string));
   memset(inpstr, 0, sizeof(string));
   sprintf(string, "This is line %6d of %6d\n", 0, ITERATIONS);
   length= strlen(string);

   writer.open("Media.out");
   for(i= 1; i<=ITERATIONS; i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= writer.write(string, length);
     if( !verify(L == length) )
       break;
   }
   writer.close();

   memset(string, 0, sizeof(string));
   reader.open("Media.out");
   for(i=1;;i++)
   {
     sprintf(string, "This is line %6d of %6d", i, ITERATIONS);
     C= reader.readLine(inpstr, sizeof(inpstr));
     if( C <  0 )
       break;

     if( !verify(strcmp(string,inpstr) == 0) )
     {
       debugf("Expected(%s) Got(%s)\n", string, inpstr);
       break;
     }
   }
   reader.close();

   verify( C == Media::RC_EOF );
   verify( strcmp(inpstr, "") == 0 );
   if( !verify(i == (ITERATIONS+1)) )
     debugf("i(%d) inpstr(%s)\n", i, inpstr);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testSkipline
//
// Purpose-
//       Test the Writer write and Reader skipLine methods.
//
//----------------------------------------------------------------------------
void
   testSkipline( void )             // Test Writer write, Reader skipLine
{
   verify_info(); debugf("testSkipline()\n");

   Media::Byte         string[128];
   Media::Byte         inpstr[128];

   TempMedia           media;       // The Media
   MediaReader         reader(4096);// The MediaReader
   MediaWriter         writer(4096);// The MediaWriter
   int                 C;           // Delimiter character
   unsigned long       L;           // Actual length
   unsigned long       length;      // Desired length
   int                 i;

   reader.attach(media);
   writer.attach(media);
   memset(string, 0, sizeof(string));
   memset(inpstr, 0, sizeof(string));
   sprintf(string, "This is line %6d of %6d\n", 0, ITERATIONS);
   length= strlen(string);

   writer.open("Media.out");
   for(i= 1; i<=ITERATIONS; i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= writer.write(string, length);
     if( !verify(L == length) )
       break;
   }
   writer.close();

   memset(string, 0, sizeof(string));
   reader.open("Media.out");
   for(i=1;;i++)
   {
     sprintf(string, "This is line %6d of %6d", i, ITERATIONS);
     if( (i&1) == 1 )
       C= reader.skipLine();
     else
       C= reader.readLine(inpstr, sizeof(inpstr));
     if( C <  0 )
       break;

     if( (i&1) == 0 && !verify(strcmp(string,inpstr) == 0) )
     {
       debugf("Expected(%s) Got(%s)\n", string, inpstr);
       break;
     }
   }
   reader.close();

   verify( C == Media::RC_EOF );
   if( !verify(i == (ITERATIONS+1)) )
     debugf("i(%d) inpstr(%s)\n", i, inpstr);
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
extern int
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Initialization
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugSetIntensiveMode();
     verify_info(); debugf("HCDM\n");
   #endif

   //-------------------------------------------------------------------------
   // Run the tests
   //-------------------------------------------------------------------------
   try
   {
     //-----------------------------------------------------------------------
     // Prerequisite tests
     //-----------------------------------------------------------------------
     if( 1 )
     {
       testConstructors();
       testFileMedia();
       testTempMedia();
     }
     if( error_count() != 0 )
       throw "Prerequisite test failure";

     //-----------------------------------------------------------------------
     // Object tests
     //-----------------------------------------------------------------------
     if( 1 )
     {
       testPushPull();
       testPutGet();
       testWriteRead();
       testPrintf();
       testReadline();
       testSkipline();
     }
   }
   catch(const char* X)
   {
     error_found();
     verify_info(); debugf("EXCEPTION(const char*((%s))\n", X);
   }
   catch(...)
   {
     verify("EXCEPTION(...)");
   }

//----------------------------------------------------------------------------
// Testing complete
//----------------------------------------------------------------------------
   verify_exit();
}

