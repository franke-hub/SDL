//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       Parser.cpp
//
// Purpose-
//       Parse an input file given a set of rules.
//
// Last change date-
//       2007/01/01
//
// Usage-
//       cat input-file | parse "rule-specifier" {-O "output-specifier"}
//
//       The rule specifier is as follows:
//       Default for rule separator should be by word.
//           Use 'word' to extract within words.
//
//       Examples:
//-------
//           stdin: "Mary had a little lamb."
//         Command: parse {who} was     {what} -O who='{who}', what='{what}'
//          Result: (failure); no match for 'was'
//
//         Command: parse {who} had     {what} -O who='{who}', what='{what}'
//          Result: who='Mary', what='a little lamb.'
//
//         Command: parse {who} had .   {what} -O who='{who}', what='{what}'
//          Result: who='Mary', what='little lamb.'
//
//         Command: parse {who} had ..  {what} -O who='{who}', what='{what}'
//          Result: who='Mary', what='lamb.'
//
//         Command: parse {who} had ... {what} -O who='{who}', what='{what}'
//          Result: who='Mary', what=''
//
//-------
//           stdin: type{a} = "<alpha-target> this that other"\n
//                  type{b} = "<beta-target> this that other"\n
//                  type{c} = "<gamma-target> this that other"\n
//         Command: parse type{a} . '<' {target} '>' {caps} '\n' -O {target}
//          Result: alpha-target
//
//         Command: parse type{b} . '<' {target} '>' {caps} '\n' -O {target}
//          Result: beta-target
//
//         Command: parse type{d} . '<' {target} '>' {caps} '\n' -O {target}
//          Result: (failure)
//
//                  These are OK because 'type{x}' does not BOTH
//                  begin with '{' and end with '}'.
//                  '{{' and '}}' not needed.
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/define.h>
#include <com/params.h>
#include <com/Unconditional.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "PARSER  " // File name, for messages

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define STDERR stderr
#ifdef  HCDM
#undef  STDERR
#define STDERR stdout
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define BUFF_SIZE             65536 // Buffer segment size, in bytes
#define HASH_SIZE               128 // The size of the symbol hash table
#define RULE_SIZE              8192 // Rule size, in bytes

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static int           verbose;       // Verbosity
static char          inpRule[RULE_SIZE]; // Parsing rule
static char          outRule[RULE_SIZE]; // Result rule
static char          tString[RULE_SIZE]; // Temporary string

//----------------------------------------------------------------------------
//
// Struct-
//       BufferSegment
//
// Purpose-
//       Contain part of a buffer.
//
//----------------------------------------------------------------------------
struct BufferSegment                // A buffer segment
{
   BufferSegment*    next;          // The next BufferSegment in the buffer
   unsigned          size;          // The number of valid bytes
   char*             data[BUFF_SIZE]; // The actual data
}; // struct BufferSegment

//----------------------------------------------------------------------------
//
// Struct-
//       Symbol
//
// Purpose-
//       Define a symbol.
//
//----------------------------------------------------------------------------
struct Symbol                       // Symbol
{
   Symbol*           next;          // The next Symbol on the hash list
   const char*       name;          // The Symbol's name
   const char*       value;         // The Symbol's value
}; // struct Symbol

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static Symbol*       symbolHash[HASH_SIZE]; // The Symbol (hash) table
static Symbol        dotSymbol= {NULL, ".", NULL}; // The '.' Symbol

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugSymbolHash
//
// Purpose-
//       Display the Symbol table.
//
//----------------------------------------------------------------------------
void                                // (Conditionally used)
   debugSymbolHash( void )          // Display the Symbol table
{
   Symbol*           ptrSymbol;     // -> Symbol
   const char*       ptrC;

   int               i;

   fprintf(STDERR, "Symbol table:\n");
   for(i=0; i<HASH_SIZE; i++)
   {
     ptrSymbol= symbolHash[i];
     while( ptrSymbol != NULL )
     {
       ptrC= ptrSymbol->value;
       if( ptrC == NULL )
         ptrC= "";
       fprintf(STDERR, "'%s'='%s'\n", ptrSymbol->name, ptrC);

       ptrSymbol= ptrSymbol->next;
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       isSpace
//
// Purpose-
//       Is this character a whitespace character?
//
//----------------------------------------------------------------------------
static inline int                   // TRUE if whitespace
   isSpace(                         // Is this a whitespace character?
     int             c)             // The character to check
{
   if( c == ' '
       || c == '\t'
       || c == '\n'
       || c == '\r' )
     return TRUE;

   return FALSE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       findBlank
//
// Purpose-
//       Find the next whitespace character in a string.
//
//----------------------------------------------------------------------------
static int                          // The actual character
   findBlank(                       // Find next whitespace character
     const char*&    string)        // In this string
{
   while( !isSpace(*string)
            && *string != '\0' )
     string++;

   return *string;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       hashf
//
// Purpose-
//       Get hash value for (Symbol) name.
//
//----------------------------------------------------------------------------
static unsigned                     // The hash index
   hashf(                           // Get hash value
     const char*     name)          // For this (Symbol) name
{
   unsigned          result;        // Resultant

   result= 0;
   while( *name != '\0' )
   {
     result <<= 6;
     result  += (*name);

     name++;
   }

   result %= HASH_SIZE;
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       skipBlank
//
// Purpose-
//       Skip whitespace characters in a string.
//
//----------------------------------------------------------------------------
static int                          // The next character
   skipBlank(                       // Skip whitespace characters
     const char*&    string)        // In this string
{
   while( isSpace(*string) )
     string++;

   return *string;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter fault exit.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Parameter fault exit
{
   fprintf(STDERR, "Parse {controls} \"input-rule\" {-O \"output-rule\"}\n");
   fprintf(STDERR, "\n");
   fprintf(STDERR, "Controls:\n");
   fprintf(STDERR, "  (To be determined.)\n");
   fprintf(STDERR, "\n");
   fprintf(STDERR, "input-rule\n");
   fprintf(STDERR, "  The parsing rule.\n");
   fprintf(STDERR, "  (To be determined.)\n");
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   char*             argp;          // Argument pointer
   int               argi;          // Argument index

   int               error;         // Error encountered indicator
   int               ruleX;         // Rule index
   int               ruleMode;      // Rule mode

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no errors found
   verbose= 0;                      // Default, no verbosity
   inpRule[0]= '\0';                // Default, no input rules
   outRule[0]= '\0';                // Default, no output rules
   ruleMode= 0;                     // Processing input rules

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-'               // If this parameter is in switch format
         && ruleMode == 0 )         // but not in output mode
     {
       argp++;                      // Skip over the switch char

       if( swname("O", argp) )      // If beginning output rules
         ruleMode= 1;

       else if( swname("D", argp) ) // If Debug mode
         verbose= swatob("D", argp);

       else if( swname("D:", argp) )// If Debug mode
         verbose= swatol("D:", argp);

       else                         // If invalid switch
       {
         error= TRUE;
         fprintf(STDERR, "Invalid parameter '%s'\n",
                         argv[argi]);
       }
     }
     else                           // If rule fragment
     {
       if( ruleMode > 0 )           // If processing output rules
       {
         ruleX= strlen(outRule);    // Current rule index
         if( (ruleX+strlen(argp)+2) >= sizeof(outRule) )
         {
           error= TRUE;
           fprintf(STDERR, "Too many rules: '%s'\n", argp);
           continue;
         }

         if( ruleX != 0 )           // If not the first rule
           strcat(outRule, " ");

         strcat(outRule, argp);
       }
       else                         // If processing input rules
       {
         ruleX= strlen(inpRule);    // Current rule index
         if( (ruleX+strlen(argp)+2) >= sizeof(inpRule) )
         {
           error= TRUE;
           fprintf(STDERR, "Too many rules: '%s'\n", argp);
           continue;
         }

         if( ruleX != 0 )           // If not the first rule
           strcat(inpRule, " ");

         strcat(inpRule, argp);
       }
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   ruleX= strlen(inpRule);          // Current rule size
   if( ruleX == 0 )                 // If no rule specified
   {
     error= TRUE;
     fprintf(STDERR, "No rule specified\n");
   }

   if( error )                      // If error encountered
     info();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize.
//
//----------------------------------------------------------------------------
void
   init( void )                     // Initialize
{
   int               i;

   for(i=0; i<HASH_SIZE; i++)
     symbolHash[i]= NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       load
//
// Purpose-
//       Load the input file into a buffer.
//
// Testing-
//       CERTIFIED.
//
//----------------------------------------------------------------------------
char*                               // The resultant (contiguous) buffer
   load(                            // Load the input file
     FILE*           h)             // The file handle
{
   char*             result;        // Resultant contiguous buffer
   BufferSegment*    head;          // The first BufferSegment
   BufferSegment*    tail;          // The current BufferSegment
   unsigned          size;          // Total buffer size

   head= (BufferSegment*)must_malloc(sizeof(BufferSegment));
   tail= head;
   size= 0;
   for(;;)
   {
     tail->next= NULL;
     for(;;)                        // Read with retry recovery
     {
       errno= 0;                    // Default, no error
       tail->size= fread(tail->data, 1, sizeof(tail->data), h);
       if( tail->size > 0 || errno != EAGAIN )
         break;
     }

     if( tail->size == 0 )          // If end of file
       break;                       // Done

     size += tail->size;            // Count this element

     tail->next= (BufferSegment*)must_malloc(sizeof(BufferSegment));
     tail= tail->next;
   }

   result= (char*)must_malloc(size+1);// Allocate a contiguous buffer
   size= 0;
   while( head != NULL )
   {
     if( head->size > 0 )
     {
       memcpy(&result[size], head->data, head->size);
       size += head->size;
     }

     tail= head;
     head= tail->next;
     free(tail);
   }

   result[size]= '\0';
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       getQuoted
//
// Purpose-
//       Extract a quoted string.
//
//----------------------------------------------------------------------------
static char*                        // Resultant
   getQuoted(                       // Extract quoted string
     const char*&    inpstr,        // String
     char*           outstr)        // Resultant
{
   char*             result= outstr;// Resultant
   int               delim;         // Delimiter

   delim= *inpstr;                  // Get the delimiter
   inpstr++;
   while( *inpstr != delim )        // Find the delimiter
   {
     if( *inpstr == '\0' )          // If end of string instead
       break;

     *outstr= *inpstr;              // Copy the character
     outstr++;
     inpstr++;
   }

   *outstr= '\0';

   #ifdef HCDM
     fprintf(STDERR, "Quoted(%s)\n", result);
   #endif

   if( strcmp(result, "\\n") == 0 )
     strcpy(result, "\n");

   else if( strcmp(result, "\\t") == 0 )
     strcpy(result, "\t");

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       getWordof
//
// Purpose-
//       Extract a word from a string.
//
//----------------------------------------------------------------------------
static char*                        // Resultant
   getWordof(                       // Extract quoted string
     const char*&    inpstr,        // String
     char*           outstr)        // Resultant
{
   char*             result= outstr;// Resultant

   while( !isSpace(*inpstr) )       // Find the delimiter
   {
     if( *inpstr == '\0' )          // If end of string instead
       break;

     *outstr= *inpstr;              // Copy the character
     outstr++;
     inpstr++;
   }

   *outstr= '\0';
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       getSymbol
//
// Purpose-
//       Return the Symbol matching the string.
//
//----------------------------------------------------------------------------
static Symbol*                      // -> Symbol descriptor
   getSymbol(                       // Get associated Symbol
     const char*     name)          // The Symbol's name
{
   Symbol*           ptrSymbol;     // -> Symbol
   unsigned          H;             // Hash index

   H= hashf(name);
   ptrSymbol= symbolHash[H];
   while( ptrSymbol != NULL )
   {
     if( strcmp(ptrSymbol->name, name) == 0 )
       break;

     ptrSymbol= ptrSymbol->next;
   }

   return ptrSymbol;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       setSymbol
//
// Purpose-
//       Set Symbol value.
//
//----------------------------------------------------------------------------
static Symbol*                      // -> Symbol descriptor
   setSymbol(                       // Get associated Symbol
     const char*     name,          // The Symbol's name
     const char*     value)         // The Symbol's value
{
   char*             ptrC;          // -> char
   Symbol*           ptrSymbol;     // -> Symbol
   unsigned          H;             // Hash index

   H= hashf(name);
   ptrSymbol= symbolHash[H];
   while( ptrSymbol != NULL )
   {
     if( strcmp(ptrSymbol->name, name) == 0 )
       break;

     ptrSymbol= ptrSymbol->next;
   }

   if( ptrSymbol != NULL )
   {
     if( ptrSymbol->value != NULL )
       free((void*)ptrSymbol->value);

     ptrSymbol->value= NULL;
   }
   else
   {
     ptrSymbol= (Symbol*)must_malloc(sizeof(Symbol));
     memset(ptrSymbol, 0, sizeof(Symbol));

     ptrC= (char*)must_malloc(strlen(name)+1);
     strcpy(ptrC, name);
     ptrSymbol->name= ptrC;

     ptrSymbol->next= symbolHash[H];
     symbolHash[H]= ptrSymbol;
   }

   if( value != NULL )
   {
     ptrC= (char*)must_malloc(strlen(value)+1);
     strcpy(ptrC, value);
     ptrSymbol->value= ptrC;
   }

   return ptrSymbol;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       setSymbolValue
//
// Purpose-
//       Set Symbol value.
//
//----------------------------------------------------------------------------
static void
   setSymbolValue(                  // Get associated Symbol
     Symbol*         ptrSymbol,     // -> Symbol
     const char*     value,         // The Symbol's value (origin)
     const char*     limit)         // The Symbol's value (limit)
{
   char              tString[RULE_SIZE]; // Temporary string
   int               L;

   L= limit-value;
   memcpy(tString, value, limit-value);
   tString[L]= '\0';
   while( tString[L-1] == ' ' )     // Remove trailing blanks
     tString[L-1]= '\0';

   setSymbol(ptrSymbol->name, tString);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       isSymbol
//
// Purpose-
//       Determine whether the next string word is a symbol name.
//
//----------------------------------------------------------------------------
static const char*                  // -> Symbol name
   isSymbol(                        // Is the next string word a Symbol name
     const char*     inpString)     // The string to examine
{
   const char*       string= inpString; // -> string

   if( *string == '.' )             // If special symbol
     return ".";                    // Return its name

   if( *string != '{' )             // If no begin marker
     return NULL;                   // Not a symbol

   findBlank(string);
   string--;

   if( *string != '}' )             // If no end marker
     return NULL;                   // Not a symbol

   memcpy(tString, inpString+1, string-inpString);
   tString[string-inpString-1]= '\0';

   return tString;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parse
//
// Purpose-
//       Parse buffer according to specified rules.
//
//----------------------------------------------------------------------------
static int                          // Return code
   parse(                           // Parse buffer using rules
     const char*     buff,          // The parse buffer
     const char*     rule)          // The parse rules
{
   const char*       ptrC;          // -> String
   const char*       ptrName;       // -> Symbol Name
   const char*       ptrValue;      // -> Symbol Value
   Symbol*           ptrSymbol;     // -> Symbol

inString:
   #ifdef HCDM
     printf("INSTRING:\n");
     printf(">>>>RULE(%s)\n", rule);
     printf(">>>>BUFF:\n"
            "%s"
            "<<<<\n", buff);
   #endif

   if( isSpace(*buff) && isSpace(*rule) )
     skipBlank(buff);

   skipBlank(rule);

   if( *rule == '\0' )              // If end of RULE
     return 0;

   if( *rule == '\'' || *rule == '\"' ) // If begin quoted string
   {
     getQuoted(rule, tString);
     if( *rule == '\0' )
     {
       if( verbose > 0 )
         fprintf(STDERR, "Mismatched quotes(%s)\n", tString);
       return 1;
     }
     rule++;

     buff= strstr(buff, tString);
     if( buff == NULL )
     {
       if( verbose > 0 )
         fprintf(STDERR, "Not found: '%s'\n", tString);
       return 1;
     }

     buff += strlen(tString);
     goto inString;
   }

   ptrName= isSymbol(rule);
   if( ptrName == NULL )
   {
     skipBlank(buff);
     getWordof(rule, tString);
     if( memcmp(buff, tString, strlen(tString)) != 0 )
     {
       if( verbose > 0 )
         fprintf(STDERR, "Not found: '%s'\n", tString);
       return 1;
     }
     skipBlank(rule);

     buff += strlen(tString);
     skipBlank(buff);
     goto inString;
   }

   //-------------------------------------------------------------------------
   // Found a new symbol
   //-------------------------------------------------------------------------
newSymbol:
   if( strcmp(ptrName, ".") == 0 )
   {
     rule++;
     ptrSymbol= &dotSymbol;
   }
   else
   {
     rule += strlen(ptrName) + 2;   // Skip the name in the RULE
     ptrSymbol= setSymbol(ptrName, NULL); // Define the (empty) Symbol
   }

   if( isSpace(*buff) && isSpace(*rule) )
     skipBlank(buff);

   ptrValue= buff;                  // Save the Symbol value origin

   #ifdef HCDM
     printf("INSYMBOL(%s):\n", ptrSymbol->name);
     printf("    RULE(%s)\n", rule);
     printf(">>>>BUFF:\n"
            "%s"
            "<<<<\n", buff);
   #endif

   if( isSpace(*buff) && isSpace(*rule) )
     skipBlank(buff);

   skipBlank(rule);

   if( *rule == '\0' )              // If end of RULE
   {
     setSymbol(ptrSymbol->name, ptrValue);
     return 0;
   }

   if( *rule == '\'' || *rule == '\"' ) // If begin quoted string
   {
     getQuoted(rule, tString);
     if( *rule == '\0' )
     {
       if( verbose > 0 )
         fprintf(STDERR, "Mismatched quotes(%s)\n", tString);
       return 1;
     }
     rule++;

     buff= strstr(buff, tString);
     if( buff == NULL )
     {
       if( verbose > 0 )
         fprintf(STDERR, "Not found: '%s'\n", tString);
       return 1;
     }

     setSymbolValue(ptrSymbol, ptrValue, buff);
     buff += strlen(tString);
     goto inString;
   }

   ptrName= isSymbol(rule);
   if( ptrName == NULL )
   {
     getWordof(rule, tString);
     skipBlank(buff);
     ptrC= buff;
     buff= strstr(ptrC, tString);
     if( buff == NULL
         || (buff != ptrC && !isSpace(*(buff-1))) )
     {
       if( verbose > 0 )
         fprintf(STDERR, "Not found: '%s'\n", tString);
       return 1;
     }

     setSymbolValue(ptrSymbol, ptrValue, buff);
     buff += strlen(tString);
     goto inString;
   }

   setSymbolValue(ptrSymbol, ptrValue, buff);
   goto newSymbol;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       write
//
// Purpose-
//       Write buffer according to specified rules.
//
//----------------------------------------------------------------------------
static int                          // Return code
   write(                           // Write buffer using rules
     const char*     rule)          // The output rules
{
   char*             ptrName;       // -> Symbol Name
   Symbol*           ptrSymbol;     // -> Symbol

inString:
   while( *rule != '{' )
   {
     if( *rule == '\0' )
       return 0;

     printf("%c", *rule);
     rule++;
   }
   rule++;

   if( *rule == '{' )
   {
     rule++;
     printf("{");
     goto inString;
   }

   ptrName= tString;
   while( *rule != '}' )
   {
     if( *rule == '\0' )
     {
       printf(tString);
       return 0;
     }

     *ptrName= *rule;
     ptrName++;
     rule++;
   }
   rule++;

   *ptrName= '\0';
   ptrSymbol= getSymbol(tString);   // Locate the Symbol
   if( ptrSymbol == NULL )
   {
     printf("{%s}", tString);
     goto inString;
   }

   if( ptrSymbol->value != NULL )
     printf("%s", ptrSymbol->value);

   goto inString;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
int                                 // Main return code
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   int               returncd;      // This routine's return code
   char*             source;        // Source buffer

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   init();

   //-------------------------------------------------------------------------
   // Test
   //-------------------------------------------------------------------------
   source= load(stdin);
   returncd= parse(source, inpRule);

   #ifdef HCDM
     debugSymbolHash();
     printf("::::(%d)\n", returncd);
   #endif

   if( returncd == 0 )
   {
     returncd= write(outRule);
     printf("\n");
   }

   return returncd;
}
