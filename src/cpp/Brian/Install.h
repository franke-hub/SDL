//----------------------------------------------------------------------------
//
//       Copyright (c) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Install.h
//
// Purpose-
//       Define the Install extension.
//
// Last change date-
//       2019/01/01
//
//----------------------------------------------------------------------------
#ifndef INSTALL_H_INCLUDED
#define INSTALL_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Install
//
// Purpose-
//       Define the Install extension.
//
// Implementation notes-
//       The base class installs additional extensions.
//       An Install need only contain a destructor and a default constructor.
//
// Implementation notes-
//       Install classes are created after static initialization completes.
//
//----------------------------------------------------------------------------
class Install {                     // The standard Install extension
//----------------------------------------------------------------------------
// Install::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Install( void );                // Destructor
   Install( void );                 // Constructor

   Install(const Install&) = delete; // Disallowed copy constructor
   Install& operator=(const Install&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Install::Methods
//----------------------------------------------------------------------------
public:
// None defined
}; // class Install
#endif // INSTALL_H_INCLUDED
