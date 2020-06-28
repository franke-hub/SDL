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
//       macro/try_catch.h
//
// Purpose-
//       TRY_CATCH macro definition.
//
// Last change date-
//       2020/06/27
//
//----------------------------------------------------------------------------
#undef  TRY_CATCH                   // (Allow redefine)
#define TRY_CATCH(x) try { x        \
   } catch(pub::Exception& X) {     \
     debugf("%s\n", std::string(X).c_str()); \
   } catch(std::exception& X) {     \
     printf("std::exception.what(%s))\n", X.what()); \
   } catch(const char* X) {         \
     printf("catch(const char* '%s')\n", X); \
   } catch(...) {                   \
     printf("catch(...)\n");        \
   }
