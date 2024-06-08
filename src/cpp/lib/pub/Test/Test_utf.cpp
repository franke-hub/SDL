//----------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_utf.cpp
//
// Purpose-
//       Test Utf.h
//
// Last change date-
//       2024/06/07
//
//----------------------------------------------------------------------------
#include <endian.h>                 // For endian subroutines

#include <pub/Debug.h>              // For debugging subroutines
#include "pub/TEST.H"               // For VERIFY, ...
#include "pub/Wrapper.h"            // For pub::Wrapper

// The tested includes
#include "pub/Utf.h"                // For pub:: Utf, ...

// Namespace accessors
#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For Utf.h classes
using namespace PUB::debugging;     // For debugging namespace
using PUB::Wrapper;                 // For pub::Wrapper class

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Example
//
// Purpose-
//       Test (Example.h)
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors encountered
   test_Example( void )             // Test Example.h
{
   int                 error_count= 0; // Number of errors encountered

   if( opt_verbose )
     debugf("\ntest_Example\n");

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_utf
//
// Purpose-
//       Test Utf.h: Utf
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors found
   test_utf( void )                 // Test Utf.h: Utf
{
   if( opt_verbose )
     debugf("\ntest_utf\n");

   int                 error_count= 0; // Number of errors encountered

   // Test Utf types
   Utf::utf8_t     utf8= 0;         // The UTF-8  encoding type
   Utf::utf16_t    utf16= 0;        // The UTF-16 encoding type
   Utf::utf16BE_t  utf16BE= 0;      // The UTF-16BE encoding type
   Utf::utf16LE_t  utf16LE= 0;      // The UTF-16LE encoding type
   Utf::utf32_t    utf32= 0;        // The UTF-32 encoding type
   Utf::utf32BE_t  utf32BE= 0;      // The UTF-32BE encoding type
   Utf::utf32LE_t  utf32LE= 0;      // The UTF-32LE encoding type

   Utf::Column     column= 0;       // Offset/index in codepoints
   Utf::Points     points= 0;       // Length in codepoints
   Utf::Offset     offset= 0;       // Offset/index in native units
   Utf::Length     length= 0;       // Length in native units

   error_count += VERIFY( utf8    == 0 ); // (Reference the encoding types)
   error_count += VERIFY( utf16   == 0 );
   error_count += VERIFY( utf16BE == 0 );
   error_count += VERIFY( utf16LE == 0 );
   error_count += VERIFY( utf32   == 0 );
   error_count += VERIFY( utf32BE == 0 );
   error_count += VERIFY( utf32LE == 0 );

   error_count += VERIFY( column  == 0 );
   error_count += VERIFY( points  == 0 );
   error_count += VERIFY( offset  == 0 );
   error_count += VERIFY( length  == 0 );

   // Test is_combining()
   error_count += VERIFY( Utf::is_combining(0x00'0000) == false );
   error_count += VERIFY( Utf::is_combining(0x00'02FF) == false );
   error_count += VERIFY( Utf::is_combining(0x00'0300) == true  );
   error_count += VERIFY( Utf::is_combining(0x00'036F) == true  );
   error_count += VERIFY( Utf::is_combining(0x00'0370) == false );
   error_count += VERIFY( Utf::is_combining(0x00'1AAF) == false );
   error_count += VERIFY( Utf::is_combining(0x00'1AB0) == true  );
   error_count += VERIFY( Utf::is_combining(0x00'1AFF) == true  );
   error_count += VERIFY( Utf::is_combining(0x00'1B00) == false );
   error_count += VERIFY( Utf::is_combining(0x00'1DBF) == false );
   error_count += VERIFY( Utf::is_combining(0x00'1DC0) == true  );
   error_count += VERIFY( Utf::is_combining(0x00'1DFF) == true  );
   error_count += VERIFY( Utf::is_combining(0x00'1E00) == false );
   error_count += VERIFY( Utf::is_combining(0x00'20CF) == false );
   error_count += VERIFY( Utf::is_combining(0x00'20D0) == true  );
   error_count += VERIFY( Utf::is_combining(0x00'20FF) == true  );
   error_count += VERIFY( Utf::is_combining(0x00'2100) == false );
   error_count += VERIFY( Utf::is_combining(0x00'FE1F) == false );
   error_count += VERIFY( Utf::is_combining(0x00'FE20) == true  );
   error_count += VERIFY( Utf::is_combining(0x00'FE2F) == true  );
   error_count += VERIFY( Utf::is_combining(0x00'FE30) == false );
   error_count += VERIFY( Utf::is_combining(0x10'FFFF) == false );
   error_count += VERIFY( Utf::is_combining(0x11'0000) == false );

   // Test is_unicode()
   error_count += VERIFY( Utf::is_unicode(0x00'0000) == true  );
   error_count += VERIFY( Utf::is_unicode(0x00'D7FF) == true  );
   error_count += VERIFY( Utf::is_unicode(0x00'D800) == false );
   error_count += VERIFY( Utf::is_unicode(0x00'DC00) == false );
   error_count += VERIFY( Utf::is_unicode(0x00'DFFF) == false );
   error_count += VERIFY( Utf::is_unicode(0x00'E000) == true  );
   error_count += VERIFY( Utf::is_unicode(0x10'FFFF) == true  );
   error_count += VERIFY( Utf::is_unicode(0x11'0000) == false );

   // Test strlen (strlen doesn't care about endian, so neither do we.)
   Utf::utf16_t s16_0[]= {0};
   Utf::utf16_t s16_1[]= {1, 0};
   Utf::utf16_t s16_7[]= {1, 2, 3, 4, 5, 6, 7, 0};

   Utf::utf32_t s32_0[]= {0};
   Utf::utf32_t s32_1[]= {1, 0};
   Utf::utf32_t s32_7[]= {1, 2, 3, 4, 5, 6, 7, 0};

   error_count += VERIFY( Utf::strlen(s16_0) == 0 );
   error_count += VERIFY( Utf::strlen(s16_1) == 1 );
   error_count += VERIFY( Utf::strlen(s16_7) == 7 );

   error_count += VERIFY( sizeof(s16_0) ==  2 );
   error_count += VERIFY( sizeof(s16_1) ==  4 );
   error_count += VERIFY( sizeof(s16_7) == 16 );

   error_count += VERIFY( Utf::strlen(s32_0) == 0 );
   error_count += VERIFY( Utf::strlen(s32_1) == 1 );
   error_count += VERIFY( Utf::strlen(s32_7) == 7 );

   error_count += VERIFY( sizeof(s32_0) ==  4 );
   error_count += VERIFY( sizeof(s32_1) ==  8 );
   error_count += VERIFY( sizeof(s32_7) == 32 );

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_utf8
//
// Purpose-
//       Test utf8_decoder, utf8_encoder
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors found
   test_utf8( void )                // Test Utf.h: utf8_decoder, utf8_encoder
{
   if( opt_verbose )
     debugf("\ntest_utf8\n");

   int                 error_count= 0; // Number of errors encountered

   // Test encoder/decoder match
   for(uint32_t code= 1; code < 0x11'0000; ++code) {
     Utf::utf8_t buffer[32];
     pub::utf8_decoder decoder;
     pub::utf8_encoder encoder;

     encoder.reset(buffer, 32);
     unsigned one= encoder.encode(code);
     unsigned two= encoder.encode(code);
                   encoder.encode(0);
     error_count += VERIFY( one == two ); // Encoding lengths must be equal
     decoder.reset(buffer, encoder.get_offset());

     if( code < 0x00'D800 || code > 0x00'DFFF ) {
       uint32_t
       edoc= decoder.decode();
       error_count += VERIFY( edoc == code );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == code );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );

       if( opt_verbose && code == 0x00'0041 )
         debugf("U8: %.6X {0x%.2X} {0x%.2X}\n", code, buffer[1], buffer[2]);
       if( opt_verbose && code == 0x10'0041 )
         debugf("U8: %.6X {0x%.2X,0x%.2X,0x%.2X,0x%.2X} {0x%.2X}\n", code
               ,  buffer[4], buffer[5], buffer[6], buffer[7], buffer[8]);
     } else {
       uint32_t
       edoc= decoder.decode();
       error_count += VERIFY( edoc == Utf::UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == Utf::UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );

       decoder.reset(buffer, encoder.get_offset());
       buffer[0]= 0xE0 | ((code >> 12) & 0x1F);
       buffer[1]= 0x80 | ((code >>  6) & 0x3F);
       buffer[2]= 0x80 | ((code >>  0) & 0x3F);
       error_count += VERIFY( decoder.decode() == Utf::UNI_REPLACEMENT );

       encoder.reset(buffer, 32);   // Insure encoded as UNI_REPLACEMENT
       encoder.encode(Utf::UNI_REPLACEMENT);
       error_count += VERIFY( buffer[0] == buffer[3] );
       error_count += VERIFY( buffer[1] == buffer[4] );
       error_count += VERIFY( buffer[2] == buffer[5] );

       if( opt_verbose && code == 0x00'D841 )
         debugf("U8: %.6X {0x%.2X,0x%.2X,0x%.2X} {0x%.2X}\n", code
               ,  buffer[3], buffer[4], buffer[5], buffer[6]);
     }
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_utf16
//
// Purpose-
//       Test utf16_decoder, utf16_encoder
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors found
   test_utf16( void )               // Test Utf.h: utf16_decoder, utf16_encoder
{
   if( opt_verbose )
     debugf("\ntest_utf16\n");

   int                 error_count= 0; // Number of errors encountered

   // Test encoder/decoder match
   for(uint32_t code= 1; code < 0x11'0000; ++code) {
     if( code == Utf::BYTE_ORDER_MARK || code == Utf::MARK_ORDER_BYTE )
       continue;

     Utf::utf16_t buffer[32];
     pub::utf16_decoder decoder;
     pub::utf16_encoder encoder;

     if( code < 0x00'D800 || code > 0x00'DFFF ) {
       encoder.reset(buffer, 32);
       unsigned one= encoder.encode(code);
       unsigned two= encoder.encode(code);
                     encoder.encode(0);
       error_count += VERIFY( one == two ); // Encoding lengths must be equal
       decoder.reset(buffer, encoder.get_offset());

       uint32_t
       edoc= decoder.decode();
       error_count += VERIFY( edoc == code );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == code );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       if( error_count ) {
         debugf("BE: encode(0x%.6x) decode(0x%.6x)\n", code, edoc);
         break;
       }

       if( opt_verbose && code == 0x00'0041 )
         debugf("BE: 0x%.6X: {0x%.4X} {0x%.4X}\n", code
               , ntohs(buffer[1]), ntohs(buffer[2]));
       if( opt_verbose && code == 0x01'0041 )
         debugf("BE: 0x%.6X: {0x%.4X,0x%.4X} {0x%.4X}\n", code
               , ntohs(buffer[2]), ntohs(buffer[3]), ntohs(buffer[4]));

       // Try little endian mode
       encoder.reset(buffer, 32);
       encoder.set_mode(Utf::MODE_LE);
       one= encoder.encode(code);
       two= encoder.encode(code);
            encoder.encode(0);
       error_count += VERIFY( one == two ); // Encoding lengths must be equal
       decoder.reset(buffer, encoder.get_offset());
       decoder.set_mode(Utf::MODE_LE);

       edoc= decoder.decode();
       error_count += VERIFY( edoc == code );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == code );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       if( error_count ) {
         debugf("LE: encode(0x%.6x) decode(0x%.6x)\n", code, edoc);
         break;
       }

       if( opt_verbose && code == 0x00'0041 )
         debugf("LE: 0x%.6X: {0x%.4X} {0x%.4X}\n", code
               , ntohs(buffer[1]), ntohs(buffer[2]));
       if( opt_verbose && code == 0x01'0041 )
         debugf("LE: 0x%.6X: {0x%.4X,0x%.4X} {0x%.4X}\n", code
               , ntohs(buffer[2]), ntohs(buffer[3]), ntohs(buffer[4]));
     } else {
       encoder.reset(buffer, 32);
       unsigned one= encoder.encode(code);
       unsigned two= encoder.encode(code);
                     encoder.encode(0);
       error_count += VERIFY( one == two ); // Encoding lengths must be equal
       decoder.reset(buffer, encoder.get_offset());

       uint32_t
       edoc= decoder.decode();
       error_count += VERIFY( edoc == Utf::UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == Utf::UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       if( error_count ) {
         debugf("BE: encode(0x%.6x) decode(0x%.6x)\n", code, edoc);
         break;
       }

       decoder.reset(buffer, encoder.get_offset());
       buffer[0]= htobe16(code);
       error_count += VERIFY( decoder.decode() == Utf::UNI_REPLACEMENT );

       encoder.reset(buffer, 32);
       encoder.encode(Utf::UNI_REPLACEMENT);
       error_count += VERIFY( buffer[0] == buffer[1] );

       if( error_count || (opt_verbose && code == 0x00'D841) )
         debugf("BE: 0x%.6X: {0x%.4X} {0x%.4X} {0x%.4X}\n", code
               , ntohs(buffer[0]), ntohs(buffer[1]), ntohs(buffer[2]));

       // Try little endian mode
       encoder.reset(buffer, 32);
       encoder.set_mode(Utf::MODE_LE);
       one= encoder.encode(code);
       two= encoder.encode(code);
            encoder.encode(0);
       error_count += VERIFY( one == two ); // Encoding lengths must be equal
       decoder.reset(buffer, encoder.get_offset());
       decoder.set_mode(Utf::MODE_LE);

       edoc= decoder.decode();
       error_count += VERIFY( edoc == Utf::UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == Utf::UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       if( error_count ) {
         debugf("LE: encode(0x%.6x) decode(0x%.6x)\n", code, edoc);
         break;
       }

       decoder.reset(buffer, encoder.get_offset());
       decoder.set_mode(Utf::MODE_LE);
       buffer[0]= htole16(code);
       error_count += VERIFY( decoder.decode() == Utf::UNI_REPLACEMENT );

       encoder.reset(buffer, 32);
       encoder.set_mode(Utf::MODE_LE);
       encoder.encode(Utf::UNI_REPLACEMENT);
       error_count += VERIFY( buffer[0] == buffer[1] );

       if( error_count || (opt_verbose && code == 0x00'DC41))
         debugf("LE: 0x%.6X: {0x%.4X} {0x%.4X} {0x%.4X}\n", code
               , ntohs(buffer[0]), ntohs(buffer[1]), ntohs(buffer[2]));
     }
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_utf32
//
// Purpose-
//       Test utf32_decoder, utf32_encoder
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors found
   test_utf32( void )               // Test Utf.h: utf32_decoder, utf32_encoder
{
   if( opt_verbose )
     debugf("\ntest_utf32\n");

   int                 error_count= 0; // Number of errors encountered

   // Test encoder/decoder match
   for(uint32_t code= 1; code < 0x11'0000; ++code) {
     if( code == Utf::BYTE_ORDER_MARK || code == Utf::MARK_ORDER_BYTE )
       continue;

     Utf::utf32_t buffer[32];
     pub::utf32_decoder decoder;
     pub::utf32_encoder encoder;

     if( code < 0x00'D800 || code > 0x00'DFFF ) {
       encoder.reset(buffer, 32);
       unsigned one= encoder.encode(code);
       unsigned two= encoder.encode(code);
                     encoder.encode(0);
       error_count += VERIFY( one == two ); // Encoding lengths must be equal
       decoder.reset(buffer, encoder.get_offset());

       uint32_t
       edoc= decoder.decode();
       error_count += VERIFY( edoc == code );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == code );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       if( error_count ) {
         debugf("BE: encode(0x%.6x) decode(0x%.6x)\n", code, edoc);
         debugf("BE: 0x%.6X,0x%.8X\n", ntohl(buffer[0]), ntohl(buffer[1]));
         break;
       }

       if( opt_verbose && code == 0x00'0041 )
         debugf("BE: 0x%.6X: 0x%.8X,0x%.8X\n", code
               , ntohl(buffer[0]), ntohl(buffer[1]));

       // Try little endian mode
       encoder.reset(buffer, 32);
       encoder.set_mode(Utf::MODE_LE);
       one= encoder.encode(code);
       two= encoder.encode(code);
            encoder.encode(0);
       error_count += VERIFY( one == two ); // Encoding lengths must be equal
       decoder.reset(buffer, encoder.get_offset());
       decoder.set_mode(Utf::MODE_LE);

       edoc= decoder.decode();
       error_count += VERIFY( edoc == code );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == code );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       if( error_count ) {
         debugf("LE: encode(0x%.8x) decode(0x%.8x)\n", code, edoc);
         break;
       }

       if( opt_verbose && code == 0x00'0041 )
         debugf("LE: 0x%.6X: 0x%.8X,0x%.8X\n", code
               , ntohl(buffer[0]), ntohl(buffer[1]));
     } else {
       encoder.reset(buffer, 32);
       unsigned one= encoder.encode(code);
       unsigned two= encoder.encode(code);
                     encoder.encode(0);
       error_count += VERIFY( one == two ); // Encoding lengths must be equal
       decoder.reset(buffer, encoder.get_offset());

       uint32_t
       edoc= decoder.decode();
       error_count += VERIFY( edoc == Utf::UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == Utf::UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       if( error_count ) {
         debugf("BE: encode(0x%.6x) decode(0x%.6x)\n", code, edoc);
         break;
       }

       decoder.reset(buffer, encoder.get_offset());
       buffer[0]= htobe32(code);
       error_count += VERIFY( decoder.decode() == Utf::UNI_REPLACEMENT );

       encoder.reset(buffer, 32);
       encoder.encode(Utf::UNI_REPLACEMENT);
       error_count += VERIFY( buffer[0] == buffer[1] );

       if( opt_verbose && code == 0x00'D841 )
         debugf("BE: 0x%.6X: 0x%.8X,0x%.8X\n", code
               , ntohl(buffer[0]), ntohl(buffer[1]));

       // Try little endian mode
       encoder.reset(buffer, 32);
       encoder.set_mode(Utf::MODE_LE);
       one= encoder.encode(code);
       two= encoder.encode(code);
            encoder.encode(0);
       error_count += VERIFY( one == two ); // Encoding lengths must be equal
       decoder.reset(buffer, encoder.get_offset());
       decoder.set_mode(Utf::MODE_LE);

       edoc= decoder.decode();
       error_count += VERIFY( edoc == Utf::UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == Utf::UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       if( error_count ) {
         debugf("LE: encode(0x%.8x) decode(0x%.8x)\n", code, edoc);
         break;
       }

       decoder.reset(buffer, encoder.get_offset());
       buffer[0]= htole32(code);
       error_count += VERIFY( decoder.decode() == Utf::UNI_REPLACEMENT );

       encoder.reset(buffer, 32);
       encoder.set_mode(Utf::MODE_LE);
       encoder.encode(Utf::UNI_REPLACEMENT);
       error_count += VERIFY( buffer[0] == buffer[1] );

       if( opt_verbose && code == 0x00'DC41 )
         debugf("LE: 0x%.6X: 0x%.8X,0x%.8X\n", code
               , ntohl(buffer[0]), ntohl(buffer[1]));
     }
   }

   return error_count;
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
   //-------------------------------------------------------------------------
   // Initialize
   Wrapper  tc;                     // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_init([tr](int, char*[])
   {
     setlocale(LC_NUMERIC, "");     // Allows printf("%'d\n", 123456789);
     return 0;
   });

   tc.on_main([tr](int, char*[])
   {
     int error_count= 0;

     error_count += test_utf();     // Test Utf
     error_count += test_utf8();    // Test utf8_decoder, utf8_encoder
     error_count += test_utf16();   // Test utf16_decoder, utf16_encoder
     error_count += test_utf32();   // Test utf32_decoder, utf32_encoder

     if( error_count || opt_verbose ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   return tc.run(argc, argv);
}
