//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       NetCIFAR10.h
//
// Purpose-
//       Define CIFAR10 VideoInp/Source
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef NETCIFAR10_H_INCLUDED
#define NETCIFAR10_H_INCLUDED

#include "NetVideo.h"
#include "X11Device.h"

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
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

namespace NETWORK_H_NAMESPACE {
#undef index                        // Some fool defined this as a macro
//----------------------------------------------------------------------------
//
// Class-
//       VideoSourceCIFAR10
//
// Purpose-
//       Fetch CIFAR10 input data.
//
//----------------------------------------------------------------------------
class VideoSourceCIFAR10 : public VideoSource { // Video Source: CIFAR10
//----------------------------------------------------------------------------
// Attributes
//----------------------------------------------------------------------------
protected:
enum                                // Generic enum
{  DIM= 32                          // Image height and width
,  DIM2= DIM * DIM                  // Image area
,  ZOOM= DIM * 8                    // Zoomed height and width
,  IMAGES= 10000                    // The number of images in the file
};

std::string            fileName;    // The source file name
unsigned char          i_type;      // Image type
unsigned char          i_red[DIM2]; // Image red channel
unsigned char          i_green[DIM2]; // Image green channel
unsigned char          i_blue[DIM2]; // Image blue channel

X11Device              disp;        // The X11 display
FILE*                  file;        // The source file
unsigned               index;       // The current image index

public:
virtual
   ~VideoSourceCIFAR10( void )      // Destructor
{
   if( file )
     fclose(file);
}

   VideoSourceCIFAR10(              // Constructor
     const char*       fileName)    // Source file name
:  VideoSource()
,  fileName(fileName)
,  disp(ZOOM, ZOOM)
,  file(nullptr)
,  index(0)
{
   file= fopen(fileName, "rb");
   if( file == nullptr )
   {
     using std::string;
     string error = string("File(") + this->fileName
                  + string(") OPEN failure");
     throw BuildError(error);
   }
}

virtual int                         // Expected output classifiation
   fetch(                           // Load the next image
     Pixel*            addr,        // Target address
     size_t            x,           // X dimension
     size_t            y)           // Y dimension
{
   size_t              L;           // Number of bytes read

   assert( x == DIM );
   assert( y == DIM );

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
     using std::string;
     string error = string("File(") + fileName
                  + string(") READ failure");
     throw NetworkException(error);
   }

   // Build the image, loading the Target
   Magick::Image image("32x32", "white");
   for(int y= 0; y<DIM; y++)
   {
     for(int x= 0; x<DIM; x++)
     {
       unsigned index= y * DIM + x;

       // Store the target Pixel
       addr->R= i_red[index];
       addr->G= i_green[index];
       addr->B= i_blue[index];
       addr->W= ((unsigned)addr->R + addr->G + addr->B) / 3;

       // Update the display image
       Magick::Color color(disp.toRange(addr->R),
                           disp.toRange(addr->G),
                           disp.toRange(addr->B),
                           0);
       image.pixelColor(x, y, color);

       addr++;
     }
   }

   // Expand and display the image
   image.zoom(Magick::Geometry("256x256"));
   disp.fromMagickImage(image);
   assert( i_type < 10 );
   disp.title(typeName[i_type]);
   disp.expose();

   return i_type;
}
}; // class VideoSourceCIFAR10

//----------------------------------------------------------------------------
//
// Class-
//       VideoInpCIFAR10
//
// Purpose-
//       CIFAR10 Video inputinput data.
//
// Implementation notes-
//       The default source provides pseudo-random data, and we don't
//       particularly care that it's not mathmatically random.
//
//----------------------------------------------------------------------------
class VideoInpCIFAR10 : public VideoInp { // Video Input: CIFAR10
public:
virtual
   ~VideoInpCIFAR10()               // Virtual destructor
{  IFDEBUG( debugf("VideoInpCIFAR10(%p).~VideoInpCIFAR10\n", this); )
}

   VideoInpCIFAR10(                 // Data constructor
     Layer&            layer,       // Our containing Layer
     VideoSourceCIFAR10&
                       source)      // Our video source
:  VideoInp(32, 32, layer, source)
{  IFDEBUG( debugf("VideoInpCIFAR10(%p).VideoInpCIFAR10()\n", this); )
}
}; // class VideoInpCIFAR10
}  // namespace NETWORK_H_NAMESPACE
#endif // NETCIFAR10_H_INCLUDED
