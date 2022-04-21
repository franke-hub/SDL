#!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2018-2022 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##----------------------------------------------------------------------------
##
## Title-
##       test_UTF8.sh
##
## Function-
##       Run UTF8 regression set
##
## Last change date-
##       2022/03/04
##
## Implementation notes-
##       TODO: Add --utf8.expect= to Quick, all tests OK
##
##############################################################################
F=$(basename "$0")
echo -e "\nRunning $F ======================================================="
echo "HCDM: Quick --utf8.decode= LOOPS"
echo "Needs work, test skipped"
exit 0

DO=Quick
## D=$(dirname "$0")
## DO=$D/.ECHO

##############################################################################
## Function OK: Run test, success expected
function OK
{
  $DO "$@"
  rc=$?
  [ $rc == 0 ] && return

  echo "$DO xxx returned $rc, but 0 expected"
  exit 1
}

##############################################################################
## Function NG: Run test, failure expected
function NG
{
  $DO "$@"
  rc=$?
  [ $rc != 0 ] && return

  echo "$DO $1 returned 0, but non-zero expected"
  exit 1
}

##############################################################################
echo -e "\n\n================================================================"
echo "Invalid start characters"
NG --utf8.decode=80                 ## error[1] Smallest invalid start char
NG --utf8.decode=A0                 ## error[1] Middle invalid start char
NG --utf8.decode=BF                 ## error[1] Largest invalid start char
NG --utf8.decode=F5808080           ## error[4] Smallest out of range
NG --utf8.decode=F880808080         ## error[5] Smallest (/invalid start)
NG --utf8.decode=FC8080808080       ## error[6] Smallest (/invalid start)
NG --utf8.decode=FE8080808080       ## error[7] Smallest (/invalid start)
NG --utf8.decode=FF808080808080     ## error[8] Smallest (/invalid start)
NG --utf8.decode=2080               ## Largest invalid second character
NG --utf8.decode=20BF               ## Largest invalid second character

##############################################################################
echo -e "\n\n================================================================"
echo "Invalid secondary characters"
NG --utf8.decode="D080 D000"
NG --utf8.decode="D080 D07F"
NG --utf8.decode="C280 C2C0"
NG --utf8.decode="C280 C2FF"

NG --utf8.decode="E0A080 E80080"
NG --utf8.decode="E0A080 E88080 E880C0"

NG --utf8.decode="F4808080 F100BFBF"
NG --utf8.decode="F4808080 F1B0C2BF"
NG --utf8.decode="F4808080 F1B080FF"

##############################################################################
echo -e "\n\n================================================================"
echo "Invalid encodings: overlong"
NG --utf8.decode=C080               ## error[2] Smallest too small
NG --utf8.decode=C0BF               ## error[2] Too small
NG --utf8.decode=C1BF               ## error[2] Largest too small

NG --utf8.decode=E08080             ## error[3] Smallest too small
NG --utf8.decode=E09090             ## error[3] Too small
NG --utf8.decode=E09FBF             ## error[3] Largest  too small

NG --utf8.decode=F0808080           ## error[4] Smallest too small
NG --utf8.decode=F08FBFBF           ## error[4] Largest  too small
NG --utf8.decode=F4908080           ## error[4] Smallest too large
NG --utf8.decode=F7BFBFBF           ## error[4] Largest  too large

##############################################################################
echo -e "\n\n================================================================"
echo "Invalid encodings: D800-DFFF"
NG --utf8.decode=EDA080
NG --utf8.decode=EDA18C             ## Sample error defined in spec
NG --utf8.decode=EDBEB4             ## Sample error defined in spec
NG --utf8.decode=EDBFBF

##############################################################################
echo -e "\n\n================================================================"
echo "Edge sequences"
NG --utf8.decode="C280 C1BF"        ## error[2] Largest  too small
NG --utf8.decode="E0A080 E09FBF"    ## error[3] Largest  too small
NG --utf8.decode="ED9FBF EDA080"    ## D7FF.D800
NG --utf8.decode="EE8080 EDBFBF"    ## E000,DFFF
NG --utf8.decode="F0908080 F08FBFBF" ## error[4] Largest  too small
NG --utf8.decode="F48FBFBF F4908080" ## error[4] Smallest too large

OK --utf8.encode=D7FF                ## valid, top of lower range
NG --utf8.encode=D800                ## error, first in D800..DFFF
NG --utf8.encode=D900                ## error, within   D800..DFFF
NG --utf8.encode=DFFF                ## error, last  in D800..DFFF
OK --utf8.encode=E000                ## valid

OK --utf8.encode=10FFFF              ## valid, highest valid
NG --utf8.encode=110000              ## error, too large
NG --utf8.encode=1FFFFF              ## error, too large
NG --utf8.encode=200000              ## error, too large

##############################################################################
echo -e "\n\n================================================================"
echo "Valid sequences"
OK --utf8.decode="00 01 40 7E 7F"
OK --utf8.decode="C280 C281 D080 DFBE DFBF"
OK --utf8.decode="E0A080 E0A081 E88080 EFBFBE EFBFBF"
OK --utf8.decode="F0908080 F0908081 F2808080 F48FBFBE F48FBFBF"

##############################################################################
echo -e "\n\n================================================================"
echo "Full sequence test"
OK --utf8

echo "HCDM bringup test"
OK --utf8.decode="01 02 03" --utf8.expect="01 02 03"

