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
//       Quick.cpp
//
// Purpose-
//       Quick tests.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>                  // For isprint()
#include <getopt.h>                 // For getopt()
#include <stdio.h>                  // For fprintf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp

#include <memory>                   // For std::unique_ptr, ...

#include <com/Debug.h>
#include <com/Random.h>

#include "Word.h"

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_debug= 0; // --debug
static int             opt_help= false; // --help or error
static int             opt_index;   // Option index
static const char*     opt_loader= nullptr; // --loader
static int             opt_verbose= false; // --verbose

static struct option   OPTS[]=      // Options
{  {"help",    no_argument,       &opt_help,    true}
,  {"verbose", no_argument,       &opt_verbose, true}

,  {"debug",   optional_argument, nullptr,      0}
,  {"loader",  required_argument, nullptr,      0}
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX
{  OPT_HELP
,  OPT_VERBOSE
,  OPT_DEBUG
,  OPT_LOADER
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter description.
//
//----------------------------------------------------------------------------
static void
   info( void)                      // Parameter description
{
   fprintf(stderr, "Quick [options]\n"
                   "Options:\n"
                   "  --help\tThis help message\n"
                   "  --debug\t{=value}\n"
                   "  --loader\t=file_name\n"
          );

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis example.
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     const char*       argv[])      // Argument array
{
   int                 C;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   opterr= 0;                       // Do not write error messages

   while( (C= getopt_long(argc, (char**)argv, ":", OPTS, &opt_index)) != -1 )
     switch( C )
     {
       case 0:
         switch( opt_index )
         {
           case OPT_DEBUG:
             if ( optarg )
               opt_debug= atoi(optarg);
             else
               opt_debug= 1;
             break;

           case OPT_LOADER:
             opt_loader= optarg;
             break;

           default:
             break;
         }
         break;

       case ':':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "Option requires an argument '%s'.\n",
                           argv[optind-1]);
         else
           fprintf(stderr, "Option requires an argument '-%c'.\n", optopt);
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "Unknown option '%s'.\n", argv[optind-1]);
         else if( isprint(optopt) )
           fprintf(stderr, "Unknown option '-%c'.\n", optopt);
         else
           fprintf(stderr, "Unknown option character '0x%x'.\n", optopt);
         break;

       default:
         fprintf(stderr, "%4d SNO ('%c',0x%x).\n", __LINE__, C, C);
         exit( EXIT_FAILURE );
     }

   if( opt_help )
     info();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_word
//
// Purpose-
//       Test Word.h
//
//----------------------------------------------------------------------------
static const char*     word_sample[]=
{  ""
,  "a"
,  "an"
,  "the"
,  "this"
,  "that"
,  0
}; // word_sample[]

static int             word_select[]=
{  0
,  0
,  0
,  0
,  0
,  0
,  0
}; // word_select[]

#define SAMPLE_SIZE 6

//----------------------------------------------------------------------------
// word_error: word_loader error handler
//----------------------------------------------------------------------------
static void
   word_error(                      // Word loader error
     size_t            line,        // The file line
     const char*       text)        // The message text
{
   fprintf(stderr, "Word::word_loader: File(%s) line(%zd) %s\n",
                   opt_loader, line, text);
   throw std::runtime_error("initialization failure");
}

//----------------------------------------------------------------------------
// word_loader: word loader
//----------------------------------------------------------------------------
static void
   word_loader(Word_refs& word)     // Initialize the word table
{
   // Load from sample
   if( opt_loader == nullptr )      // If --loader omitted
   {
     for(int i= 1; word_sample[i]; i++)
       word.insert(word_sample[i], 1);

     return;
   }

   // Load from file
   Word::Text file= opt_loader;
   FILE* inp= fopen(file, "rb");
   if( inp == nullptr )
     word_error(0, "file does not exist");

   Word::Count line= 1;             // Line number
   char buffer[4096];               // Collection buffer
   int C= fgetc(inp);               // Get first character
   for(;;)                          // Interpret the file
   {
     size_t count= 0;
     while( isspace(C) )
     {
       if( C == '\n' ) line++;
       C= fgetc(inp);
     }

     if( C < 0 ) break;

     while( C != ' ' )
     {
       if( C < '0' || C > '9' )
         word_error(line, "invalid count");

       C= C - '0';                  // Numeric value
       count *= 10;                 // Update count
       count += C;
       if( count > size_t(0x0100000000) )
         word_error(line, "count too large");

       C= fgetc(inp);
     }
     if( count == 0 )
       word_error(line, "count is zero");

     while( isspace(C) )
     {
       if( C == '\n' )
         word_error(line, "text missing");

       C= fgetc(inp);
     }

     int i= 0;
     while( C > 0 && !isspace(C) )
     {
       buffer[i++]= C;
       if( i >= sizeof(buffer) )
         word_error(line, "text too long");

       C= fgetc(inp);
     }

     buffer[i]= '\0';

     while( C != '\n' )
     {
       if( !isspace(C) )
         word_error(line, "text contains spaces");

       C= fgetc(inp);
     }

     word.insert(buffer, count);
   }
}

static void
   test_word( void )                // Test Word.h
{
   debugf("\ntest_word\n");

   // Test Word.h
   Word_refs word;
   word_loader(word);

   for(int i=0; word_sample[i]; i++)
   {
     word.ref(word_sample[i]);
       for(int j=i+1; word_sample[j]; j++)
         word.ref(word_sample[j]);
   }

   // Debugging options
   if( opt_debug > 1 )
   {
     if( opt_debug == 201 )
     {
       debugf("Testing: Word::random_select\n");

       Random::standard.randomize();
       unsigned int RANDOM_COUNT= 1000000;
       Word::Total total_have= 0;
       for(int i= 0; i<RANDOM_COUNT; i++)
       {
         Word::Index X= word.random_select();
         Word::Text  T= word.index(X);
         if( false ) debugf("Random: %6d '%s'\n", X, T);

         for(int j= 1; j<SAMPLE_SIZE; j++)
         {
           if( strcmp(T, word_sample[j]) == 0 )
           {
             word_select[j]++;
             total_have++;
             break;
           }
         }
       }

       Word::Total total_want= 0;
       for(int i= 0; i<SAMPLE_SIZE; i++)
         total_want += word.get_count(word_sample[i]);

       for(int i= 0; i<SAMPLE_SIZE; i++)
       {
         Word::Index X= word.index(word_sample[i]);
         double want= word.get_count(X);
         want /= total_want;
         double have= word_select[i];
         have /= total_have;
         debugf("[%2d] want(%6.2f) have(%6.2f) %s\n",
                i, want*100.0, have*100.0, word_sample[i]);
       }
     }
     else
       word.debug(opt_debug);
   }
   else if( opt_debug )
   {
     int used= word.get_used();
     debugf("Word: w_total(%zd) w_used(%d)\n", word.get_total(), used);
     for(int i=0; i<used; i++)
       debugf("[%6d] %10d %s\n", i, word.get_count(i), word.index(i));
     debugf("[%6s] %10zd *TOTAL*\n", "", word.get_total());
   }

   // List words sorted by probability
   if( opt_loader )                 // If loader file selected
   {
     double max_prob= 0.0;
     double total= word.get_total();
     int    used=  word.get_used();
     std::unique_ptr<Word::Index[]> word_index(new Word::Index[used]);
     std::unique_ptr<double[]>      word_prob(new double[used]);
     for(int i= 0; i<used; i++)     // Initialize probability array
     {
       double p= word.get_count(i) / total;
       word_index[i]= i;
       word_prob[i]=  p;

       if( p > max_prob )
         max_prob= p;
     }

     for(int i= 0; i<used; i++)     // Bubble sort probability array
     {
       for(int j= i+1; j<used; j++)
       {
         if( word_prob[i] < word_prob[j] )
         {
           int    index=  word_index[i];
           double prob=   word_prob[i];
           word_index[i]= word_index[j];
           word_prob[i]=  word_prob[j];
           word_index[j]= index;
           word_prob[j]=  prob;
         }
       }
     }

     for(int i= 0; i<used; i++)     // Display the sorted array
     {
       Word::Index index= word_index[i];
       double      prob=  word_prob[i];

       prob /= max_prob;
       debugf("%12.6E %s\n", prob, word.index(index));
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
   parm(argc, argv);                // Sample parameter analysis

   try {
     test_word();                   // Test Word.h
   } catch( std::exception& X) {
     debugf("std::exception.what(%s)\n", X.what());
   } catch( const char* X) {
     debugf("catch(const char*(%s))\n", X);
   } catch( ... ) {
     debugf("catch(...)\n");
   }

   return 0;
}

