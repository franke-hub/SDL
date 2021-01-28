//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       JpegDecoder.cpp
//
// Purpose-
//       JpegDecoder implementation.
//
// Last change date-
//       2021/01/28
//
//----------------------------------------------------------------------------
#include <setjmp.h>                 // For jpeg library interface
#include <stdio.h>                  // For fprintf
#include <stdlib.h>                 // For standard library (free, exit)
#include <sys/types.h>              // (Needed by jpeglib.h)
#include <jpeglib.h>                // For jpeg library

#include <com/Logger.h>
#include <gui/Types.h>

#include "JpegDecoder.h"

using namespace gui;

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
   fprintf(stderr, "%4d: djpeg errorExit(%s)\n", __LINE__, buffer);

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
// Method-
//       JpegDecoder::~JpegDecoder
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   JpegDecoder::~JpegDecoder( void ) // Destructor
{
   // Free the buffer
   if( buffer ) {
     free(buffer);
     buffer= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       JpegDecoder::JpegDecoder
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   JpegDecoder::JpegDecoder( void)  // Constructor
:  Decoder() { }

//----------------------------------------------------------------------------
//
// Method-
//       JpegDecoder::decode
//
// Purpose-
//       Decode a file.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   JpegDecoder::decode(             // Decode a file
     const char*       fileName)    // File name
{
   struct jpeg_decompress_struct
                       cinfo;
   struct my_error_mgr jerr;
   FILE*               infile;      /* source file */
   JSAMPARRAY          outrow;      /* Output row buffer */
   int                 row_stride;  /* physical row width in output buffer */

   JSAMPLE*            col;         // Column pointer

   #ifdef HCDM
     Logger::log("\n");
     Logger::log("%4d: JpegDecoder(%s)..\n", __LINE__, fileName);
   #endif

   //-------------------------------------------------------------------------
   // Decode the JPEG file
   //-------------------------------------------------------------------------
   if ((infile = fopen(fileName, "rb")) == NULL) {
     fprintf(stderr, "can't open %s\n", fileName);
     return 1;
   }

   //-------------------------------------------------------------------------
   // Delete any existing Buffer
   //-------------------------------------------------------------------------
   if( buffer ) {
     free(buffer);
     buffer= nullptr;
   }

   //-------------------------------------------------------------------------
   // Decode step 1: allocate and initialize JPEG decompression object
   //-------------------------------------------------------------------------
   cinfo.err = jpeg_std_error(&jerr.pub);
   jerr.pub.error_exit = my_error_exit;
   /* Establish the setjmp return context for my_error_exit to use. */
   if (setjmp(jerr.setjmp_buffer)) {
     /* If we get here, the JPEG code has signaled an error.
      * We need to clean up the JPEG object, close the input file, and return.
      */
     jpeg_destroy_decompress(&cinfo);
     fclose(infile);
     return 2;
   }

   /* Now we can initialize the JPEG decompression object. */
   jpeg_create_decompress(&cinfo);

   //-------------------------------------------------------------------------
   // Decode step 2: specify data source
   //-------------------------------------------------------------------------
   jpeg_stdio_src(&cinfo, infile);

   //-------------------------------------------------------------------------
   // Decode step 3: read file parameters
   //-------------------------------------------------------------------------
   (void) jpeg_read_header(&cinfo, TRUE);
   /* We can ignore the return value from jpeg_read_header since
    *   (a) suspension is not possible with the stdio data source, and
    *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
    * See libjpeg.doc for more info.
    */

   //-------------------------------------------------------------------------
   // Decode step 4: set decompression parameters
   //-------------------------------------------------------------------------
   width= cinfo.image_width;
   height= cinfo.image_height;
   buffer= (uint32_t*)malloc( width * height * sizeof(uint32_t));
   if( buffer == nullptr ) {
     fprintf(stderr, "No storage\n");
     exit(EXIT_FAILURE);
   }

   //-------------------------------------------------------------------------
   // Decode step 5: start decompressor
   //-------------------------------------------------------------------------
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
   outrow = (*cinfo.mem->alloc_sarray)
                 ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

   //-------------------------------------------------------------------------
   // Decode step 6: operate decompressor
   //-------------------------------------------------------------------------
   /* Here we use the library's state variable cinfo.output_scanline as the
    * loop counter, so that we don't have to keep track ourselves.
    */

   uint32_t* pixel= buffer;
   while (cinfo.output_scanline < cinfo.output_height) {
     /* jpeg_read_scanlines expects an array of pointers to scanlines.
      * Here the array is only one element long, but you could ask for
      * more than one scanline at a time if that's more convenient.
      */
     (void) jpeg_read_scanlines(&cinfo, outrow, 1);

     /* Assume put_scanline_someplace wants a pointer and sample count. */
//// put_scanline_someplace(outrow[0], row_stride);
     col= outrow[0];
     for(unsigned x= 0; x<width; x++)
     {
       *pixel= (col[0] << 16) | (col[1] << 8) | col[2];
       pixel++;
       col += 3;
     }
   }

   //-------------------------------------------------------------------------
   // Decode step 7: finish decompression
   //-------------------------------------------------------------------------
   (void) jpeg_finish_decompress(&cinfo);
   /* We can ignore the return value since suspension is not possible
    * with the stdio data source.
    */

   //-------------------------------------------------------------------------
   // Decode step 8: release decompression object
   //-------------------------------------------------------------------------
   /* This is an important step since it will release a good deal of memory. */
   jpeg_destroy_decompress(&cinfo);

   /* After finish_decompress, we can close the input file.
    * Here we postpone it until after no more JPEG errors are possible,
    * so as to simplify the setjmp error logic above.  (Actually, I don't
    * think that jpeg_destroy can do an error exit, but why assume anything...)
    */
   fclose(infile);

   #ifdef HCDM
     Logger::log("%4d: ..JpegDecoder()\n",  __LINE__);
   #endif
   return 0;
}

