//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       djpeg.cpp
//
// Purpose-
//       JPEG decode/display.
//
// Last change date-
//       2020/03/25
//
//----------------------------------------------------------------------------
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <com/Logger.h>

#include "Constant.h"
#include "gui/Types.h"
#include "gui/Object.h"
#include "gui/Window.h"
using namespace GUI;

#include <jpeglib.h>                // (NEW: Standard location)

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             errorCount= 0; // Error count

//----------------------------------------------------------------------------
//
// Subroutine-
//       msgout
//
// Function-
//       Debugging logger facility.
//
//----------------------------------------------------------------------------
static inline void
   msgout(                          // Debugging logger facility
     const char*     fmt,           // The PRINTF format string
                     ...)           // The remaining arguments
{
   va_list           argptr;        // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   Logger::get()->vlogf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       msgerr
//
// Purpose-
//       Indicate error.
//
//----------------------------------------------------------------------------
static inline void
   msgerr(                          // Error log
     const char*       format,      // Format string
                       ...)         // Remaining arguments
{
   va_list             argptr;      // Argument list pointer

   errorCount++;

   va_start(argptr, format);        // Initialize va_ functions
   Logger::get()->vlogf(format, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       my_error_exit
//
// Purpose-
//       JPEG error exit handler.
//
//----------------------------------------------------------------------------
extern "C" {
struct my_error_mgr {
  struct jpeg_error_mgr pub;    /* "public" fields */

  jmp_buf setjmp_buffer;        /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
   char                buffer[JMSG_LENGTH_MAX]; // Message buffer

   // Format the message
   buffer[0]= '\0';
   (*cinfo->err->format_message) (cinfo, buffer);
   msgerr("%4d: djpeg errorExit(%s)\n", __LINE__, buffer);

   /* Always display the message. */
   /* We could postpone this until after returning, if we chose. */
   (*cinfo->err->output_message) (cinfo);

   /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
   my_error_ptr myerr = (my_error_ptr) cinfo->err;

   /* Return control to the setjmp point */
   longjmp(myerr->setjmp_buffer, 1);
}
} // extern "C"

//----------------------------------------------------------------------------
//
// Subroutine-
//       jpegWindow
//
// Purpose-
//       Display JPEG image.
//
//----------------------------------------------------------------------------
static void
   jpegWindow(                      // Display JPEG image
     const char*       fileName)    // Input filename
{
   struct jpeg_decompress_struct
                       cinfo;
   struct my_error_mgr jerr;
   FILE*               infile;      /* source file */
   JSAMPARRAY          buffer;      /* Output row buffer */
   int                 row_stride;  /* physical row width in output buffer */

   JSAMPLE*            col;         // Column pointer
   XYLength            length;      // Length
   Pixel*              pixel;       // -> Pixel
   XOffset_t           x;           // Horizontal offset

   #ifdef HCDM
     msgout("\n");
     msgout("%4d: jpegWindow(%s)..\n", __LINE__, fileName);
   #endif

   //-------------------------------------------------------------------------
   // Decode the JPEG file
   //-------------------------------------------------------------------------
   #ifdef HCDM
     msgout("%4d: HCDM 0 file(%s)..\n", __LINE__, fileName);
   #endif
   if ((infile = fopen(fileName, "rb")) == NULL) {
     fprintf(stderr, "can't open %s\n", fileName);
     return;
   }

   //-------------------------------------------------------------------------
   // Decode step 1: allocate and initialize JPEG decompression object
   //-------------------------------------------------------------------------
   #ifdef HCDM
     msgout("%4d: HCDM 1\n", __LINE__);
   #endif
   cinfo.err = jpeg_std_error(&jerr.pub);
   jerr.pub.error_exit = my_error_exit;
   /* Establish the setjmp return context for my_error_exit to use. */
   if (setjmp(jerr.setjmp_buffer)) {
     /* If we get here, the JPEG code has signaled an error.
      * We need to clean up the JPEG object, close the input file, and return.
      */
     jpeg_destroy_decompress(&cinfo);
     fclose(infile);
     return;
   }

   /* Now we can initialize the JPEG decompression object. */
   jpeg_create_decompress(&cinfo);

   //-------------------------------------------------------------------------
   // Decode step 2: specify data source
   //-------------------------------------------------------------------------
   #ifdef HCDM
     msgout("%4d: HCDM 2\n", __LINE__);
   #endif
   jpeg_stdio_src(&cinfo, infile);

   //-------------------------------------------------------------------------
   // Decode step 3: read file parameters
   //-------------------------------------------------------------------------
   #ifdef HCDM
     msgout("%4d: HCDM 3\n", __LINE__);
   #endif
   (void) jpeg_read_header(&cinfo, TRUE);
   /* We can ignore the return value from jpeg_read_header since
    *   (a) suspension is not possible with the stdio data source, and
    *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
    * See libjpeg.doc for more info.
    */

   //-------------------------------------------------------------------------
   // Decode step 4: set decompression parameters
   //-------------------------------------------------------------------------
   #ifdef HCDM
     msgout("%4d: HCDM 4\n", __LINE__);
   #endif
   length.x= cinfo.image_width; length.y= cinfo.image_height;
   Window window(length);           // Working window

   //-------------------------------------------------------------------------
   // Decode step 5: start decompressor
   //-------------------------------------------------------------------------
   #ifdef HCDM
     msgout("%4d: HCDM 5\n", __LINE__);
   #endif
   (void) jpeg_start_decompress(&cinfo);
   /* We can ignore the return value since suspension is not possible
    * with the stdio data source.
    */

   /* We may need to do some setup of our own at this point before reading
    * the data.  After jpeg_start_decompress() we have the correct scaled
    * output image dimensions available, as well as the output colormap
    * if we asked for color quantization.
    * In this example, we need to make an output work buffer of the right size.
    */
   /* JSAMPLEs per row in output buffer */
   row_stride = cinfo.output_width * cinfo.output_components;
   /* Make a one-row-high sample array that will go away when done with image */
   buffer = (*cinfo.mem->alloc_sarray)
                 ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

   //-------------------------------------------------------------------------
   // Decode step 6: operate decompressor
   //-------------------------------------------------------------------------
   #ifdef HCDM
     msgout("%4d: HCDM 6\n", __LINE__);
   #endif
   /* Here we use the library's state variable cinfo.output_scanline as the
    * loop counter, so that we don't have to keep track ourselves.
    */

   pixel= window.getPixel(0,0);
   while (cinfo.output_scanline < cinfo.output_height) {
     /* jpeg_read_scanlines expects an array of pointers to scanlines.
      * Here the array is only one element long, but you could ask for
      * more than one scanline at a time if that's more convenient.
      */
     (void) jpeg_read_scanlines(&cinfo, buffer, 1);

     /* Assume put_scanline_someplace wants a pointer and sample count. */
//// put_scanline_someplace(buffer[0], row_stride);
     col= buffer[0];
     for(x= 0; x<window.getLength().x; x++)
     {
       pixel->setColor(Pixel::getColor(col[0], col[1], col[2]));
       pixel++;
       col += 3;
     }
   }

   //-------------------------------------------------------------------------
   // Decode step 7: finish decompression
   //-------------------------------------------------------------------------
   #ifdef HCDM
     msgout("%4d: HCDM 7\n", __LINE__);
   #endif
   (void) jpeg_finish_decompress(&cinfo);
   /* We can ignore the return value since suspension is not possible
    * with the stdio data source.
    */

   //-------------------------------------------------------------------------
   // Decode step 8: release decompression object
   //-------------------------------------------------------------------------
   #ifdef HCDM
     msgout("%4d: HCDM 8\n", __LINE__);
   #endif
   /* This is an important step since it will release a good deal of memory. */
   jpeg_destroy_decompress(&cinfo);

   /* After finish_decompress, we can close the input file.
    * Here we postpone it until after no more JPEG errors are possible,
    * so as to simplify the setjmp error logic above.  (Actually, I don't
    * think that jpeg_destroy can do an error exit, but why assume anything...)
    */
   fclose(infile);

   //-------------------------------------------------------------------------
   // Make window visible
   //-------------------------------------------------------------------------
   #ifdef HCDM
     msgout("%4d: HCDM 9.0\n", __LINE__);
   #endif
   window.setAttribute(Object::VISIBLE, TRUE);
   #ifdef HCDM
     msgout("%4d: HCDM 9.1\n", __LINE__);
   #endif
   window.change();
   #ifdef HCDM
     msgout("%4d: HCDM 9.9\n", __LINE__);
   #endif
   window.wait();

   // For destructors, avoid visiblity
   window.setAttribute(Object::VISIBLE, FALSE);
   #ifdef HCDM
     msgout("%4d: ..jpegWindow()\n",  __LINE__);
   #endif
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
     char*             argv[])      // Argument array
{
   int                 i;

   try {
     for(i= 1; i<argc; i++)
       jpegWindow(argv[i]);

   } catch(const char* e) {
     errorCount++;
     msgerr("%4d: Exception !!NOT EXPECTED!! %s\n", __LINE__, e);

   } catch( ... ) {
     errorCount++;
     msgerr("%4d: Exception !!NOT EXPECTED!! ... UNKNOWN\n", __LINE__);
   }

   printf("%4d: errorCount(%d)\n", __LINE__, errorCount);
   return errorCount;
}

