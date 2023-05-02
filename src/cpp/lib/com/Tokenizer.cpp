//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Tokenizer.cpp
//
// Purpose-
//       Tokenizer object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include <com/define.h>
#include <com/Debug.h>

#include "com/Tokenizer.h"

//----------------------------------------------------------------------------
//
// Method-
//       Tokenizer::~Tokenizer
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Tokenizer::~Tokenizer( void )    // Destructor
{
   if( delim != NULL )
   {
     free(delim);
     delim= NULL;
   }

   if( string != NULL )
   {
     free(string);
     string= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Tokenizer::Tokenizer
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   Tokenizer::Tokenizer(            // Constructor
     const char*       string)      // The source string
:  delim(NULL)
,  length(0)
,  offset(0)
,  string(NULL)
{
   this->string= strdup(string);
   length= strlen(this->string);

   while( offset < length && this->string[offset] == ' ' )
     offset++;
}

   Tokenizer::Tokenizer(            // Constructor
     const char*       string,      // The source string
     const char*       delim)       // The token delimiter
:  delim(NULL)
,  length(0)
,  offset(0)
,  string(NULL)
{
   this->string= strdup(string);
   if( delim != NULL )
     this->delim= strdup(delim);
   length= strlen(this->string);
}

//----------------------------------------------------------------------------
//
// Method-
//       Tokenizer::nextToken
//
// Purpose-
//       Return the next token.
//
//----------------------------------------------------------------------------
const char*                         // The next token, NULL if none
   Tokenizer::nextToken( void )     // Get next token
{
   char*               result= NULL;// Resultant

   if( offset < length )
   {
     result= string + offset;

     if( delim == NULL )
     {
       while( offset < length && string[offset] != ' ' )
         offset++;

       string[offset++]= '\0';

       while( offset < length && string[offset] == ' ' )
         offset++;
     }
     else
     {
       char* ending= strstr(result, delim);
       if( ending == NULL )
         offset= length;
       else
       {
         *ending= '\0';
         offset= (ending - string) + strlen(delim);
       }
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Tokenizer::remainder
//
// Purpose-
//       Return the remaining string.
//
//----------------------------------------------------------------------------
const char*                         // The remaining string, NULL if none
   Tokenizer::remainder( void )     // Get remaining string
{
   const char*         result= NULL;// Resultant

   if( offset < length )
     result= string + offset;

   return result;
}

