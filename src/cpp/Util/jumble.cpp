//----------------------------------------------------------------------------
//
//       Copyright (c) 2009-2022 Frank Eskesen.
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
//       2022/02/09
//
//----------------------------------------------------------------------------
#include <stdio.h>                  // For printf, sprintf
#include <string.h>                 // For strcpy, ...
#include <sys/stat.h>               // For stat

#include <hunspell/hunspell.hxx>    // For hunspell C++ interface

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/List.h>               // For List

using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= true                       // Hard Core Debug Mode?
,  BRINGUP= false                   // Bringup (test) mode?
,  MAXCOL= 80                       // Maximum column length
,  MAXLEN= 4096                     // Maximum word length
}; // Generic enum

typedef const char     CC;          // (Shortcut)
static constexpr CC*   AFF_HOME= "/usr/share/myspell/en_US.aff";
static constexpr CC*   DIC_HOME= "/usr/share/myspell/en_US.dic";

//----------------------------------------------------------------------------
// Struct Word (word list item)
//----------------------------------------------------------------------------
struct Word : public pub::List<Word>::Link { // Word list item
char*                  word= nullptr; // The word string
};

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Hunspell*       hs= nullptr; // The Hunspell object
static int             outcol= 0;   // Output column
static pub::List<Word> list;        // The Word List

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
   char copy[MAXLEN];               // A copy of the word string
   strcpy(copy, string);            // Copy the string

   int L= (int)strlen(copy);        // The current string length
   while( L >= 3 ) {                // Find all unique words with length >= 3
     std::string S= copy;
     if( hs->spell(S) ) {
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
   if( BRINGUP )
     printf("%s\n", string);
   else {
     if( (outcol+length) > MAXCOL ) {
       printf("\n");
       outcol= 0;
     } else if( outcol > 0 ) {
       printf(" ");
     }

     printf("%s", string);
     outcol += (length + 1);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize
//
//----------------------------------------------------------------------------
static void
   init( void )
{
   hs= new Hunspell(AFF_HOME, DIC_HOME);
   hs->add_dic("/home/eskesen/Library/Spelling/local.dic");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       Terminate
//
//----------------------------------------------------------------------------
static void
   term( void )
{  delete hs; }

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

   if( BRINGUP )
     tracef("%2d, %s\n", index, string); // For debugging only

   if( index >= length ) {
     display(length, string);
     add_word(string);
   } else {
     if( BRINGUP )
       tracef("%2d, %s => %2d, %s\n", index, string, index+1, string);

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
           if( BRINGUP )
             tracef("%2d, %s => %2d, %s\n", index, string, index+1, modstr);
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
   init();

   // Jumble all arguments
   for(int i= 1; i<argc; i++) {
     int L= strlen(argv[i]);
     if( L < MAXLEN ) {
       if( argv[i][0] != '-' ) {    // If word parameter
         jumble(0, L, argv[i]);
         if( outcol > 0 ) {
           printf("\n");
           outcol= 0;
         }
       }
     }
   }

   // Sort the Word list
   pub::List<Word> sort;            // The sorted Word List
   Word* head= list.get_head();     // Reset the list, returning it
   while( head != nullptr ) {       // (Bubble) sort the list
     Word* low= head;
     Word* next= low->get_next();
     while( next != nullptr ) {
       if( strcmp(low->word, next->word) > 0 )
         low= next;

       next= next->get_next();
     }

     list.remove(low, low);
     sort.fifo(low);
     head= list.get_head();
   }
   if( sort.get_head() )
     list.insert(nullptr, sort.get_head(), sort.get_tail());

   // Display the Word list
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

   term();
   return 0;
}
