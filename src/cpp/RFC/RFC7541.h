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
//       RFC7541.h
//
// Purpose-
//       RFC7541, HTTP/2 HPACK compression
//
// Last change date-
//       2023/10/19
//
//----------------------------------------------------------------------------
#ifndef _RFC7541_H_INCLUDED
#define _RFC7541_H_INCLUDED

#include <string>                   // For std::string
#include <vector>                   // For std::vector

#include <cstdint>                  // For size_t, uint8_t, ...
#include <cstdlib>                  // For free, ...

#include <pub/Ioda.h>               // For pub::Ioda, pub::IodaReader
#include <pub/List.h>               // For pub::List

//----------------------------------------------------------------------------
//
// Namespace-
//       RFC7541
//
// Purpose-
//       HTTP/2 HPACK compression.
//
//----------------------------------------------------------------------------
namespace RFC7541 {                 // Class RFC7541, HTTP/2 HPACK compression
//----------------------------------------------------------------------------
// RFC7541::Forward references
//----------------------------------------------------------------------------
struct Property;                    // Property, used by Entry

//----------------------------------------------------------------------------
// RFC7541::Typedefs and enumerations
//----------------------------------------------------------------------------
// These typedefs and enumerations are freely used by RFC7541 methods
typedef uint8_t        octet;       // An 8-bit character
typedef uint32_t       Value_t;     // An unsigned 32-bit value

/*****************************************************************************
Array_ix and Entry_ix values describe different ways of used to access the
Pack::entry_array. Array_ix is a physical index; constant once assigned.
The Array_ix is inverted: Use Pack::entry_size - Array_ix to access the array.

Entry_ix is a logical index, used to access both the pre-defined static_entry
array and the Pack::entry_array. While active, an Entry's Entry_ix increases
as newer Entries are added, but its Array_ix remains constant.

Index_ix is standard index used to index. An Index_ix directly accesses a
Pack::entry_array element.

Even when encoder and decoder implementations differ, the logical Entry_ix
index has the same value when encoded that a decoder will need to create
its matching Property and Entry. Array_ix values are unshared, implementation-
specific values.
*****************************************************************************/
typedef Value_t        Array_ix;    // A Pack::entry_array inverted index
typedef Value_t        Entry_ix;    // The logical Entry index
typedef Value_t        Index_ix;    // A standard (entry_size-Array_ix) index

enum DEFAULTS                       // Implementation defaults
{  DEFAULT_ENCODE_SIZE= 0x0001'0000 // Default encode_size (64K)

,  SPEC_ENTRY_SIZE= 32              // Specification-defined entry overhead
}; // enum DEFAULTS

// Note that applications must consider storage availabilty.
// If a malloc or new operation fails, a std::bad_alloc exception is thrown.
enum LIMITS                         // Implementation limitations
{  HEADER_TABLE_LIMIT= 0x8000'0000  // Maximum SETTINGS_HEADER_TABLE_SIZE
,  HEADER_LIST_LIMIT=  0xFFFF'FFFF  // Maximum SETTINGS_HEADER_LIST_SIZE
// HEADER_ITEM_LIMIT=  0x0000'0000  // Maximum name or value length (TBD)
}; // enum LIMITS

//----------------------------------------------------------------------------
//
// Enum-
//       RFC7541::ENCODE_TYPE
//
// Purpose-
//       HPACK encoding type definitions
//
// Implementation notes-
//       If an ET_INDEX  name isn't in table, type becomes ET_INSERT_NOINDEX
//       If an ET_INDEX  value isn't in table, type becomes ET_INSERT
//       If an ET_INSERT name isn't in table, type becomes ET_INSERT_NOINDEX
//       If an ET_NEVER  name isn't in table, type becomes ET_NEVER_NOINDEX
//       If an ET_CONST  name isn't in table, type becomes ET_CONST_NOINDEX
//
//----------------------------------------------------------------------------
enum ENCODE_TYPE        // bitcode
{  ET_NOT_ALLOWED= 8    // 10000000 // NOT ALLOWED (Not used in type_to tables)
,  ET_INDEX= 0          // 1xxxxxxx // Index only
,  ET_INSERT_NOINDEX    // 01000000 // Literal name and value inserted
,  ET_INSERT            // 01xxxxxx // Indexed name, literal value inserted
,  ET_RESIZE            // 001xxxxx // Dynamic table size change

// ET_NEVER* implies that the name is not indexed in this instance, and
//     *MUST NOT* be indexed by intermediaries. This encoding is intended to
//     "protect header field values that are not to be put at risk by
//     compressing them."
// ET_CONST* implies that the name is not indexed in this instance, but may be
//     indexed by intermediaries.
,  ET_NEVER_NOINDEX     // 00010000 // Literal name, value. Const table
,  ET_NEVER             // 0001xxxx // Indexed name, literal value. Const table
,  ET_CONST_NOINDEX     // 00000000 // Literal name, value. Const table
,  ET_CONST             // 0000xxxx // Indexed name, literal value. Const table
}; // enum RFC7541::ENCODE_TYPE

//----------------------------------------------------------------------------
//
// Class-
//       connection_error
//
// Purpose-
//       Connection error exception
//
//----------------------------------------------------------------------------
class connection_error : public std::runtime_error {
using std::runtime_error::runtime_error;
}; // class connection_error

//----------------------------------------------------------------------------
//
// Subroutine-
//       RFC7541::get_encode_bits
//       RFC7541::get_encode_mask
//       RFC7541::get_encode_type
//
// Purpose-
//       Get the encode mask DATA width, in bits
//       Get the encode mask from type
//       Get the encode type from encoding
//
//----------------------------------------------------------------------------
extern octet type_to_bits[8];
          /* type_to_bits= {   7,    0,    6,    5,    0,    4,    0,    4}*/
extern octet type_to_mask[8];
          /* type_to_mask= {0x80, 0x40, 0x40, 0x20, 0x10, 0x10, 0x00, 0x00}*/
static inline int
   get_encode_bits(ENCODE_TYPE t)   // Get the encode mask DATA width, in bits
{  return type_to_bits[t]; }

static inline int
   get_encode_mask(ENCODE_TYPE t)   // Get the encode mask
{  return type_to_mask[t]; }

extern ENCODE_TYPE
   get_encode_type(int C);          // Convert input octet into ENCODE_TYPE

//----------------------------------------------------------------------------
//
// Struct-
//       RFC7541::Entry
//
// Purpose-
//       RFC7541 Dynamic table Entry descriptor
//
//----------------------------------------------------------------------------
struct Entry : public pub::List<Entry>::Link { // RFC7541 Entry descriptor
const char*            name= nullptr; // Entry name, c-string
const char*            value= nullptr; // Entry value, nullptr or c-string
Array_ix               index= 0;     // Name index, constant once assigned
octet                  _002C[4]= {}; // Unused, for alignment

   Entry( void ) = default;         // Default constructor
   Entry(const Entry&);             // Copy constructor
   Entry(Entry&&);                  // Move constructor

   Entry(                           // Name/value onstructor
     const char*       name,        // Entry name
     const char*       value,       // Entry value
     Array_ix          index= 0);   // Name index

   Entry(                           // Property& constructor
     const Property&   property);   // Source property

virtual
   ~Entry( void );                  // Destructor

virtual bool
   is_dynamic( void ) const
{  return true; }

virtual bool
   is_static( void ) const
{  return false; }

explicit
   operator Property( void ) const; // Cast to Property

void
   debug(const char*   info= "") const; // Debugging display
}; // struct Entry

//----------------------------------------------------------------------------
//
// Struct-
//       RFC7541::Entry_const
//
// Purpose-
//       RFC7541 Static table Entry descriptor (CONSTANT)
//
// Implementation notes-
//       Used to insert static_entry values into an Entry_map
//
//----------------------------------------------------------------------------
struct Entry_const : public Entry { // RFC7541 constant Entry
   Entry_const(                     // Constructor
     const char*       name,        // Entry name
     const char*       value,       // Entry value
     Entry_ix          index);      // Name index (Always < STATIC_ENTRY_DIM)

   Entry_const(const Entry&);       // Create from const Entry&

virtual
   ~Entry_const( void )             // Destructor
{  name= value= nullptr; }          // (Do not delete name or value)

virtual bool
   is_dynamic( void ) const
{  return false; }

virtual bool
   is_static( void ) const
{  return true; }
}; // struct Entry_const

//----------------------------------------------------------------------------
//
// Class-
//       RFC7541::Entry_map
//
// Purpose-
//       RFC7541::Entry hash map
//
//----------------------------------------------------------------------------
class Entry_map {                   // Entry hash map
public:
// RFC7541::Entry_map::Typedefs and enumerations - - - - - - - - - - - - - - -
typedef std::string                 string;
typedef pub::List<Entry>            Hash_entry;
typedef Hash_entry*                 Hash_table;

// RFC7541::Entry_map::Attributes- - - - - - - - - - - - - - - - - - - - - - -
Hash_table             hash_table= nullptr; // The hash table

// RFC7541::Entry_map::Constructors/Destructor - - - - - - - - - - - - - - - -
   Entry_map( void );              // Default constructor

[[deprecated("NOT IMPLEMENTED")]]   // Maybe this will be used someday
   Entry_map(size_t count);         // Constructor (setting hash_size)

   ~Entry_map( void );              // Destructor

// RFC7541::Entry_map::Methods - - - - - - - - - - - - - - - - - - - - - - - -
void
   debug(const char*   info= "") const; // Debugging display

void
   insert(                          // Insert the map entry
     Entry*            entry);      // For this Entry

Entry*                              // The map entry
   locate(                          // Get map entry
     const char*       name,        // For this Name and
     const char*       value= nullptr); // (Optionally) for this Entry

void
   remove(                          // Remove the map entry
     Entry*            entry);      // For this Entry

void
   reset( void );                   // Empty the Entry_map
}; // class Entry_map

//----------------------------------------------------------------------------
//
// Struct-
//       RFC7541::Integer
//
// Purpose-
//       Integer decoder/encoder
//
//----------------------------------------------------------------------------
struct Integer {                    // RFC7541 integer decoder/encoder
typedef pub::Ioda         Writer;   // Output Ioda
typedef pub::IodaReader   Reader;   // Output Ioda
typedef uint32_t          Value_t;  // Integer: Decoder/encoder integer type

static Value_t                      // The decoded integer
   decode(                          // Decode integer
     Reader&           reader,      // (INPUT) ReaderIoda
     int               bits= 7);    // Number of size bits in first byte

// Parameters stamp and bits defaulted for an ET_INDEX Entry_ix value -or- a
// Huffman encoded text length.
static void
   encode(                          // Encode integer
     Writer&           writer,      // (OUTPUT) Writer Ioda
     Value_t           value,       // Integer value
     int               stamp= 0x80, // First byte encoding bits (ONLY)
     int               bits= 7);    // Number of DATA size bits in first byte
}; // class RFC7541::Integer

//----------------------------------------------------------------------------
//
// Struct-
//       RFC7541::Property
//       RFC7541::Properties
//
// Purpose-
//       Name/value string pair descriptor container
//       Property vector
//
// Implementaton notes:
//       ENCODE_TYPES ET_INSERT, ET_NEVER, and ET_CONST may be specified
//       whether or not an index is available. When no matching name index is
//       in encoder storage, the literal name value is encoded. The inverse is
//       not true. ET_*_NOINDEX types always use literal encodings.
//
//       To avoid name length attacks, do not use ET_NEVER unless the name is
//       defined in the static index table. Use ET_NEVER_NOINDEX instead.
//
//       ET_INSERT, ET_NEVER, and ET_CONST index lookup failure is not an
//       error. The encoding type is respectively converted to
//       ET_INSERT_NOINDEX, ET_NEVER_NOINDEX, or ET_CONST_NOINDEX.
//
//       ET_INSERT, ET_NEVER, and ET_CONST *could be* automatically upgraded
//       to ET_INDEX if an index name and value match is found.
//       Need to consider whether or not this is desired. See et_index field.
//
//----------------------------------------------------------------------------
struct Property {                   // Name/Value pair container
typedef std::string    string;      // (Import std::string)
enum {H_DEFAULT= false};            // Implementation Huffman encoding default
                                    // (Be aware: this may change.)


string                 name;        // Property name
string                 value;       // Property value
octet                  et= ET_INDEX; // Transfer encoding type
octet                  n_encoded= H_DEFAULT; // Huffman encoded name?
octet                  v_encoded= H_DEFAULT; // Huffman encoded value?
octet                  _0013[5]= {}; // Reserved for alignment

   Property( void ) = default;      // Default constructor

   Property(const Property& P)      // Copy constructor
{  copy(P); }

   Property(Property&& P)           // Move constructor
{  move(std::move(P)); }

   Property(                        // Name/Value constructor
     const string&     name,        // Name
     const string&     value,       // Value
     octet             et= ET_INDEX, // Transfer encoding type
     bool              n_encoded= H_DEFAULT, // Huffman encoded name?
     bool              v_encoded= H_DEFAULT);// Huffman encoded value?

   Property(                        // Entry& constructor
     const Entry&      entry);      // (Source Entry);

   ~Property( void ) = default;     // Destructor

void
   debug(const char* info= "") const; // Debugging display

void
   copy(const Property&);           // Copy Property

void
   move(Property&&);                // Move Property

Property&
   operator=(const Property& P)     // Copy assignment
{  copy(P); return *this; }

Property&
   operator=(Property&& P)          // Move assignment
{  move(std::move(P)); return *this; }

explicit
   operator Entry( void ) const;    // (Entry) cast operator

// Implementation note: only the name and value are compared.
bool
   operator==(const Property& rhs) const;

bool
   operator!=(const Property& rhs) const
{  return !operator==(rhs); }
}; // class RFC7541::Property

//============================================================================
class Properties : public std::vector<Property> {
public:
// RFC7541::Typedefs and enumerations- - - - - - - - - - - - - - - - - - - - -
typedef std::string    string;      // (Import std::string)

// RFC7541::Properties::Constructors - - - - - - - - - - - - - - - - - - - - -
   using std::vector<Property>::vector;

// RFC7541::Properties::methods- - - - - - - - - - - - - - - - - - - - - - - -
void
   append(const Property& property) // Insert property at end
{  insert(end(), property); }

void
   append(                          // Create and insert Property
     const string&     name,        // Name
     const string&     value,       // Value
     octet             et= ET_INDEX, // Insert property into map
     bool              n_encoded= false, // Huffman encoded name?
     bool              v_encoded= false) // Huffman encoded value?
{
   Property property(name, value, et, n_encoded, v_encoded);
   append(property);
}

void
   debug(const char* info= "") const; // Debugging display

void
   reset( void )                    // Remove all entries
{  clear(); }
}; // class Properties

//----------------------------------------------------------------------------
//
// Class-
//       RFC7541::Huff
//
// Purpose-
//       HTTP/2 HPACK Huffman compressed data container
//
// Implementation note-
//       Use size_t rather than Value_t for generality
//       TODO: Remove non-Reader/Writer methods in Phase-1
//
//----------------------------------------------------------------------------
class Huff {
// RFC7541::Huff::Typdefs and enumerations - - - - - - - - - - - - - - - - - -
public:
typedef std::string       string;   // (Using std::string)
typedef pub::Ioda         Writer;   // Output Ioda
typedef pub::IodaReader   Reader;   // Output Ioda

// RFC7541::Huff::Attributes - - - - - - - - - - - - - - - - - - - - - - - - -
private:
octet*                 addr= nullptr; // The compressed data adress
size_t                 size= 0;     // The compressed data length

// RFC7541::Huff::Constructors, destructor - - - - - - - - - - - - - - - - - -
public:
   Huff( void ) = default;          // The default constructor

   Huff(const string& s)            // Copy(string) constructor
{  *this= encode(s); }

   Huff(const Huff& h)              // Copy constructor
{  copy(h); }

   Huff(Huff&& h)                   // Move constructor
:  addr(h.addr), size(h.size)
{  h.addr= nullptr; h.size= 0; }

   ~Huff( void )                    // Destructor
{  free(addr); addr= nullptr; size= 0; }

// RFC7541::Huff::Assignment operators - - - - - - - - - - - - - - - - - - - -
Huff& operator=(const string& s)   // Assign from string
{  *this= encode(s); return *this; }

Huff& operator=(const Huff& h)      // Copy assignment
{  copy(h); return *this; }

Huff& operator=(Huff&& h)           // Move assignment
{  free(addr); addr= h.addr; size=h.size; h.addr= nullptr; h.size= 0;
   return *this;
}

// RFC7541::Huff::Comparison operators - - - - - - - - - - - - - - - - - - - -
bool  operator==(const Huff& h) const; // Equality comparison
bool  operator!=(const Huff& h) const; // Inequality comparison

// RFC7541::Huff::Accessors- - - - - - - - - - - - - - - - - - - - - - - - - -
const octet*                        // The compressed data address
   get_addr( void ) const           // Get compressed data address
{  return addr; }

size_t                              // The compressed data length
   get_size( void ) const           // Get compressed data length
{  return size; }

// RFC7541::Huff::Methods- - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   debug(const char* info= "") const; // Debugging display

string                              // Resultant string
   decode( void ) const             // Decode (this) compressed string
{  return decode(addr, size); }

static string                       // Resultant string
   decode(                          // Decode a compressed string
     const octet*      addr,        // Compressed string address
     size_t            size);       // Compressed string length

static void                         // The encoded length
   encode(                          // Encode string (ONLY)
     Writer&           writer,      // (OUTPUT) Writer Ioda
     const string&     s);          // String to encode

static Huff                         // Resultant Huff
   encode(                          // Create
     const string&      s);         // Using this string

static size_t                       // The encoded length
   encoded_length(                  // Get encoded length (in bytes)
     const string&     s);          // For this string

// RFC7541::Huff::Internal methods - - - - - - - - - - - - - - - - - - - - - -
private:
void
   copy(const Huff&);               // Copy from const Huff&
}; // class RFC7541::Huff

//----------------------------------------------------------------------------
//
// Class-
//       RFC7541::Pack
//
// Purpose-
//       HTTP/2 HPACK compression encoder/decoder
//
// Implementation notes-
//       Phase[0] prototype version
//
//----------------------------------------------------------------------------
class Pack {                        // HPACK encoder/decoder
// RFC7541::Pack::Typedefs and enumerations- - - - - - - - - - - - - - - - - -
public:
typedef std::string       string;   // (Using std::string)
typedef pub::Ioda::Buffer Buffer;   // I/O Data Area: (input) Buffer, unused
typedef pub::Ioda::Reader Reader;   // I/O Data Area: IodaReader
typedef pub::Ioda::Writer Writer;   // I/O Data Area: Writer

enum {DYNAMIC_ENTRY_0= 62};         // Entry_ix for the first dynamic entry

// RFC7541::Pack::Attributes - - - - - - - - - - - - - - - - - - - - - - - - -
private: // Production
public:  // Phase[0]
Value_t                encode_size= 0; // Encoder/decoder storage size
Value_t                entry_size= 0; // Number of entry_array entries (total)
Value_t                entry_used= 0; // Number of entry_array entries used
Array_ix               entry_ins= 1;  // Current entry_array insertion index
Array_ix               entry_old= 1;  // Current entry_array array oldest index
Value_t                value_used= 0; // Allocated data storage size

Entry**                entry_array= nullptr; // The Entry array
Entry_map              entry_map;     // The Entry hash map

// Operational controls
static int             hcdm;        // Hard Core Debug Mode?
static int             verbose;     // Verbosity, higher is more verbose

// TODO: REMOVE
mutable int            debug_recursion= 0; // Debug recursion indicator

// RFC7541::Pack::Constructors, destructor - - - - - - - - - - - - - - - - - -
public:
   Pack( void );                    // Default constructor, DEFAULT_ENCODE_SIZE
   Pack(Value_t);                   // Constructor, specifying encode_size
   Pack(const Pack&) = delete;      // No copy constructor
   Pack(Pack&&) = delete;           // No move constructor

   ~Pack( void );                   // Destructor

void init(Value_t size= 0);         // Initialize (construct) with initial size
void term( void );                  // Terminate  (delete)

// RFC7541::Pack::Debugging methods- - - - - - - - - - - - - - - - - - - - - -
void
   debug(const char* info= "") const; // Debugging display

// RFC7541::Pack::Assignment operators - - - - - - - - - - - - - - - - - - - -
Pack& operator=(const Pack&) = delete; // No copy assignment operator
Pack& operator=(Pack&&) = delete;   // No move assignment operator

// RFC7541::Pack::Comparison operators - - - - - - - - - - - - - - - - - - - -
bool  operator==(const Pack& h) const; // Equality comparison
bool  operator!=(const Pack& h) const; // Inequality comparison

// RFC7541::Pack::Accessor methods - - - - - - - - - - - - - - - - - - - - - -
Value_t
   get_encode_size( void ) const    // Get encode storage size (limit)
{  return encode_size; }

Value_t
   get_encode_used( void ) const    // Get encode used storage
{  return (entry_used * SPEC_ENTRY_SIZE) + value_used; }

// RFC7541::Pack::Methods- - - - - - - - - - - - - - - - - - - - - - - - - - -
Properties
   decode(Reader&);                // Decode packed data

void
   encode(Writer&, const Properties&); // Encode Properties

const Entry*                        // The Entry
   get_entry(                       // Get Entry
     Entry_ix          entry)       // From this index
{  return entix2entry(entry); }     // (Public access version)

void
   reset( void )                    // Reset the Pack object, emptying it
{  resize(0); }                     // To its default initial state

void
   reset(Value_t size)              // Reset the Pack object, setting new size
{  resize(0); if( size ) resize(size); }

void
   resize(Value_t);                 // Update the encoding storage size

void
   resize(Writer&, Value_t);        // Encode encode_size update

// RFC7541::Pack::Internal methods - - - - - - - - - - - - - - - - - - - - - -
private:
const Entry*                        // The Entry*
   entix2entry(                     // Get Entry*
     Entry_ix          entry) const; // For this logical index

Entry_ix                            // The logical index
   entry2entix(                     // Get logical index
     const Entry*      entry) const; // For this Entry

void
   evict(                           // Evict entries from encoding storage
     size_t            size= 0);    // Until an entry of this size will fit

void
   insert(Entry*);                  // Insert Entry into table

void
   insert(                          // Allocate and insert Entry into table
     const Property&   property);   // Using this Property

void
   remove( void );                  // Remove the oldest index

static string                       // Resultant string
   string_decode(                   // Decode input string
     Reader&           reader);     // (INPUT) IodaReader

static void
   string_encode(                   // Copy string to Writer
     Writer&           writer,      // (OUTPUT) Ioda::Writer
     string            text,        // The encode string
     bool              encoded);    // Use Huffman encoding?
}; // class RFC7541::Pack

//----------------------------------------------------------------------------
//
// (Static) subroutine-
//       RFC7541::debug
//
// Purpose-
//       (Bringup) debugging display
//
//----------------------------------------------------------------------------
void
   debug(const char* info= ""); // Debugging display

//----------------------------------------------------------------------------
//
// (Static) utility subroutine-
//       RFC7541::load_properties
//
// Purpose-
//       Load Properties
//
// Implementation notes-
//       Parameters TBD
//
//----------------------------------------------------------------------------
Properties                          // The Properties
   load_properties( void );         // Load Properties, parameter(s) TBD
}  // namespace RFC7541
#endif // _RFC7541_H_INCLUDED
