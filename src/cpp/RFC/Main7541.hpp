//----------------------------------------------------------------------------
//
//       Copyright (C) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Main7541.hpp
//
// Purpose-
//       RFC7541 Examples
//
// Last change date-
//       2023/10/13
//
// Implementation notes-
//       Included by Main.hpp
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Subroutine-
//       intx_verify
//
// Purpose-
//       Test RFC7541: HPACK encoding/decoding integer example
//
//----------------------------------------------------------------------------
static octet integer_head[8]= {0x80, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80};
//                                0     1     2     3     4     5     6     7
static inline int
   intx_verify(                     // Verify integer example
     const char*       id,          // Test identifier
     Value_t           value,       // Test integer
     int               bits)        // Number of encoding bits
{
   if( opt_verbose )
     debugf("\n%s Verify encode/decode integer(%d:%d)\n", id, value, bits);
   int error_count= 0;

   // Input/output objects
   Ioda       writer;
   IodaReader reader(writer);

   // Test objects
   Integer    integer;
   Value_t    decoded;

   if( bits ) {
     integer.encode(writer, value, integer_head[bits], bits);
     decoded= integer.decode(reader, bits);
     error_count += VERIFY( value == decoded );
   } else {
     integer.encode(writer, value, 0x00, 7);
     decoded= integer.decode(reader, 7);
     error_count += VERIFY( value == decoded );
   }

   if( opt_verbose || error_count ) {
     if( error_count )
       debugf("%s Error: value(%d) != decoded(%d)\n", id, value, decoded);
     writer.dump("intx_verify");
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       example_C1
//
// Purpose-
//       Test RFC7541: HPACK encoding/decoding examples C.1.*
//
//----------------------------------------------------------------------------
static inline int
   example_C1( void )               // Test RFC7541: Examples C.1.*
{
   int        error_count= 0;
   // C.1.1 Example 1: Encoding 10 using a 5-bit prefix
   error_count += intx_verify("C.1.1", 10, 5);

   // C.1.2 Example 2: Encoding 1337 using a 5-bit prefix
   error_count += intx_verify("C.1.2", 1337, 5);

   // C.1.2 Example 3: Encoding 42
   error_count += intx_verify("C.1.3", 42, 0);

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       prop_verify
//
// Purpose-
//       Test RFC7541: HPACK encoding/decoding test, with verification
//
//----------------------------------------------------------------------------
static inline int
   prop_verify(                     // HPACK test, with verification
     const char*       id,          // Test identifier
     IodaReader&       reader,      // Associated Reader
     Ioda&             writer,      // Associates Writer
     Pack&             inp_pack,    // Input HPACK decoder
     Pack&             out_pack,    // Output HPACK encoder
     const Properties& out_prop)    // Output Properties
{
   // Hard Core Debug Mode (if needed)
   const char* oops= "NOT NEEDED ANY MORE";
   if( strcmp(id, oops) == 0 ) {
     debugf("\n\n%s initial state -----------------------------------\n", id);
     out_pack.debug(oops);
     debugf("\n");
     out_pack.debug(oops);
     debugf("\n");
   }

   if( opt_verbose )
     debugf("\n\n%s encode-------------------------------------------\n", id);
   out_pack.encode(writer, out_prop);
   if( opt_verbose ) {
     out_pack.debug("out_pack encoded");
     writer.dump("writer encoded");
   }

   if( opt_verbose )
     debugf("\n\n%s decode-------------------------------------------\n", id);
   Properties inp_prop= inp_pack.decode(reader);
   if( opt_verbose ) {
     inp_pack.debug("inp_pack decoded");
     reader.dump("reader decoded");
   }

   int error_count= int(out_prop != inp_prop);
   if( error_count) {
     debugf("\n");
     debugf("%s Error: out_prop != inp_prop\n", id);
     out_prop.debug("out_prop");
     debugf("\n");
     inp_prop.debug("inp_prop");
   }

   error_count += VERIFY( reader.get_length() == 0 );
   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       example_C2
//
// Purpose-
//       Test RFC7541: HPACK encoding/decoding examples C.2.*
//
//----------------------------------------------------------------------------
static inline int
   example_C2( void )               // Test RFC7541: Examples C.2.*
{
   int        error_count= 0;

   // Input/output objects
   Ioda       writer;
   IodaReader reader(writer);

   // Test objects
   Properties out_prop;
   Pack       inp_pack;
   Pack       out_pack;

   //=========================================================================
   // C.2.1 Literal field with indexing
   writer.reset(); reader.reset();
   out_prop.reset(); inp_pack.reset(); out_pack.reset();

   if( false ) {                    // (For error path test)
     out_prop.append("**FORCED**", "**ERROR**", ET_INDEX, false, false);
     out_pack.encode(writer, out_prop);
   }
   out_prop.append("custom-key", "custom_header", ET_INDEX, false, false);
   if( prop_verify("C.2.1", reader, writer, inp_pack, out_pack, out_prop) )
     return 1;

   //=========================================================================
   // C.2.2 Literal field without (sic) indexing
   //   Indexed name, literal value.
   //   The indexed name is not added to the dynamic table.
   writer.reset(); reader.reset();
   out_prop.reset(); inp_pack.reset(); out_pack.reset();

   out_prop.append(":path", "/sample/path", ET_CONST, false, false);
   if( prop_verify("C.2.2", reader, writer, inp_pack, out_pack, out_prop) )
     return 1;

   //=========================================================================
   // C.2.3 Literal header field never indexed
   writer.reset(); reader.reset();
   out_prop.reset(); inp_pack.reset(); out_pack.reset();

   out_prop.append(":password", "secret", ET_NEVER_NOINDEX, false, false);
   if( prop_verify("C.2.3", reader, writer, inp_pack, out_pack, out_prop) )
     return 1;

   //=========================================================================
   // C.2.4 Indexed header field
   writer.reset(); reader.reset();
   out_prop.reset(); inp_pack.reset(); out_pack.reset();

   out_prop.append(":method", "GET", ET_INDEX, false, false);
   // Added to test
   out_prop.append(":method", "GET", ET_INSERT, false, false);
   out_prop.append(":method", "GET", ET_NEVER, false, false);
   out_prop.append(":method", "GET", ET_CONST, false, false);
   if( prop_verify("C.2.4", reader, writer, inp_pack, out_pack, out_prop) )
     return 1;

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       example_C3
//
// Purpose-
//       Test RFC7541: HPACK encoding/decoding examples C.3.*
//
//----------------------------------------------------------------------------
static inline int
   example_C3( void )               // Test RFC7541: Examples C.3.*
{
   int        error_count= 0;

   // Input/output objects
   Ioda       writer;
   IodaReader reader(writer);

   // Test objects
   Properties out_prop;
   Pack       inp_pack;
   Pack       out_pack;

   //=========================================================================
   // C.3.1 First Request
   writer.reset(); reader.reset();
   out_prop.reset(); inp_pack.reset(); out_pack.reset();

   out_prop.append(":method", "GET", ET_INDEX, false, false);
   out_prop.append(":scheme", "http", ET_INDEX, false, false);
   out_prop.append(":path",   "/",    ET_INDEX, false, false);
   out_prop.append(":authority", "www.example.com", ET_INDEX, false, false);
   if( prop_verify("C.3.1", reader, writer, inp_pack, out_pack, out_prop) )
     return 1;

   //=========================================================================
   // C.3.2 Second Request (reusing out_pack and inp_pack)
   writer.reset(); reader.reset();
   out_prop.reset(); // inp_pack.reset(); out_pack.reset();

   out_prop.append(":method", "GET", ET_INDEX, false, false);
   out_prop.append(":scheme", "http", ET_INDEX, false, false);
   out_prop.append(":path",   "/", ET_INDEX, false, false);
   out_prop.append(":authority", "www.example.com", ET_INDEX, false, false);
   out_prop.append("cache-control", "no-cache", ET_INDEX, false, false);
   if( prop_verify("C.3.2", reader, writer, inp_pack, out_pack, out_prop) )
     return 1;

   //=========================================================================
   // C.3.3 Third Request (reusing out_pack and inp_pack)
   writer.reset(); reader.reset();
   out_prop.reset(); // inp_pack.reset(); out_pack.reset();

   out_prop.append(":method", "GET", ET_INDEX, false, false);
   out_prop.append(":scheme", "https", ET_INDEX, false, false);
   out_prop.append(":path",   "/index.html", ET_INDEX, false, false);
   out_prop.append(":authority", "www.example.com", ET_INDEX, false, false);
   out_prop.append("custom-key", "custom-value", ET_INDEX, false, false);
   if( prop_verify("C.3.3", reader, writer, inp_pack, out_pack, out_prop) )
     return 1;

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       example_C4
//
// Purpose-
//       Test RFC7541: HPACK encoding/decoding examples C.4.*
//       (Same as C.3.* with Huffman encoding)
//
//----------------------------------------------------------------------------
static inline int
   example_C4( void )               // Test RFC7541: Examples C.4.*
{
   int        error_count= 0;

   // Input/output objects
   Ioda       writer;
   IodaReader reader(writer);

   // Test objects
   Properties out_prop;
   Pack       inp_pack;
   Pack       out_pack;

   //=========================================================================
   // C.4.1 First Request
   writer.reset(); reader.reset();
   out_prop.reset(); inp_pack.reset(); out_pack.reset();

   out_prop.append(":method", "GET", ET_INDEX, true, true);
   out_prop.append(":scheme", "http", ET_INDEX, true, true);
   out_prop.append(":path",   "/", ET_INDEX, true, true);
   out_prop.append(":authority", "www.example.com", ET_INDEX, true, true);
   if( prop_verify("C.4.1", reader, writer, inp_pack, out_pack, out_prop) )
     return 1;

   //=========================================================================
   // C.4.2 Second Request (reusing out_pack and inp_pack)
   writer.reset(); reader.reset();
   out_prop.reset(); // inp_pack.reset(); out_pack.reset();

   out_prop.append(":method", "GET", ET_INDEX, true, true);
   out_prop.append(":scheme", "http", ET_INDEX, true, true);
   out_prop.append(":path",   "/", ET_INDEX, true, true);
   out_prop.append(":authority", "www.example.com", ET_INDEX, true, true);
   out_prop.append("cache-control", "no-cache", ET_INDEX, true, true);
   if( prop_verify("C.4.2", reader, writer, inp_pack, out_pack, out_prop) )
     return 1;

   //=========================================================================
   // C.4.3 Third Request (reusing out_pack and inp_pack)
   writer.reset(); reader.reset();
   out_prop.reset(); // inp_pack.reset(); out_pack.reset();

   out_prop.append(":method", "GET", ET_INDEX, true, true);
   out_prop.append(":scheme", "https", ET_INDEX, true, true);
   out_prop.append(":path",   "/index.html", ET_INDEX, true, true);
   out_prop.append(":authority", "www.example.com", ET_INDEX, true, true);
   out_prop.append("custom-key", "custom-value", ET_INDEX, true, true);
   if( prop_verify("C.4.3", reader, writer, inp_pack, out_pack, out_prop) )
     return 1;

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       example_C5
//
// Purpose-
//       Test RFC7541: HPACK encoding/decoding examples C.5.*
//       (Response examples without Huffman encoding)
//
//----------------------------------------------------------------------------
static inline int
   example_C5( void )               // Test RFC7541: Examples C.5.*
{
   int        error_count= 0;

   // Input/output objects
   Ioda       writer;
   IodaReader reader(writer);

   // Test objects
   Properties out_prop;
   Pack       inp_pack;
   Pack       out_pack;

   //=========================================================================
   // C.5.0 Resize
   inp_pack.resize(256);            // TODO: Send resize request
   out_pack.resize(256);

   if( opt_verbose ) {
     inp_pack.verbose= 1;
     out_pack.verbose= 1;
   }

   //=========================================================================
   // C.5.1 First Response
   writer.reset(); reader.reset();
   out_prop.reset(); // inp_pack.reset(); out_pack.reset();

   out_prop.append(":status", "302", ET_INDEX, false, false);
   out_prop.append("cache-control", "private", ET_INDEX, false, false);
   out_prop.append("date",   "Mon, 21 Oct 2014 20:13:21 GMT", ET_INDEX, false, false);
   out_prop.append("location", "https://www.example.com", ET_INDEX, false, false);
   if( prop_verify("C.5.1", reader, writer, inp_pack, out_pack, out_prop) )
     return 1;

   //=========================================================================
   // C.5.2 Second Response (reusing out_pack and inp_pack)
   //   The (:status,302) entry is evicted to make space for (:status,307)
   writer.reset(); reader.reset();
   out_prop.reset(); // inp_pack.reset(); out_pack.reset();

   out_prop.append(":status", "307", ET_INDEX);
   out_prop.append("cache-control", "private", ET_INDEX, false, false);
   out_prop.append("date",   "Mon, 21 Oct 2014 20:13:21 GMT", ET_INDEX, false, false);
   out_prop.append("location", "https://www.example.com", ET_INDEX, false, false);
   if( prop_verify("C.5.2", reader, writer, inp_pack, out_pack, out_prop) )
     return 1;

   //=========================================================================
   // C.5.3 Third Response (reusing out_pack and inp_pack)
   //   Several header fields are evicted
   writer.reset(); reader.reset();
   out_prop.reset(); // inp_pack.reset(); out_pack.reset();

   out_prop.append(":status", "200", ET_INDEX, false, false);
   out_prop.append("cache-control", "private", ET_INDEX, false, false);
   out_prop.append("date",   "Mon, 21 Oct 2014 20:13:22 GMT", ET_INDEX, false, false);
   out_prop.append("location", "https://www.example.com", ET_INDEX, false, false);
   out_prop.append("content-encoding", "gzip", ET_INDEX, false, false);
   out_prop.append("set-cookie", "foo=ASDJKHQKBZXOQWEOPIUAXQWEOIU; "
                                 "max-age=3600; version=1"
                               , ET_INDEX, false, false);
   if( prop_verify("C.5.3", reader, writer, inp_pack, out_pack, out_prop) )
     return 1;

   inp_pack.verbose= 0;
   out_pack.verbose= 0;

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       example_C6
//
// Purpose-
//       Test RFC7541: HPACK encoding/decoding examples C.6.*
//       (Response examples using Huffman encoding)
//       Since evictions are based on the decoded lengths, the same evictions
//       occur as in example_C5.
//
//----------------------------------------------------------------------------
static inline int
   example_C6( void )               // Test RFC7541: Examples C.6.*
{
   int        error_count= 0;

   // Input/output objects
   Ioda       writer;
   IodaReader reader(writer);

   // Test objects
   Properties out_prop;
   Pack       inp_pack;
   Pack       out_pack;

   //=========================================================================
   // C.6.0 Resize
   inp_pack.resize(256);            // TODO: Send resize request
   out_pack.resize(256);

   if( opt_verbose ) {
     inp_pack.verbose= 1;
     out_pack.verbose= 1;
   }

   //=========================================================================
   // C.6.1 First Response
   writer.reset(); reader.reset();
   out_prop.reset(); // inp_pack.reset(); out_pack.reset();

   out_prop.append(":status", "302", ET_INDEX, true, true);
   out_prop.append("cache-control", "private", ET_INDEX, true, true);
   out_prop.append("date", "Mon, 21 Oct 2014 20:13:21 GMT", ET_INDEX
                  , true, true);
   out_prop.append("location", "https://www.example.com", ET_INDEX
                  , true, true);
   if( prop_verify("C.6.1", reader, writer, inp_pack, out_pack, out_prop) )
     return 1;

   //=========================================================================
   // C.6.2 Second Response (reusing out_pack and inp_pack)
   //   The (:status,302) entry is evicted to make space for (:status,307)
   writer.reset(); reader.reset();
   out_prop.reset(); // inp_pack.reset(); out_pack.reset();

   out_prop.append(":status", "307", ET_INDEX, true, true);
   out_prop.append("cache-control", "private", ET_INDEX, true, true);
   out_prop.append("date", "Mon, 21 Oct 2014 20:13:21 GMT", ET_INDEX
                  , true, true);
   out_prop.append("location", "https://www.example.com", ET_INDEX
                  , true, true);
   if( prop_verify("C.6.2", reader, writer, inp_pack, out_pack, out_prop) )
     return 1;

   //=========================================================================
   // C.6.3 Third Response (reusing out_pack and inp_pack)
   //   Several header fields are evicted
   writer.reset(); reader.reset();
   out_prop.reset(); // inp_pack.reset(); out_pack.reset();

   out_prop.append(":status", "200", ET_INDEX, true, true);
   out_prop.append("cache-control", "private", ET_INDEX, true, true);
   out_prop.append("date",   "Mon, 21 Oct 2014 20:13:22 GMT", ET_INDEX
                  , true, true);
   out_prop.append("location", "https://www.example.com", ET_INDEX
                  , true, true);
   out_prop.append("content-encoding", "gzip", ET_INDEX, true, true);
   out_prop.append("set-cookie", "foo=ASDJKHQKBZXOQWEOPIUAXQWEOIU; "
                                 "max-age=3600; version=1"
                  , ET_INDEX, true, true);
   if( prop_verify("C.6.3", reader, writer, inp_pack, out_pack, out_prop) )
     return 1;

   inp_pack.verbose= 0;
   out_pack.verbose= 0;

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       exam_7541
//
// Purpose-
//       Test RFC7541: HPACK encoding/decoding examples
//
//----------------------------------------------------------------------------
static inline int
   exam_7541( void )                // Test RFC7541: Examples
{
   if( opt_verbose )
     debugf("\ntest_examples:\n");
   int error_count= 0;

   error_count += example_C1();     // C.1.1, C.1.2, C.1.3
   error_count += example_C2();     // C.2.1, C.2.2, C.2.3, C.2.4
   error_count += example_C3();     // C.3.1, C.3.2, C.3.3
   error_count += example_C4();     // C.4.1, C.4.2, C.4.3
   error_count += example_C5();     // C.5.1, C.5.2, C.5.3
   error_count += example_C6();     // C.6.1, C.6.2, C.6.3

   if( opt_verbose  )
     debugf("\n\n--------------------------------------------------------\n");
   return error_count;
}
