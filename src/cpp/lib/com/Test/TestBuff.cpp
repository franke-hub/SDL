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
//       TestBuff.cpp
//
// Purpose-
//       Test the Buffer object.
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
#include "com/Buffer.h"

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

   MediaBuffer         buffer;
   MediaBuffer         sizedBuffer(8192);
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
//       testFileBuffer
//
// Purpose-
//       Test the FileBuffer object.
//
//----------------------------------------------------------------------------
void
   testFileBuffer( void )           // Test FileBuffer object
{
   verify_info(); debugf("testFileBuffer()\n");

   Media::Byte         string[128];
   Media::Byte         inpstr[128];

   FileBuffer          buffer;      // The FileBuffer
   unsigned long       L;           // Actual length
   unsigned long       length;      // Desired length
   int                 i;

   memset(string, 0, sizeof(string));
   memset(inpstr, 0, sizeof(string));
   sprintf(string, "This is line %6d of %6d\n", 0, ITERATIONS);
   length= strlen(string);

   buffer.open("MediaTest.out", Media::MODE_WRITE);
   for(i= 1; i<=ITERATIONS; i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= buffer.write(string, length);
     if( !verify(L == length) )
       break;
   }
   buffer.close();

   buffer.open("MediaTest.out", Media::MODE_READ);
   for(i=1;;i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= buffer.read(inpstr, length);
     if( L == 0 )
       break;

     verify( L == length );
     if( !verify(strcmp(string,inpstr) == 0) )
     {
       debugf("Expected(%s) Got(%s)\n", string, inpstr);
       break;
     }
   }
   buffer.close();

   if( !verify(i == (ITERATIONS+1)) )
     debugf("i(%d) inpstr(%s)\n", i, inpstr);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testTempBuffer
//
// Purpose-
//       Test the TempBuffer object.
//
//----------------------------------------------------------------------------
void
   testTempBuffer( void )           // Test TempBuffer object
{
   verify_info(); debugf("testTempBuffer()\n");

   Media::Byte         string[128];
   Media::Byte         inpstr[128];

   TempBuffer          buffer;      // The TempBuffer
   unsigned long       L;           // Actual length
   unsigned long       length;      // Desired length
   int                 i;

   memset(string, 0, sizeof(string));
   memset(inpstr, 0, sizeof(string));
   sprintf(string, "This is line %6d of %6d\n", 0, ITERATIONS);
   length= strlen(string);

   buffer.open("MediaTest.out", Media::MODE_WRITE);
   for(i= 1; i<=ITERATIONS; i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= buffer.write(string, length);
     if( !verify(L == length) )
       break;
   }
   buffer.close();

   buffer.open("MediaTest.out", Media::MODE_READ);
   for(i=1;;i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= buffer.read(inpstr, length);
     if( L == 0 )
       break;

     verify( L == length );
     if( !verify(strcmp(string,inpstr) == 0) )
     {
       debugf("Expected(%s) Got(%s)\n", string, inpstr);
       break;
     }
   }
   buffer.close();

   if( !verify(i == (ITERATIONS+1)) )
     debugf("i(%d) inpstr(%s)\n", i, inpstr);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testPushPull
//
// Purpose-
//       Test the Buffer push and pull methods.
//
//----------------------------------------------------------------------------
void
   testPushPull( void )             // Test Buffer push/pull
{
   verify_info(); debugf("testPushPull()\n");

   Media::Byte         string[128];
   Media::Byte         inpstr[128];

   Media::Byte*        pushData;    // Push data
   const Media::Byte*  pullData;    // Pull data
   TempMedia           media;       // The Media
   MediaBuffer         buffer(4096);// The MediaBuffer
   unsigned long       length;      // Desired length
   int                 i;

   buffer.attach(media);
   memset(string, 0, sizeof(string));
   memset(inpstr, 0, sizeof(string));
   sprintf(string, "This is line %6d of %6d\n", 0, ITERATIONS);
   length= strlen(string);

   buffer.open("Media.out", Media::MODE_WRITE);
   for(i= 1; i<=ITERATIONS; i++)
   {
     pushData= buffer.push(length);
     if( !verify(pushData != NULL) )
       break;

     sprintf(pushData, "This is line %6d of %6d\n", i, ITERATIONS);
   }
   buffer.close();

   buffer.open("Media.out", Media::MODE_READ);
   for(i=1;;i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);

     pullData= buffer.pull(length);
     if( pullData == NULL )
       break;

     memcpy(inpstr, pullData, length);

     if( !verify(strcmp(string,inpstr) == 0) )
     {
       debugf("Expected(%s) Got(%s)\n", string, inpstr);
       break;
     }
   }
   buffer.close();

   if( !verify(i == (ITERATIONS+1)) )
     debugf("i(%d) inpstr(%s)\n", i, inpstr);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testPutGet
//
// Purpose-
//       Test the Buffer put and get methods.
//
//----------------------------------------------------------------------------
void
   testPutGet( void )               // Test Buffer put/get
{
   verify_info(); debugf("testPutGet()\n");

   Media::Byte         string[128];
   Media::Byte         inpstr[128];

   TempMedia           media;       // The Media
   MediaBuffer         buffer(4096);// The MediaBuffer
   int                 C= EOF;      // Input character
   unsigned long       length;      // Desired length
   int                 i, j;

   buffer.attach(media);
   buffer.attach(media);
   memset(string, 0, sizeof(string));
   memset(inpstr, 0, sizeof(string));
   sprintf(string, "This is line %6d of %6d\n", 0, ITERATIONS);
   length= strlen(string);

   buffer.open("Media.out", Media::MODE_WRITE);
   for(i= 1; i<=ITERATIONS; i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     for(j= 0; string[j] != '\0'; j++)
       buffer.put(string[j]);
   }
   buffer.close();

   buffer.open("Media.out", Media::MODE_READ);
   for(i=1;;i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     for(j= 0; j<length; j++)
     {
       C= buffer.get();
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
   buffer.close();

   if( !verify(i == (ITERATIONS+1)) )
     debugf("i(%d) inpstr(%s)\n", i, inpstr);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testWriteRead
//
// Purpose-
//       Test the Buffer write and read methods.
//
//----------------------------------------------------------------------------
void
   testWriteRead( void )            // Test Buffer write/read
{
   verify_info(); debugf("testWriteRead()\n");

   Media::Byte         string[128];
   Media::Byte         inpstr[128];

   TempMedia           media;       // The Media
   MediaBuffer         buffer(4096);// The MediaBuffer
   unsigned long       L;           // Actual length
   unsigned long       length;      // Desired length
   int                 i;

   buffer.attach(media);
   memset(string, 0, sizeof(string));
   memset(inpstr, 0, sizeof(string));
   sprintf(string, "This is line %6d of %6d\n", 0, ITERATIONS);
   length= strlen(string);

   buffer.open("Media.out", Media::MODE_WRITE);
   for(i= 1; i<=ITERATIONS; i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= buffer.write(string, length);
     if( !verify(L == length) )
       break;
   }
   buffer.close();

   buffer.open("Media.out", Media::MODE_READ);
   for(i=1;;i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= buffer.read(inpstr, length);
     if( L == 0 )
       break;

     verify( L == length );
     if( !verify(strcmp(string,inpstr) == 0) )
     {
       debugf("Expected(%s) Got(%s)\n", string, inpstr);
       break;
     }
   }
   buffer.close();

   if( !verify(i == (ITERATIONS+1)) )
     debugf("i(%d) inpstr(%s)\n", i, inpstr);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testPrintf
//
// Purpose-
//       Test the Buffer printf method.
//
//----------------------------------------------------------------------------
void
   testPrintf( void )               // Test Buffer printf, read
{
   verify_info(); debugf("testPrintf()\n");

   Media::Byte         string[128];
   Media::Byte         inpstr[128];

   TempMedia           media;       // The Media
   MediaBuffer         buffer(4096);// The MediaBuffer
   unsigned long       L;           // Actual input length
   unsigned long       length;      // Desired input length
   int                 i;

   buffer.attach(media);
   memset(string, 0, sizeof(string));
   memset(inpstr, 0, sizeof(string));
   sprintf(string, "This is line %6d of %6d\n", 0, ITERATIONS);
   length= strlen(string);

   buffer.open("Media.out", Media::MODE_WRITE);
   for(i= 1; i<=ITERATIONS; i++)
     buffer.printf("This is line %6d of %6d\n", i, ITERATIONS);
   buffer.close();

   buffer.open("Media.out", Media::MODE_READ);
   for(i=1;;i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= buffer.read(inpstr, length);
     if( L == 0 )
       break;

     verify( L == length );
     if( !verify(strcmp(string,inpstr) == 0) )
     {
       debugf("Expected(%s) Got(%s)\n", string, inpstr);
       break;
     }
   }
   buffer.close();

   if( !verify(i == (ITERATIONS+1)) )
     debugf("i(%d) inpstr(%s)\n", i, inpstr);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testReadline
//
// Purpose-
//       Test the Buffer write and readLine methods.
//
//----------------------------------------------------------------------------
void
   testReadline( void )             // Test Buffer write/readLine
{
   verify_info(); debugf("testReadline()\n");

   Media::Byte         string[128];
   Media::Byte         inpstr[128];

   TempMedia           media;       // The Media
   MediaBuffer         buffer(4096);// The MediaBuffer
   int                 C;           // Delimiter character
   unsigned long       L;           // Actual length
   unsigned long       length;      // Desired length
   int                 i;

   buffer.attach(media);
   memset(string, 0, sizeof(string));
   memset(inpstr, 0, sizeof(string));
   sprintf(string, "This is line %6d of %6d\n", 0, ITERATIONS);
   length= strlen(string);

   buffer.open("Media.out", Media::MODE_WRITE);
   for(i= 1; i<=ITERATIONS; i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= buffer.write(string, length);
     if( !verify(L == length) )
       break;
   }
   buffer.close();

   memset(string, 0, sizeof(string));
   buffer.open("Media.out", Media::MODE_READ);
   for(i=1;;i++)
   {
     sprintf(string, "This is line %6d of %6d", i, ITERATIONS);
     C= buffer.readLine(inpstr, sizeof(inpstr));
     if( C <  0 )
       break;

     if( !verify(strcmp(string,inpstr) == 0) )
     {
       debugf("Expected(%s) Got(%s)\n", string, inpstr);
       break;
     }
   }
   buffer.close();

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
//       Test the Buffer write and skipLine methods.
//
//----------------------------------------------------------------------------
void
   testSkipline( void )             // Test Buffer write/skipLine
{
   verify_info(); debugf("testSkipline()\n");

   Media::Byte         string[128];
   Media::Byte         inpstr[128];

   TempMedia           media;       // The Media
   MediaBuffer         buffer(4096);// The MediaBuffer
   int                 C;           // Delimiter character
   unsigned long       L;           // Actual length
   unsigned long       length;      // Desired length
   int                 i;

   buffer.attach(media);
   memset(string, 0, sizeof(string));
   memset(inpstr, 0, sizeof(string));
   sprintf(string, "This is line %6d of %6d\n", 0, ITERATIONS);
   length= strlen(string);

   buffer.open("Media.out", Media::MODE_WRITE);
   for(i= 1; i<=ITERATIONS; i++)
   {
     sprintf(string, "This is line %6d of %6d\n", i, ITERATIONS);
     L= buffer.write(string, length);
     if( !verify(L == length) )
       break;
   }
   buffer.close();

   memset(string, 0, sizeof(string));
   buffer.open("Media.out", Media::MODE_READ);
   for(i=1;;i++)
   {
     sprintf(string, "This is line %6d of %6d", i, ITERATIONS);
     if( (i&1) == 1 )
       C= buffer.skipLine();
     else
       C= buffer.readLine(inpstr, sizeof(inpstr));
     if( C <  0 )
       break;

     if( (i&1) == 0 && !verify(strcmp(string,inpstr) == 0) )
     {
       debugf("Expected(%s) Got(%s)\n", string, inpstr);
       break;
     }
   }
   buffer.close();

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
     debugSetIntensiveMode();            // Hard Core Debug Mode
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
       testFileBuffer();
       testTempBuffer();
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

