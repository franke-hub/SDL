#!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2018-2020 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##----------------------------------------------------------------------------
##
## Title-
##       utf8.wh
##
## Function-
##       Make + run regression set
##
## Last change date-
##       2020/09/20
##
##############################################################################

DO=Quick
make $DO
if [[ $? != 0 ]] ; then
  exit $?
fi

##############################################################################
echo -e "\n\n================================================================"
echo "Invalid start characters"
$DO --utf8.decode=80               ## error[1] Smallest
$DO --utf8.decode=A0               ## error[1] Sample
$DO --utf8.decode=BF               ## error[1] Largest
$DO --utf8.decode=F5808080         ## error[4] Smallest
$DO --utf8.decode=F880808080       ## error[5] Smallest (/invalid start)
$DO --utf8.decode=FC8080808080     ## error[6] Smallest (/invalid start)
$DO --utf8.decode=FE8080808080     ## error[7] Smallest (/invalid start)
$DO --utf8.decode=FF808080808080   ## error[8] Smallest (/invalid start)
$DO --utf8.decode=20BF             ## Second character

##############################################################################
echo -e "\n\n================================================================"
echo "Invalid secondary characters"
$DO --utf8.decode="D080 D000"
$DO --utf8.decode="D080 D07F"
$DO --utf8.decode="C280 C2C0"
$DO --utf8.decode="C280 C2FF"

$DO --utf8.decode="E0A080 E80080"
$DO --utf8.decode="E0A080 E88080 E880C0"

$DO --utf8.decode="F4808080 F100BFBF"
$DO --utf8.decode="F4808080 F1B0C2BF"
$DO --utf8.decode="F4808080 F1B080FF"

##############################################################################
echo -e "\n\n================================================================"
echo "Invalid encodings: overlong"
$DO --utf8.decode=C080             ## error[2] Smallest too small
$DO --utf8.decode=C0BF             ## error[2] Too small
$DO --utf8.decode=C1BF             ## error[2] Largest too small

$DO --utf8.decode=E08080           ## error[3] Smallest too small
$DO --utf8.decode=E09090           ## error[3] Too small
$DO --utf8.decode=E09FBF           ## error[3] Largest  too small

$DO --utf8.decode=F0808080         ## error[4] Smallest too small
$DO --utf8.decode=F08FBFBF         ## error[4] Largest  too small
$DO --utf8.decode=F4908080         ## error[4] Smallest too large
$DO --utf8.decode=F7BFBFBF         ## error[4] Largest  too large

##############################################################################
echo -e "\n\n================================================================"
echo "Invalid encodings: D800-DFFF"
$DO --utf8.decode=EDA080
$DO --utf8.decode=EDA18C           ## Sample error defined in spec
$DO --utf8.decode=EDBEB4           ## Sample error defined in spec
$DO --utf8.decode=EDBFBF

##############################################################################
echo -e "\n\n================================================================"
echo "Edge sequences"
$DO --utf8.decode="C280 C1BF"      ## error[2] Largest  too small
$DO --utf8.decode="E0A080 E09FBF"  ## error[3] Largest  too small
$DO --utf8.decode="ED9FBF EDA080"  ## D7FF.D800
$DO --utf8.decode="EE8080 EDBFBF"  ## E000,DFFF
$DO --utf8.decode="F0908080 F08FBFBF" ## error[4] Largest  too small
$DO --utf8.decode="F48FBFBF F4908080" ## error[4] Smallest too large

$DO --utf8.encode=D7FF ## valid
$DO --utf8.encode=D800 ## error
$DO --utf8.encode=DFFF ## error
$DO --utf8.encode=E000 ## valid

$DO --utf8.encode=10FFFF ## valid
$DO --utf8.encode=110000 ## error
$DO --utf8.encode=1FFFFF ## error
$DO --utf8.encode=200000 ## error

##############################################################################
echo -e "\n\n================================================================"
echo "Full sequence test"
$DO --utf8

##############################################################################
echo -e "\n\n================================================================"
echo "Valid sequences"
$DO --utf8.decode="00 01 40 7E 7F"
$DO --utf8.decode="C280 C281 D080 DFBE DFBF"
$DO --utf8.decode="E0A080 E0A081 E88080 EFBFBE EFBFBF"
$DO --utf8.decode="F0908080 F0908081 F2808080 F48FBFBE F48FBFBF"

