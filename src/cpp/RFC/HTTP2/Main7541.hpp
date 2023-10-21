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
//       2023/10/19
//
// Implementation notes-
//       Included by Main.cpp
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Subroutine-
//       str_expand
//
// Purpose-
//       Expand string to equivalent character array.
//
//----------------------------------------------------------------------------
static inline string                // The expanded text string
   str_expand(                      // Get expanded text string
     const string      S)           // For this text string
{
   string out;
   for(size_t i= 0; i<S.size(); ++i) {
     out += (i == 0) ? "{" : ", ";
     char buffer[8];
     sprintf(buffer, "0x%.2x", S[i] & 0x00ff);
     out += buffer;
   }
   out += "}";

   return out;
}

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
     int               bits,        // Number of encoding bits
     const string&     verify)      // Verification string
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

   if( (string)writer != verify ) { // If encoding error
     ++error_count;
     debugf("%s Error: invalid encoding\n", id);
     debugf("..Expected: %s\n", str_expand(verify).c_str());
     debugf("..  Actual: %s\n", str_expand((string)writer).c_str());
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
   const char txt_C_11[]= {0xEA};
   string C_11(txt_C_11, 1);
   error_count += intx_verify("C.1.1", 10, 5, C_11);

   // C.1.2 Example 2: Encoding 1337 using a 5-bit prefix
   const char txt_C_12[]= {0xFF, 0x9A, 0x0A};
   string C_12(txt_C_12, 3);
   error_count += intx_verify("C.1.2", 1337, 5, C_12);

   // C.1.2 Example 3: Encoding 42
   const char txt_C_13[]= {0x2A};
   string C_13(txt_C_13, 1);
   error_count += intx_verify("C.1.3", 42, 0, C_13);

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
     Ioda&             writer,      // Associated Writer
     Pack&             inp_pack,    // Input HPACK decoder
     Pack&             out_pack,    // Output HPACK encoder
     const Properties& out_prop,    // Output Properties
     const string&     verify)      // Verification string
{
   // Hard Core Debug Mode (if needed)
   const char* oops= "C.X.Y";       // (If needed, use an actual test id)
   if( strcmp(id, oops) == 0 ) {
     debugf("\n\n%s initial state -----------------------------------\n", id);
     out_pack.debug(oops);
     debugf("\n");
     inp_pack.debug(oops);
     debugf("\n");
   }

   if( opt_verbose )
     debugf("\n\n%s encode-------------------------------------------\n", id);
   out_pack.encode(writer, out_prop);
   if( opt_verbose ) {
     out_pack.debug("out_pack encoded");
     writer.dump("writer encoded");
   }

   if( (string)writer != verify ) { // If encoding error
     ++error_count;
     debugf("%s Error: invalid encoding\n", id);
     debugf("..Expected: %s\n", str_expand(verify).c_str());
     debugf("..  Actual: %s\n", str_expand((string)writer).c_str());
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

   error_count += VERIFY( out_pack == inp_pack );
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
   out_prop.reset(); inp_pack.reset(256); out_pack.reset(256);

   if( false ) {                    // (For error path test)
     out_prop.append("**FORCED**", "**ERROR**", ET_INDEX, false, false);
     out_pack.encode(writer, out_prop);
   }
   out_prop.append("custom-key", "custom-header", ET_INDEX, false, false);

   const char txt_C_21[]=
       {0x40
       ,0x0A ,0x63,0x75,0x73,0x74,0x6F,0x6D,0x2D,0x6B,0x65,0x79
       ,0x0D ,0x63,0x75,0x73,0x74,0x6F,0x6D,0x2D,0x68
             ,0x65,0x61,0x64,0x65,0x72};
   string C_21(txt_C_21, 26);
   if( prop_verify("C.2.1",reader,writer,inp_pack,out_pack,out_prop,C_21) )
     return 1;

   error_count += VERIFY( out_pack.get_encode_used() == 55 );
   const Entry* entry= out_pack.get_entry(DYNAMIC_ENTRY_0);
   error_count += VERIFY( strcmp(entry->name,  "custom-key") == 0 );
   error_count += VERIFY( strcmp(entry->value, "custom-header") == 0 );

   //=========================================================================
   // C.2.2 Literal field without (sic) indexing
   //   Indexed name, literal value.
   //   The indexed name is not added to the dynamic table.
   writer.reset(); reader.reset();
   out_prop.reset(); inp_pack.reset(256); out_pack.reset(256);

   out_prop.append(":path", "/sample/path", ET_CONST, false, false);

   const char txt_C_22[]=
       {0x04                        // static_entry[4]
       ,0x0C, 0x2F,0x73,0x61,0x6D,0x70,0x6C,0x65,0x2F,0x70,0x61,0x74,0x68};
   string C_22(txt_C_22, 14);
   if( prop_verify("C.2.2",reader,writer,inp_pack,out_pack,out_prop,C_22) )
     return 1;

   error_count += VERIFY( out_pack.get_encode_used() == 0 );

   //=========================================================================
   // C.2.3 Literal header field never indexed
   writer.reset(); reader.reset();
   out_prop.reset(); inp_pack.reset(256); out_pack.reset(256);

   out_prop.append("password", "secret", ET_NEVER_NOINDEX, false, false);

   const char txt_C_23[]=
       {0x10
       ,0x08, 0x70,0x61,0x73,0x73,0x77,0x6F,0x72,0x64
       ,0x06, 0x73,0x65,0x63,0x72,0x65,0x74};
   string C_23(txt_C_23, 17);
   if( prop_verify("C.2.3",reader,writer,inp_pack,out_pack,out_prop,C_23) )
     return 1;

   error_count += VERIFY( out_pack.get_encode_used() == 0 );

   //=========================================================================
   // C.2.4 Indexed header field
   writer.reset(); reader.reset();
   out_prop.reset(); inp_pack.reset(256); out_pack.reset(256);

   out_prop.append(":method", "GET", ET_INDEX, false, false);

   if( true ) {                     // (As documented)
     const char txt_C_24[]=
         {0x82};
     string C_24(txt_C_24, 1);
     if( prop_verify("C.2.4",reader,writer,inp_pack,out_pack,out_prop,C_24) )
       return 1;

     error_count += VERIFY( out_pack.get_encode_used() == 0 );
   } else {                         // (Added to test)
     out_prop.append(":method", "GET", ET_INSERT, false, false); // Added
     out_prop.append(":method", "GET", ET_NEVER, false, false);  // Added
     out_prop.append(":method", "GET", ET_CONST, false, false);  // Added

     const char txt_C_24[]=
         {0x82
         ,0x42, 0x03,0x47,0x45,0x54   // Added, indexed
         ,0x12, 0x03,0x47,0x45,0x54   // Added, never indexed
         ,0x02, 0x03,0x47,0x45,0x54   // Added, indexing allowed
         };
     string C_24(txt_C_24, 16);
     if( prop_verify("C.2.4",reader,writer,inp_pack,out_pack,out_prop,C_24) )
       return 1;

     error_count += VERIFY( out_pack.get_encode_used() == 42 );
     entry= out_pack.get_entry(DYNAMIC_ENTRY_0);
     error_count += VERIFY( strcmp(entry->name,  ":method") == 0 );
     error_count += VERIFY( strcmp(entry->value, "GET") == 0 );
   }

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
   out_prop.reset(); inp_pack.reset(256); out_pack.reset(256);

   out_prop.append(":method", "GET", ET_INDEX, false, false);
   out_prop.append(":scheme", "http", ET_INDEX, false, false);
   out_prop.append(":path",   "/",    ET_INDEX, false, false);
   out_prop.append(":authority", "www.example.com", ET_INDEX, false, false);

   const char txt_C_31[]=
       {0x82
       ,0x86
       ,0x84
       ,0x41
       ,0x0F, 0x77,0x77,0x77,0x2E,0x65,0x78,0x61,0x6D
            , 0x70,0x6C,0x65,0x2E,0x63,0x6F,0x6D};
   string C_31(txt_C_31, 20);
   if( prop_verify("C.3.1",reader,writer,inp_pack,out_pack,out_prop,C_31) )
     return 1;

   error_count += VERIFY( out_pack.get_encode_used() == 57 );
   const Entry* entry= out_pack.get_entry(DYNAMIC_ENTRY_0);
   error_count += VERIFY( strcmp(entry->name,  ":authority") == 0 );
   error_count += VERIFY( strcmp(entry->value, "www.example.com") == 0 );

   //=========================================================================
   // C.3.2 Second Request (reusing out_pack and inp_pack)
   writer.reset(); reader.reset();
   out_prop.reset();

   out_prop.append(":method", "GET", ET_INDEX, false, false);
   out_prop.append(":scheme", "http", ET_INDEX, false, false);
   out_prop.append(":path",   "/", ET_INDEX, false, false);
   out_prop.append(":authority", "www.example.com", ET_INDEX, false, false);
   out_prop.append("cache-control", "no-cache", ET_INDEX, false, false);

   const char txt_C_32[]=
       {0x82
       ,0x86
       ,0x84
       ,0xBE
       ,0x58
       ,0x08, 0x6E,0x6F,0x2D,0x63,0x61,0x63,0x68,0x65};
   string C_32(txt_C_32, 14);
   if( prop_verify("C.3.2",reader,writer,inp_pack,out_pack,out_prop,C_32) )
     return 1;

   error_count += VERIFY( out_pack.get_encode_used() == 110 );
   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 0);
   error_count += VERIFY( strcmp(entry->name,  "cache-control") == 0 );
   error_count += VERIFY( strcmp(entry->value, "no-cache") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 1);
   error_count += VERIFY( strcmp(entry->name,  ":authority") == 0 );
   error_count += VERIFY( strcmp(entry->value, "www.example.com") == 0 );

   //=========================================================================
   // C.3.3 Third Request (reusing out_pack and inp_pack)
   writer.reset(); reader.reset();
   out_prop.reset();

   out_prop.append(":method", "GET", ET_INDEX, false, false);
   out_prop.append(":scheme", "https", ET_INDEX, false, false);
   out_prop.append(":path",   "/index.html", ET_INDEX, false, false);
   out_prop.append(":authority", "www.example.com", ET_INDEX, false, false);
   out_prop.append("custom-key", "custom-value", ET_INDEX, false, false);

   const char txt_C_33[]=
       {0x82
       ,0x87
       ,0x85
       ,0xBF
       ,0x40
       ,0x0A, 0x63,0x75,0x73,0x74,0x6F,0x6D,0x2D,0x6B,0x65,0x79
       ,0x0C, 0x63,0x75,0x73,0x74,0x6F,0x6D,0x2D,0x76,0x61,0x6C,0x75,0x65};
   string C_33(txt_C_33, 29);
   if( prop_verify("C.3.3",reader,writer,inp_pack,out_pack,out_prop,C_33) )
     return 1;

   error_count += VERIFY( out_pack.get_encode_used() == 164 );
   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 0);
   error_count += VERIFY( strcmp(entry->name,  "custom-key") == 0 );
   error_count += VERIFY( strcmp(entry->value, "custom-value") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 1);
   error_count += VERIFY( strcmp(entry->name,  "cache-control") == 0 );
   error_count += VERIFY( strcmp(entry->value, "no-cache") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 2);
   error_count += VERIFY( strcmp(entry->name,  ":authority") == 0 );
   error_count += VERIFY( strcmp(entry->value, "www.example.com") == 0 );

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
   out_prop.reset(); inp_pack.reset(256); out_pack.reset(256);

   out_prop.append(":method", "GET", ET_INDEX, true, true);
   out_prop.append(":scheme", "http", ET_INDEX, true, true);
   out_prop.append(":path",   "/", ET_INDEX, true, true);
   out_prop.append(":authority", "www.example.com", ET_INDEX, true, true);

   const char txt_C_41[]=
       {0x82
       ,0x86
       ,0x84
       ,0x41
       ,0x8C, 0xF1,0xE3,0xC2,0xE5,0xF2,0x3A,0x6B,0xA0,0xAB,0x90,0xF4,0xFF};
   string C_41(txt_C_41, 17);
   if( prop_verify("C.4.1",reader,writer,inp_pack,out_pack,out_prop,C_41) )
     return 1;

   error_count += VERIFY( out_pack.get_encode_used() == 57 );
   const Entry* entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 0);
   error_count += VERIFY( strcmp(entry->name,  ":authority") == 0 );
   error_count += VERIFY( strcmp(entry->value, "www.example.com") == 0 );

   //=========================================================================
   // C.4.2 Second Request (reusing out_pack and inp_pack)
   writer.reset(); reader.reset();
   out_prop.reset(); // inp_pack.reset(256); out_pack.reset(256);

   out_prop.append(":method", "GET", ET_INDEX, true, true);
   out_prop.append(":scheme", "http", ET_INDEX, true, true);
   out_prop.append(":path",   "/", ET_INDEX, true, true);
   out_prop.append(":authority", "www.example.com", ET_INDEX, true, true);
   out_prop.append("cache-control", "no-cache", ET_INDEX, true, true);

   const char txt_C_42[]=
       {0x82
       ,0x86
       ,0x84
       ,0xBE
       ,0x58
       ,0x86, 0xA8,0xEB,0x10,0x64,0x9C,0xBF};
   string C_42(txt_C_42, 12);
   if( prop_verify("C.4.2",reader,writer,inp_pack,out_pack,out_prop,C_42) )
     return 1;

   error_count += VERIFY( out_pack.get_encode_used() == 110 );
   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 0);
   error_count += VERIFY( strcmp(entry->name,  "cache-control") == 0 );
   error_count += VERIFY( strcmp(entry->value, "no-cache") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 1);
   error_count += VERIFY( strcmp(entry->name,  ":authority") == 0 );
   error_count += VERIFY( strcmp(entry->value, "www.example.com") == 0 );

   //=========================================================================
   // C.4.3 Third Request (reusing out_pack and inp_pack)
   writer.reset(); reader.reset();
   out_prop.reset(); // inp_pack.reset(256); out_pack.reset(256);

   out_prop.append(":method", "GET", ET_INDEX, true, true);
   out_prop.append(":scheme", "https", ET_INDEX, true, true);
   out_prop.append(":path",   "/index.html", ET_INDEX, true, true);
   out_prop.append(":authority", "www.example.com", ET_INDEX, true, true);
   out_prop.append("custom-key", "custom-value", ET_INDEX, true, true);

   const char txt_C_43[]=
       {0x82
       ,0x87
       ,0x85
       ,0xBF
       ,0x40
       ,0x88, 0x25,0xA8,0x49,0xE9,0x5B,0xA9,0x7D,0x7F
       ,0x89, 0x25,0xA8,0x49,0xE9,0x5B,0xB8,0xE8,0xB4,0xBF};
   string C_43(txt_C_43, 24);
   if( prop_verify("C.4.3",reader,writer,inp_pack,out_pack,out_prop,C_43) )
     return 1;

   error_count += VERIFY( out_pack.get_encode_used() == 164 );
   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 0);
   error_count += VERIFY( strcmp(entry->name,  "custom-key") == 0 );
   error_count += VERIFY( strcmp(entry->value, "custom-value") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 1);
   error_count += VERIFY( strcmp(entry->name,  "cache-control") == 0 );
   error_count += VERIFY( strcmp(entry->value, "no-cache") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 2);
   error_count += VERIFY( strcmp(entry->name,  ":authority") == 0 );
   error_count += VERIFY( strcmp(entry->value, "www.example.com") == 0 );

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
   Pack       inp_pack(512);
   Pack       out_pack(512);

   if( opt_verbose ) {
     inp_pack.verbose= opt_verbose;
     out_pack.verbose= opt_verbose;
   }

   //=========================================================================
   // C.5.1 First Response
   out_prop.reset();

   if( opt_verbose )
     debugf("\n\nC.5.1 encode resize-------------------------------------\n");
   out_pack.resize(writer, 256);    // (Added to test)

   out_prop.append(":status", "302", ET_INDEX, false, false);
   out_prop.append("cache-control", "private", ET_INDEX, false, false);
   out_prop.append("date",   "Mon, 21 Oct 2013 20:13:21 GMT", ET_INDEX, false, false);
   out_prop.append("location", "https://www.example.com", ET_INDEX, false, false);

   const char txt_C_51[]=
       {0x3F,0xE1,0x01              // (ET_RESIZE: 256)
       ,0x48
       ,0x03, 0x33,0x30,0x32
       ,0x58
       ,0x07, 0x70,0x72,0x69,0x76,0x61,0x74,0x65
       ,0x61
       ,0x1D, 0x4D,0x6F,0x6E,0x2C,0x20,0x32,0x31,0x20
            , 0x4F,0x63,0x74,0x20,0x32,0x30,0x31,0x33
            , 0x20,0x32,0x30,0x3A,0x31,0x33,0x3A,0x32
            , 0x31,0x20,0x47,0x4D,0x54
       ,0x6E
       ,0x17, 0x68,0x74,0x74,0x70,0x73,0x3A,0x2F,0x2F
            , 0x77,0x77,0x77,0x2E,0x65,0x78,0x61,0x6D
            , 0x70,0x6C,0x65,0x2E,0x63,0x6F,0x6D};
   string C_51(txt_C_51, 73);
   if( prop_verify("C.5.1",reader,writer,inp_pack,out_pack,out_prop,C_51) )
     return 1;

   error_count += VERIFY( out_pack.get_encode_used() == 222 );
   const Entry* entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 0);
   error_count += VERIFY( strcmp(entry->name,  "location") == 0 );
   error_count += VERIFY( strcmp(entry->value, "https://www.example.com") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 1);
   error_count += VERIFY( strcmp(entry->name,  "date") == 0 );
   error_count += VERIFY( strcmp(entry->value, "Mon, 21 Oct 2013 20:13:21 GMT") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 2);
   error_count += VERIFY( strcmp(entry->name,  "cache-control") == 0 );
   error_count += VERIFY( strcmp(entry->value, "private") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 3);
   error_count += VERIFY( strcmp(entry->name,  ":status") == 0 );
   error_count += VERIFY( strcmp(entry->value, "302") == 0 );

   //=========================================================================
   // C.5.2 Second Response (reusing out_pack and inp_pack)
   //   The (:status,302) entry is evicted to make space for (:status,307)
   writer.reset(); reader.reset();
   out_prop.reset();

   out_prop.append(":status", "307", ET_INDEX);
   out_prop.append("cache-control", "private", ET_INDEX, false, false);
   out_prop.append("date",   "Mon, 21 Oct 2013 20:13:21 GMT", ET_INDEX, false, false);
   out_prop.append("location", "https://www.example.com", ET_INDEX, false, false);

   const char txt_C_52[]=
       {0x48
       ,0x03,0x33,0x30,0x37
       ,0xC1
       ,0xC0
       ,0xBF};
   string C_52(txt_C_52, 8);
   if( prop_verify("C.5.2",reader,writer,inp_pack,out_pack,out_prop,C_52) )
     return 1;

   error_count += VERIFY( out_pack.get_encode_used() == 222 );
   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 0);
   error_count += VERIFY( strcmp(entry->name,  ":status") == 0 );
   error_count += VERIFY( strcmp(entry->value, "307") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 1);
   error_count += VERIFY( strcmp(entry->name,  "location") == 0 );
   error_count += VERIFY( strcmp(entry->value, "https://www.example.com") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 2);
   error_count += VERIFY( strcmp(entry->name,  "date") == 0 );
   error_count += VERIFY( strcmp(entry->value, "Mon, 21 Oct 2013 20:13:21 GMT") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 3);
   error_count += VERIFY( strcmp(entry->name,  "cache-control") == 0 );
   error_count += VERIFY( strcmp(entry->value, "private") == 0 );

   //=========================================================================
   // C.5.3 Third Response (reusing out_pack and inp_pack)
   //   Several header fields are evicted
   writer.reset(); reader.reset();
   out_prop.reset();

   out_prop.append(":status", "200", ET_INDEX, false, false);
   out_prop.append("cache-control", "private", ET_INDEX, false, false);
   out_prop.append("date",   "Mon, 21 Oct 2013 20:13:22 GMT", ET_INDEX, false, false);
   out_prop.append("location", "https://www.example.com", ET_INDEX, false, false);
   out_prop.append("content-encoding", "gzip", ET_INDEX, false, false);
   out_prop.append("set-cookie", "foo=ASDJKHQKBZXOQWEOPIUAXQWEOIU; "
                                 "max-age=3600; version=1"
                               , ET_INDEX, false, false);

   const char txt_C_53[]=
       {0x88
       ,0xC1
       ,0x61
       ,0x1D, 0x4D,0x6F,0x6E,0x2C,0x20,0x32,0x31,0x20
            , 0x4F,0x63,0x74,0x20,0x32,0x30,0x31,0x33
            , 0x20,0x32,0x30,0x3A,0x31,0x33,0x3A,0x32
            , 0x32,0x20,0x47,0x4D,0x54
       ,0xC0
       ,0x5A
       ,0x04, 0x67,0x7A,0x69,0x70
       ,0x77
       ,0x38, 0x66,0x6F,0x6F,0x3D,0x41,0x53,0x44,0x4A
            , 0x4B,0x48,0x51,0x4B,0x42,0x5A,0x58,0x4F
            , 0x51,0x57,0x45,0x4F,0x50,0x49,0x55,0x41
            , 0x58,0x51,0x57,0x45,0x4F,0x49,0x55,0x3B
            , 0x20,0x6D,0x61,0x78,0x2D,0x61,0x67,0x65
            , 0x3D,0x33,0x36,0x30,0x30,0x3B,0x20,0x76
            , 0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x31};
   string C_53(txt_C_53, 98);
   if( prop_verify("C.5.3",reader,writer,inp_pack,out_pack,out_prop,C_53) )
     return 1;

   const char* comparand= nullptr;
   error_count += VERIFY( out_pack.get_encode_used() == 215 );
   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 0);
   error_count += VERIFY( strcmp(entry->name,  "set-cookie") == 0 );
   comparand= "foo=ASDJKHQKBZXOQWEOPIUAXQWEOIU; "
              "max-age=3600; version=1";
   error_count += VERIFY( strcmp(entry->value, comparand) == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 1);
   error_count += VERIFY( strcmp(entry->name,  "content-encoding") == 0 );
   error_count += VERIFY( strcmp(entry->value, "gzip") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 2);
   error_count += VERIFY( strcmp(entry->name,  "date") == 0 );
   error_count += VERIFY( strcmp(entry->value, "Mon, 21 Oct 2013 20:13:22 GMT") == 0 );

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

   if( opt_verbose ) {
     inp_pack.verbose= opt_verbose;
     out_pack.verbose= opt_verbose;
   }

   //=========================================================================
   // C.6.1 First Response
   out_prop.reset();

   if( opt_verbose )
     debugf("\n\nC.6.1 encode resize-------------------------------------\n");
   out_pack.resize(writer, 256);    // (Added to test)

   out_prop.append(":status", "302", ET_INDEX, true, true);
   out_prop.append("cache-control", "private", ET_INDEX, true, true);
   out_prop.append("date", "Mon, 21 Oct 2013 20:13:21 GMT", ET_INDEX
                  , true, true);
   out_prop.append("location", "https://www.example.com", ET_INDEX
                  , true, true);

   const char txt_C_61[]=
       {0x3F,0xE1,0x01              // (ET_RESIZE: 256)
       ,0x48
       ,0x82, 0x64,0x02
       ,0x58
       ,0x85, 0xAE,0xC3,0x77,0x1A,0x4B
       ,0x61
       ,0x96 ,0xD0,0x7A,0xBE,0x94,0x10,0x54,0xD4,0x44
             ,0xA8,0x20,0x05,0x95,0x04,0x0B,0x81,0x66
             ,0xE0,0x82,0xA6,0x2D,0x1B,0xFF
       ,0x6E
       ,0x91, 0x9D,0x29,0xAD,0x17,0x18,0x63,0xC7,0x8F
            , 0x0B,0x97,0xC8,0xE9,0xAE,0x82,0xAE,0x43,0xD3};
   string C_61(txt_C_61, 57);
   if( prop_verify("C.6.1",reader,writer,inp_pack,out_pack,out_prop,C_61) )
     return 1;

   error_count += VERIFY( out_pack.get_encode_used() == 222 );
   const Entry* entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 0);
   error_count += VERIFY( strcmp(entry->name,  "location") == 0 );
   error_count += VERIFY( strcmp(entry->value, "https://www.example.com") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 1);
   error_count += VERIFY( strcmp(entry->name,  "date") == 0 );
   error_count += VERIFY( strcmp(entry->value, "Mon, 21 Oct 2013 20:13:21 GMT") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 2);
   error_count += VERIFY( strcmp(entry->name,  "cache-control") == 0 );
   error_count += VERIFY( strcmp(entry->value, "private") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 3);
   error_count += VERIFY( strcmp(entry->name,  ":status") == 0 );
   error_count += VERIFY( strcmp(entry->value, "302") == 0 );

   //=========================================================================
   // C.6.2 Second Response (reusing out_pack and inp_pack)
   //   The (:status,302) entry is evicted to make space for (:status,307)
   writer.reset(); reader.reset();
   out_prop.reset();

   out_prop.append(":status", "307", ET_INDEX, true, true);
   out_prop.append("cache-control", "private", ET_INDEX, true, true);
   out_prop.append("date", "Mon, 21 Oct 2013 20:13:21 GMT", ET_INDEX
                  , true, true);
   out_prop.append("location", "https://www.example.com", ET_INDEX
                  , true, true);

   const char txt_C_62[]=
       {0x48
       ,0x83, 0x64,0x0E,0xFF
       ,0xC1
       ,0xC0
       ,0xBF};
   string C_62(txt_C_62, 8);
   if( prop_verify("C.6.2",reader,writer,inp_pack,out_pack,out_prop,C_62) )
     return 1;

   error_count += VERIFY( out_pack.get_encode_used() == 222 );
   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 0);
   error_count += VERIFY( strcmp(entry->name,  ":status") == 0 );
   error_count += VERIFY( strcmp(entry->value, "307") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 1);
   error_count += VERIFY( strcmp(entry->name,  "location") == 0 );
   error_count += VERIFY( strcmp(entry->value, "https://www.example.com") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 2);
   error_count += VERIFY( strcmp(entry->name,  "date") == 0 );
   error_count += VERIFY( strcmp(entry->value, "Mon, 21 Oct 2013 20:13:21 GMT") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 3);
   error_count += VERIFY( strcmp(entry->name,  "cache-control") == 0 );
   error_count += VERIFY( strcmp(entry->value, "private") == 0 );

   //=========================================================================
   // C.6.3 Third Response (reusing out_pack and inp_pack)
   //   Several header fields are evicted
   writer.reset(); reader.reset();
   out_prop.reset();

   out_prop.append(":status", "200", ET_INDEX, true, true);
   out_prop.append("cache-control", "private", ET_INDEX, true, true);
   out_prop.append("date",   "Mon, 21 Oct 2013 20:13:22 GMT", ET_INDEX
                  , true, true);
   out_prop.append("location", "https://www.example.com", ET_INDEX
                  , true, true);
   out_prop.append("content-encoding", "gzip", ET_INDEX, true, true);
   out_prop.append("set-cookie", "foo=ASDJKHQKBZXOQWEOPIUAXQWEOIU; "
                                 "max-age=3600; version=1"
                  , ET_INDEX, true, true);

   const char txt_C_63[]=
       {0x88
       ,0xC1
       ,0x61
       ,0x96, 0xD0,0x7A,0xBE,0x94,0x10,0x54,0xD4,0x44
            , 0xA8,0x20,0x05,0x95,0x04,0x0B,0x81,0x66
            , 0xE0,0x84,0xA6,0x2D,0x1B,0xFF
       ,0xC0
       ,0x5A
       ,0x83, 0x9B,0xD9,0xAB
       ,0x77
       ,0xAD, 0x94,0xE7,0x82,0x1D,0xD7,0xF2,0xE6,0xC7
            , 0xB3,0x35,0xDF,0xDF,0xCD,0x5B,0x39,0x60
            , 0xD5,0xAF,0x27,0x08,0x7F,0x36,0x72,0xC1
            , 0xAB,0x27,0x0F,0xB5,0x29,0x1F,0x95,0x87
            , 0x31,0x60,0x65,0xC0,0x03,0xED,0x4E,0xE5
            , 0xB1,0x06,0x3D,0x50,0x07};
   string C_63(txt_C_63, 79);
   if( prop_verify("C.6.3",reader,writer,inp_pack,out_pack,out_prop,C_63) )
     return 1;

   const char* comparand= nullptr;
   error_count += VERIFY( out_pack.get_encode_used() == 215 );
   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 0);
   error_count += VERIFY( strcmp(entry->name,  "set-cookie") == 0 );
   comparand= "foo=ASDJKHQKBZXOQWEOPIUAXQWEOIU; "
              "max-age=3600; version=1";
   error_count += VERIFY( strcmp(entry->value, comparand) == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 1);
   error_count += VERIFY( strcmp(entry->name,  "content-encoding") == 0 );
   error_count += VERIFY( strcmp(entry->value, "gzip") == 0 );

   entry= out_pack.get_entry(DYNAMIC_ENTRY_0 + 2);
   error_count += VERIFY( strcmp(entry->name,  "date") == 0 );
   error_count += VERIFY( strcmp(entry->value, "Mon, 21 Oct 2013 20:13:22 GMT") == 0 );

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
