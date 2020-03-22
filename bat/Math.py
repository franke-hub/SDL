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
##       Math.py
##
## Purpose-
##       Use python for floating point arithmetic.
##
## Last change date-
##       2019/10/26
##
## Usage-
##       Math.py value -op- value
##         Where -op- is +, -, x, or /. (* cannot be used)
##
## Implementation note-
##       See ./test/curltimer for sample usage.
##
##############################################################################
if __name__ == '__main__':
    ## NOTE: We have to use 'x' for the multiply operator

    import sys
    if len(sys.argv) != 4:
        print(sys.argv[0], "requires exactly 3 arguments: value -op- value")
        sys.exit()

    if sys.argv[2] == "+":
        print((float(sys.argv[1]) + float(sys.argv[3])))
    elif sys.argv[2] == "-":
        print((float(sys.argv[1]) - float(sys.argv[3])))
    elif sys.argv[2].lower() == "x":
        print((float(sys.argv[1]) * float(sys.argv[3])))
    elif sys.argv[2] == "/":
        print((float(sys.argv[1]) / float(sys.argv[3])))
    else:
        print("Invalid operator '%s'" % sys.argv[2])

