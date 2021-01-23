//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       GetImage.cpp
//
// Purpose-
//       Decode and display CIFAR-10 images.
//
// Last change date-
//       2020/10/06
//
//----------------------------------------------------------------------------
#include <list>
#include <string>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "X11Device.h"              // (Include BEFORE Magick++.h)
#include <Magick++.h>
using namespace Magick;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef USE_IMAGE_LIST              // Use list<Image>
#undef  USE_IMAGE_LIST              // If defined, Use list<Image>>
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     fileName= nullptr; // The source file name
static int             swDebug;     // Debugging control

static const
   struct timespec     delay= {0, 500000000}; // Delay: seconds, nanoseconds
static const char*     typeName[]=
{  "plane"
,  "auto"
,  "bird"
,  "cat"
,  "deer"
,  "dog"
,  "frog"
,  "horse"
,  "ship"
,  "truck"
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       toRange
//
// Purpose-
//       Convert value to QuantumRange value.
//
//----------------------------------------------------------------------------
static inline unsigned              // Resultant
   toRange(                         // Convert value to QuantumRange value
     unsigned          inp)         // The source value, range 0..255
{
   assert( inp < 256 );
   if( QuantumRange == 65535 )
     return inp << 8;

   return inp;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Informational exit
//
//----------------------------------------------------------------------------
static void
   info(                            // Informational exit
     const char*     sourceName)    // The source fileName
{
   fprintf(stderr, "%s <options> {Image-set}\n", sourceName);
   fprintf(stderr, "\n");
   fprintf(stderr, "Options:\n");
   fprintf(stderr, "-v\tVerify parameters\n");
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Analyze parameters
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   bool              error;         // TRUE if error encountered
   bool              verify;        // TRUE if verify required

   int               i, j;

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   error= false;                    // Default, no error found
   verify= false;

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   for(j=1; j<argc; j++)            // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       if( strcmp(argv[j], "-help") == 0 )
         error= true;

       else
       {
         for(i=1; argv[j][i] != '\0'; i++) // Examine the switch list
         {
           switch(argv[j][i])       // Examine the switch
           {
             case 'd':              // -d (debug)
               swDebug= true;
               break;

             case 'h':              // -h (help)
               error= true;
               break;

             case 'v':              // -v (verify)
               verify= true;
               break;

             default:               // If invalid switch
               error= true;
               fprintf(stderr, "Invalid switch '%c'\n", (int)argv[j][i]);
               break;
           }
         }
       }
     }
     else if( fileName )            // If filename already specified
     {
       error= true;
       fprintf(stderr, "Invalid parameter: '%s'\n", argv[j]);
     }
     else
       fileName= argv[j];
   }

   //-------------------------------------------------------------------------
   // Check and validate
   //-------------------------------------------------------------------------
   if( fileName == nullptr )
   {
     error= true;
     fprintf(stderr, "Missing filename\n");
   }

   if( error )
     info(argv[0]);

   if( verify )
   {
     fprintf(stderr, "%10d debug\n", swDebug);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       loadImageMagick
//
// Purpose-
//       Load images.
//
//----------------------------------------------------------------------------
static inline void
   loadImageMagick( void )          // Load images using ImageMagick
{
   static const int    DIM= 32;     // Image height and width
   static const int    DIM2= DIM * DIM; // Image area
   static unsigned     ZOOM= DIM * 8; // Magnified height and width

   size_t              L;           // Number of bytes read
   unsigned char       i_type;      // Image type
   unsigned char       i_red[DIM2]; // Image red channel
   unsigned char       i_green[DIM2]; // Image green channel
   unsigned char       i_blue[DIM2]; // Image blue channel
// Image               image("32x32", "white"); // The display image
   Magick::Image       image("32x32", "white"); // The display image

   X11Device           disp(ZOOM, ZOOM); // The X11 display

   // Open the image file
   FILE* file= fopen(fileName, "rb");
   if( file == nullptr )
   {
     fprintf(stderr, "File(%s) OPEN failure\n", fileName);
     exit(EXIT_FAILURE);
   }

   // Display the images
   image.animationDelay(500);
   image.animationIterations(1);
   #ifdef USE_IMAGE_LIST
     int MAX= 100;
     std::list<Image> imageList;
   #else
     int MAX= 10000;
   #endif
   for(int i= 0; i<MAX; i++)
   {
     L= fread(&i_type, sizeof(unsigned char), sizeof(i_type), file);
     bool ERROR= L != sizeof(i_type);

     L= fread(i_red, sizeof(unsigned char), DIM2, file);
     ERROR |= L != DIM2;

     L= fread(i_green, sizeof(unsigned char), DIM2, file);
     ERROR |= L != DIM2;

     L= fread(i_blue, sizeof(unsigned char), DIM2, file);
     ERROR |= L != DIM2;

     if( ERROR )
     {
       fprintf(stderr, "File(%s) READ error\n", fileName);
       exit(EXIT_FAILURE);
     }

//----------------------------------------------------------------------------
#ifdef USE_IMAGE_LIST
     // Image display list (with zoom)
     image.resize("32x32");
     for(int y= 0; y<DIM; y++)
     {
       for(int x= 0; x<DIM; x++)
       {
         Color color(toRange(i_red[y*DIM+x]),
                     toRange(i_green[y*DIM+x]),
                     toRange(i_blue[y*DIM+x]),
                     0);

         image.pixelColor(x, y, color);
         assert( image.pixelColor(x,y) == color );
       }
     }

     // Display the image
     image.zoom(Geometry("256x256"));
     imageList.push_back(image);

//----------------------------------------------------------------------------
#elif 1
     // Image display with optional interpolated zoom
     image.resize("32x32");
     for(int y= 0; y<DIM; y++)
     {
       for(int x= 0; x<DIM; x++)
       {
         Color color(toRange(i_red[y*DIM+x]),
                     toRange(i_green[y*DIM+x]),
                     toRange(i_blue[y*DIM+x]),
                     0);

         image.pixelColor(x, y, color);
         assert( image.pixelColor(x,y) == color );
       }
     }

     // Display the image
     if( true )
       image.zoom(Geometry("256x256"));
     if( false )
       image.sharpen();

//----------------------------------------------------------------------------
#elif 1
     // Pixelated ZOOM
     image.resize("256x256");
     for(int y= 0; y<DIM; y++)
     {
       for(int x= 0; x<DIM; x++)
       {
         Color color(toRange(i_red[y*DIM+x]),
                     toRange(i_green[y*DIM+x]),
                     toRange(i_blue[y*DIM+x]),
                     0);
         int ZOOM= 8;
         int ZOOM_X= x * ZOOM;
         int ZOOM_Y= y * ZOOM;
         for(int X= 0; X<ZOOM; X++)
           for(int Y=0; Y<ZOOM; Y++)
              image.pixelColor(ZOOM_X+X, ZOOM_Y+Y, color);
         assert( image.pixelColor(ZOOM_X,ZOOM_Y) == color );
       }
     }

//----------------------------------------------------------------------------
#else
     // Use PixelPacket with optional interpolated zoom
     // Note: Shows packet layout: y * x
     image.resize("32x32");
     image.modifyImage();
     Pixels cache(image);
     PixelPacket* packet= cache.get(0, 0, DIM, DIM);

     for(int y= 0; y<DIM; y++)
     {
       for(int x= 0; x<DIM; x++)
       {
         packet->red= toRange(i_red[y*DIM+x]);
         packet->green= toRange(i_green[y*DIM+x]);
         packet->blue= toRange(i_blue[y*DIM+x]);
         packet->opacity= 0;

         packet++;
       }
     }

     // Display the image
     cache.sync();
     if( true )
       image.zoom(Geometry("256x256"));
#endif

     //-----------------------------------------------------------------------
     // Display the image
#ifndef USE_IMAGE_LIST
#if false
     image.display();
#else
     disp.fromMagickImage(image);
     assert( i_type < 10 );
     if( false )
       printf("[%5d] %s\n", i, typeName[i_type]);
     disp.title(typeName[i_type]);
     disp.expose();

     if( true  )
       nanosleep(&delay, nullptr);
#endif
#endif
   }

   fclose(file);

//----------------------------------------------------------------------------
// Animated list
#ifdef USE_IMAGE_LIST
   Image animation;
   appendImages(&animation, imageList.begin(), imageList.end());

   animation.animationDelay(6000);
   animation.animationIterations(1);
   if( true  )
     animation.display();
   else
     displayImages(imageList.begin(), imageList.end());
#endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       loadX11
//
// Purpose-
//       Load images using X11.
//
//----------------------------------------------------------------------------
static inline void
   loadX11( void )                  // Load images using ImageMagick
{
   static const int    DIM= 32;     // Image height and width
   static const int    DIM2= DIM * DIM; // Image area
   static unsigned     ZOOM= DIM * 8; // Magnified height and width

   size_t              L;           // Number of bytes read
   unsigned char       i_type;      // Image type
   unsigned char       i_red[DIM2]; // Image red channel
   unsigned char       i_green[DIM2]; // Image green channel
   unsigned char       i_blue[DIM2]; // Image blue channel

   X11Device           disp(ZOOM, ZOOM); // The X11 display

   // Open the image file
   FILE* file= fopen(fileName, "rb");
   if( file == nullptr )
   {
     fprintf(stderr, "File(%s) OPEN failure\n", fileName);
     exit(EXIT_FAILURE);
   }

   // Display the images
   int MAX= 10000;
   for(int i= 0; i<MAX; i++)
   {
     L= fread(&i_type, sizeof(unsigned char), sizeof(i_type), file);
     bool ERROR= L != sizeof(i_type);

     L= fread(i_red, sizeof(unsigned char), DIM2, file);
     ERROR |= L != DIM2;

     L= fread(i_green, sizeof(unsigned char), DIM2, file);
     ERROR |= L != DIM2;

     L= fread(i_blue, sizeof(unsigned char), DIM2, file);
     ERROR |= L != DIM2;

     if( ERROR )
     {
       fprintf(stderr, "File(%s) READ error\n", fileName);
       exit(EXIT_FAILURE);
     }

//----------------------------------------------------------------------------
     // Build the image
     // Use ImageMagic PixelPacket for interpolated zoom
//   Image image("32x32", "white"); // The display image
     Magick::Image image("32x32", "white"); // The display image
     image.modifyImage();
//   Pixels cache(image);
//   PixelPacket* packet= cache.get(0, 0, DIM, DIM);

     for(unsigned y= 0; y<DIM; y++)
     {
       for(unsigned x= 0; x<DIM; x++)
       {
//       packet->red= toRange(i_red[y*DIM+x]);
//       packet->green= toRange(i_green[y*DIM+x]);
//       packet->blue= toRange(i_blue[y*DIM+x]);
//       packet->opacity= 0;
//       packet++;

         Color color(toRange(i_red[y*DIM+x]),
                     toRange(i_green[y*DIM+x]),
                     toRange(i_blue[y*DIM+x]),
                     0);

         image.pixelColor(x, y, color);
         assert( image.pixelColor(x,y) == color );
       }
     }

     // Zoom the image
//   cache.sync();
     image.zoom(Geometry("256x256"));

     // Display the image
     disp.fromMagickImage(image);
     assert( i_type < 10 );
     if( false )
       printf("[%5d] %s\n", i, typeName[i_type]);
     disp.title(typeName[i_type]);
     disp.expose();

     if( true  )
       nanosleep(&delay, nullptr);
   }

   // Close the file
   fclose(file);
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
     char*           argv[])        // Argument array
{
   //-------------------------------------------------------------------------
   // Parameter validation
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Run the test suite
   //-------------------------------------------------------------------------
   loadImageMagick();
// loadX11();

   //-------------------------------------------------------------------------
   // Return to caller
   //-------------------------------------------------------------------------
   printf("..DONE..\n");
   return 0;
}
