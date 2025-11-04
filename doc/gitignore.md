<!-- -------------------------------------------------------------------------
//
//       Copyright (C) 2025 Frank Eskesen.
//
//       This file is free content, distributed under the MIT-0 license.
//       (See accompanying file LICENSE.MIT-0 or the original contained
//       within https://opensource.org/license/mit-0)
//
// SPDX-License-Identifier: MIT-0
//----------------------------------------------------------------------------
//
// Title-
//       ~/.gitignore.md
//
// Purpose-
//       Sample .gitignore files
//
// Last change date-
//       2025/11/01
//
//------------------------------------------------------------------------ -->

These .gitignore files are commonly used:

```
##############################################################################
##
##       Copyright (C) 2025 Frank Eskesen.
##
##       This file is free content, distributed under the MIT-0 license.
##       (See accompanying file LICENSE.MIT-0 or the original contained
##       within https://opensource.org/license/mit-0)
##
## SPDX-License-Identifier: MIT-0
##############################################################################
## .gitignore (Ignore everything)
##############################################################################
/*

##############################################################################
##
##       Copyright (C) 2025 Frank Eskesen.
##
##       This file is free content, distributed under the MIT-0 license.
##       (See accompanying file LICENSE.MIT-0 or the original contained
##       within https://opensource.org/license/mit-0)
##
## SPDX-License-Identifier: MIT-0
##############################################################################
## .gitignore (Ignore this file)
##############################################################################
/.gitignore

##############################################################################
##
##       Copyright (C) 2025 Frank Eskesen.
##
##       This file is free content, distributed under the MIT-0 license.
##       (See accompanying file LICENSE.MIT-0 or the original contained
##       within https://opensource.org/license/mit-0)
##
## SPDX-License-Identifier: MIT-0
##############################################################################
## ~/.gitignore
##############################################################################
/.gitignore
/__*

## Global rule: Ignore any file anywhere with a name that begins with "__:
*/__*

##############################################################################
##
##       Copyright (C) 2025 Frank Eskesen.
##
##       This file is free content, distributed under the MIT-0 license.
##       (See accompanying file LICENSE.MIT-0 or the original contained
##       within https://opensource.org/license/mit-0)
##
## SPDX-License-Identifier: MIT-0
##############################################################################
## .gitignore (Ignore this file and .OBSOLETE subdirectory)
##############################################################################
/.gitignore
/.OBSOLETE

##############################################################################
##
##       Copyright (c) 2025 Frank Eskesen.
##
##       This file is free content, distributed under the MIT-0 license.
##       (See accompanying file LICENSE.MIT-0 or the original contained
##       within https://opensource.org/license/mit-0)
##
## SPDX-License-Identifier: MIT-0
##############################################################################
## .gitignore (Ignore everything EXCEPT .gitignore and .README)
##############################################################################
/*
!/.gitignore
!/.README

##############################################################################
##
##       Copyright (C) 2025 Frank Eskesen.
##
##       This file is free content, distributed under the MIT-0 license.
##       (See accompanying file LICENSE.MIT-0 or the original contained
##       within https://opensource.org/license/mit-0)
##
## SPDX-License-Identifier: MIT-0
##############################################################################
## .gitignore (For object subdirectories)
##############################################################################
/*
!/.gitignore
!/D
!/H
!/L
!/M
!/Makefile
!/S
!/Test
```
