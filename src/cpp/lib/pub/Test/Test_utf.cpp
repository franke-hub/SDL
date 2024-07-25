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
//       2024/07/25
//
//----------------------------------------------------------------------------
#include <endian.h>                 // For endian subroutines

#include <pub/Debug.h>              // For debugging subroutines
#include <pub/utility.h>            // For debugging subroutines
#include "pub/TEST.H"               // For VERIFY, ...
#include "pub/Wrapper.h"            // For pub::Wrapper

// The tested includes
#include "pub/Utf.h"                // For pub:: Utf, ...
#include "pub/bits/Utf_type.i"      // Import Utf.h types

// Namespace accessors
#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For Utf.h classes
using namespace PUB::debugging;     // For debugging namespace
using PUB::Wrapper;                 // For pub::Wrapper class

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
enum Glyph                          // Glyph definitions
{  ASCII_NUL=          0x00'0000    // (ASCII NUL character)
,  DOTTED_CIRCLE=      0x00'25CC    // Dotted circle, combining base
,  COMBO_LEFT=         0x00'0300    // Left diacritical combining mark
,  COMBO_RIGHT=        0x00'0301    // Right diacritical combining mark
}; // Glyph

// Byte Order Mark isn't needed for single NUL character.
static const utf32_t   test00[]=    // COL O32 O16 O08
{  ASCII_NUL                        //   0   0   0   0
}; // test00                        //   0   1   1   1 (EOF/LENGTH)

// Byte Order Mark isn't needed for single NUL character, but it doesn't hurt.
// (The BYTE_ORDER_MARK characters are SKIPPED [not encoded] for UTF-8.)
static const utf32_t   test01[]=    // COL O32 O16 O08
{  BYTE_ORDER_MARK32                //   -   0   0   *
,  ASCII_NUL                        //   0   1   1   0
}; // test01                        //   0   2   2   1 (EOF/LENGTH)

// Byte Order Mark required: The target machine's endian mode isn't known.
static const utf32_t   test02[]=    // COL O32 O16 O08
{  BYTE_ORDER_MARK32                //   -   0   0   *
,  COMBO_LEFT                       //   0   1   1   0
,  COMBO_RIGHT                      //   -   2   2   2
,  DOTTED_CIRCLE                    //   1   3   3   4
,  COMBO_LEFT                       //   -   4   4   7
,  COMBO_RIGHT                      //   -   5   5   9
,  DOTTED_CIRCLE                    //   2   6   6  11
}; // test02                        //   2   7   7  14 (EOF/LENGTH)

// Byte Order Mark required: The target machine's endian mode isn't known.
static const utf32_t   test03[]=    // COL O32 O16 O08
{  BYTE_ORDER_MARK32                //   -   0   0   *
,  ASCII_NUL                        //   0   1   1   0
,  DOTTED_CIRCLE                    //   1   2   2   1
,  COMBO_LEFT                       //   -   3   3   4
,  COMBO_RIGHT                      //   -   4   4   6
,  DOTTED_CIRCLE                    //   2   5   5   8
,  COMBO_RIGHT                      //   -   6   6  11
,  COMBO_LEFT                       //   -   7   7  13
,  0x01'2345                        //   3   8   8  15
,  'x'                              //   4   9  10  19
,  'y'                              //   5  10  11  20
,  'z'                              //   6  11  12  21
,  ASCII_NUL                        //   7  12  13  22
}; // test03                        //   7  14  14  23 (EOF/LENGTH)

//----------------------------------------------------------------------------
//
// Subroutine-
//       VERIFY_current
//
// Purpose-
//       Verify decoder.current method
//
//----------------------------------------------------------------------------
static inline int                   // Error count (0 or 1)
   VERIFY_current(                  // Verify method current()
     int               line,        // Caller's line number
     uint32_t          expect,      // Expected result
     const utf8_decoder&
                       decoder,     // The decoder
     Column            column,      // The expected column
     Offset            offset)      // The expected offset
{
     uint32_t actual= decoder.current();
     if( decoder.get_column() == column && decoder.get_offset() == offset
         && actual == expect )
       return 0;

debugf("\n%4d HCDM VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n", line);
     debugf("%4d Error: VERIFY(decoder.current())\n", line);
     debugf("  Actual: 0x%.6X= current() column(%zd) offset(%zd)\n"
           , actual, decoder.get_column(), decoder.get_offset());
     debugf("  Expect: 0x%.6X= current() column(%zd) offset(%zd) length(%zd)\n"
           , expect, column, offset, decoder.get_length());
debugf("%4d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n", line);
decoder.debug("VERIFY_current");

     return 1;
}

static inline int                   // Error count (0 or 1)
   VERIFY_current(                  // Verify method current()
     int               line,        // Caller's line number
     uint32_t          expect,      // Expected result
     const utf16_decoder&
                       decoder,     // The decoder
     Column            column,      // The expected column
     Offset            offset)      // The expected offset
{
     uint32_t actual= decoder.current();
     if( decoder.get_column() == column && decoder.get_offset() == offset
         && actual == expect )
       return 0;

debugf("\n%4d HCDM VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n", line);
     debugf("%4d Error: VERIFY(decoder.current())\n", line);
     debugf("  Actual: 0x%.6X= current() column(%zd) offset(%zd)\n"
           , actual, decoder.get_column(), decoder.get_offset());
     debugf("  Expect: 0x%.6X= current() column(%zd) offset(%zd) length(%zd)\n"
           , expect, column, offset, decoder.get_length());
debugf("%4d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n", line);
decoder.debug("VERIFY_current");

     return 1;
}

static inline int                   // Error count (0 or 1)
   VERIFY_current(                  // Verify method current()
     int               line,        // Caller's line number
     uint32_t          expect,      // Expected result
     const utf32_decoder&
                       decoder,     // The decoder
     Column            column,      // The expected column
     Offset            offset)      // The expected offset
{
     uint32_t actual= decoder.current();
     if( decoder.get_column() == column && decoder.get_offset() == offset
         && actual == expect )
       return 0;

debugf("\n%4d HCDM VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n", line);
     debugf("%4d Error: VERIFY(decoder.current())\n", line);
     debugf("  Actual: 0x%.6X= current() column(%zd) offset(%zd)\n"
           , actual, decoder.get_column(), decoder.get_offset());
     debugf("  Expect: 0x%.6X= current() column(%zd) offset(%zd) length(%zd)\n"
           , expect, column, offset, decoder.get_length());
debugf("%4d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n", line);
decoder.debug("VERIFY_current");

     return 1;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       VERIFY_decode
//
// Purpose-
//       Verify decoder.decode operation
//
//----------------------------------------------------------------------------
static inline int                   // Error count (0 or 1)
   VERIFY_decode(                   // Verify decode operation
     int               line,        // Caller's line number
     uint32_t          expect,      // Expected result
     utf8_decoder&     decoder,     // The decoder
     Column            column,      // The expected column
     Offset            offset)      // The expected offset
{
     uint32_t actual= decoder.decode();
     if( decoder.get_column() == column && decoder.get_offset() == offset
         && actual == expect )
       return 0;

debugf("\n%4d HCDM VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n", line);
     debugf("%4d Error: VERIFY(decoder.decode())\n", line);
     debugf("  Actual: 0x%.6X= decode() column(%zd) offset(%zd)\n"
           , actual, decoder.get_column(), decoder.get_offset());
     debugf("  Expect: 0x%.6X= decode() column(%zd) offset(%zd) length(%zd)\n"
           , expect, column, offset, decoder.get_length());
debugf("%4d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n", line);

     return 1;
}

static inline int                   // Error count (0 or 1)
   VERIFY_decode(                   // Verify decode operation
     int               line,        // Caller's line number
     uint32_t          expect,      // Expected result
     utf16_decoder&    decoder,     // The decoder
     Column            column,      // The expected column
     Offset            offset)      // The expected offset
{
     uint32_t actual= decoder.decode();
     if( decoder.get_column() == column && decoder.get_offset() == offset
         && actual == expect )
       return 0;

debugf("\n%4d HCDM VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n", line);
     debugf("%4d Error: VERIFY(decoder.decode())\n", line);
     debugf("  Actual: 0x%.6X= decode() column(%zd) offset(%zd)\n"
           , actual, decoder.get_column(), decoder.get_offset());
     debugf("  Expect: 0x%.6X= decode() column(%zd) offset(%zd) length(%zd)\n"
           , expect, column, offset, decoder.get_length());
debugf("%4d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n", line);

     return 1;
}

static inline int                   // Error count (0 or 1)
   VERIFY_decode(                   // Verify decode operation
     int               line,        // Caller's line number
     uint32_t          expect,      // Expected result
     utf32_decoder&    decoder,     // The decoder
     Column            column,      // The expected column
     Offset            offset)      // The expected offset
{
     uint32_t actual= decoder.decode();
     if( decoder.get_column() == column && decoder.get_offset() == offset
         && actual == expect )
       return 0;

debugf("\n%4d HCDM VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n", line);
     debugf("%4d Error: VERIFY(decoder.decode())\n", line);
     debugf("  Actual: 0x%.6X= decode() column(%zd) offset(%zd)\n"
           , actual, decoder.get_column(), decoder.get_offset());
     debugf("  Expect: 0x%.6X= decode() column(%zd) offset(%zd) length(%zd)\n"
           , expect, column, offset, decoder.get_length());
debugf("%4d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n", line);

     return 1;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       VERIFY_set_column
//
// Purpose-
//       Verify decoder.set_column operation
//
//----------------------------------------------------------------------------
static inline int                   // Error count (0 or 1)
   VERIFY_set_column(               // Verify set_column operation
     int               line,        // Caller's line number
     Length            length,      // Expected result (units past end)
     utf8_decoder&     decoder,     // The decoder
     Column            column,      // The set_column parameter
     Column            expect_col,  // The expected column
     Offset            expect_off)  // The expected offset
{
   int error_count= 0;              // Error counter

   Length actual= decoder.set_column(column);

   // If unexpected resultants
   if( actual != length
       || decoder.get_column() != expect_col
       || decoder.get_offset() != expect_off )
     ++error_count;

   if( error_count == 0 )
     return 0;

debugf("\n%4d HCDM VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n", line);
     debugf("%4d Error: VERIFY(decoder.set_column(%zd) == %zd)\n", line
           , column, length);
     debugf("  Expect: %zd= set_column(%zd) column(%zd) offset(%zd)\n"
           , length, column, expect_col, expect_off);
     debugf("  Actual: %zd= set_column(%zd) column(%zd) offset(%zd) "
            "length(%zd)\n", actual, column
           , decoder.get_column(), decoder.get_offset(), decoder.get_length());
debugf("%4d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n", line);

     return 1;
}

static inline int                   // Error count (0 or 1)
   VERIFY_set_column(               // Verify set_column operation
     int               line,        // Caller's line number
     Length            length,      // Expected result (units past end)
     utf16_decoder&    decoder,     // The decoder
     Column            column,      // The set_column parameter
     Column            expect_col,  // The expected column
     Offset            expect_off)  // The expected offset
{
   int error_count= 0;              // Error counter

   Length actual= decoder.set_column(column);

   // If unexpected resultants
   if( actual != length
       || decoder.get_column() != expect_col
       || decoder.get_offset() != expect_off )
     ++error_count;

   if( error_count == 0 )
     return 0;

debugf("\n%4d HCDM VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n", line);
     debugf("%4d Error: VERIFY(decoder.set_column(%zd) == %zd)\n", line
           , column, length);
     debugf("  Expect: %zd= set_column(%zd) column(%zd) offset(%zd)\n"
           , length, column, expect_col, expect_off);
     debugf("  Actual: %zd= set_column(%zd) column(%zd) offset(%zd) "
            "length(%zd)\n", actual, column
           , decoder.get_column(), decoder.get_offset(), decoder.get_length());
debugf("%4d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n", line);

     return 1;
}

static inline int                   // Error count (0 or 1)
   VERIFY_set_column(               // Verify set_column operation
     int               line,        // Caller's line number
     Length            length,      // Expected result (units past end)
     utf32_decoder&    decoder,     // The decoder
     Column            column,      // The set_column parameter
     Column            expect_col,  // The expected column
     Offset            expect_off)  // The expected offset
{
   int error_count= 0;              // Error counter

   Length actual= decoder.set_column(column);

   // If unexpected resultants
   if( actual != length
       || decoder.get_column() != expect_col
       || decoder.get_offset() != expect_off )
     ++error_count;

   if( error_count == 0 )
     return 0;

debugf("\n%4d HCDM VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n", line);
     debugf("%4d Error: VERIFY(decoder.set_column(%zd) == %zd)\n", line
           , column, length);
     debugf("  Expect: %zd= set_column(%zd) column(%zd) offset(%zd)\n"
           , length, column, expect_col, expect_off);
     debugf("  Actual: %zd= set_column(%zd) column(%zd) offset(%zd) "
            "length(%zd)\n", actual, column
           , decoder.get_column(), decoder.get_offset(), decoder.get_length());
debugf("%4d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n", line);

     return 1;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       VERIFY_set_offset
//
// Purpose-
//       Verify decoder.set_offset operation
//
//----------------------------------------------------------------------------
static inline int                   // Error count (0 or 1)
   VERIFY_set_offset(               // Verify set_offset operation
     int               line,        // Caller's line number
     Length            length,      // Expected result
     utf8_decoder&     decoder,     // The decoder
     Offset            offset,      // The set_offset parameter
     Offset            expect_off)  // The expected offset
{
   int error_count= 0;              // Error counter

   Offset actual= decoder.set_offset(offset);
   Column column= decoder.get_column();

   // If unexpected result
   if( actual != length || ssize_t(column) >= 0 )
     ++error_count;

   // If offset set incorrectly
   if( decoder.get_offset() != expect_off )
     ++error_count;

   if( error_count == 0 )
     return 0;

debugf("\n%4d HCDM VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n", line);
     debugf("%4d Error: VERIFY(decoder.set_offset(%zd) == %zd)\n", line
           , offset, length);
     if( ssize_t(column) >= 0 )
       debugf("  Expect: get_column(%zd) < 0\n", column);
     debugf("  Expect: %zd= set_offset(%zd) offset(%zd) length(%zd)\n"
           , length, offset, expect_off, decoder.get_length());
     debugf("  Actual: %zd= set_offset(%zd) offset(%zd)\n"
           , actual, offset, decoder.get_offset());
debugf("%4d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n", line);

     return 1;
}

static inline int                   // Error count (0 or 1)
   VERIFY_set_offset(               // Verify set_offset operation
     int               line,        // Caller's line number
     Length            length,      // Expected result
     utf16_decoder&    decoder,     // The decoder
     Offset            offset,      // The set_offset parameter
     Offset            expect_off)  // The expected offset
{
   int error_count= 0;              // Error counter

   Offset actual= decoder.set_offset(offset);
   Column column= decoder.get_column();

   // If unexpected result
   if( actual != length || ssize_t(column) >= 0 )
     ++error_count;

   // If offset set incorrectly
   if( decoder.get_offset() != expect_off )
     ++error_count;

   if( error_count == 0 )
     return 0;

debugf("\n%4d HCDM VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n", line);
     debugf("%4d Error: VERIFY(decoder.set_offset(%zd) == %zd)\n", line
           , offset, length);
     if( ssize_t(column) >= 0 )
       debugf("  Expect: get_column(%zd) < 0\n", column);
     debugf("  Expect: %zd= set_offset(%zd) offset(%zd) length(%zd)\n"
           , length, offset, expect_off, decoder.get_length());
     debugf("  Actual: %zd= set_offset(%zd) offset(%zd)\n"
           , actual, offset, decoder.get_offset());
debugf("%4d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n", line);

     return 1;
}

static inline int                   // Error count (0 or 1)
   VERIFY_set_offset(               // Verify set_offset operation
     int               line,        // Caller's line number
     Length            length,      // Expected result
     utf32_decoder&    decoder,     // The decoder
     Offset            offset,      // The set_offset parameter
     Offset            expect_off)  // The expected offset
{
   int error_count= 0;              // Error counter

   Offset actual= decoder.set_offset(offset);
   Column column= decoder.get_column();

   // If unexpected result
   if( actual != length || ssize_t(column) >= 0 )
     ++error_count;

   // If offset set incorrectly
   if( decoder.get_offset() != expect_off )
     ++error_count;

   if( error_count == 0 )
     return 0;

debugf("\n%4d HCDM VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n", line);
     debugf("%4d Error: VERIFY(decoder.set_offset(%zd) == %zd)\n", line
           , offset, length);
     if( ssize_t(column) >= 0 )
       debugf("  Expect: get_column(%zd) < 0\n", column);
     debugf("  Expect: %zd= set_offset(%zd) offset(%zd) length(%zd)\n"
           , length, offset, expect_off, decoder.get_length());
     debugf("  Actual: %zd= set_offset(%zd) offset(%zd)\n"
           , actual, offset, decoder.get_offset());
debugf("%4d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n", line);

     return 1;
}

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
     debugf("\ntest_utf =================================================\n");

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
   error_count += VERIFY( is_combining(0x00'0000) == false );
   error_count += VERIFY( is_combining(0x00'02FF) == false );
   error_count += VERIFY( is_combining(0x00'0300) == true  );
   error_count += VERIFY( is_combining(0x00'036F) == true  );
   error_count += VERIFY( is_combining(0x00'0370) == false );
   error_count += VERIFY( is_combining(0x00'1AAF) == false );
   error_count += VERIFY( is_combining(0x00'1AB0) == true  );
   error_count += VERIFY( is_combining(0x00'1AFF) == true  );
   error_count += VERIFY( is_combining(0x00'1B00) == false );
   error_count += VERIFY( is_combining(0x00'1DBF) == false );
   error_count += VERIFY( is_combining(0x00'1DC0) == true  );
   error_count += VERIFY( is_combining(0x00'1DFF) == true  );
   error_count += VERIFY( is_combining(0x00'1E00) == false );
   error_count += VERIFY( is_combining(0x00'20CF) == false );
   error_count += VERIFY( is_combining(0x00'20D0) == true  );
   error_count += VERIFY( is_combining(0x00'20FF) == true  );
   error_count += VERIFY( is_combining(0x00'2100) == false );
   error_count += VERIFY( is_combining(0x00'FE1F) == false );
   error_count += VERIFY( is_combining(0x00'FE20) == true  );
   error_count += VERIFY( is_combining(0x00'FE2F) == true  );
   error_count += VERIFY( is_combining(0x00'FE30) == false );
   error_count += VERIFY( is_combining(0x10'FFFF) == false );
   error_count += VERIFY( is_combining(0x11'0000) == false );
   error_count += VERIFY( is_combining(UTF_EOF)   == false );

   // Test is_unicode()
   error_count += VERIFY( is_unicode(0x00'0000) == true  );
   error_count += VERIFY( is_unicode(0x00'D7FF) == true  );
   error_count += VERIFY( is_unicode(0x00'D800) == false );
   error_count += VERIFY( is_unicode(0x00'DC00) == false );
   error_count += VERIFY( is_unicode(0x00'DFFF) == false );
   error_count += VERIFY( is_unicode(0x00'E000) == true  );
   error_count += VERIFY( is_unicode(0x10'FFFF) == true  );
   error_count += VERIFY( is_unicode(0x11'0000) == false );
   error_count += VERIFY( is_unicode(UTF_EOF)   == false );

   // Test strlen (Utf::strlen doesn't care about endian, so neither do we.)
   utf8_t  s08_0[]= {0};
   utf8_t  s08_1[]= {1, 0};
   utf8_t  s08_7[]= {1, 2, 3, 4, 5, 6, 7, 0};

   utf16_t s16_0[]= {0};
   utf16_t s16_1[]= {1, 0};
   utf16_t s16_7[]= {1, 2, 3, 4, 5, 6, 7, 0};

   utf32_t s32_0[]= {0};
   utf32_t s32_1[]= {1, 0};
   utf32_t s32_7[]= {1, 2, 3, 4, 5, 6, 7, 0};

   error_count += VERIFY( utf_strlen(s08_0) == 0 );
   error_count += VERIFY( utf_strlen(s08_1) == 1 );
   error_count += VERIFY( utf_strlen(s08_7) == 7 );

   error_count += VERIFY( sizeof(s08_0) ==  1 );
   error_count += VERIFY( sizeof(s08_1) ==  2 );
   error_count += VERIFY( sizeof(s08_7) ==  8 );

   error_count += VERIFY( utf_strlen(s16_0) == 0 );
   error_count += VERIFY( utf_strlen(s16_1) == 1 );
   error_count += VERIFY( utf_strlen(s16_7) == 7 );

   error_count += VERIFY( sizeof(s16_0) ==  2 );
   error_count += VERIFY( sizeof(s16_1) ==  4 );
   error_count += VERIFY( sizeof(s16_7) == 16 );

   error_count += VERIFY( utf_strlen(s32_0) == 0 );
   error_count += VERIFY( utf_strlen(s32_1) == 1 );
   error_count += VERIFY( utf_strlen(s32_7) == 7 );

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
     debugf("\ntest_utf8 ================================================\n");

   int                 error_count= 0; // Number of errors encountered

   utf8_t              buffer[32];  // Encoding buffer
   utf32_decoder       convert;     // Conversion source
   utf8_decoder        decoder;     // Decoder
   utf8_encoder        encoder;     // Encoder

   // Test encoder/decoder match
   for(uint32_t code= 1; code < 0x11'0000; ++code) {
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
       error_count += VERIFY( edoc == UTF_EOF );

       if( opt_verbose > 1 && code == 0x00'0041 )
         debugf("U8: %.6X {0x%.2X} {0x%.2X}\n", code, buffer[1], buffer[2]);
       if( opt_verbose > 1 && code == 0x10'0041 )
         debugf("U8: %.6X {0x%.2X,0x%.2X,0x%.2X,0x%.2X} {0x%.2X}\n", code
               ,  buffer[4], buffer[5], buffer[6], buffer[7], buffer[8]);
     } else {
       uint32_t
       edoc= decoder.decode();
       error_count += VERIFY( edoc == UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == UTF_EOF );

       decoder.reset(buffer, encoder.get_offset());
       buffer[0]= 0xE0 | ((code >> 12) & 0x1F);
       buffer[1]= 0x80 | ((code >>  6) & 0x3F);
       buffer[2]= 0x80 | ((code >>  0) & 0x3F);
       error_count += VERIFY( decoder.decode() == UNI_REPLACEMENT );

       encoder.reset(buffer, 32);   // Insure encoded as UNI_REPLACEMENT
       encoder.encode(UNI_REPLACEMENT);
       error_count += VERIFY( buffer[0] == buffer[3] );
       error_count += VERIFY( buffer[1] == buffer[4] );
       error_count += VERIFY( buffer[2] == buffer[5] );

       if( opt_verbose > 1 && code == 0x00'D841 )
         debugf("U8: %.6X {0x%.2X,0x%.2X,0x%.2X} {0x%.2X}\n", code
               ,  buffer[3], buffer[4], buffer[5], buffer[6]);
     }
   }

   // Test sequences
   encoder.reset(buffer, 32);
   memset(buffer, 0xFE, 32);

   //-------------------------------------------------------------------------
   if( opt_verbose ) debugf("test_empty\n"); // test_empty -----------------
   convert.reset(test00, 0);        // test_utf8::test_empty
   encoder= convert;
   decoder= encoder;

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  0, -1, 0);
   error_count += VERIFY_set_column(__LINE__,  1, decoder,  1, -1, 0);
   error_count += VERIFY_set_column(__LINE__,  5, decoder,  5, -1, 0);
   error_count += VERIFY_set_column(__LINE__, 19, decoder, 19, -1, 0);

   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  0, 0);
   error_count += VERIFY_set_offset(__LINE__,  1, decoder,  1, 0);

   error_count += VERIFY_set_offset(__LINE__,  5, decoder,  5, 0);
   error_count += VERIFY_set_offset(__LINE__, 19, decoder, 19, 0);

   //-------------------------------------------------------------------------
   if( opt_verbose ) debugf("test00\n"); // test00 {0}------------------------
   convert.reset(test00, 1);
   convert.set_mode();
   encoder= convert;
   decoder= encoder;

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  0, 0, 0);
   error_count += VERIFY_set_column(__LINE__,  1, decoder,  1, 0, 1);
   error_count += VERIFY_set_column(__LINE__,  5, decoder,  5, 0, 1);
   error_count += VERIFY_set_column(__LINE__, 19, decoder, 19, 0, 1);

   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  0, 0);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  1, 1);
   error_count += VERIFY_set_offset(__LINE__,  4, decoder,  5, 1);
   error_count += VERIFY_set_offset(__LINE__, 18, decoder, 19, 1);

   if( opt_verbose ) debugf("test01\n"); // test01 {BOM,0}--------------------
   convert.reset(test01, 2);
   convert.set_mode();
   encoder= convert;                // ** DOES NOT CONVERT BOM **
   decoder= encoder;                // test01 {0}----------------

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  0, 0, 0);
   error_count += VERIFY_set_column(__LINE__,  1, decoder,  1, 0, 1);
   error_count += VERIFY_set_column(__LINE__,  5, decoder,  5, 0, 1);
   error_count += VERIFY_set_column(__LINE__, 19, decoder, 19, 0, 1);

   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  0, 0);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  1, 1);
   error_count += VERIFY_set_offset(__LINE__,  4, decoder,  5, 1);
   error_count += VERIFY_set_offset(__LINE__, 18, decoder, 19, 1);

   //-------------------------------------------------------------------------
   if( opt_verbose ) debugf("test02\n"); // test02 {BOM,combo,...}------------
   convert.reset(test02, 7);
   convert.set_mode();
   encoder= convert;                // ** DOES NOT CONVERT BOM **
   decoder= encoder;                // test02 {combo,...}--------

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  0,  0,  0);
   error_count += VERIFY_set_column(__LINE__,  0, decoder,  1,  1,  4);
   error_count += VERIFY_set_column(__LINE__,  0, decoder,  2,  2, 11);
   error_count += VERIFY_set_column(__LINE__,  3, decoder,  5,  2, 14);
   error_count += VERIFY_set_column(__LINE__, 17, decoder, 19,  2, 14);

   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  0,  0);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  1,  1);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  5,  5);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  8,  8);
   error_count += VERIFY_set_offset(__LINE__,  5, decoder, 19, 14);

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // test02: test copy_column
   if( opt_verbose ) debugf("test02.copy_column\n");
   decoder= encoder;                // test02 {combo,...}--------
   utf8_decoder column= decoder.copy_column(); // Copy column 0
   error_count += VERIFY_decode(__LINE__, COMBO_LEFT,    column, 0,  2);
   error_count += VERIFY_decode(__LINE__, COMBO_RIGHT,   column, 0,  4);
   error_count += VERIFY_decode(__LINE__, -1,            column, 0,  4);

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  1,  1,  4);
   column= decoder.copy_column();   // Copy column 1
   error_count += VERIFY_decode(__LINE__, DOTTED_CIRCLE, column, 0,  3);
   error_count += VERIFY_decode(__LINE__, COMBO_LEFT,    column, 0,  5);
   error_count += VERIFY_decode(__LINE__, COMBO_RIGHT,   column, 0,  7);
   error_count += VERIFY_decode(__LINE__, -1,            column, 0,  7);

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  2,  2, 11);
   column= decoder.copy_column();   // Copy column 2
   error_count += VERIFY_decode(__LINE__, DOTTED_CIRCLE, column, 0,  3);
   error_count += VERIFY_decode(__LINE__, -1,            column, 0,  3);

   //-------------------------------------------------------------------------
   if( opt_verbose ) debugf("test03\n"); // test03 {BOM,0,CHAR,combo,...}-----
   convert.reset(test03, 13);
   convert.set_mode();
   encoder= convert;                // ** DOES NOT CONVERT BOM **
   decoder= encoder;                // test03 {0,CHAR,combo,...}-

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  0,  0,  0);
   error_count += VERIFY_set_column(__LINE__,  0, decoder,  1,  1,  1);
   error_count += VERIFY_set_column(__LINE__,  0, decoder,  5,  5, 20);
   error_count += VERIFY_set_column(__LINE__, 12, decoder, 19,  7, 23);

   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  0,  0);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  5,  5);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder, 19, 19);
   error_count += VERIFY_set_offset(__LINE__,  6, decoder, 29, 23);

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // test03: test copy_column
   if( opt_verbose ) debugf("test03.copy_column\n");
   decoder= encoder;                // test03 {0,CHAR,combo,...}-
   column= decoder.copy_column();   // Copy column 0
   error_count += VERIFY_decode(__LINE__, ASCII_NUL,     column, 0,  1);
   error_count += VERIFY_decode(__LINE__, -1,            column, 0,  1);

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  1,  1,  1);
   column= decoder.copy_column();   // Copy column 1
   error_count += VERIFY_decode(__LINE__, DOTTED_CIRCLE, column, 0,  3);
   error_count += VERIFY_decode(__LINE__, COMBO_LEFT,    column, 0,  5);
   error_count += VERIFY_decode(__LINE__, COMBO_RIGHT,   column, 0,  7);
   error_count += VERIFY_decode(__LINE__, -1,            column, 0,  7);

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  2,  2,  8);
   column= decoder.copy_column();   // Copy column 2
   error_count += VERIFY_decode(__LINE__, DOTTED_CIRCLE, column, 0,  3);
   error_count += VERIFY_decode(__LINE__, COMBO_RIGHT,   column, 0,  5);
   error_count += VERIFY_decode(__LINE__, COMBO_LEFT,    column, 0,  7);
   error_count += VERIFY_decode(__LINE__, -1,            column, 0,  7);

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  3,  3, 15);
   column= decoder.copy_column();   // Copy column 3
   error_count += VERIFY_decode(__LINE__, 0x01'2345,     column, 0,  4);
   error_count += VERIFY_decode(__LINE__, -1,            column, 0,  4);

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  4,  4, 19);
   column= decoder.copy_column();   // Copy column 4
   error_count += VERIFY_decode(__LINE__, 'x',           column, 0,  1);
   error_count += VERIFY_decode(__LINE__, -1,            column, 0,  1);
   // :
   error_count += VERIFY_set_column(__LINE__,  0, decoder,  7,  7, 22);
   column= decoder.copy_column();   // Copy column 7
   error_count += VERIFY_decode(__LINE__, ASCII_NUL,     column, 0,  1);
   error_count += VERIFY_decode(__LINE__, -1,            column, 0,  1);

   error_count += VERIFY_set_column(__LINE__,  1, decoder,  8,  7, 23);
   column= decoder.copy_column();   // Copy column 8 (non-existent)
   error_count += VERIFY_decode(__LINE__, -1,            column, -1, 0);

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
     debugf("\ntest_utf16 ================================================\n");

   int                 error_count= 0; // Number of errors encountered

   utf16_t             buffer[32];  // Encoding buffer
   utf32_decoder       convert;     // Conversion source
   utf16_decoder       decoder;     // Decoder
   utf16_encoder       encoder;     // Encoder

   // Test encoder/decoder match
   memset(buffer, 0xEE, 64);
   for(uint32_t code= 1; code < 0x11'0000; ++code) {
     if( code == BYTE_ORDER_MARK || code == MARK_ORDER_BYTE )
       continue;

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
       error_count += VERIFY( edoc == UTF_EOF );
       if( error_count ) {
         debugf("BE: encode(0x%.6x) decode(0x%.6x)\n", code, edoc);
         break;
       }

       if( opt_verbose > 1 && code == 0x00'0041 )
         debugf("BE: 0x%.6X: {0x%.4X} {0x%.4X}\n", code
               , ntohs(buffer[1]), ntohs(buffer[2]));
       if( opt_verbose > 1 && code == 0x01'0041 )
         debugf("BE: 0x%.6X: {0x%.4X,0x%.4X} {0x%.4X}\n", code
               , ntohs(buffer[2]), ntohs(buffer[3]), ntohs(buffer[4]));

       // Use little endian mode
       encoder.reset(buffer, 32, MODE_LE);
       one= encoder.encode(code);
       two= encoder.encode(code);
            encoder.encode(0);
       error_count += VERIFY( one == two ); // Encoding lengths must be equal
       decoder.reset(buffer, encoder.get_offset(), MODE_LE);

       edoc= decoder.decode();
       error_count += VERIFY( edoc == code );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == code );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == UTF_EOF );
       if( error_count ) {
         debugf("LE: encode(0x%.6x) decode(0x%.6x)\n", code, edoc);
         break;
       }

       if( opt_verbose > 1 && code == 0x00'0041 )
         debugf("LE: 0x%.6X: {0x%.4X} {0x%.4X}\n", code
               , ntohs(buffer[1]), ntohs(buffer[2]));
       if( opt_verbose > 1 && code == 0x01'0041 )
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
       error_count += VERIFY( edoc == UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == UTF_EOF );
       if( error_count ) {
         debugf("BE: encode(0x%.6x) decode(0x%.6x)\n", code, edoc);
         break;
       }

       decoder.reset(buffer, encoder.get_offset());
       buffer[0]= htobe16(code);
       error_count += VERIFY( decoder.decode() == UNI_REPLACEMENT );

       encoder.reset(buffer, 32);
       encoder.encode(UNI_REPLACEMENT);
       error_count += VERIFY( buffer[0] == buffer[1] );

       if( error_count || (opt_verbose > 1 && code == 0x00'D841) )
         debugf("BE: 0x%.6X: {0x%.4X} {0x%.4X} {0x%.4X}\n", code
               , ntohs(buffer[0]), ntohs(buffer[1]), ntohs(buffer[2]));

       // Use little endian mode
       encoder.reset(buffer, 32, MODE_LE);
       one= encoder.encode(code);
       two= encoder.encode(code);
            encoder.encode(0);
       error_count += VERIFY( one == two ); // Encoding lengths must be equal
       decoder.reset(buffer, encoder.get_offset(), MODE_LE);

       edoc= decoder.decode();
       error_count += VERIFY( edoc == UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == UTF_EOF );
       if( error_count ) {
         debugf("LE: encode(0x%.6x) decode(0x%.6x)\n", code, edoc);
         break;
       }

       decoder.reset(buffer, encoder.get_offset(), MODE_LE);
       buffer[0]= htole16(code);
       error_count += VERIFY( decoder.decode() == UNI_REPLACEMENT );

       encoder.reset(buffer, 32, MODE_LE);
       encoder.encode(UNI_REPLACEMENT);
       error_count += VERIFY( buffer[0] == buffer[1] );

       if( error_count || (opt_verbose > 1 && code == 0x00'DC41))
         debugf("LE: 0x%.6X: {0x%.4X} {0x%.4X} {0x%.4X}\n", code
               , ntohs(buffer[0]), ntohs(buffer[1]), ntohs(buffer[2]));
     }
   }

   // Test sequences
   encoder.reset(buffer, 32);
   memset(buffer, 0xFE, 64);

   //-------------------------------------------------------------------------
   if( opt_verbose ) debugf("test_empty\n"); //  test_empty ------------------
   convert.reset(test00, 0);        // test_utf16::test_empty
   convert.set_mode();
   encoder= convert;
   decoder= encoder;

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  0, -1, 0);
   error_count += VERIFY_set_column(__LINE__,  1, decoder,  1, -1, 0);
   error_count += VERIFY_set_column(__LINE__,  5, decoder,  5, -1, 0);
   error_count += VERIFY_set_column(__LINE__, 19, decoder, 19, -1, 0);

   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  0, 0);
   error_count += VERIFY_set_offset(__LINE__,  1, decoder,  1, 0);

   error_count += VERIFY_set_offset(__LINE__,  5, decoder,  5, 0);
   error_count += VERIFY_set_offset(__LINE__, 19, decoder, 19, 0);

   //-------------------------------------------------------------------------
   if( opt_verbose ) debugf("test00\n"); // test00 {0}------------------------
   convert.reset(test00, 1);
   convert.set_mode();
   encoder= convert;
   decoder= encoder;

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  0, 0, 0);
   error_count += VERIFY_set_column(__LINE__,  1, decoder,  1, 0, 1);
   error_count += VERIFY_set_column(__LINE__,  5, decoder,  5, 0, 1);
   error_count += VERIFY_set_column(__LINE__, 19, decoder, 19, 0, 1);

   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  0, 0);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  1, 1);
   error_count += VERIFY_set_offset(__LINE__,  4, decoder,  5, 1);
   error_count += VERIFY_set_offset(__LINE__, 18, decoder, 19, 1);

   if( opt_verbose ) debugf("test01\n"); // test01 {BOM,0}--------------------
   convert.reset(test01, 2);
   convert.set_mode();
   encoder= convert;
   decoder= encoder;

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  0, 0, 1);
   error_count += VERIFY_set_column(__LINE__,  1, decoder,  1, 0, 2);
   error_count += VERIFY_set_column(__LINE__,  5, decoder,  5, 0, 2);
   error_count += VERIFY_set_column(__LINE__, 19, decoder, 19, 0, 2);

   // Can't set offset at BOM mark, so offset set to 1
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  0, 1);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  1, 1);
   error_count += VERIFY_set_offset(__LINE__,  3, decoder,  5, 2);
   error_count += VERIFY_set_offset(__LINE__, 17, decoder, 19, 2);

   //-------------------------------------------------------------------------
   if( opt_verbose ) debugf("test02\n"); // test02 {BOM,combo,...}------------
   convert.reset(test02, 7);
   convert.set_mode();
   encoder= convert;
   decoder= encoder;

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  0, 0, 1);
   error_count += VERIFY_set_column(__LINE__,  0, decoder,  1, 1, 3);
   error_count += VERIFY_set_column(__LINE__,  0, decoder,  2, 2, 6);
   error_count += VERIFY_set_column(__LINE__,  3, decoder,  5, 2, 7);
   error_count += VERIFY_set_column(__LINE__, 17, decoder, 19, 2, 7);

   // Can't set offset at BOM mark, so offset set to 1
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  0, 1);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  1, 1);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  5, 5);
   error_count += VERIFY_set_offset(__LINE__,  1, decoder,  8, 7);
   error_count += VERIFY_set_offset(__LINE__, 12, decoder, 19, 7);

   //-------------------------------------------------------------------------
   if( opt_verbose ) debugf("test03\n"); // test03 {BOM,0,CHAR,combo,...}-----
   convert.reset(test03, 13);
   convert.set_mode();
   encoder= convert;
   decoder= encoder;

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  0,  0,  1);
   error_count += VERIFY_set_column(__LINE__,  0, decoder,  1,  1,  2);
   error_count += VERIFY_set_column(__LINE__,  0, decoder,  5,  5, 11);
   error_count += VERIFY_set_column(__LINE__, 12, decoder, 19,  7, 14);

   // Can't set offset at BOM mark, so offset set to 1
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  0,  1);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  1,  1);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  5,  5);
   error_count += VERIFY_set_offset(__LINE__,  5, decoder, 19, 14);

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
     debugf("\ntest_utf32 ================================================\n");

   int                 error_count= 0; // Number of errors encountered

   utf32_t             buffer[32];  // Encoding buffer
   utf32_decoder       decoder;     // Decoder
   utf32_encoder       encoder;     // Encoder

   // Test encoder/decoder match
   memset(buffer, 0xEE, 128);
   for(uint32_t code= 1; code < 0x11'0000; ++code) {
     if( code == BYTE_ORDER_MARK || code == MARK_ORDER_BYTE )
       continue;

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
       error_count += VERIFY( edoc == UTF_EOF );
       if( error_count ) {
         debugf("BE: encode(0x%.6x) decode(0x%.6x)\n", code, edoc);
         debugf("BE: 0x%.6X,0x%.8X\n", ntohl(buffer[0]), ntohl(buffer[1]));
         break;
       }

       if( opt_verbose > 1 && code == 0x00'0041 )
         debugf("BE: 0x%.6X: 0x%.8X,0x%.8X\n", code
               , ntohl(buffer[0]), ntohl(buffer[1]));

       // Use little endian mode
       encoder.reset(buffer, 32);
       encoder.set_mode(MODE_LE);
       one= encoder.encode(code);
       two= encoder.encode(code);
            encoder.encode(0);
       error_count += VERIFY( one == two ); // Encoding lengths must be equal
       decoder.reset(buffer, encoder.get_offset());
       decoder.set_mode(MODE_LE);

       edoc= decoder.decode();
       error_count += VERIFY( edoc == code );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == code );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == UTF_EOF );
       if( error_count ) {
         debugf("LE: encode(0x%.8x) decode(0x%.8x)\n", code, edoc);
         break;
       }

       if( opt_verbose > 1 && code == 0x00'0041 )
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
       error_count += VERIFY( edoc == UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == UTF_EOF );
       if( error_count ) {
         debugf("BE: encode(0x%.6x) decode(0x%.6x)\n", code, edoc);
         break;
       }

       decoder.reset(buffer, encoder.get_offset());
       buffer[0]= htobe32(code);
       error_count += VERIFY( decoder.decode() == UNI_REPLACEMENT );

       encoder.reset(buffer, 32);
       encoder.encode(UNI_REPLACEMENT);
       error_count += VERIFY( buffer[0] == buffer[1] );

       if( opt_verbose > 1 && code == 0x00'D841 )
         debugf("BE: 0x%.6X: 0x%.8X,0x%.8X\n", code
               , ntohl(buffer[0]), ntohl(buffer[1]));

       // Use little endian mode
       encoder.reset(buffer, 32);
       encoder.set_mode(MODE_LE);
       one= encoder.encode(code);
       two= encoder.encode(code);
            encoder.encode(0);
       error_count += VERIFY( one == two ); // Encoding lengths must be equal
       decoder.reset(buffer, encoder.get_offset());
       decoder.set_mode(MODE_LE);

       edoc= decoder.decode();
       error_count += VERIFY( edoc == UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == UNI_REPLACEMENT );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == 0 );
       edoc= decoder.decode();
       error_count += VERIFY( edoc == UTF_EOF );
       if( error_count ) {
         debugf("LE: encode(0x%.8x) decode(0x%.8x)\n", code, edoc);
         break;
       }

       decoder.reset(buffer, encoder.get_offset());
       buffer[0]= htole32(code);
       error_count += VERIFY( decoder.decode() == UNI_REPLACEMENT );

       encoder.reset(buffer, 32);
       encoder.set_mode(MODE_LE);
       encoder.encode(UNI_REPLACEMENT);
       error_count += VERIFY( buffer[0] == buffer[1] );

       if( opt_verbose > 1 && code == 0x00'DC41 )
         debugf("LE: 0x%.6X: 0x%.8X,0x%.8X\n", code
               , ntohl(buffer[0]), ntohl(buffer[1]));
     }
   }

   // Test sequences
   error_count += VERIFY( sizeof(test00) ==  4 );
   error_count += VERIFY( sizeof(test01) ==  8 );
   error_count += VERIFY( sizeof(test02) == 28 );
   error_count += VERIFY( sizeof(test03) == 52 );

   //-------------------------------------------------------------------------
   if( opt_verbose ) debugf("test_empty\n"); //  test_empty-------------------
   decoder.reset(buffer, 0);        // test_utf32::test_empty
   decoder.set_mode();

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  0, -1, 0);
   error_count += VERIFY_set_column(__LINE__,  1, decoder,  1, -1, 0);
   error_count += VERIFY_set_column(__LINE__,  5, decoder,  5, -1, 0);
   error_count += VERIFY_set_column(__LINE__, 19, decoder, 19, -1, 0);

   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  0, 0);
   error_count += VERIFY_set_offset(__LINE__,  1, decoder,  1, 0);
   error_count += VERIFY_set_offset(__LINE__,  5, decoder,  5, 0);
   error_count += VERIFY_set_offset(__LINE__, 19, decoder, 19, 0);

   //-------------------------------------------------------------------------
   if( opt_verbose ) debugf("test00\n"); // test00 {0}------------------------
   decoder.reset(test00, 1);
   decoder.set_mode();

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  0, 0, 0);
   error_count += VERIFY_set_column(__LINE__,  1, decoder,  1, 0, 1);
   error_count += VERIFY_set_column(__LINE__,  5, decoder,  5, 0, 1);
   error_count += VERIFY_set_column(__LINE__, 19, decoder, 19, 0, 1);

   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  0, 0);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  1, 1);
   error_count += VERIFY_set_offset(__LINE__,  4, decoder,  5, 1);
   error_count += VERIFY_set_offset(__LINE__, 18, decoder, 19, 1);

   if( opt_verbose ) debugf("test01\n"); // test01 {BOM,0}--------------------
   decoder.reset(test01, 2);
   decoder.set_mode();

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  0, 0, 1);
   error_count += VERIFY_set_column(__LINE__,  1, decoder,  1, 0, 2);
   error_count += VERIFY_set_column(__LINE__,  5, decoder,  5, 0, 2);
   error_count += VERIFY_set_column(__LINE__, 19, decoder, 19, 0, 2);

   // Can't set offset at BOM mark, so offset set to 1
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  0, 1);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  1, 1);
   error_count += VERIFY_set_offset(__LINE__,  3, decoder,  5, 2);
   error_count += VERIFY_set_offset(__LINE__, 17, decoder, 19, 2);

   //-------------------------------------------------------------------------
   if( opt_verbose ) debugf("test02\n"); // test02 {BOM,combo,...}------------
   decoder.reset(test02, 7);
   decoder.set_mode();

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  0, 0, 1);
   error_count += VERIFY_set_column(__LINE__,  0, decoder,  1, 1, 3);
   error_count += VERIFY_set_column(__LINE__,  0, decoder,  2, 2, 6);
   error_count += VERIFY_set_column(__LINE__,  3, decoder,  5, 2, 7);
   error_count += VERIFY_set_column(__LINE__, 17, decoder, 19, 2, 7);

   // Can't set offset at BOM mark, so offset set to 1
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  0, 1);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  1, 1);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  5, 5);
   error_count += VERIFY_set_offset(__LINE__,  1, decoder,  8, 7);
   error_count += VERIFY_set_offset(__LINE__, 12, decoder, 19, 7);

   //-------------------------------------------------------------------------
   if( opt_verbose ) debugf("test03\n"); // test03 {BOM,0,CHAR,combo,...}-----
   decoder.reset(test03, 13);
   decoder.set_mode();

   error_count += VERIFY_set_column(__LINE__,  0, decoder,  0,  0,  1);
   error_count += VERIFY_set_column(__LINE__,  0, decoder,  1,  1,  2);
   error_count += VERIFY_set_column(__LINE__,  0, decoder,  5,  5, 10);
   error_count += VERIFY_set_column(__LINE__, 12, decoder, 19,  7, 13);

   // Can't set offset at BOM mark, so offset set to 1
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  0,  1);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  1,  1);
   error_count += VERIFY_set_offset(__LINE__,  0, decoder,  5,  5);
   error_count += VERIFY_set_offset(__LINE__,  6, decoder, 19, 13);

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_assign
//
// Purpose-
//       Test assignment operators
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors found
   test_assign( void )              // Test Utf.h: assignment operators
{
   if( opt_verbose )
     debugf("\ntest_assign ==============================================\n");

   int                 error_count= 0; // Number of errors encountered

   utf8_t              buffer08[32]; // Encoding buffer
   utf8_decoder        decoder08;   // Decoder
   utf8_encoder        encoder08(buffer08, 32); // Encoder

   utf16_t             buffer16[32]; // Encoding buffer
   utf16_decoder       decoder16;   // Decoder
   utf16_encoder       encoder16(buffer16, 32); // Encoder

   utf32_t             buffer32[32]; // Encoding buffer
   utf32_decoder       decoder32;   // Decoder
   utf32_encoder       encoder32(buffer32, 32); // Encoder

   utf32_decoder       decoder;     // (TEST) decoder

   // Test sequences
   memset(buffer08, 0xE1, sizeof(buffer08));
   memset(buffer16, 0xE2, sizeof(buffer16));
   memset(buffer32, 0xE4, sizeof(buffer32));

   //-------------------------------------------------------------------------
   if( opt_verbose ) debugf("test_empty\n"); // test_empty -------------------
   decoder.reset(test00, 0);
   for(int i= 0; i<3; ++i) {
     switch(i) {
       case 2:
         encoder08= decoder08;
         encoder16= decoder08;
         encoder32= decoder08;
         break;

       case 1:
         encoder08= decoder16;
         encoder16= decoder16;
         encoder32= decoder16;
         break;

       case 0:
       default:
         encoder08= decoder;
         encoder16= decoder;
         encoder32= decoder;
     }

     decoder08= encoder08;
     decoder16= encoder16;
     decoder32= encoder32;

     error_count += VERIFY_decode(__LINE__, UTF_EOF, decoder08, -1, 0);
     error_count += VERIFY_decode(__LINE__, UTF_EOF, decoder16, -1, 0);
     error_count += VERIFY_decode(__LINE__, UTF_EOF, decoder32, -1, 0);
   }

   //-------------------------------------------------------------------------
   if( opt_verbose ) debugf("test00\n"); // test00 {0}------------------------
   decoder.reset(test00, 1);
   for(int i= 0; i<3; ++i) {
     switch(i) {
       case 2:
         encoder08= decoder08;
         encoder16= decoder08;
         encoder32= decoder08;
         break;

       case 1:
         encoder08= decoder16;
         encoder16= decoder16;
         encoder32= decoder16;
         break;

       case 0:
       default:
         encoder08= decoder;
         encoder16= decoder;
         encoder32= decoder;
     }

     decoder08= encoder08;
     decoder16= encoder16;
     decoder32= encoder32;

     error_count += VERIFY_decode(__LINE__, ASCII_NUL, decoder08, 0, 1);
     error_count += VERIFY_decode(__LINE__, ASCII_NUL, decoder16, 0, 1);
     error_count += VERIFY_decode(__LINE__, ASCII_NUL, decoder32, 0, 1);

     error_count += VERIFY_decode(__LINE__, UTF_EOF,   decoder08, 0, 1);
     error_count += VERIFY_decode(__LINE__, UTF_EOF,   decoder16, 0, 1);
     error_count += VERIFY_decode(__LINE__, UTF_EOF,   decoder32, 0, 1);
   }

   //-------------------------------------------------------------------------
   if( opt_verbose ) debugf("test01\n"); // test01 {BOM,0}--------------------
   decoder.reset(test01, 2, MODE_RESET);
   decoder.set_mode();
   for(int i= 0; i<3; ++i) {
     Offset O= 1;
     switch(i) {
       case 2:
         encoder08= decoder08;
         encoder16= decoder08;
         encoder32= decoder08;
         O= 0;
         break;

       case 1:
         encoder08= decoder16;
         encoder16= decoder16;
         encoder32= decoder16;
         break;

       case 0:
       default:
         encoder08= decoder;
         encoder16= decoder;
         encoder32= decoder;
     }

     decoder08= encoder08;
     decoder16= encoder16;
     decoder32= encoder32;

     error_count += VERIFY_decode(__LINE__, ASCII_NUL, decoder08, 0,   1);
     error_count += VERIFY_decode(__LINE__, ASCII_NUL, decoder16, 0, O+1);
     error_count += VERIFY_decode(__LINE__, ASCII_NUL, decoder32, 0, O+1);

     error_count += VERIFY_decode(__LINE__, UTF_EOF,   decoder08, 0,   1);
     error_count += VERIFY_decode(__LINE__, UTF_EOF,   decoder16, 0, O+1);
     error_count += VERIFY_decode(__LINE__, UTF_EOF,   decoder32, 0, O+1);
   }

   //-------------------------------------------------------------------------
   if( opt_verbose ) debugf("test02\n"); // test02 {BOM,combo,...}------------
   decoder.reset(test02, 7, MODE_RESET);
   decoder.set_mode();
   for(int i= 0; i<3; ++i) {
     Offset O= 1;
     switch(i) {
       case 2:
         encoder08= decoder08;
         encoder16= decoder08;
         encoder32= decoder08;
         O= 0;
         break;

       case 1:
         encoder08= decoder16;
         encoder16= decoder16;
         encoder32= decoder16;
         break;

       case 0:
       default:
         encoder08= decoder;
         encoder16= decoder;
         encoder32= decoder;
     }

     decoder08= encoder08;
     decoder16= encoder16;
     decoder32= encoder32;

     error_count += VERIFY_decode(__LINE__, COMBO_LEFT,     decoder08, 0,   2);
     error_count += VERIFY_decode(__LINE__, COMBO_LEFT,     decoder16, 0, O+1);
     error_count += VERIFY_decode(__LINE__, COMBO_LEFT,     decoder32, 0, O+1);

     error_count += VERIFY_current(__LINE__, COMBO_RIGHT,   decoder08, 0,   2);
     error_count += VERIFY_current(__LINE__, COMBO_RIGHT,   decoder16, 0, O+1);
     error_count += VERIFY_current(__LINE__, COMBO_RIGHT,   decoder32, 0, O+1);

     error_count += VERIFY_decode(__LINE__, COMBO_RIGHT,    decoder08, 1,   4);
     error_count += VERIFY_decode(__LINE__, COMBO_RIGHT,    decoder16, 1, O+2);
     error_count += VERIFY_decode(__LINE__, COMBO_RIGHT,    decoder32, 1, O+2);

     error_count += VERIFY_current(__LINE__, DOTTED_CIRCLE, decoder08, 1,   4);
     error_count += VERIFY_current(__LINE__, DOTTED_CIRCLE, decoder16, 1, O+2);
     error_count += VERIFY_current(__LINE__, DOTTED_CIRCLE, decoder32, 1, O+2);

     error_count += VERIFY_decode(__LINE__, DOTTED_CIRCLE,  decoder08, 1,   7);
     error_count += VERIFY_decode(__LINE__, DOTTED_CIRCLE,  decoder16, 1, O+3);
     error_count += VERIFY_decode(__LINE__, DOTTED_CIRCLE,  decoder32, 1, O+3);

     error_count += VERIFY_decode(__LINE__, COMBO_LEFT,     decoder08, 1,   9);
     error_count += VERIFY_decode(__LINE__, COMBO_LEFT,     decoder16, 1, O+4);
     error_count += VERIFY_decode(__LINE__, COMBO_LEFT,     decoder32, 1, O+4);

     error_count += VERIFY_current(__LINE__, COMBO_RIGHT,   decoder08, 1,   9);
     error_count += VERIFY_current(__LINE__, COMBO_RIGHT,   decoder16, 1, O+4);
     error_count += VERIFY_current(__LINE__, COMBO_RIGHT,   decoder32, 1, O+4);

     error_count += VERIFY_decode(__LINE__, COMBO_RIGHT,    decoder08, 2,  11);
     error_count += VERIFY_decode(__LINE__, COMBO_RIGHT,    decoder16, 2, O+5);
     error_count += VERIFY_decode(__LINE__, COMBO_RIGHT,    decoder32, 2, O+5);

     error_count += VERIFY_decode(__LINE__, DOTTED_CIRCLE,  decoder08, 2,  14);
     error_count += VERIFY_decode(__LINE__, DOTTED_CIRCLE,  decoder16, 2, O+6);
     error_count += VERIFY_decode(__LINE__, DOTTED_CIRCLE,  decoder32, 2, O+6);

     error_count += VERIFY_decode(__LINE__, UTF_EOF, decoder08, 2,  14);
     error_count += VERIFY_decode(__LINE__, UTF_EOF, decoder16, 2, O+6);
     error_count += VERIFY_decode(__LINE__, UTF_EOF, decoder32, 2, O+6);
   }

   //-------------------------------------------------------------------------
   if( opt_verbose ) debugf("test03\n"); // test03 {BOM,0,CHAR,combo,...}-----
   decoder.reset(test03, 13, MODE_RESET);
   decoder.set_mode();
   for(int i= 0; i<3; ++i) {
     Offset O= 1;
     switch(i) {
       case 2:
         encoder08= decoder08;
         encoder16= decoder08;
         encoder32= decoder08;
         O= 0;
         break;

       case 1:
         encoder08= decoder16;
         encoder16= decoder16;
         encoder32= decoder16;
         break;

       case 0:
       default:
         encoder08= decoder;
         encoder16= decoder;
         encoder32= decoder;
     }

     decoder08= encoder08;
     decoder16= encoder16;
     decoder32= encoder32;

     error_count += VERIFY_decode(__LINE__, ASCII_NUL,      decoder08, 1,    1);
     error_count += VERIFY_decode(__LINE__, ASCII_NUL,      decoder16, 1, O+ 1);
     error_count += VERIFY_decode(__LINE__, ASCII_NUL,      decoder32, 1, O+ 1);

     error_count += VERIFY_current(__LINE__, DOTTED_CIRCLE, decoder08, 1,    1);
     error_count += VERIFY_current(__LINE__, DOTTED_CIRCLE, decoder16, 1, O+ 1);
     error_count += VERIFY_current(__LINE__, DOTTED_CIRCLE, decoder32, 1, O+ 1);

     error_count += VERIFY_decode(__LINE__, DOTTED_CIRCLE,  decoder08, 1,    4);
     error_count += VERIFY_decode(__LINE__, DOTTED_CIRCLE,  decoder16, 1, O+ 2);
     error_count += VERIFY_decode(__LINE__, DOTTED_CIRCLE,  decoder32, 1, O+ 2);

     error_count += VERIFY_current(__LINE__, COMBO_LEFT,    decoder08, 1,    4);
     error_count += VERIFY_current(__LINE__, COMBO_LEFT,    decoder16, 1, O+ 2);
     error_count += VERIFY_current(__LINE__, COMBO_LEFT,    decoder32, 1, O+ 2);

     error_count += VERIFY_decode(__LINE__, COMBO_LEFT,     decoder08, 1,    6);
     error_count += VERIFY_decode(__LINE__, COMBO_LEFT,     decoder16, 1, O+ 3);
     error_count += VERIFY_decode(__LINE__, COMBO_LEFT,     decoder32, 1, O+ 3);

     error_count += VERIFY_current(__LINE__, COMBO_RIGHT,   decoder08, 1,    6);
     error_count += VERIFY_current(__LINE__, COMBO_RIGHT,   decoder16, 1, O+ 3);
     error_count += VERIFY_current(__LINE__, COMBO_RIGHT,   decoder32, 1, O+ 3);

     error_count += VERIFY_decode(__LINE__, COMBO_RIGHT,    decoder08, 2,    8);
     error_count += VERIFY_decode(__LINE__, COMBO_RIGHT,    decoder16, 2, O+ 4);
     error_count += VERIFY_decode(__LINE__, COMBO_RIGHT,    decoder32, 2, O+ 4);

     error_count += VERIFY_decode(__LINE__, DOTTED_CIRCLE,  decoder08, 2,   11);
     error_count += VERIFY_decode(__LINE__, DOTTED_CIRCLE,  decoder16, 2, O+ 5);
     error_count += VERIFY_decode(__LINE__, DOTTED_CIRCLE,  decoder32, 2, O+ 5);

     error_count += VERIFY_decode(__LINE__, COMBO_RIGHT,    decoder08, 2,   13);
     error_count += VERIFY_decode(__LINE__, COMBO_RIGHT,    decoder16, 2, O+ 6);
     error_count += VERIFY_decode(__LINE__, COMBO_RIGHT,    decoder32, 2, O+ 6);

     error_count += VERIFY_current(__LINE__, COMBO_LEFT,    decoder08, 2,   13);
     error_count += VERIFY_current(__LINE__, COMBO_LEFT,    decoder16, 2, O+ 6);
     error_count += VERIFY_current(__LINE__, COMBO_LEFT,    decoder32, 2, O+ 6);

     error_count += VERIFY_decode(__LINE__, COMBO_LEFT,     decoder08, 3,   15);
     error_count += VERIFY_decode(__LINE__, COMBO_LEFT,     decoder16, 3, O+ 7);
     error_count += VERIFY_decode(__LINE__, COMBO_LEFT,     decoder32, 3, O+ 7);

     error_count += VERIFY_decode(__LINE__, 0x01'2345,      decoder08, 4,   19);
     error_count += VERIFY_decode(__LINE__, 0x01'2345,      decoder16, 4, O+ 9);
     error_count += VERIFY_decode(__LINE__, 0x01'2345,      decoder32, 4, O+ 8);

     error_count += VERIFY_decode(__LINE__, utf32_t('x'),   decoder08, 5,   20);
     error_count += VERIFY_decode(__LINE__, utf32_t('x'),   decoder16, 5, O+10);
     error_count += VERIFY_decode(__LINE__, utf32_t('x'),   decoder32, 5, O+ 9);

     error_count += VERIFY_decode(__LINE__, utf32_t('y'),   decoder08, 6,   21);
     error_count += VERIFY_decode(__LINE__, utf32_t('y'),   decoder16, 6, O+11);
     error_count += VERIFY_decode(__LINE__, utf32_t('y'),   decoder32, 6, O+10);

     error_count += VERIFY_decode(__LINE__, utf32_t('z'),   decoder08, 7,   22);
     error_count += VERIFY_decode(__LINE__, utf32_t('z'),   decoder16, 7, O+12);
     error_count += VERIFY_decode(__LINE__, utf32_t('z'),   decoder32, 7, O+11);

     error_count += VERIFY_decode(__LINE__, ASCII_NUL,      decoder08, 7,   23);
     error_count += VERIFY_decode(__LINE__, ASCII_NUL,      decoder16, 7, O+13);
     error_count += VERIFY_decode(__LINE__, ASCII_NUL,      decoder32, 7, O+12);

     error_count += VERIFY_decode(__LINE__, UTF_EOF,        decoder08, 7,   23);
     error_count += VERIFY_decode(__LINE__, UTF_EOF,        decoder16, 7, O+13);
     error_count += VERIFY_decode(__LINE__, UTF_EOF,        decoder32, 7, O+12);
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
     error_count += test_utf8();    // Test utf8_decoder,  utf8_encoder
     error_count += test_utf16();   // Test utf16_decoder, utf16_encoder
     error_count += test_utf32();   // Test utf32_decoder, utf32_encoder

     error_count += test_assign();  // Test assignment operators

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
