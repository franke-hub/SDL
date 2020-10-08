//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HtmlNodeVisitor.cpp
//
// Purpose-
//       HtmlNodeVisitor implementation methods.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include "Common.h"
#include "HtmlNode.h"
#include "HtmlNodeVisitor.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Method-
//       HtmlNodeVisitor::~HtmlNodeVisitor
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   HtmlNodeVisitor::~HtmlNodeVisitor( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlNodeVisitor::HtmlNodeVisitor
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   HtmlNodeVisitor::HtmlNodeVisitor( void ) // Default constructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlNodeVisitor::visit
//
// Purpose-
//       Visit a HtmlNode
//
//----------------------------------------------------------------------------
int                                 // Return code (0 to visit subtree)
   HtmlNodeVisitor::visit(          // Visit a HtmlNode
     HtmlNode*         node)        // -> HtmlNode
{
   // Since this NodeVisitor does nothing, there is no point in visiting any
   // child nodes since this HtmlNodeVisitor will do nothing with them, either.
   (void)node;                      // (Parameter ignored)
   return (-1);
}

