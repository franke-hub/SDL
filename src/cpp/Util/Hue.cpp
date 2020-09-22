//----------------------------------------------------------------------------
//
//       Copyright (C) 2030 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Hue.cpp
//
// Purpose-
//       Find best [R,G,B] color match.
//
// Last change date-
//       2020/09/21
//
// Implementation note-
//       The matching algorithm is simplistic, giving equal weight to colors.
//
//----------------------------------------------------------------------------
#include <limits.h>                 // For INT_MAX
#include <stdio.h>                  // For printf
//nclude <stdlib.h>
#include <string.h>                 // For strdup

#include <pub/List.h>               // For pub::List
#include <pub/Tokenizer.h>          // For pub::Tokenizer

//----------------------------------------------------------------------------
// struct Color: Color/Name combination
//----------------------------------------------------------------------------
struct Color : public pub::List<Color>::Link { // Color/Name combination
int                    R;           // Red component
int                    G;           // Green component
int                    B;           // Blue component
const char*            N;           // The color's name


   Color(                           // Constructor
     int               R,           // Red component
     int               G,           // Green component
     int               B,           // Blue component
     const char*       N)           // Color name
:  R(R), G(G), B(B), N(N) {}
};

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static char            buffer[4096]; // (Way big) input buffer
static pub::List<Color>
                       list;        // The Color list

//----------------------------------------------------------------------------
//
// Subroutine-
//       loader
//
// Purpose-
//       Load "rgb.txt" data file
//
//----------------------------------------------------------------------------
static int                          // Return code, 0 OK
   loader( void )                   // Load data file
{
   std::string name= "/home/";      // The rgb.txt file name
   const char* user= getenv("USER"); // Get the user name
   if( user == nullptr ) {
     fprintf(stderr, "'USER' environment variable not set\n");
     return 1;
   }
   name += user;                    // Add USER name
   name += "/.REF/rgb.txt";         // Complete the file name

   FILE* file= fopen(name.c_str(), "rb"); // The color definition file
   if( file == nullptr ) {
     fprintf(stderr, "Unable to open(%s)\n", name.c_str());
     return 1;
   }

   // Skip the first line
   for(;;) {
     size_t L= fread(buffer, 1, 1, file);
     if( L == 0 ) {
       fprintf(stderr, "File(%s) invalid format\n", name.c_str());
       return 1;
     }
     if( buffer[0] == '\n' ) break;
   }

   // Read the file
   for(unsigned line= 2;;line++) {
     // Read next line
     unsigned X= 0;                // Buffer index
     size_t L= fread(buffer, 1, 1, file);
     if( L == 0 ) break;
     if( buffer[0] == '\n' || buffer[0] == '\r' ) continue;

     // Fill the line
     for(;;) {
       if( buffer[X] == '\t' || buffer[X] == '\r' ) buffer[X]= ' ';
       X++;
       if( X >= sizeof(buffer) ) {
         buffer[sizeof(buffer) - 1]= '\0';
         fprintf(stderr, "file(%s) line(%d) overlong(%s)\n", name.c_str()
                       , line, buffer);
         return 1;
       }

       L= fread(buffer + X, 1, 1, file);
       if( L == 0 ) break;
       if( buffer[X] == '\n' ) break;
     }
     buffer[X]= '\0';

     // Line complete, strip trailing blanks
     while( X > 0 ) {
       if( buffer[X-1] != ' ' )
         break;

       buffer[--X]= '\0';
     }

     // Extract R,G,B name components
     pub::Tokenizer izer(buffer);
     auto iter= izer.begin();
     int R= atoi((iter++)().c_str());
     int G= atoi((iter++)().c_str());
     int B= atoi((iter++)().c_str());
     const char* N= strdup(iter.remainder());
     if( N[0] == '\0' ) {
       fprintf(stderr, "file(%s) line(%d) malformed(%s)\n", name.c_str()
                     , line, buffer);
       return 1;
     }

     Color* color= new Color(R, G, B, N);
     list.fifo(color);
   }

   if( list.get_head() == nullptr ) {
     fprintf(stderr, "File(%s) empty\n", name.c_str());
     return 1;
   }

   //------------------------------------------------------------------------
   // Bringup: debugging display
   if( false ) {
     for(Color* color= list.get_head(); color; color= color->get_next() ) {
       printf("%3d %3d %3d '%s'\n", color->R, color->G, color->B, color->N);
     }
     return 1;
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Display usage information
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Display usage information
{
   printf("Hue R G B\n"
          "Find best [R,G,B] color match in data file.\n\n"
          "Data file: \"~/.REF/rgb.txt\"\n"
          "Colors R, G, and B are the red, green, and blue color components\n"
          "specified as numeric values between 0 and 255\n"
          "(The first line in the data file is ignored.)\n"
   );
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
     const char*       argv[])      // Argument array
{
   if( loader() ) {
     info();
     return 1;
   }

   if( argc != 4 ) {
     info();
     return 1;
   }

   int R= atoi(argv[1]);
   int G= atoi(argv[2]);
   int B= atoi(argv[3]);
   Color* best= nullptr;
   int diff= INT_MAX;

   for(Color* color= list.get_head(); color; color= color->get_next() ) {
     int test= 0;
     if( R >= color->R )
       test += (R - color->R);
     else
       test += (color->R - R);

     if( G >= color->G )
       test += (G - color->G);
     else
       test += (color->G - G);

     if( B >= color->B )
       test += (B - color->B);
     else
       test += (color->B - B);

     if( test < diff ) {
       best= color;
       diff= test;
     }
   }

   printf("%3d %3d %3d %s\n", best->R, best->G, best->B, best->N);

   return 0;
}

