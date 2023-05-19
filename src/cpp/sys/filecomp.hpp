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
//       filecomp.hpp
//
// Purpose-
//       Internal classes and subroutines for filecomp.cpp
//
// Last change date-
//       2023/05/19
//
// Implementation note-
//       *ONLY* included from filecomp.cpp
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Line
//
// Purpose-
//       An immutable file line.
//
//----------------------------------------------------------------------------
class Line {                        // File line
//----------------------------------------------------------------------------
// Line::Attributes
//----------------------------------------------------------------------------
protected:
Line*                  next= nullptr; // The next Data Line
const char*            text;        // The associated text

//----------------------------------------------------------------------------
// Line::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   Line(                            // Constructor
     Line*             _line,       // The prior Line
     const char*       _text)       // The associated text
:  text(_text)
{
   if( _line )
     _line->next= this;
}

   ~Line( void ) = default;

//----------------------------------------------------------------------------
// Line::Methods
//----------------------------------------------------------------------------
Line*                               // The next Line
   get_next( void )                 // Get next Line
{  return next; }

const char*                         // The Line's text
   get_text( void )                 // Get Line's text
{  return text; }
}; // class Line

//----------------------------------------------------------------------------
//
// Class-
//       Data
//
// Purpose-
//       Read/only file data container.
//
//----------------------------------------------------------------------------
class Data {                        // File data container
//----------------------------------------------------------------------------
// Data::Attributes
//----------------------------------------------------------------------------
protected:
char*                  _data= nullptr; // The allocated file data
Line*                  _head= nullptr; // The first line

//----------------------------------------------------------------------------
// Data::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   Data(                            // Constructor
     const char*       name);       // Associated file name

   ~Data( void );                   // Destructor

//----------------------------------------------------------------------------
// Data::Methods
//----------------------------------------------------------------------------
Line*                               // The first Line
   get_head( void )                 // Get first Line
{  return _head; }
}; // class Data

//----------------------------------------------------------------------------
//
// Method-
//       Data::Data
//
// Purpose-
//       Constructor: Load file data, separating it into lines
//
//----------------------------------------------------------------------------
   Data::Data(                      // Constructor
     const char*       name)        // The file name to load
{
   struct stat st;                  // File stats
   int rc= stat(name, &st); // Get file information
   if( rc != 0 )                    // If failure
   {
     fprintf(stderr, "stat(%s) failure(%d) %d:%s\n", name, rc
                   , errno, strerror(errno));
     throw std::runtime_error("Open failure");
   }

   size_t size= st.st_size;         // The size of the file
   _data= (char*)malloc(size + 1);  // (Add '\0' to the end of the data)
   if( _data == nullptr )
     throw std::bad_alloc();

   // Load the file
   FILE* f= fopen(name, "rb");
   size_t L= fread(_data, 1, size+1, f);
   if( L != size )
   {
     fprintf(stderr, "File(%s) read failure %d:%s\n", name
                   , errno, strerror(errno));
     throw std::runtime_error("Read failure");
   }
   _data[size]= '\0';               // Add '\0' delimiter
   fclose(f);

   // Insure that the file does not contain a '\0' delimiter
   char* last= strchr(_data, '\0'); // Locate first '\0' delimiter
   if( size_t(last-_data) < size )  // If file contains '\0' delimiter
   {
     fprintf(stderr, "File(%s) contains '\\0' delimiter\n", name);
     throw std::runtime_error("Unusable file");
   }

   // Parse _data into lines (Performance critical path)
   Line* prev= nullptr;
   char* used= _data;
   while( used < last )
   {
     char* from= used;              // First line text character
     Line* line= new Line(prev, from);
     if( prev == nullptr )
       _head= line;
     prev= line;

     char* next= strchr(used, '\n'); // Get next line delimiter
     if( next )
     {
       *next= '\0';                 // Replace with string delimiter
       used= next + 1;              // Next line origin
       while( next > from )         // Discard '\r' characters next to '\n'
       {
         next--;
         if( *next != '\r' )
           break;

         *next= '\0';
       }
     } else {                       // Last line missing '\n'
       fprintf(stderr, "File(%s) last line missing '\\n'\n", name);
       break;
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Data::~Data
//
// Purpose-
//       Destructor: Delete file data and lines
//
//----------------------------------------------------------------------------
   Data::~Data( void )              // Destructor
{
   free(_data);                     // Delete the file data, if any

   Line* line= _head;
   while( line ) {
     Line* next= line->get_next();
     delete line;
     line= next;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       wildchar::strcmp
//
// Purpose-
//       Wildcard string compare.
//
//----------------------------------------------------------------------------
namespace wildchar {
static int                          // Resultant 0, !0
   strcmp(                          // Wildchar string compare
     const char*       L_,          // Left hand side (May contain wildchars)
     const char*       R_)          // Right hand side
{
   const unsigned char* W= (const unsigned char*)L_;
   const unsigned char* R= (const unsigned char*)R_;
   while( true ) {
     if( *W == '\\' )
       ++W;

     int diff= *W - *R;
     if( diff != 0 ) {
       if( *W == '*' ) {
         while( *W == '*' )
           W++;
         if( *W == '\0' )
           return 0;
         while( *R != '\0' ) {
           diff= strcmp((char*)W, (char*)R);
           if( diff == 0 )
             break;
           R++;
         }
         return diff;
       } else if( *W == '?' ) {
         if( *R == '\0' )
           return diff;
       } else
         return diff;
     } else if( *W == '\0' )
       break;

     W++;
     R++;
   }

   return 0;
}
}  // namespace wildchar
