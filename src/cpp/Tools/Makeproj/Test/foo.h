//----------------------------------------------------------------------------
//
//       Copyright (c) 2006 Frank Eskesen.
//
//       This file is free content, distributed under the "un-license,"
//       explicitly released into the Public Domain.
//       (See accompanying file LICENSE.UNLICENSE or the original
//       contained within http://unlicense.org)
//
//----------------------------------------------------------------------------
//
// Title-
//       foo.h
//
// Purpose-
//       Sample include.
//
// Last change date-
//       2006/01/01
//
//----------------------------------------------------------------------------
   begin("foo.h");
   iam("foo.h");
   inca("bar.h");
#include <bar.h>
   incq("bar.h");
#include "bar.h"
   inca("top.h");
#include <top.h>
   incq("top.h");
#include "top.h"
   finis();

