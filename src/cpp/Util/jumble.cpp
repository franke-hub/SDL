//----------------------------------------------------------------------------
//
//       Copyright (c) 2009-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       jumble.cpp
//
// Purpose-
//       Display all possiblities for a jumbled word.
//
// Last change date-
//       2021/09/01
//
//----------------------------------------------------------------------------
#include <stdio.h>                   // For printf, sprintf
#include <string.h>                  // For strcpy, ...
#include <sys/stat.h>                // For stat

#include <pub/Debug.h>               // For namespace pub::debugging
#include <pub/List.h>                // For List

using namespace pub::debugging;      // For debugging



//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define BRINGUP false                // Testing bringup mode

#define MAXCOL 80                    // Maximum column length
#define MAXLEN 4096                  // Modified word length

//----------------------------------------------------------------------------
// Struct Word (word list item)
//----------------------------------------------------------------------------
struct Word : public pub::List<Word>::Link { // Word list item
char*                  word= nullptr; // The word string
};

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             outcol= 0;    // Output column
static pub::List<Word> list;         // The Word List
static int             spell= false; // Generate spelled word list?

//----------------------------------------------------------------------------
//
// Subroutine-
//       add_word
//
// Purpose-
//       Add word and its parts to the Word List
//
//----------------------------------------------------------------------------
static void
   add_word(                        // Display string
     const char*       string)      // The word string
{
   char buff[MAXLEN + 64];          // Work area for sprintf
   char copy[MAXLEN];               // A copy of the word string
   strcpy(copy, string);            // Copy the string

   int L= (int)strlen(copy);        // The current string length
   while( L >= 3 ) {                // Find all unique words with length >= 3
     sprintf(buff, "echo %s | aspell list >/tmp/aspell.out", copy);
     system(buff);                  // Test the (copy) word
     struct stat info;
     stat("/tmp/aspell.out", &info);
     if( info.st_size == 0 ) {      // If not misspelled
       Word* word= list.get_head();
       while( word ) {              // Scan list for duplicate
         if( strcmp(copy, word->word) == 0 )
           break;

         word= word->get_next();
       }

       if( word == nullptr ) {      // If not already on the list
         word= new Word();
         word->word= strdup(copy);
         list.fifo(word);
       }
     }

     copy[--L]= '\0';
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       display
//
// Purpose-
//       Display a string.
//
//----------------------------------------------------------------------------
static void
   display(                         // Display string
     int               length,      // The string length
     const char*       string)      // The jumbled word
{
   #if( BRINGUP )
     printf("%s\n", string);
   #else
     if( (outcol+length) > MAXCOL ) {
       printf("\n");
       outcol= 0;
     } else if( outcol > 0 ) {
       printf(" ");
     }

     printf("%s", string);
     outcol += (length + 1);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       jumble
//
// Purpose-
//       Display all unique possibilities for a jumbled word.
//
//----------------------------------------------------------------------------
static void
   jumble(                          // Display jumbled possibilities
     int               index,       // The jumble index
     int               length,      // The length of the jumbled word
     const char*       string)      // The jumbled word
{
   char                modstr[MAXLEN]; // The modified word

   #if( BRINGUP )
     tracef("%2d, %s\n", index, string); // For debugging only
   #endif

   if( index >= length ) {
     display(length, string);
     if( spell )
       add_word(string);
   } else {
     #if( BRINGUP )
       tracef("%2d, %s => %2d, %s\n", index, string, index+1, string);
     #endif

     jumble(index+1, length, string);

     for(int x= index+1; x<length; x++) {
       strcpy(modstr, string);
       char C= modstr[x];
       if( modstr[index] != C ) {
         int isValid= true;         // If next not a duplicate
         char D= string[x];
         for(int y= x+1; y<length; y++) {
           if( D == string[y] ) {
             isValid= false;
             break;
           }
         }

         if( isValid ) {
           modstr[x]= modstr[index];
           modstr[index]= C;
           #if( BRINGUP )
             tracef("%2d, %s => %2d, %s\n", index, string, index+1, modstr);
           #endif
           jumble(index+1, length, modstr);
         }
       }
     }
   }
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
   int                 i;

   for(i= 1; i<argc; i++) {
     int L= strlen(argv[i]);
     if( L < MAXLEN ) {
       if( argv[i][0] != '-' ) {    // If word parameter
         jumble(0, L, argv[i]);
         if( outcol > 0 ) {
           printf("\n");
           outcol= 0;
         }
       } else {                     // If switch
         if( argv[i][1] == 's' )    // If spell switch
           spell= true;
       }
     }
   }

   Word* word= list.get_head();     // Display words
   if( word ) {                     // If any words
     printf("\n\nWord list:\n");
     while( word ) {
       printf("%s\n", word->word);
       word= word->get_next();
     }

     for(word= list.get_head(); word; word= list.get_head()) {                      // Empty the list
       list.remove(word, word);
       delete word;
     }
   }

   return 0;
}
