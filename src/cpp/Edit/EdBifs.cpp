//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2016 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdBifs.cpp
//
// Purpose-
//       Editor built-in functions.
//
// Last change date-
//       2016/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <com/istring.h>
#include <com/Parser.h>

#include "Editor.h"

#include "Active.h"
#include "EdLine.h"
#include "EdRing.h"
#include "EdView.h"
#include "Status.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define MAX_PARAM              2048 // Largest parameter string length

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define DIM(x) (sizeof(x)/sizeof(x[0]))
#ifdef _OS_WIN
  #define WEXITSTATUS(rc) 0
#else
  #include <sys/wait.h>
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       findBlank
//
// Purpose-
//       Find next whitespace in string
//
//----------------------------------------------------------------------------
static const char*                  // Next whitespace character
   findBlank(                       // Find next whitespace character
     const char*       text)        // In this string
{
   Parser parser(text);
   return parser.findSpace();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       skipBlank
//
// Purpose-
//       Find next non-whitespace in string
//
//----------------------------------------------------------------------------
static const char*                  // Next non-whitespace character
   skipBlank(                       // Find next non-whitespace character
     const char*       text)        // In this string
{
   Parser parser(text);
   return parser.skipSpace();
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::parser
//
// Purpose-
//       Parameter parser, extract next parameter.
//
//----------------------------------------------------------------------------
const char*                         // Updated position pointer
   Editor::parser(                  // Command parser
     const char*       ptrchs,      // Current position
     char*             string)      // Output string [length <= MAX_PARAM]
{
   const char*         origin;      // String origin
   unsigned            length;      // String length

   //-------------------------------------------------------------------------
   // Insure that parameters exist
   //-------------------------------------------------------------------------
   string[0]= '\0';                 // No parameter
   ptrchs= skipBlank(ptrchs);       // Skip leading blanks
   if( *ptrchs == '\0' )            // If no parameters
     return ptrchs;

   //-------------------------------------------------------------------------
   // Extract the parameter
   //-------------------------------------------------------------------------
   origin= ptrchs;                  // String origin
   if( *origin == '\"' )            // If quoted string
   {
     origin++;
     ptrchs= origin;
     while( *ptrchs != '\"' && *ptrchs != '\0' )
       ptrchs++;
     if( *ptrchs == '\0' )
     {
       warning("Syntax error: unmatched quote");
       return ptrchs;
     }
     length= ptrchs - origin;       // String length
     ptrchs++;                      // Skip the quote
     if( *ptrchs != ' ' && *ptrchs != '\0' )
     {
       warning("Syntax error: missing blank");
       return ptrchs;
     }
   }
   else
   {
     ptrchs= findBlank(ptrchs);     // End of parameter
     length= ptrchs - origin;       // String length
   }

   if( length > (MAX_PARAM-1) )
     length= MAX_PARAM-1;
   memcpy(string, origin, length);  // Set the resultant
   string[length]= '\0';            // Terminate the string
   return ptrchs;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_bot
//
// Purpose-
//       Goto end of file.
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_bot(                     // End of file
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   edit->activate(edit->dataView->moveLast());
   edit->dataView->column(0);
   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_change
//
// Purpose-
//       Change command.
//
// Format-
//       c /from string/to string/<ignored>
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_change(                  // Change command
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   const char*         result;      // Resultant string
   const char*         endstr;      // The endstring delimiter
   const char*         midstr;      // The midstring delimiter

   int                 c;           // The string delimiter
   int                 l;           // Length value

   c= *ptrchs;                      // Get string delimiter
   if( c == '\0' )                  // If no delimiter
     return "No change string";

   ptrchs++;                        // String origin
   midstr= strchr(ptrchs, c);       // Locate the first delimiter
   if( midstr == NULL )             // If invalid string
     return "Invalid change string";

   midstr++;                        // String origin
   endstr= strchr(midstr, c);       // Locate the last delimiter
   if( endstr == NULL )             // If no delimiter
     endstr= strchr(ptrchs, 0);     // End of string, always present

   //-------------------------------------------------------------------------
   // Set the locate string
   //-------------------------------------------------------------------------
   l= (int)(midstr - ptrchs) - 1;   // String length
   if( l >= Editor::MAX_LOCATE )
     return "String too long";

   memcpy(edit->locateString, ptrchs, l); // Set the string
   edit->locateString[l]= '\0';     // Set the string delimiter
   edit->locateLength= l;           // Set the string length

   //-------------------------------------------------------------------------
   // Set the change string
   //-------------------------------------------------------------------------
   l= (int)(endstr - midstr);       // String length
   if( l >= Editor::MAX_CHANGE )
     return "String too long";

   memcpy(edit->changeString, midstr, l); // Set the string
   edit->changeString[l]= '\0';     // Set the string delimiter
   edit->changeLength= l;           // Set the string length

   //-------------------------------------------------------------------------
   // Perform the change
   //-------------------------------------------------------------------------
   result= edit->change();
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_detab
//
// Purpose-
//       Remove tabs from file
//
// Format-
//       <ignored>
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_detab(                   // Remove tabs from file
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   Active*             active;      // Active line
   EdLine*             line;        // Working line
   char*               text;        // -> Source text
   char*               tabs;        // -> Tab in string
   int                 L;           // Generic length
   EdRing*             ring;        // Working ring

   if( *ptrchs != '\0' )            // If parameter specified
     return "Unexpected parameter";

   //-------------------------------------------------------------------------
   // Remove tabs from the current file
   //-------------------------------------------------------------------------
   ring= edit->dataView->getRing(); // Get the current ring
   for(line= (EdLine*)ring->lineList.getHead();
       line != NULL;
       line= (EdLine*)line->getNext())
   {
     if( line->text != NULL )       // If text exists
     {
       active= NULL;                // No change yet

       text= line->text;
       tabs= strchr(text, '\t');    // Locate the first tab
       while( tabs != NULL )        // Remove tabs from the line
       {
         if( active == NULL )
         {
           active= edit->workActive;
           active->fetch(ring, line);   // Fetch the line
           active->clear(0);
         }

         L= tabs-text;              // Length of substring
         active->appendString(text, L); // Append the text

         L= active->getUsed();
         L= edit->tabRight(L);
         active->expand(L-1);

         text= tabs + 1;            // Skip past tab
         tabs= strchr(text, '\t');  // Locate the next tab
       }

       if( active != NULL )
       {
         active->appendString(text);
         active->store();
       }
     }
   }

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   edit->defer(Editor::RESHOW_ALL);
   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_dos
//
// Purpose-
//       DOS escape command.
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_dos(                     // DOS escape
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   const char*         result;      // Result string
   int                 oldErrno;    // Saved errno
   Terminal*           terminal= edit->getTerminal();

   int                 rc;

   result= NULL;
   terminal->setAttribute(VGAColor::Grey, VGAColor::Black);
   terminal->clearScreen();         // Clear the screen
   terminal->suspend();             // Suspend the terminal
   errno= 0;                        // Clear the error
   rc= system(ptrchs);              // Execute the command
   if( errno != 0 || WEXITSTATUS(rc) != 0 )
     result= "Command failed";
   oldErrno= errno;
   rc= system("echo Press any key to continue");

   printf("\n");
   if( oldErrno != 0 )              // If error
   {
     errno= oldErrno;
     perror(result);
   }
   terminal->rd();                  // Wait for (and ignore) keypress
   terminal->resume();              // Resume terminal operation
   terminal->physicalXY(0,0);       // Reposition the cursor

   edit->defer(Editor::RESHOW_ALL); // Reshow always required
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_edit
//
// Purpose-
//       Edit command.
//
// Format-
//       e filedesc ...
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_edit(                    // Edit command
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   const char*         result;      // Return string

   char                string[MAX_PARAM]; // Parameter string

   //-------------------------------------------------------------------------
   // Parse the argument list
   //-------------------------------------------------------------------------
   ptrchs= skipBlank(ptrchs);       // Skip leading blanks
   if( *ptrchs == '\0' )            // If no parameters
     return "Missing filename";

   //-------------------------------------------------------------------------
   // Load the files, show the last one loaded
   //-------------------------------------------------------------------------
   result= NULL;
   for(;;)                          // Load the files
   {
     ptrchs= edit->parser(ptrchs, string); // Next filename
     if( *string == '\0' )
       break;

     result= edit->insertRing(string); // Load the file
     if( result != NULL )           // If load failure
       break;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_exit
//
// Purpose-
//       Editor exit.
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_exit(                    // Quit this file
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   const char*         result;      // Return message
   EdRing*             ring;        // Working ring

   while( edit->online )            // While files left to edit
   {
     ring= edit->dataView->getRing();
     result= edit->safeExit(ring);  // Updates edit->active, view->getRing()
     if( result != NULL )
       return result;
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_file
//
// Purpose-
//       Write the file and remove the associated ring.
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_file(                    // Write the file and exit
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   const char*         result;      // Return message
   EdRing*             ring= edit->dataView->getRing();

   if( *ptrchs != '\0' )            // If parameter specified
     return "Unexpected parameter";

   if( ring == edit->utilRing || ring == edit->histRing )
     return "Protected";

   result= ring->write();           // Save the file
   if( result == NULL )             // If saved
     edit->removeRing(ring);        // Remove the file

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_get
//
// Purpose-
//       Get (read) a file
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_get(                     // Get a file
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   const char*         result;      // Resultant
   char                workName[FILENAME_MAX+1]; // Resolved name
   EdLine*             line= edit->dataView->getLine();
   EdRing*             ring= edit->dataView->getRing();

   ptrchs= skipBlank(ptrchs);       // Skip over blanks
   if( *ptrchs == '\0'              // If missing filename
       ||*findBlank(ptrchs) != '\0' ) // If multiple filenames
     return "Invalid filename";

   result= FileName::resolve(workName, ptrchs);
   if( result != NULL )
   {
     edit->warning("Error(%s) in(%s)", workName, ptrchs);
     return result;
   }

   result= ring->append(workName, line);
   edit->defer(Editor::RESHOW_BUF);
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_locate
//
// Purpose-
//       Locate command.
//
// Format-
//       l /locate string/<ignored>
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_locate(                  // Locate command
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   const char*         endstr;      // The endstring delimiter

   int                 c;           // The string delimiter
   int                 l;           // Length value

   c= *ptrchs;                      // Get string delimiter
   if( c == '\0' )                  // If no delimiter
     return "No locate string";

   ptrchs++;                        // String origin
   endstr= strchr(ptrchs, c);       // Locate the last delimiter
   if( endstr == NULL )             // If no delimiter
     endstr= strchr(ptrchs, 0);     // End of string, always present

   //-------------------------------------------------------------------------
   // Set the locate string
   //-------------------------------------------------------------------------
   l= (int)(endstr - ptrchs);       // String length
   if( l >= Editor::MAX_LOCATE      // If string too long
       || l >= Editor::MAX_CHANGE )
     return "String too long";

   memcpy(edit->locateString, ptrchs, l); // Set the string
   edit->locateString[l]= '\0';     // Set the string delimiter
   edit->locateLength= l;           // Set the string length

   //-------------------------------------------------------------------------
   // Disable the change string (make it identical to locate string)
   //-------------------------------------------------------------------------
   memcpy(edit->changeString, edit->locateString, l+1); // Set the string
   edit->changeLength= l;           // Set the string length

   //-------------------------------------------------------------------------
   // Perform the locate
   //-------------------------------------------------------------------------
   return edit->locate();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_margins
//
// Purpose-
//       Set margins
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_margins(                 // Set margins
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   const char*         result;      // Return message
   char                string[MAX_PARAM]; // Parameter string
   unsigned int        left;        // Left margin
   unsigned int        right;       // Right margin

   //-------------------------------------------------------------------------
   // Parse the argument list
   //-------------------------------------------------------------------------
   result= "Invalid margins";       // Default error
   errno= 0;
   ptrchs= edit->parser(ptrchs, string);
   if( string[0] == '\0' )
     return result;
   left= atoi(string);

   ptrchs= edit->parser(ptrchs, string);
   ptrchs= skipBlank(ptrchs);
   if( string[0] == '\0' || *ptrchs != '\0' )
     return result;
   right= atoi(string);

   //-------------------------------------------------------------------------
   // Validate the margins
   //-------------------------------------------------------------------------
   if( errno != 0
       ||left >= right )
     return result;

   //-------------------------------------------------------------------------
   // Set the margins
   //-------------------------------------------------------------------------
   edit->marginLeft= left;
   edit->marginRight= right;
   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_mode
//
// Purpose-
//       Set the default insert mode.
//
// Format-
//       mode {bsd | dos | unix}
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_mode(                    // Convert to CR/LF format
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   const char*         result;      // Return message
   EdLine::Delimiter   delimiter;   // The default delimiter
   EdLine*             line;        // Working line pointer
   EdRing*             ring= edit->dataView->getRing();

   result= "Specify DOS or UNIX";   // Default error return message
   ptrchs= skipBlank(ptrchs);       // Skip over blanks
   if( *ptrchs == '\0' )            // If missing mode
     return result;

   if( *findBlank(ptrchs) != '\0' ) // If multiple modes
     return result;

   if( stricmp(ptrchs, "DOS") == 0 )
   {
     ring->mode= EdRing::FM_DOS;
     delimiter= EdLine::DT_CRLF;
   }

   else if( stricmp(ptrchs, "BSD") == 0
            || stricmp(ptrchs, "UNIX") == 0 )
   {
     ring->mode= EdRing::FM_UNIX;
     delimiter= EdLine::DT_LF;
   }
   else
     return result;

   //-------------------------------------------------------------------------
   // Convert all lines to the default format
   //-------------------------------------------------------------------------
   line= (EdLine*)ring->lineList.getHead(); // Top of file line
   for(;;)                          // Set the specified mode
   {
     line= (EdLine*)line->getNext();
     if( line->getNext() == NULL )  // If last line
       break;                       // We're done

     if( line->ctrl.delim != delimiter ) // If wrong delmiter
     {
       line->ctrl.delim= delimiter; // Set default delimiter
       ring->changed= TRUE;         // The file has been changed
     }
   }

   edit->status->defer(Status::RESHOW_ALL);
   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_nop
//
// Purpose-
//       Dummy command.
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_nop(                     // Dummy command
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_number
//
// Purpose-
//       Locate file linenumber
//
// Format-
//       linenumber <ignored>
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_number(                  // Locate line number
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   char                c;           // Current character
   unsigned long       l;           // Linenumber value
   EdLine*             line;        // Working line
   EdRing*             ring;        // Working ring

   ptrchs= skipBlank(ptrchs);       // Skip over blanks

   l= 0;
   c= *ptrchs;                      // Get the first character
   while(c != ' ' && c != '\0')     // Extract the line number
   {
     if( c < '0' || c > '9' )       // If invalid number
       return "Invalid number";

     l= l*10 + (c-'0');
     ptrchs++;
     c= *ptrchs;
   }

   //-------------------------------------------------------------------------
   // Locate the file line
   //-------------------------------------------------------------------------
   ring= edit->dataView->getRing();
   line= (EdLine*)ring->lineList.getHead();
   while( l > 0 && line->getNext() != NULL ) // Locate the row
   {
     line= (EdLine*)line->getNext();
     l--;
   }

   //-------------------------------------------------------------------------
   // Position on the located line
   //-------------------------------------------------------------------------
   edit->activate(line);
   edit->dataView->column(0);
   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_quit
//
// Purpose-
//       Exit the file.
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_quit(                    // Quit this file
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   if( *ptrchs != '\0' )
     return "Unexpected parameter";

   return edit->removeRing(edit->dataView->getRing());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_save
//
// Purpose-
//       Save the file.
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_save(                    // Write the file
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   EdRing*             ring= edit->dataView->getRing();

   if( *ptrchs != '\0' )            // If parameter specified
     return "Unexpected parameter";

   return ring->write();            // Save the file
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_screen
//
// Purpose-
//       Set screen count
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_screen(                  // Set screen count
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   unsigned int        count;       // The screen count
   const char*         result;      // Return message
   char                string[MAX_PARAM]; // Parameter string
   EdView*             view;        // Working View

   //-------------------------------------------------------------------------
   // Parse the argument list
   //-------------------------------------------------------------------------
   result= "Invalid count";         // Default error
   errno= 0;
   ptrchs= edit->parser(ptrchs, string);
   if( string[0] == '\0' )
     return result;
   count= atoi(string);

   //-------------------------------------------------------------------------
   // Validate the margins
   //-------------------------------------------------------------------------
   if( errno != 0
       || count < 1 || count > 4)
     return result;

   //-------------------------------------------------------------------------
   // Set the screen count
   //-------------------------------------------------------------------------
   if( count > edit->viewCount )
   {
     while( count > edit->viewCount )
     {
       try {
         view= new EdView(edit, NULL);
       } catch(...) {
         return "No storage";
       }

       edit->viewList.fifo(view);
       view->activate((EdRing*)edit->ringList.getHead());
       edit->viewCount++;
     }
   }
   else
   {
     for(view= (EdView*)edit->histView->getNext();
         view != NULL;
         view= (EdView*)view->getNext())
       view->setActive(NULL);

     while( edit->viewCount > count )
     {
       view= (EdView*)edit->viewList.getTail();
       edit->viewList.remove(view, view);
       delete view;

       edit->viewCount--;
     }

     edit->dataView= (EdView*)edit->histView->getNext();
     edit->dataView->setActive(edit->dataActive);
   }

   // workView == histView, so it doesn't need updating

   edit->resize();
   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_tabs
//
// Purpose-
//       Set tab stops.
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_tabs(                    // Set tab stops
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   char                string[MAX_PARAM]; // Parameter string
   unsigned int        argc= Editor::MAX_TABS; // Argument count
   unsigned int        argv[Editor::MAX_TABS]; // Argument value array
   unsigned int        t;           // Prior tab stop value
   unsigned int        T;           // This tab stop value

   unsigned int        i;

   //-------------------------------------------------------------------------
   // Parse the argument list
   //-------------------------------------------------------------------------
   ptrchs= skipBlank(ptrchs);       // Skip leading blanks
   if( *ptrchs == '\0' )            // If no parameters
     return "Specify tabs";

   //-------------------------------------------------------------------------
   // Validate the tabs
   //-------------------------------------------------------------------------
   t=0;
   errno= 0;
   for(argc= 0; argc<Editor::MAX_TABS; argc++)
   {
     ptrchs= edit->parser(ptrchs, string);
     if( string[0] == '\0' )
       break;

     T= atoi(string);
     if( errno != 0 || T <= t )
       return "Invalid tabs";

     argv[argc]= T;
   }

   //-------------------------------------------------------------------------
   // Set the tabs
   //-------------------------------------------------------------------------
   edit->tabUsed= argc;
   for(i=0; i<argc; i++)
     edit->tabStop[i]= argv[i];

   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_top
//
// Purpose-
//       Goto top of file.
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_top(                     // Top of file
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   edit->activate(edit->dataView->moveFirst());
   edit->dataView->column(0);
   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtin_undo
//
// Purpose-
//       Undo last delete.
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   builtin_undo(                    // Undo last delete
     Editor*           edit,        // The Editor object
     const char*       ptrchs)      // Command string
{
   if( *ptrchs != '\0' )
     return "Unexpected parameter";

   return edit->undo();
}

//----------------------------------------------------------------------------
// Builtin control table
//----------------------------------------------------------------------------
typedef const char* (Builtin)(Editor*, const char*);
struct Command {
   char            name[9];         // The command name (string)
   Builtin*        function;        // The command function
}; // struct Command

static Command     command[]=       // Command array
{
   {"L       ", &builtin_locate},   // Locate
   {"C       ", &builtin_change},   // Change
   {"E       ", &builtin_edit},     // Edit
   {"EDIT    ", &builtin_edit},     // Edit
   {"DETAB   ", &builtin_detab},    // Remove tabs from file
   {"TOP     ", &builtin_top},      // Top (of file)
   {"BOT     ", &builtin_bot},      // Bot (of file)
   {"EXIT    ", &builtin_exit},     // Exit
   {"FILE    ", &builtin_file},     // File
   {"GET     ", &builtin_get},      // Get
   {"MARGINS ", &builtin_margins},  // Set margins
   {"MODE    ", &builtin_mode},     // Set mode
   {"SAVE    ", &builtin_save},     // Save
   {"SCREEN  ", &builtin_screen},   // Set screen count
   {"TABS    ", &builtin_tabs},     // Set tabs
   {"UNDO    ", &builtin_undo},     // Undo delete
   {"QUIT    ", &builtin_quit},     // Quit
   {"DOS     ", &builtin_dos},      // DOS command
   {"NOP     ", &builtin_nop},      // NOP (used for testing)
};

//----------------------------------------------------------------------------
//
// Method-
//       Editor::execute
//
// Purpose-
//       Execute a command.
//
// Notes-
//       This modifies the histActive line, then fetches it again.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Editor::execute( void )          // Execute command
{
   const char*         result;      // Resultant
   char                cmdname[8];  // The command name
   const char*         ptrcmd;      // -> Command name
   const char*         ptrchs;      // -> Command string

   int                 i;

   #ifdef HCDM
     tracef("%4d Editor(%p)::execute(%s)\n", __LINE__, this,
            histActive->getText());
   #endif

   //-------------------------------------------------------------------------
   // Exit if comment
   //-------------------------------------------------------------------------
   ptrcmd= (char*)histActive->getText(); // Address the command
   if( *ptrcmd == '\0' )            // If comment command
     return NULL;                   // Return, command complete

   if( *ptrcmd == '*' )             // If comment command
     return NULL;                   // Return, command complete

   //-------------------------------------------------------------------------
   // Process default locate command
   //-------------------------------------------------------------------------
   if( *ptrcmd == '/' )             // If locate command
     return builtin_locate(this, ptrcmd);

   //-------------------------------------------------------------------------
   // Process default linenumber command
   //-------------------------------------------------------------------------
   if( *ptrcmd >= '0' && *ptrcmd <= '9' ) // If linenumber locate
     return builtin_number(this, ptrcmd);

   //-------------------------------------------------------------------------
   // Extract the command name
   //-------------------------------------------------------------------------
   memcpy(cmdname, "        ", 8);  // Initialize the command name
   for(i=0; i<8; i++)
   {
     if( ptrcmd[i] == '\0' )        // If end of command
       break;                       // Break, command name complete
     cmdname[i]= toupper(ptrcmd[i]);
     if( cmdname[i] == ' ' )
       break;
   }

   //-------------------------------------------------------------------------
   // Process builtin commands
   //-------------------------------------------------------------------------
   ptrchs= findBlank(ptrcmd);
   ptrchs= skipBlank(ptrchs);
   for(i=0; i<DIM(command); i++)
   {
     if( memcmp(cmdname, command[i].name, 8) == 0 )
     {
       result= (*command[i].function)(this, ptrchs);
       return result;
     }
   }

   //-------------------------------------------------------------------------
   // System command
   //-------------------------------------------------------------------------
   return builtin_dos(this, ptrcmd);// Process system command
}

