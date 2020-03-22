#!/bin/python
##----------------------------------------------------------------------------
##
##       Copyright (C) 2016-2019 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##----------------------------------------------------------------------------
##
## Title-
##       Format.py
##
## Purpose-
##       Use python formatting to display float value(s).
##
## Last change date-
##       2019/10/26
##
## Usage-
##       Format.py "format string" value ...
##
## Implementation note-
##       See ./test/curltimer for sample usage.
##
##############################################################################
if __name__ == '__main__':
    import sys
    if len(sys.argv) < 3:
        print(sys.argv[0], "requires at least 2 arguments: string value ...")
        sys.exit()

    inps = list(sys.argv[2:])
    outs = []
    for inp in inps:
        try:
            f = float(inp)
            outs += [f]
        except:
            outs += [inp]

    print(str(sys.argv[1]) % tuple(outs))

