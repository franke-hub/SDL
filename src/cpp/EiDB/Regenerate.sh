#!/bin/bash
##############################################################################
##
##       Copyright (C) 2014-2021 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##############################################################################
set -x

##############################################################################
## Small file regression test
exScan -v- -wild -out INP/sample.eidb AAACCCGGGTTT AAACCCGGGTTT >OUT/regress01.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan01 failure"
  exit 1
fi

exScan -v- -wild INP/sample.eidb ANACNCGNGTNT ARACYCGRGTYT >OUT/regress02.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan02 failure"
  exit 1
fi

exScan -v- -wild INP/sample.eidb AWACSCGSGTWT AMACMCGKGTKT >OUT/regress03.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan03 failure"
  exit 1
fi

exScan -v- -wild INP/sample.eidb AAACBCGBGTBT ADACCCGDGTDT >OUT/regress04.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan04 failure"
  exit 1
fi

exScan -v- -wild INP/sample.eidb AVACVCGVGTTT AHACHCGGGTHT >OUT/regress05.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan05 failure"
  exit 1
fi

exScan -v- -wild -out -rev -sum INP/sample.eidb TTTGGGCCCAAA AAACCCGGGTTT >OUT/regress06.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan06 failure"
  exit 1
fi

exScan -v- -wild -minsize:71 -maxsize:72 INP/sample.eidb TTTGGGCCCAAA AAACCCGGGTTT >OUT/regress07.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan07 failure"
  exit 1
fi

exScan -v- -out -sum INP/sample.eidb TTTGGGCCCAAA AAACCCGGGTTT >OUT/regress08.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan08 failure"
  exit 1
fi

exScan -v- -out -union INP/sample.eidb TTTGGGCCCAAA AAACCCGGGTTT >OUT/regress09.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan09 failure"
  exit 1
fi

exScan -v- -atg -out -union INP/sample.eidb AACC TTAA >OUT/regress0a.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan0a failure"
  exit 1
fi

exScan -v- -out -first INP/sample.eidb AACC TTAA >OUT/regress0b.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan0b failure"
  exit 1
fi

exScan -v- -out -last  INP/sample.eidb AACC TTAA >OUT/regress0c.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan0c failure"
  exit 1
fi

exScan -v- -out -first -last INP/sample.eidb AACC TTAA >OUT/regress0d.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan0d failure"
  exit 1
fi

exScan -v- -out -atg -first -last INP/sample.eidb AACC TTAA >OUT/regress0e.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan0e failure"
  exit 1
fi

exScan -v- -out -atg -first INP/sample.eidb AACC TTAA >OUT/regress0f.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan0f failure"
  exit 1
fi

exScan -v- -out -atg -last INP/sample.eidb AACC TTAA >OUT/regress0g.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan0g failure"
  exit 1
fi

exScan -v- -out -rev -first -last INP/sample.eidb AACC TTAA >OUT/regress0h.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan0h failure"
  exit 1
fi

exScan -v- -out -rev -first INP/sample.eidb AACC TTAA >OUT/regress0i.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan0i failure"
  exit 1
fi

exScan -v- -out -rev -last INP/sample.eidb AACC TTAA >OUT/regress0j.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan0j failure"
  exit 1
fi

exScan -v- -out -only -first -last INP/sample.eidb AACC TTAA >OUT/regress0k.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan0k failure"
  exit 1
fi

exScan -v- -out -only -first INP/sample.eidb AACC TTAA >OUT/regress0l.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan0l failure"
  exit 1
fi

exScan -v- -out -only -last INP/sample.eidb AACC TTAA >OUT/regress0m.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan0m failure"
  exit 1
fi

inScan -v- -wild -out INP/sample.eidb aaacccgggttt aaacccgggttt >OUT/regress11.old
if [[ $? != 0 ]] ; then
  echo "Regression inscan11 failure"
  exit 1
fi

inScan -v- -wild INP/sample.eidb anacncgngtnt aracycgrgtyt >OUT/regress12.old
if [[ $? != 0 ]] ; then
  echo "Regression inscan12 failure"
  exit 1
fi

inScan -v- -wild INP/sample.eidb awacscgsgtwt amacmcgkgtkt >OUT/regress13.old
if [[ $? != 0 ]] ; then
  echo "Regression inscan13 failure"
  exit 1
fi

inScan -v- -wild INP/sample.eidb aaacbcgbgtbt adacccgdgtdt >OUT/regress14.old
if [[ $? != 0 ]] ; then
  echo "Regression inscan14 failure"
  exit 1
fi

inScan -v- -wild INP/sample.eidb avacvcgvgttt ahachcgggtht >OUT/regress15.old
if [[ $? != 0 ]] ; then
  echo "Regression inscan15 failure"
  exit 1
fi

inScan -v- -wild -out -rev -sum INP/sample.eidb tttgggcccaaa aaacccgggttt >OUT/regress16.old
if [[ $? != 0 ]] ; then
  echo "Regression inscan16 failure"
  exit 1
fi

inScan -v- -wild -minsize:71 -maxsize:72 INP/sample.eidb tttgggcccaaa aaacccgggttt >OUT/regress17.old
if [[ $? != 0 ]] ; then
  echo "Regression inscan17 failure"
  exit 1
fi

##############################################################################
## border regression test
border -v- -out -all64 INP/sample.eidb TTT.TTT >OUT/border01.old
if [[ $? != 0 ]] ; then
  echo "Regression border01 failure"
  exit 1
fi

border3 -v- -out -all64 INP/sample.eidb TTT.TTT >OUT/border03.old
if [[ $? != 0 ]] ; then
  echo "Regression border03 failure"
  exit 1
fi

##############################################################################
## exCodon regression test
exCodon -v- -out INP/Codons.eidb >OUT/codon01.old
if [[ $? != 0 ]] ; then
  echo "Regression codon01 failure"
  exit 1
fi

exCodon -v- -out -atg INP/Codons.eidb >OUT/codon02.old
if [[ $? != 0 ]] ; then
  echo "Regression codon02 failure"
  exit 1
fi

exCodon -v- -atg -wild INP/Codons.eidb >OUT/codon03.old
if [[ $? != 0 ]] ; then
  echo "Regression codon03 failure"
  exit 1
fi

##############################################################################
## exPhase regression test
set -x
exPhase Phases.eidb
if [[ $? != 0 ]] ; then
  echo "Regression phase1X failure"
  exit 1
fi

mv -f Phases.eidb.0 OUT/pt01.Phases.eidb.0
if [[ $? != 0 ]] ; then
  echo "Regression phase10 failure"
  exit 1
fi

mv -f Phases.eidb.1 OUT/pt01.Phases.eidb.1
if [[ $? != 0 ]] ; then
  echo "Regression phase11 failure"
  exit 1
fi

mv -f Phases.eidb.2 OUT/pt01.Phases.eidb.2
if [[ $? != 0 ]] ; then
  echo "Regression phase12 failure"
  exit 1
fi

exPhase -atg Phases.eidb
if [[ $? != 0 ]] ; then
  echo "Regression phase2X failure"
  exit 1
fi

mv -f Phases.eidb.0 OUT/pt02.Phases.eidb.0
if [[ $? != 0 ]] ; then
  echo "Regression phase20 failure"
  exit 1
fi

mv -f Phases.eidb.1 OUT/pt02.Phases.eidb.1
if [[ $? != 0 ]] ; then
  echo "Regression phase21 failure"
  exit 1
fi

mv -f Phases.eidb.2 OUT/pt02.Phases.eidb.2
if [[ $? != 0 ]] ; then
  echo "Regression phase22 failure"
  exit 1
fi

exPhase -atg -wild Phases.eidb
if [[ $? != 0 ]] ; then
  echo "Regression phase3X failure"
  exit 1
fi

mv -f Phases.eidb.0 OUT/pt03.Phases.eidb.0
if [[ $? != 0 ]] ; then
  echo "Regression phase30 failure"
  exit 1
fi

mv -f Phases.eidb.1 OUT/pt03.Phases.eidb.1
if [[ $? != 0 ]] ; then
  echo "Regression phase31 failure"
  exit 1
fi

mv -f Phases.eidb.2 OUT/pt03.Phases.eidb.2
if [[ $? != 0 ]] ; then
  echo "Regression phase32 failure"
  exit 1
fi

##############################################################################
## exPhase -end regression test
exPhase -end Phases.eidb
if [[ $? != 0 ]] ; then
  echo "Regression phase1X failure"
  exit 1
fi

mv -f Phases.eidb.0 OUT/pt01e.Phases.eidb.0
if [[ $? != 0 ]] ; then
  echo "Regression phase10 failure"
  exit 1
fi

mv -f Phases.eidb.1 OUT/pt01e.Phases.eidb.1
if [[ $? != 0 ]] ; then
  echo "Regression phase11 failure"
  exit 1
fi

mv -f Phases.eidb.2 OUT/pt01e.Phases.eidb.2
if [[ $? != 0 ]] ; then
  echo "Regression phase12 failure"
  exit 1
fi

exPhase -end -atg Phases.eidb
if [[ $? != 0 ]] ; then
  echo "Regression phase2X failure"
  exit 1
fi

mv -f Phases.eidb.0 OUT/pt02e.Phases.eidb.0
if [[ $? != 0 ]] ; then
  echo "Regression phase20 failure"
  exit 1
fi

mv -f Phases.eidb.1 OUT/pt02e.Phases.eidb.1
if [[ $? != 0 ]] ; then
  echo "Regression phase21 failure"
  exit 1
fi

mv -f Phases.eidb.2 OUT/pt02e.Phases.eidb.2
if [[ $? != 0 ]] ; then
  echo "Regression phase22 failure"
  exit 1
fi

exPhase -end -atg -wild Phases.eidb
if [[ $? != 0 ]] ; then
  echo "Regression phase3X failure"
  exit 1
fi

mv -f Phases.eidb.0 OUT/pt03e.Phases.eidb.0
if [[ $? != 0 ]] ; then
  echo "Regression phase30 failure"
  exit 1
fi

mv -f Phases.eidb.1 OUT/pt03e.Phases.eidb.1
if [[ $? != 0 ]] ; then
  echo "Regression phase31 failure"
  exit 1
fi

mv -f Phases.eidb.2 OUT/pt03e.Phases.eidb.2
if [[ $? != 0 ]] ; then
  echo "Regression phase32 failure"
  exit 1
fi

##############################################################################
## generate regression test
generate -gene:32 -seed:32 -hcdm INP/generate.dat >OUT/generate01.old
if [[ $? != 0 ]] ; then
  echo "Regression generate01 failure"
  exit 1
fi

generate -gene:32 -seed:32 -show INP/generate.dat AC.CT AG.GT >OUT/generate02.old
if [[ $? != 0 ]] ; then
  echo "Regression generate02 failure"
  exit 1
fi

generate -gene:100000 -seed:32 INP/generate.dat AC.CT AG.GT >OUT/generate03.old
if [[ $? != 0 ]] ; then
  echo "Regression generate03 failure"
  exit 1
fi

##############################################################################
## Big file regression tests
if [[ ! -s "BIG/gb115.dEID" ]] ; then
  echo "Missing BIG/gb115.dEID"
  exit 0
fi
exScan -v- -wild BIG/gb115.dEID              AACCTTGG GGTTCCAA >OUT/regress51.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan51 failure"
  exit 1
fi

exScan -v- -wild -first BIG/gb115.dEID       AACCTTGG GGTTCCAA >OUT/regress52.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan52 failure"
  exit 1
fi

exScan -v- -wild -maxsize:400 BIG/gb115.dEID AACCTTGG GGTTCCAA >OUT/regress53.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan53 failure"
  exit 1
fi

exScan -v- -wild -minsize:32 BIG/gb115.dEID  AACCTTGG GGTTCCAA >OUT/regress54.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan54 failure"
  exit 1
fi

exScan -v- -wild -rev BIG/gb115.dEID         AACCTTGG GGTTCCAA >OUT/regress55.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan55 failure"
  exit 1
fi

inScan -v- -wild BIG/gb115.dEID              aaccttgg ggttccaa >OUT/regress61.old
if [[ $? != 0 ]] ; then
  echo "Regression inscan61 failure"
  exit 1
fi

inScan -v- -wild -maxsize:400 BIG/gb115.dEID aaccttgg ggttccaa >OUT/regress63.old
if [[ $? != 0 ]] ; then
  echo "Regression inscan63 failure"
  exit 1
fi

inScan -v- -wild -minsize:32 BIG/gb115.dEID  aaccttgg ggttccaa >OUT/regress64.old
if [[ $? != 0 ]] ; then
  echo "Regression inscan64 failure"
  exit 1
fi

inScan -v- -wild -rev BIG/gb115.dEID         aaccttgg ggttccaa >OUT/regress65.old
if [[ $? != 0 ]] ; then
  echo "Regression inscan65 failure"
  exit 1
fi

##############################################################################
## Other bigfile tests
exScan -v- -wild -sum BIG/gb115.dEID         GAC ATG >OUT/exscan70.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan70 failure"
  exit 1
fi

exScan -v- -atg  -sum BIG/gb115.dEID         GAC ATG >OUT/exscan71.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan71 failure"
  exit 1
fi

exScan -v- -atg -wild -sum BIG/gb115.dEID    GAC ATG >OUT/exscan72.old
if [[ $? != 0 ]] ; then
  echo "Regression exscan72 failure"
  exit 1
fi

border -v- -wild -all16 BIG/gb115.dEID AACC.TTGG GGT.TCCAA >OUT/border71.old
if [[ $? != 0 ]] ; then
  echo "Regression border71 failure"
  exit 1
fi

border3 -v- -wild -all16 BIG/gb115.dEID AACC.TTGG GGT.TCCAA >OUT/border73.old
if [[ $? != 0 ]] ; then
  echo "Regression border73 failure"
  exit 1
fi

## Takes too long: not interested in debugging it
generate -gene:100000 -peakmin:100 -peakmax:600 -peakscale:100 -seed:32 -symmetric INP/generate.dat AC.CT:.9 AG.GT:.1 >OUT/generate70.old
if [[ $? != 0 ]] ; then
  echo "Regression generate70 failure"
  exit 1
fi

generate -gene:100000 -stop:200 -seed:32 -symmetric INP/generate.dat AC.CT:.9 AG.GT:.1 >OUT/generate71.old
if [[ $? != 0 ]] ; then
  echo "Regression generate71 failure"
  exit 1
fi

echo "OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!"
echo "Regeneration successful!!"

