//----------------------------------------------------------------------------
//
//       Copyright (c) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Picker.cpp
//
// Purpose-
//       Like the magic eight-ball.
//
// Last change date-
//       2014/01/01
//
// Usage notes-
//       stdin contains input lines: The first line is the question;
//       The remaining lines are the possible answers.
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <com/Debug.h>
#include <com/List.h>
#include <com/Random.h>
#include <com/Reader.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Line;

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static unsigned        count;       // The number of lines
static List<Line>      list;        // List of input lines

//----------------------------------------------------------------------------
//
// Class-
//       Line
//
// Purpose-
//       Define an input line.
//
//----------------------------------------------------------------------------
class Line : public List<Line>::Link { // Input Line
//----------------------------------------------------------------------------
// Line::Attributes
//----------------------------------------------------------------------------
public:
char*                  text;        // The input text

//----------------------------------------------------------------------------
// Line::Constructors
//----------------------------------------------------------------------------
public:
   ~Line( void );
   Line( void );

//----------------------------------------------------------------------------
// Line::Methods
//----------------------------------------------------------------------------
public:
int                                 // Return code, 0 OK
   load(                            // Load the input line
     Reader&           reader);     // From this Reader
}; // class Line

//----------------------------------------------------------------------------
// Class Line::implementation
//----------------------------------------------------------------------------
   Line::~Line( void )
{
   if( text != NULL )
   {
     free(text);
     text= NULL;
   }
}

   Line::Line( void )
:  List<Line>::Link()
,  text(NULL)
{
}

int
   Line::load(                       // Load line
     Reader&           reader)       // From this Reader
{
   char                buffer[4096]; // Input buffer

   int C= reader.readLine(buffer, sizeof(buffer));
   if( C < 0 )
     return C;

   text= strdup(buffer);
   if( text == NULL )
     throwf("Storage Shortage");

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       picker
//
// Purpose-
//       Pick one of the input lines.
//
//----------------------------------------------------------------------------
static void
   picker(                          // Pick an input line
     Reader&           reader)      // From this Reader
{
   Line*               line;        // Working -> Line
   Line                query;       // The query

   if( query.load(reader) != 0 )
   {
     printf("No question (in stdin)\n");
     return;
   }

   count= 0;
   for(;;)                          // Load the response list
   {
     line= new Line();
     if( line->load(reader) < 0 )
       break;

     count++;
     list.fifo(line);
   }

   delete line;                     // Delete the unused line
   if( count == 0 )
   {
     printf("No answers (in stdin)\n");
     return;
   }

   if( count == 0 )
   {
     printf("No answers (in stdin)\n");
     return;
   }

   PerfectRandom rng;               // Our random number generator
   rng.randomize();
   unsigned index= rng.modulus(count);

   line= list.getHead();            // First entry is index[0]
   for(unsigned i= 0; i<index; i++)
   {
     line= line->getNext();
   }

   printf("Question: %s\n", query.text);
   printf("   Reply: %s\n", line->text);
   if( count == 1 )
     printf("          (The only possible reply.)\n");

   // Cleanup
   line= list.remq();
   while( line != NULL )
   {
     delete line;
     line= list.remq();
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
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   //-------------------------------------------------------------------------
   // Was a file specified?
   //-------------------------------------------------------------------------
   FileReader reader;
   const char* fileName= "<";       // Default, stdin
   if( argc > 1 )
     fileName= argv[1];

   reader.open(fileName);
   picker(reader);

   return 0;
}

