//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Include-
//       EdTabs.hpp
//
// Purpose-
//       Editor: Handle tabs and margins
//
// Last change date-
//       2024/04/02
//
// Implementation notes-
//       (Only) included by EdBifs.cpp
//
//       The forward and reverse tab keys only control cursor positioning.
//       (The {alt-\,tab} sequence inserts a tab into the file.)
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Subroutine-
//       command_detab
//
// Purpose-
//       Remove all tabs from file
//
// Implementation notes-
//       Uses default tab spacing.
//
//----------------------------------------------------------------------------
static const char*                  // Error message, nullptr expected
   command_detab(char*)             // De-tab command
{
   enum {TABS= Editor::TAB_DEFAULT}; // (Always use [only] the default tab)
   EdView* data= editor::data;
   EdFile* file= editor::file;
   EdLine* cur= data->cursor;       // Save the cursor line

   if( file->protect )              // Do not modify protected files
     return "Read/only";

   // Command detab cannot be undone. Disallow if unsaved changes exist
   if( file->changed )              // If the file has unsaved changes
     return "Cancelled: save or undo changes first";

   for(EdLine* line= file->line_list.get_head(); line; line= line->get_next()) {
     if( line->flags & EdLine::F_PROT ) // If the line is protected
       continue;                    // (Skip line; don't remove tabs)
     Active* active= nullptr;
     const char* text= line->text;  // Using the text line
     const char* tabs= strchr(text, '\t'); // Locate the first tab
     while( tabs ) {                // Remove tabs from the line
       if( active == nullptr ) {
         active= &data->active;
         active->reset("");
         data->cursor= line;
       }

       size_t L= tabs - text;       // Length of text
       active->append_text(text, L); // Append the text
       L= active->get_used();
       L += TABS;
       L &= ~(TABS - 1);
       active->fetch(L-1);
       if( L > active->get_used() )
         active->append_text(" ");

       text= tabs + 1;            // Skip past this tab
       tabs= strchr(text, '\t');  // Locate the next tab
     }

     if( active ) {               // If tabs found
       active->append_text(text); // Append trailing text
       active->append_text(" ");  // (Indicate changed)
       file->chglock= true;       // (File changed, UNDO not possible)
       const char* buffer= active->get_changed();
       if( buffer )               // (Should always be non-null)
         line->text= editor::allocate(buffer); // (We never delete text)
     }
   }

   // Reset the active line and redraw (whether or not needed)
   data->cursor= cur;
   data->active.reset(cur->text);
   editor::outs->draw();

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       command_margins
//
// Purpose-
//       Set/display margins.
//
//----------------------------------------------------------------------------
static const char*                  // Error message, nullptr expected
   command_margins(                 // Margins command
     char*             parm)        // (Mutable) parameter string
{
   if( parm && *parm == '\0' )
     parm= nullptr;

   if( parm ) {
     Tokenizer T(parm);
     Iterator tix= T.begin();

     size_t l_margin;
     size_t r_margin;
     const char* error= number((tix++)().c_str(), l_margin);
     if( error )
       return error;

     error= number((tix++)().c_str(), r_margin);
     if( error )
       return error;

     if( l_margin <= 0 || l_margin > r_margin || tix != T.end() )
       return "Invalid margins";

     editor::margins[0]= l_margin;
     editor::margins[1]= r_margin;
     return nullptr;
   }

   editor::hist->info_message= true;
   return_string= "Margins: {" + std::to_string(editor::margins[0]) + ","
                               + std::to_string(editor::margins[1]) + "}";
   return return_string.c_str();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       command_tabs
//
// Purpose-
//       Set/display tabs.
//
//----------------------------------------------------------------------------
static const char*                  // Error message, nullptr expected
   command_tabs(                    // Tabs command
     char*             parm)        // (Mutable) parameter string
{
   if( parm && *parm == '\0' )
     parm= nullptr;

   size_t count= 0;
   if( parm ) {
     Tokenizer T(parm);
     Iterator tix= T.begin();

     const char* error= nullptr;
     while( tix != T.end() ) {
       if( ++count >= Editor::TAB_DIM )
         return "Too many tabs";

       size_t tab;
       error= number((tix++)().c_str(), tab);
       if( error ) {
         editor::tabs[0]= 0;
         return error;
       }

       editor::tabs[count]= tab;
     }
     editor::tabs[0]= count;

     // Verify tab sequence. Invalid sequences reset
     if( count ) {
       if( editor::tabs[1] < 2 ) {
         editor::tabs[1]= 0;
         return "First tab must be >= 2";
       }

       for(size_t i= 2; i <= count; ++i) {
         if( editor::tabs[i] <= editor::tabs[i-1] ) {
           editor::tabs[0]= 0;
           return "Invalid tab sequence";
         }
       }
     }
     return nullptr;
   }

   // Display tabs
   editor::hist->info_message= true;
   count= editor::tabs[0];
   if( count == 0 )
     return "Tabs defaulted";

   return_string= "Tabs: {";
   for(size_t i= 1; i <= count; ++i) {
     if( i > 1 )
       return_string += ",";
     return_string += std::to_string(editor::tabs[i]);
   }
   return_string +="}";
   return return_string.c_str();
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::tab_forward
//
// Purpose-
//       Get next tab column.
//
//----------------------------------------------------------------------------
size_t                              // The next tab column
   editor::tab_forward(             // Get the tab column
     size_t            column)      // After this zero-origin column
{
   enum {DT= Editor::TAB_DEFAULT};  // The default tab spacing

   size_t used= tabs[0];            // The tab count

   ++column;                        // One-origin column
   for(size_t i= 1; i <= used; ++i) {
     if( tabs[i] > column )
       return tabs[i] - 1;
   }

   size_t default_tab= ((column+DT)/DT*DT);
   return default_tab - 1;          // Next default tab stop
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::tab_reverse
//
// Purpose-
//       Get prior tab column.
//
//----------------------------------------------------------------------------
size_t                              // The prior tab column
   editor::tab_reverse(             // Get the tab column
     size_t            column)      // Before this zero-origin column
{
   enum {DT= Editor::TAB_DEFAULT};  // The default tab spacing

   size_t used= tabs[0];            // The tab count

   ++column;                        // One-origin column
   if( (used == 0 && column <= DT) || column <= tabs[1] )
      return 0;

   size_t default_tab= ((column-DT)/DT*DT);
   if( used == 0 || default_tab > tabs[used] )
     return default_tab - 1;

   for(size_t i= used; i > 0; --i) {
     if( tabs[i] < column )
       return tabs[i] - 1;
   }

   return 0;                        // (Unexpected)
}
