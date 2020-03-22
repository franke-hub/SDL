//----------------------------------------------------------------------------
//
//       Copyright (C) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Page.java
//
// Purpose-
//       Page descriptor.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Page
//
// Purpose-
//       Page descriptor.
//
//----------------------------------------------------------------------------
public class Page
{
//----------------------------------------------------------------------------
// Page.attributes
//----------------------------------------------------------------------------
static final int       size= 1024;  // Number of words in a Page
static final int       mask= size - 1; // Address mask for Page offset

   Object[]            word;        // Page content

//----------------------------------------------------------------------------
//
// Method-
//       Page.Page
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Page( )                          // Constructor
{
   word= new Object[size];
}

//----------------------------------------------------------------------------
//
// Method-
//       Page.getAttr
//
// Purpose-
//       Get attributes.
//
//----------------------------------------------------------------------------
public int                          // The Attributes
   getAttr( )                       // Get Attributes
{
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Page.clrAttr
//
// Purpose-
//       Clear attributes.
//
//----------------------------------------------------------------------------
public void
   clrAttr(                         // Clear Attributes
     int               attr)        // Attributes to clear
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Page.setAttr
//
// Purpose-
//       Set attributes.
//
//----------------------------------------------------------------------------
public void
   setAttr(                         // Set Attributes
     int               attr)        // Attributes to set
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Page.copy
//
// Purpose-
//       Copy from source Page into this Page.
//
//----------------------------------------------------------------------------
public void
   copy(                            // Copy Page
     Page              source)      // Source Page
{
   int                 i;

   for(i= 0; i<size; i++)
     this.word[i]= source.word[i];

   source.setAttr(PageAddr.REF);
}

//----------------------------------------------------------------------------
//
// Method-
//       Page.zero
//
// Purpose-
//       Zero this Page.
//
//----------------------------------------------------------------------------
public void
   zero( )                          // Zero Page
{
   int                 i;

   for(i= 0; i<size; i++)
     this.word[i]= null;
}

//----------------------------------------------------------------------------
//
// Method-
//       Page.fetch
//
// Purpose-
//       Fetch from Page.
//
//----------------------------------------------------------------------------
public Object                       // Resultant
   fetch(                           // Fetch from Page
     int               index)       // Word index
{
   return word[index];
}

//----------------------------------------------------------------------------
//
// Method-
//       Page.store
//
// Purpose-
//       Store into Page.
//
//----------------------------------------------------------------------------
public void
   store(                           // Store into Page
     int               index,       // Word index
     Object            object)      // Word value
{
   word[index]= object;
}
} // Class Page

