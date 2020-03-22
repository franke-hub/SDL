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
//       Dumpgif.cpp
//
// Purpose-
//       Dump GIF file.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/Debug.h>
#include <obj/Object.h>
using obj::Exception;

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define INTFIELD(name) \
            printf("%10ld, 0x%.8lx = %s\n", (long)name, (long)name, #name)
#define INTSPACE() "                         "

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static FILE*           fileHand;    // Current file handle
static const char*     fileName;    // Current file name

static int             global_color_len; // Size of global color table
static int             image_count= 0; // Number of images

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_first= false; // --first
static int             opt_help= false; // --help (or error)
static int             opt_index;   // Option index
static int             opt_verbose= false; // --verbose
static int             opt_xmp= false; // --xmp

static struct option   OPTS[]=      // Options
{  {"help",    no_argument,       &opt_help,    true}
,  {"first",   no_argument,       &opt_first,   true}
,  {"verbose", no_argument,       &opt_verbose, true}
,  {"xmp",     no_argument,       &opt_xmp,     true}

,  {0, 0, 0, 0}                     // (End of option list)
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       seekf
//
// Purpose-
//       Seek into file.
//
//----------------------------------------------------------------------------
#if 0
static void                         // Exception iff error
   seekf(                           // Seek into file
     long              offset)      // Seek offset
{
   unsigned            S;           // Seek offset

   S= fseek(fileHand, offset, SEEK_SET);
   if( S != 0 )
   {
     fprintf(stderr, "File(%s): ", fileName);
     perror("seek error:");
     throw Exception("Seek error");
   }
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       readf
//
// Purpose-
//       Read from file.
//
//----------------------------------------------------------------------------
static int                          // Number of bytes read (EOF not allowed!)
   readf(                           // Read from file
     void*             addr,        // Input address
     size_t            size)        // Input length
{
   int                 L;           // Number of bytes read

   L= fread(addr, 1, size, fileHand);
   if( L <= 0 )
   {
     fprintf(stderr, "File(%s): ", fileName);
     perror("read error: ");
     throw Exception("Read error");
   }
   else if( L < size )
   {
     fprintf(stderr, "File(%s): %d= read(%zd)\n", fileName, L, size);
     throw Exception("Read error");
   }

#if 0
   debugf("readf(%p,%zd)\n", addr, size);
   snap(addr, size);
#endif

   return L;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       tc
//
// Purpose-
//       Return delimited string (text to const char*)
//
//----------------------------------------------------------------------------
static const char*                  // Delimited string
   tc(                              // Return delimited string
     const char*       text,        // Text address
     unsigned          size)        // Text length
{
static char            buffer[256]; // Return buffer

   if( size >= sizeof(buffer) )
     throw Exception("tc size > 256");

   memcpy(buffer, text, size);
   buffer[size]= '\0';
   return buffer;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       tf
//
// Purpose-
//       Return " True" or "False"
//
//----------------------------------------------------------------------------
static const char*                  // "True" or "False"
   tf(                              // Return "True" or "False"
     int               cc)          // Condition code
{
static const char*     is_false= "False";
static const char*     is_true=  " True";

   if( cc )
     return is_true;

   return is_false;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       blk_APP
//
// Purpose-
//       Dump an Applicatiopm Extension, type 0x21,0xFF
//
//----------------------------------------------------------------------------
static void
   blk_APP( void )                  // Application Extension
{
   struct APP {                     // Application Descriptor
     uint8_t           bs;          // Block size
     char              name[8];     // Application identifier
     char              auth[3];;    // Authentication code
   }                   app;

   //-------------------------------------------------------------------------
   // Read and display the Application Descriptor
   //-------------------------------------------------------------------------
   readf(&app, sizeof(app));

   printf("\n");
   printf("Application descriptor:\n");
   printf("Name: '%s'\n", tc(app.name, sizeof(app.name)));
   printf("Auth: '%s'\n", tc(app.auth, sizeof(app.auth)));

   //-------------------------------------------------------------------------
   // Dump the application blocks
   //-------------------------------------------------------------------------
   bool    is_xmp= (memcmp(app.name, "XMP Data", 8) == 0);

   char    buff[264];
   uint8_t len;

   FILE*   file= nullptr;
   if( is_xmp && opt_xmp )
   {
     if( (strlen(fileName) + 4) >= sizeof(buff) )
       throw Exception("Filename too long for .xmp");

     strcpy(buff, fileName);
     strcat(buff, ".xmp");
     file= fopen(buff, "wb");
     if( file == nullptr )
     {
       fprintf(stderr, "File(%s) ", buff);
       perror("open error: ");
       throw Exception("Open failure");
     }
   }

   for(;;)
   {
     readf(&len, sizeof(len));
     if( len == 0 )
       break;

     readf(buff, len);
     if( opt_verbose )
     {
       printf("..Block %d\n", len);
       snap(buff, len);
     }

     if( file != nullptr )
     {
       if( memchr(buff, 0x01, len) )
       {
         len= (char*)memchr(buff, 0x01, len) - buff;
         is_xmp= false;
       }

       size_t L= fwrite(buff, 1, len, file);
       if( L != len )
       {
         fprintf(stderr, "File(%s%s) Write error\n", fileName, ".xmp");
         fclose(file);
         throw Exception("I/O error");
       }

       if( !is_xmp )
       {
         fclose(file);
         file= nullptr;
       }
     }
   }

   if( file )
     fclose(file);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       blk_CMT
//
// Purpose-
//       Dump a Comment Extension, type 0x21,0xFE
//
//----------------------------------------------------------------------------
static void
   blk_CMT( void )                  // Comment Extension
{
   unsigned char buffer[256];
   uint8_t len;

   //-------------------------------------------------------------------------
   // Read and display the comment
   //-------------------------------------------------------------------------
   printf("\n");
   printf("Comment:\n");
   for(;;)
   {
     readf(&len, sizeof(len));      // Read the comment length
     if( len == 0 )
       break;

     readf(buffer, len);
     buffer[len]= '\0';
     printf("%s", buffer);
   }

   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       blk_GCE
//
// Purpose-
//       Dump a Graphics Control Extension, type 0x21,0xF9
//
//----------------------------------------------------------------------------
static void
   blk_GCE( void )                  // Graphics Control Extension
{
   struct GCE {                     // Graphics Control Extension
     uint8_t           bs;          // Block size
     uint8_t           flags;       // Flags
     uint16_t          delay;       // Delay time
     uint8_t           tci;         // Transparent Color Index
     uint8_t           bt;          // Block Terminator
   }                   gce;

   //-------------------------------------------------------------------------
   // Read and display the Image Descriptor
   //-------------------------------------------------------------------------
   readf(&gce, sizeof(gce));

   if( image_count == 0 || !opt_first )
   {
     printf("\n");
     printf("Graphics Control Extension:\n");
     INTFIELD(gce.bs);                // Block size
     INTFIELD(gce.flags);             // Flags
     INTFIELD(gce.delay);             // Delay time
     INTFIELD(gce.bt);                // Block terminator
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       blk_IMG
//
// Purpose-
//       Dump an image descriptor, type 0x2c
//
//----------------------------------------------------------------------------
static void
   blk_IMG( void )                  // Image descriptor
{
   struct IMG {                     // Logical Screen Descriptor
     uint16_t          x_off;       // X offset (LEFT)
     uint16_t          y_off;       // Y offset (TOP)
     uint16_t          x_size;      // Width
     uint16_t          y_size;      // Height
     uint8_t           flags;       // Flags
   }                   img;

   //-------------------------------------------------------------------------
   // Read and display the Image Descriptor
   //-------------------------------------------------------------------------
   readf(&img, 9);                  // Structure size padded

   if( image_count == 0 || !opt_first )
   {
     printf("\n");
     printf("Image Descriptor:\n");
     INTFIELD(img.x_off);             // LEFT offset
     INTFIELD(img.y_off);             // TOP offset
     INTFIELD(img.x_size);            // Width
     INTFIELD(img.y_size);            // Height
     INTFIELD(img.flags);             // Flags
     printf(INTSPACE() "%s = has_local_color_table\n", tf(img.flags & 0x80));
     printf(INTSPACE() "%s = interlaced\n", tf(img.flags & 0x40));
     printf(INTSPACE() "%s = sorted\n", tf(img.flags & 0x20));
     printf(INTSPACE() "%5d = local_color_table size\n", img.flags & 0x07);
   }

   //-------------------------------------------------------------------------
   // Display the Local Color Table
   //-------------------------------------------------------------------------
   if( img.flags & 0x80 )           // If Local Color Table present
   {
     int M= (img.flags & 0x07) + 1;
     if( image_count == 0 || !opt_first )
     {
       printf("\n");
       printf("Local Color Table: %d\n", M);
     }
     M= 1 << M;
     for(int i= 0; i<M; i++)
     {
       unsigned char rgb[3];
       readf(&rgb, sizeof(rgb));

       if( opt_verbose && (image_count == 0 || !opt_first) )
         printf("%.3d: %.2x,%.2x,%.2x\n", i, rgb[0], rgb[1], rgb[2]);
     }
   }

   //-------------------------------------------------------------------------
   // Skip over the compressed image
   //-------------------------------------------------------------------------
   unsigned char buffer[256];
   uint8_t len;

   readf(&len, sizeof(len));        // Read the LWT code size
   if( image_count == 0 || !opt_first )
     printf("\nMinimum code size: %d bits\n", len);

   size_t image_size= 0;
   for(;;)                          // Read the data blocks
   {
     readf(&len, sizeof(len));      // Read the LWT code size
     image_size++;
     if( len == 0 )
       break;

     readf(buffer, len);
     image_size += len;
   }
   if( image_count == 0 || !opt_first )
     printf("Compressed image size: %zd\n", image_size);

   image_count++;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       blk_PTE
//
// Purpose-
//       Dump a Plain Text Extension, type 0x21,0x01
//
//----------------------------------------------------------------------------
static void
   blk_PTE( void )                  // Plain Text Extension
{
   throw Exception("blk_PTE Not Coded Yet");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       gif_BLK
//
// Purpose-
//       Dump by block identifier
//
//----------------------------------------------------------------------------
static int                          // Return code
   gif_BLK( void )                  // Display by block type
{
   for(;;)
   {
     unsigned char id;              // The block identifier
     readf(&id, sizeof(id));
     switch( id )
     {
       case 0x21:                   // Graphics Control Extension
         readf(&id, sizeof(id));    // Read extension code
         if( id == 0x01 )
           blk_PTE();
         else if( id == 0xF9 )
           blk_GCE();
         else if( id == 0xFE )
           blk_CMT();
         else if( id == 0xFF )
           blk_APP();
         else
         {
           fprintf(stderr, "\nUndefined GCE %.2x\n", id);
           return 1;
         }
         break;

       case 0x2C:                    // Image
         blk_IMG();
         break;

       case 0x3B:                    // Trailer
         return 0;
         break;

       default:
         fprintf(stderr, "\nUndefined block type %.2x\n", id);
         return 1;
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       gif_GCT
//
// Purpose-
//       Dump the Global Color Table
//
//----------------------------------------------------------------------------
static void
   gif_GCT( void )                  // Display the Global Color Table
{
   printf("\n");
   printf("Global Color Table: ");
   if( global_color_len == 0 )
   {
     printf("Not present\n");
     return;
   }

   printf("%d\n", global_color_len);
   for(int i= 0; i<global_color_len; i++)
   {
     unsigned char rgb[3];
     readf(&rgb, sizeof(rgb));

     if( opt_verbose )
       printf("%.3d: %.2x,%.2x,%.2x\n", i, rgb[0], rgb[1], rgb[2]);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       gif_HDR
//
// Purpose-
//       Test the file identifier
//
//----------------------------------------------------------------------------
static void
   gif_HDR( void )                  // Read and verify file header
{
   char                header[8];   // The GIF header

   readf(&header, 6);
   if( memcmp(header, "GIF", 3) != 0 )
     throw Exception("Format error: no GIF header");

   if( memcmp(header+3, "87a", 3) != 0 && memcmp(header+3, "89a", 3) != 0 )
   {
     header[6]= '\0';
     fprintf(stderr, "File(%s): invalid GIF version(%s)\n", fileName, header+3);
     // Attempt to handle anyway
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       gif_LSD
//
// Purpose-
//       Dump the Logical Screen Descriptor
//
//----------------------------------------------------------------------------
static void
   gif_LSD( void )                  // Display the Logical Screen Descriptor
{
   struct LSD {                     // Logical Screen Descriptor
     uint16_t          x_size;      // Width
     uint16_t          y_size;      // Height
     uint8_t           flags;       // Flags
     uint8_t           bg;          // Background color index
     uint8_t           par;         // Pixel Aspect Ratio
   }                   lsd;

   //-------------------------------------------------------------------------
   // Read and display the Logical Screen Descriptor
   //-------------------------------------------------------------------------
   readf(&lsd, 7);                  // Struct length is 8!

   //-------------------------------------------------------------------------
   // Display it
   //-------------------------------------------------------------------------
   printf("\n");
   printf("Logical Screen Descriptor:\n");
   INTFIELD(lsd.x_size);            // Width
   INTFIELD(lsd.y_size);            // Height
   INTFIELD(lsd.flags);             // Flags
   printf(INTSPACE() "%s = has_global_color_table\n", tf(lsd.flags & 0x80));
   printf(INTSPACE() "%5d = resolution\n", (lsd.flags & 0x70) >> 4);
   printf(INTSPACE() "%s = sorted\n", tf(lsd.flags & 0x08));
   printf(INTSPACE() "%5d = global_color_table_size\n", lsd.flags & 0x07);
   INTFIELD(lsd.bg);                // Background color index
   INTFIELD(lsd.par);               // Pixel Aspect Ratio

   global_color_len= 0;             // Set global color length
   if( lsd.flags & 0x80 )
   {
     unsigned shift= (lsd.flags & 0x07) + 1;
     global_color_len= 1 << shift;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       readGIF
//
// Purpose-
//       Dump the file.
//
//----------------------------------------------------------------------------
static int                          // Return code
   readGIF( void )                  // Parse a GIF file
{
   //-------------------------------------------------------------------------
   // Dump the file
   //-------------------------------------------------------------------------
   image_count= 0;
   gif_HDR();
   gif_LSD();
   gif_GCT();
   gif_BLK();
   printf("Images: %d\n", image_count);

   //-------------------------------------------------------------------------
   // Done
   //-------------------------------------------------------------------------
   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Informational exit.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Informational exit
{
   fprintf(stderr, "Dumpgif options filename...\n");
   fprintf(stderr, "options:\n"
                   "  --help\tThis help message\n"
                   "  --first\tOnly display first image info\n"
                   "  --xmp\t\tXMP data to file\n"
                   "  --verbose\tVerbose output\n"
          );
   exit(EXIT_FAILURE);
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
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 C;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   opterr= 0;                       // Do not write error messages

   while( (C= getopt_long(argc, argv, "", OPTS, &opt_index)) != -1 )
     switch( C )
     {
       case 0:
#if 1
         {{{{
         const char* option_= OPTS[opt_index].name;

         if( false )
         {
           printf("%4d Found option(%d=%s)", __LINE__, opt_index, option_);
           if( optarg )
             printf(" value(%s)", optarg);
           printf("\n");
         }

         switch( opt_index )
         {
           default:
             break;
         }
         }}}}
#endif
         break;

       case ':':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "%4d Option requires an argument '%s'.\n", __LINE__,
                           argv[optind-1]);
         else
           fprintf(stderr, "%4d Option requires an argument '-%c'.\n", __LINE__,
                           optopt);
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "%4d Unknown option '%s'.\n", __LINE__,
                           argv[optind-1]);
         else if( isprint(optopt) )
           fprintf(stderr, "%4d Unknown option '-%c'.\n", __LINE__, optopt);
         else
           fprintf(stderr, "%4d Unknown option character '0x%x'.\n", __LINE__,
                           optopt);
         break;

       default:
         fprintf(stderr, "%4d ShoudNotOccur ('%c',0x%x).\n", __LINE__, C, C);
         break;
     }

   if( opt_help )
     info();
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
   int                 rc= 1;       // Return code (no/last file)

   // Parameter analysis
   parm(argc, argv);

   // Dump the object files
   for(int i= optind; i<argc; i++)
   {
     if( i != optind )
       printf("\n");

     fileName= argv[i];
     fileHand= fopen(fileName, "rb");     // Open the file
     if( fileHand == NULL )
     {
       fprintf(stderr, "File(%s): ", fileName);
       perror("Open failure: ");
       continue;
     }

     printf("File(%s)\n", fileName);

     try {
       rc= readGIF();
     } catch(Exception X) {
       printf("Error: %s\n", X.string().c_str());
       rc= 2;
     } catch(std::exception X) {
       printf("Error: exception(%s)\n", X.what());
       rc= 2;
     }

     fclose(fileHand);
   }

   return rc;
}

