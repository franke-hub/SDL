<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2023 Frank Eskesen.
//
//       This file is free content, distributed under cc by-sa version 4.0
//       with attribution required.
//       (See accompanying file LICENSE.BY_SA-4.0 or the original contained
//       within https://creativecommons.org/licenses/by-sa/4.0/us/legalcode)
//
// SPDX-License-Identifier: CC-BY-4.0
//----------------------------------------------------------------------------
//
// Title-
//       ~/doc/cpp/Console.md
//
// Purpose-
//       Console.h reference manual
//
// Last change date-
//       2023/11/16
//
-------------------------------------------------------------------------- -->
## pub::Console
\#include <pub/Console.h>

All Console methods are static. No Console object exists.

Applications use the Console::getch method to get characters from STDIN without
automatic character echoing.

#### Member functions

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [getch](./pub_console.md#getch) | Get the next character. |
| [gets](./pub_console.md#gets) | Read input string from the Console. |
| [putch](./pub_console.md#putch) | Write character to STDOUT. |
| [printf](./pub_console.md#printf) | Print to STDOUT. |
| [start](./pub_console.md#start) | Start Console operation. |
| [stop](./pub_console.md#stop) | Stop Console operation. |
| [wait](./pub_console.md#wait) | Wait for Console stop. |
