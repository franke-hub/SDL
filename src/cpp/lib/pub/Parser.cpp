//----------------------------------------------------------------------------
//
//       Copyright (c) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       pub/Parser.cpp
//
// Purpose-
//       Implement pub/Parser.h
//
// Last change date-
//       2020/12/26
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <errno.h>                  // For errno
#include <stdio.h>                  // For fopen, fclose, fread

#include "pub/Parser.h"             // Implementation class

namespace pub {
//----------------------------------------------------------------------------
//
// Subroutine-
//       error
//
// Purpose-
//       Write error message.
//
//----------------------------------------------------------------------------
static void
   error(                           // Handle error
     const char*       file,        // The parameter file name
     size_t            line,        // The line number
     const char*       mess)        // The error message
{  fprintf(stderr, "Parser: File(%s) Line(%zd) %s\n", file, line, mess); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       nextc
//
// Purpose-
//       Get next character, ignoring '\r'
//
//----------------------------------------------------------------------------
static int                          // The next non-'\r' character
   nextc(                           // Get next character
     FILE*             F)           // For this FILE
{
   int C= '\r';
   while( C == '\r' )
     C= fgetc(F);

   return C;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       skip_blanks
//
// Purpose-
//       Skip blanks in the parameter file.
//
//----------------------------------------------------------------------------
static int                          // The next non-blank character
   skip_blanks(                     // Skip blanks
     FILE*             F)           // For this FILE
{
   int C= ' ';
   while( C == ' ' )
     C= nextc(F);

   return C;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       skip_line
//
// Purpose-
//       Skip to end of line (or end of file).
//
//----------------------------------------------------------------------------
static int                          // The delimiting character
   skip_line(                       // Skip to end of line
     FILE*             F)           // For this FILE
{
   int C= ' ';
   while( C != '\n' && C != EOF )
     C= nextc(F);

   return C;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Parser::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   Parser::debug( void ) const      // Debuging display
{
   for(Section* S= sect_list.get_head(); S; S= S->get_next()) {
     if( S == nullptr ) break;

     printf("[%s]\n", S->sect_name.c_str());
     for(Parameter* P= S->parm_list.get_head(); P; P= P->get_next()) {
       if( P == nullptr ) break;

       printf("'%s'='%s'\n", P->parm_name.c_str(), P->parm_value.c_str());
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Parser::open
//
// Purpose-
//       Load the parameter file.
//
//----------------------------------------------------------------------------
int                                 // Return code, error counter
   Parser::open(                    // Load the parameter file
     const char*       file_name)   // The parameter file name
{
   close();                         // First, reset the Parser

   int error_count= 0;              // Number of errors encountered

   FILE* F= fopen(file_name, "rb"); // Open the parameter file
   if( F == nullptr )               // If open failure
     return -1;

   errno= 0;                        // (No read errors encountered)
   Section* sect= new Section();    // Create a new (NULL) Section
   sect_list.fifo(sect);            // (Always present, even if empty)

   //-------------------------------------------------------------------------
   // Load the file
   size_t file_line= 0;
   int C= '\n';
   for(;;) {
     if( C == EOF )
       break;
     if( C == '\n' )
       file_line++;
     C= skip_blanks(F);
     if( C == ';' )
       C= skip_line(F);
     if( C == '\n' || C == EOF )
       continue;

     // Handle section definition
     if( C == '[' ) {
       std::string sect_name;
       for(;;) {
         C= nextc(F);
         if( C == ']' )
           break;
         if( C == '\n' || C == EOF ) {
           error_count++;
           error(file_name, file_line, "Malformed section");
           break;
         }

         sect_name += C;
       }
       if( C != ']' )
         continue;

       // Re-use section if present
       for(sect= sect_list.get_head(); sect; sect= sect->get_next()) {
         if( sect->sect_name == sect_name )
           break;
       }

       if( sect == nullptr ) {
         sect= new Section();
         sect->sect_name= sect_name;
         sect_list.fifo(sect);
       }

       C= skip_blanks(F);
       if( C == ';' )
         C= skip_line(F);
       if( C != '\n' && C != EOF ) {
         error_count++;
         error(file_name, file_line, "Characters after [] ignored");
         C= skip_line(F);
       }
       continue;
     }

     // Handle parameter name
     std::string parm_name;
     if( C == '\'' || C == '\"' ) { // If quoted name
       int D= C;                    // Set delimiter
       for(;;) {
         C= nextc(F);
         if( C == D ||  C == '\n' || C == EOF )
           break;
         parm_name += C;
       }
       if( C != D ) {
         error_count++;
         error(file_name, file_line, "Malformed name string");
         continue;
       }
       C= nextc(F);
     } else {                       // If non-quoted string
       for(;;) {
         if( C == ' ' ||  C == '=' || C == '\n' || C == EOF )
           break;
         parm_name += C;

         C= nextc(F);
       }
     }

     if( C == ' ' )
       C= skip_blanks(F);
     if( C == ';' )
       C= skip_line(F);
     if( C != '=' && C != '\n' && C != EOF ) {
       error_count++;
       error(file_name, file_line, "Malformed name");
       C= skip_line(F);
       continue;
     }

     // NOTE: At this point C == '=' || C == '\n' || C == EOF
     if( parm_name == "" ) {
       error_count++;
       error(file_name, file_line, "Missing name");
       if( C != '\n' &&  C != EOF )
         C=  skip_line(F);
       continue;
     }

     // Look for duplicated parameter
     Parameter* parm= nullptr;
     for(parm= sect->parm_list.get_head(); parm; parm= parm->get_next() ) {
       if( parm->parm_name == parm_name )
         break;
     }

     if( parm ) {
       parm_name= "Duplicate parameter(" + parm_name + ") ignored";
       error_count++;
       error(file_name, file_line, parm_name.c_str());
       if( C == '=' )
         C=  skip_line(F);
       continue;
     }

     parm= new Parameter();
     parm->parm_name= parm_name;
     sect->parm_list.fifo(parm);
     if( C != '=' )
       continue;

     C= skip_blanks(F);
     if( C == ';' )
       C= skip_line(F);
     if( C == '\n' ||  C == EOF )
       continue;

     // Handle parameter value
     std::string parm_value;
     if( C == '\'' || C == '\"' ) { // If quoted name
       int D= C;                    // Set delimiter
       for(;;) {
         C= nextc(F);
         if( C == D ||  C == '\n' || C == EOF )
           break;
         parm_value += C;
       }
       if( C != D ) {
         error_count++;
         error(file_name, file_line, "Malformed value string");
         continue;
       }
       C= nextc(F);
     } else {                       // If non-quoted string
       for(;;) {
         if( C == ' ' || C == '\n' || C == EOF )
           break;
         parm_value += C;

         C= nextc(F);
       }
     }
     parm->parm_value= parm_value;

     // Skip to end of line (with checking)
     if( C == ' ' )
       C= skip_blanks(F);
     if( C == ';' )
       C= skip_line(F);
     if( C == '\n' || C == EOF )
       continue;

     error_count++;
     error(file_name, file_line, "Malformed value");
     C= skip_line(F);
   }

   int rc= fclose(F);
   if( rc < 0 )
     error_count= rc;

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Parser::close
//
// Purpose-
//       Reset the Parser.
//
//----------------------------------------------------------------------------
void
   Parser::close( void )            // Reset the Parser
{
   for(;;) {                        // Delete all Sections
     Section* sect= sect_list.remq();
     if( sect == nullptr ) break;

     for(;;) {                      // Delete all the Section's Parameters
       Parameter* parm= sect->parm_list.remq();
       if( parm == nullptr ) break;

       delete parm;
     }

     delete sect;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Parser::getValue
//
// Purpose-
//       Extract a parameter value.
//
//----------------------------------------------------------------------------
const char*                         // The parameter value
   Parser::get_value(               // Extract parameter value
     const char*       sect_name,   // The section name
     const char*       parm_name)   // The parameter name
{
   if( sect_name == nullptr )
     sect_name= "";

   Section* sect= nullptr;
   for(sect= sect_list.get_head(); sect; sect= sect->get_next()) {
     if( sect->sect_name == sect_name )
       break;
   }
   if( sect == nullptr )
     return nullptr;
   if( parm_name == nullptr )       // If section test
     return "";

   Parameter* parm= nullptr;
   for(parm= sect->parm_list.get_head(); parm; parm= parm->get_next()) {
     if( parm->parm_name == parm_name )
       break;
   }
   if( parm )
     return parm->parm_value.c_str();

   return nullptr;
}
}  // namespace pub
