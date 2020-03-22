//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       XmlList.h
//
// Purpose-
//       Describe an XML List.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef XMLLIST_H_INCLUDED
#define XMLLIST_H_INCLUDED

#ifndef LIST_H_INCLUDED
#include "List.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class XmlNode;

//----------------------------------------------------------------------------
//
// Class-
//       XmlList
//
// Purpose-
//       A generic XML List.
//
//----------------------------------------------------------------------------
class XmlList : public List<XmlNode> { // Generic XML List
//----------------------------------------------------------------------------
// XmlList::Constructors
//----------------------------------------------------------------------------
public:
   ~XmlList( void ) {}              // Default destructor
   XmlList( void )                  // Default constructor
:  List<XmlNode>()  {}

private:                            // Bitwise copy prohibited
   XmlList(const XmlList&);
XmlList&
   operator=(const XmlList&);
}; // class XmlList

#endif // XMLLIST_H_INCLUDED
