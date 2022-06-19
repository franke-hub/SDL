//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Parser.cpp
//
// Purpose-
//       Parser implementation methods.
//
// Last change date-
//       2022/06/18
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
// Method-
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
// Method-
//       Parser::error
//
// Purpose-
//       Write error message.
//
//----------------------------------------------------------------------------
void
   Parser::error(                   // Handle error
     size_t            line,        // The line number
     const char*       mess)        // The error message
{
   if( error_count++ == 0 )
     fprintf(stderr, "Parser: File(%s)\n", file_name.c_str());

   fprintf(stderr, "Line(%3zd) %s\n", line, mess);
}


//----------------------------------------------------------------------------
//
// Method-
//       Parser::open
//
// Purpose-
//       Load the parameter file.
//
//----------------------------------------------------------------------------
long                                // Return code, error counter
   Parser::open(                    // Load the parameter file
     const char*       name)        // The Parser file name
{
   close();                         // First, reset the Parser

   error_count= 0;                  // Number of errors encountered
   file_name= name ? name : "";     // Save file name
   if( name == nullptr )
     return 0;

   FILE* F= fopen(name, "rb");      // Open the parameter file
   if( F == nullptr ) {             // If open failure
     error(-1, "Open failure");
     return error_count;
   }

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
     if( false ) {                  // Debugging (optimizer omits code)
       char buff[4];
       buff[0]= buff[1]= buff[2]= buff[3]= '\0';
       buff[0]= C;
       if( C == '\n' ) { buff[0]= '\\'; buff[1]= 'n'; }
       if( C == '\t' ) { buff[0]= '\\'; buff[1]= 't'; }
       printf("%4zd C(%s)\n", file_line, buff);
     }

     if( C == ';' )
       C= skip_line(F);
     if( C == '\n' )
       file_line++;
     C= skip_blanks(F);
     if( C == ';' || C == '\n' || C == EOF )
       continue;

     // Handle section definition
     if( C == '[' ) {
       std::string sect_name;
       for(;;) {
         C= nextc(F);
         if( C == ']' )
           break;
         if( C == '\n' || C == EOF ) {
           error(file_line, "Malformed section");
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

       continue;
     }

     // Handle parameter name
     std::string parm_name;
     if( C == '\'' || C == '\"' ) { // If quoted name
       int D= C;                    // Set delimiter
       for(;;) {
         C= nextc(F);
         if( C == D || C == '\n' || C == EOF )
           break;
         parm_name += C;
       }
       if( C != D ) {
         error(file_line, "Malformed name string");
         continue;
       }
       C= skip_blanks(F);
     } else {                       // If non-quoted string
       for(;;) {
         if( C == '=' || C == ';' || C == '\n' || C == EOF )
           break;
         parm_name += C;

         C= nextc(F);
       }

       size_t L= parm_name.length();
       while( L && parm_name[--L] == ' ' )
         parm_name= parm_name.substr(0, L);
     }

     if( C == ';' )
       C= skip_line(F);
     if( C != '=' && C != '\n' && C != EOF ) {
       error(file_line, "Malformed name");
       C= skip_line(F);
       continue;
     }

     // NOTE: At this point C == '=' || C == '\n' || C == EOF
     if( parm_name == "" ) {
       error(file_line, "Missing name");
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
       if( C != '=' ) {
         error(file_line, "Use 'parameter=' to remove value");
         continue;
       }
     } else {
       parm= new Parameter();
       parm->parm_name= parm_name;
       sect->parm_list.fifo(parm);
     }

     C= skip_blanks(F);
     if( C == ';' )
       C= skip_line(F);
     if( C == '\n' || C == EOF ) {
       parm->parm_value= "";
       continue;
     }

     // Handle parameter value
     std::string parm_value;
     if( C == '\'' || C == '\"' ) { // If quoted name
       int D= C;                    // Set delimiter
       for(;;) {
         C= nextc(F);
         if( C == D || C == '\n' || C == EOF )
           break;
         parm_value += C;
       }
       if( C != D ) {
         error(file_line, "Malformed value string");
         continue;
       }
       C= nextc(F);
     } else {                       // If non-quoted string
       for(;;) {
         if( C == ';' || C == '\n' || C == EOF )
           break;
         parm_value += C;

         C= nextc(F);
       }

       while( parm_value.back() == ' ' ) // Remove trailing blanks
         parm_value= parm_value.substr(0, parm_value.length() - 1);
     }
     parm->parm_value= parm_value;
   }

   int rc= fclose(F);
   if( rc < 0 )
     error(file_line, "Close failure");
   if( error_count )
     fprintf(stderr, "%ld Parse error%s encountered\n", error_count
                   , error_count == 1 ? "" : "s");

   return error_count;
}

//----------------------------------------------------------------------------
//
// Method-
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
// Method-
//       Parser::get_next
//
// Purpose-
//       Get next section or parameter name (for iteration.)
//
// Implementation notes-
//       Users MUST NOT modify (or free) the returned parameter value.
//
//----------------------------------------------------------------------------
const char*                         // The next parameter name
   Parser::get_next(                // Get next parameter name
     const char*       sect_name,   // The current section name
     const char*       parm_name) const // The current parameter name
{
   Section* sect= nullptr;
   if( sect_name ) {
     for(sect= sect_list.get_head(); sect; sect= sect->get_next()) {
       if( sect->sect_name == sect_name )
         break;
     }
   } else
     sect= sect_list.get_head();

   if( sect ) {
     Parameter* parm= nullptr;
     if( parm_name ) {
       for(parm= sect->parm_list.get_head(); parm; parm= parm->get_next()) {
         if( parm->parm_name == parm_name )
           break;
       }
       if( parm )
         parm= parm->get_next();
     } else
       parm= sect->parm_list.get_head();

     if( parm )
       return parm->parm_name.c_str();
   }

   return nullptr;
}

const char*                         // The next section name
   Parser::get_next(                // Get next section name
     const char*       sect_name) const // The current section name
{
   Section* sect= nullptr;
   if( sect_name ) {
     for(sect= sect_list.get_head(); sect; sect= sect->get_next()) {
       if( sect->sect_name == sect_name )
         break;
     }
     if( sect )
       sect= sect->get_next();
   } else
     sect= sect_list.get_head();

   if( sect )
     return sect->sect_name.c_str();

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       Parser::get_value
//
// Purpose-
//       Extract a parameter value.
//
//----------------------------------------------------------------------------
const char*                         // The parameter value
   Parser::get_value(               // Get parameter value
     const char*       sect_name,   // The section name
     const char*       parm_name) const // The parameter name
{
   Section* sect= nullptr;
   if( sect_name ) {
     for(sect= sect_list.get_head(); sect; sect= sect->get_next()) {
       if( sect->sect_name == sect_name )
         break;
     }
   } else
     sect= sect_list.get_head();

   if( sect ) {
     Parameter* parm= nullptr;
     if( parm_name ) {
       for(parm= sect->parm_list.get_head(); parm; parm= parm->get_next()) {
         if( parm->parm_name == parm_name )
           break;
       }
     } else
       parm= sect->parm_list.get_head();

     if( parm )
       return parm->parm_value.c_str();
   }

   return nullptr;
}
}  // namespace pub
