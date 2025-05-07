REM --------------------------------------------------------------------------
REM
REM      Copyright (C) 2014 Frank Eskesen.
REM
REM      This file is free content, distributed under the MIT license.
REM      (See accompanying file LICENSE.MIT or the original contained
REM      within https://opensource.org/licenses/MIT)
REM
REM --------------------------------------------------------------------------
@echo on

REM --------------------------------------------------------------------------
REM Setup
set DIR=D:/src/cpp/EiDB/Data
REM See also: exphase tests

REM --------------------------------------------------------------------------
REM Small file regression test
exscan -v- -wild -out %DIR%/sample.eidb AAACCCGGGTTT AAACCCGGGTTT >regress01.new
differ regress01.new d:/Local/eidb/regress01.old
if errorlevel 1 goto exscan01

exscan -v- -wild %DIR%/sample.eidb ANACNCGNGTNT ARACYCGRGTYT >regress02.new
differ regress02.new d:/Local/eidb/regress02.old
if errorlevel 1 goto exscan02

exscan -v- -wild %DIR%/sample.eidb AWACSCGSGTWT AMACMCGKGTKT >regress03.new
differ regress03.new d:/Local/eidb/regress03.old
if errorlevel 1 goto exscan03

exscan -v- -wild %DIR%/sample.eidb AAACBCGBGTBT ADACCCGDGTDT >regress04.new
differ regress04.new d:/Local/eidb/regress04.old
if errorlevel 1 goto exscan04

exscan -v- -wild %DIR%/sample.eidb AVACVCGVGTTT AHACHCGGGTHT >regress05.new
differ regress05.new d:/Local/eidb/regress05.old
if errorlevel 1 goto exscan05

exscan -v- -wild -out -rev -sum %DIR%/sample.eidb TTTGGGCCCAAA AAACCCGGGTTT >regress06.new
differ regress06.new d:/Local/eidb/regress06.old
if errorlevel 1 goto exscan06

exscan -v- -wild -minsize:71 -maxsize:72 %DIR%/sample.eidb TTTGGGCCCAAA AAACCCGGGTTT >regress07.new
differ regress07.new d:/Local/eidb/regress07.old
if errorlevel 1 goto exscan07

exscan -v- -out -sum %DIR%/sample.eidb TTTGGGCCCAAA AAACCCGGGTTT >regress08.new
differ regress08.new d:/Local/eidb/regress08.old
if errorlevel 1 goto exscan08

exscan -v- -out -union %DIR%/sample.eidb TTTGGGCCCAAA AAACCCGGGTTT >regress09.new
differ regress09.new d:/Local/eidb/regress09.old
if errorlevel 1 goto exscan09

exscan -v- -atg -out -union %DIR%/sample.eidb AACC TTAA >regress0a.new
differ regress0a.new d:/Local/eidb/regress0a.old
if errorlevel 1 goto exscan0a

exscan -v- -out -first %DIR%/sample.eidb AACC TTAA >regress0b.new
differ regress0b.new d:/Local/eidb/regress0b.old
if errorlevel 1 goto exscan0b

exscan -v- -out -last  %DIR%/sample.eidb AACC TTAA >regress0c.new
differ regress0c.new d:/Local/eidb/regress0c.old
if errorlevel 1 goto exscan0c

exscan -v- -out -first -last %DIR%/sample.eidb AACC TTAA >regress0d.new
differ regress0d.new d:/Local/eidb/regress0d.old
if errorlevel 1 goto exscan0d

exscan -v- -out -atg -first -last %DIR%/sample.eidb AACC TTAA >regress0e.new
differ regress0e.new d:/Local/eidb/regress0e.old
if errorlevel 1 goto exscan0e

exscan -v- -out -atg -first %DIR%/sample.eidb AACC TTAA >regress0f.new
differ regress0f.new d:/Local/eidb/regress0f.old
if errorlevel 1 goto exscan0f

exscan -v- -out -atg -last %DIR%/sample.eidb AACC TTAA >regress0g.new
differ regress0g.new d:/Local/eidb/regress0g.old
if errorlevel 1 goto exscan0g

exscan -v- -out -rev -first -last %DIR%/sample.eidb AACC TTAA >regress0h.new
differ regress0h.new d:/Local/eidb/regress0h.old
if errorlevel 1 goto exscan0h

exscan -v- -out -rev -first %DIR%/sample.eidb AACC TTAA >regress0i.new
differ regress0i.new d:/Local/eidb/regress0i.old
if errorlevel 1 goto exscan0i

exscan -v- -out -rev -last %DIR%/sample.eidb AACC TTAA >regress0j.new
differ regress0j.new d:/Local/eidb/regress0j.old
if errorlevel 1 goto exscan0j

exscan -v- -out -only -first -last %DIR%/sample.eidb AACC TTAA >regress0k.new
differ regress0k.new d:/Local/eidb/regress0k.old
if errorlevel 1 goto exscan0k

exscan -v- -out -only -first %DIR%/sample.eidb AACC TTAA >regress0l.new
differ regress0l.new d:/Local/eidb/regress0l.old
if errorlevel 1 goto exscan0l

exscan -v- -out -only -last %DIR%/sample.eidb AACC TTAA >regress0m.new
differ regress0m.new d:/Local/eidb/regress0m.old
if errorlevel 1 goto exscan0m

inscan -v- -wild -out %DIR%/sample.eidb aaacccgggttt aaacccgggttt >regress11.new
differ regress11.new d:/Local/eidb/regress11.old
if errorlevel 1 goto inscan11

inscan -v- -wild %DIR%/sample.eidb anacncgngtnt aracycgrgtyt >regress12.new
differ regress12.new d:/Local/eidb/regress12.old
if errorlevel 1 goto inscan12

inscan -v- -wild %DIR%/sample.eidb awacscgsgtwt amacmcgkgtkt >regress13.new
differ regress13.new d:/Local/eidb/regress13.old
if errorlevel 1 goto inscan13

inscan -v- -wild %DIR%/sample.eidb aaacbcgbgtbt adacccgdgtdt >regress14.new
differ regress14.new d:/Local/eidb/regress14.old
if errorlevel 1 goto inscan14

inscan -v- -wild %DIR%/sample.eidb avacvcgvgttt ahachcgggtht >regress15.new
differ regress15.new d:/Local/eidb/regress15.old
if errorlevel 1 goto inscan15

inscan -v- -wild -out -rev -sum %DIR%/sample.eidb tttgggcccaaa aaacccgggttt >regress16.new
differ regress16.new d:/Local/eidb/regress16.old
if errorlevel 1 goto inscan16

inscan -v- -wild -minsize:71 -maxsize:72 %DIR%/sample.eidb tttgggcccaaa aaacccgggttt >regress17.new
differ regress17.new d:/Local/eidb/regress17.old
if errorlevel 1 goto inscan17

REM --------------------------------------------------------------------------
REM border regression test
border -v- -out -all64 %DIR%/sample.eidb TTT.TTT >border01.new
differ border01.new d:/Local/eidb/border01.old
if errorlevel 1 goto border01

border3 -v- -out -all64 %DIR%/sample.eidb TTT.TTT >border03.new
differ border03.new d:/Local/eidb/border03.old
if errorlevel 1 goto border03

REM --------------------------------------------------------------------------
REM exCodon regression test
if exist Codons.eidb erase Codons.eidb
copy D:\src\cpp\EiDB\Data\Codons.eidb Codons.eidb

exCodon -v- -out Codons.eidb >codon01.new
differ codon01.new D:/Local/eidb/codon01.old
if errorlevel 1 goto codon01

exCodon -v- -out -atg Codons.eidb >codon02.new
differ codon02.new D:/Local/eidb/codon02.old
if errorlevel 1 goto codon02

exCodon -v- -atg -wild Codons.eidb >codon03.new
differ codon03.new D:/Local/eidb/codon03.old
if errorlevel 1 goto codon03

erase Codons.eidb

REM --------------------------------------------------------------------------
REM exPhase regression test
if exist Phases.eidb erase Phases.eidb
copy D:\src\cpp\EiDB\Data\Phases.eidb Phases.eidb

exPhase Phases.eidb
differ Phases.eidb.0 d:/Local/eidb/pt01.Phases.eidb.0
if errorlevel 1 goto phase10

differ Phases.eidb.1 d:/Local/eidb/pt01.Phases.eidb.1
if errorlevel 1 goto phase11

differ Phases.eidb.2 d:/Local/eidb/pt01.Phases.eidb.2
if errorlevel 1 goto phase12

exPhase -atg Phases.eidb
differ Phases.eidb.0 d:/Local/eidb/pt02.Phases.eidb.0
if errorlevel 1 goto phase20

differ Phases.eidb.1 d:/Local/eidb/pt02.Phases.eidb.1
if errorlevel 1 goto phase21

differ Phases.eidb.2 d:/Local/eidb/pt02.Phases.eidb.2
if errorlevel 1 goto phase22

exPhase -atg -wild Phases.eidb
differ Phases.eidb.0 d:/Local/eidb/pt03.Phases.eidb.0
if errorlevel 1 goto phase30

differ Phases.eidb.1 d:/Local/eidb/pt03.Phases.eidb.1
if errorlevel 1 goto phase31

differ Phases.eidb.2 d:/Local/eidb/pt03.Phases.eidb.2
if errorlevel 1 goto phase32

erase Phases.eidb

REM --------------------------------------------------------------------------
REM exPhase -end regression test
if exist Phases.eidb erase Phases.eidb
copy D:\src\cpp\EiDB\Data\Phases.eidb Phases.eidb

exPhase -end Phases.eidb
differ Phases.eidb.0 d:/Local/eidb/pt01e.Phases.eidb.0
if errorlevel 1 goto phase10

differ Phases.eidb.1 d:/Local/eidb/pt01e.Phases.eidb.1
if errorlevel 1 goto phase11

differ Phases.eidb.2 d:/Local/eidb/pt01e.Phases.eidb.2
if errorlevel 1 goto phase12

exPhase -end -atg Phases.eidb
differ Phases.eidb.0 d:/Local/eidb/pt02e.Phases.eidb.0
if errorlevel 1 goto phase20

differ Phases.eidb.1 d:/Local/eidb/pt02e.Phases.eidb.1
if errorlevel 1 goto phase21

differ Phases.eidb.2 d:/Local/eidb/pt02e.Phases.eidb.2
if errorlevel 1 goto phase22

exPhase -end -atg -wild Phases.eidb
differ Phases.eidb.0 d:/Local/eidb/pt03e.Phases.eidb.0
if errorlevel 1 goto phase30

differ Phases.eidb.1 d:/Local/eidb/pt03e.Phases.eidb.1
if errorlevel 1 goto phase31

differ Phases.eidb.2 d:/Local/eidb/pt03e.Phases.eidb.2
if errorlevel 1 goto phase32

erase Phases.eidb

REM --------------------------------------------------------------------------
REM generate regression test
generate -gene:32 -seed:32 -hcdm d:/local/eidb/generate.dat >generate01.new
differ generate01.new d:/Local/eidb/generate01.old
if errorlevel 1 goto generate01

generate -gene:32 -seed:32 -show d:/local/eidb/generate.dat AC.CT AG.GT >generate02.new
differ generate02.new d:/Local/eidb/generate02.old
if errorlevel 1 goto generate02

generate -gene:100000 -seed:32 d:/local/eidb/generate.dat AC.CT AG.GT >generate03.new
differ generate03.new d:/Local/eidb/generate03.old
if errorlevel 1 goto generate03

REM --------------------------------------------------------------------------
REM Big file exscan
exscan -v- -wild /Local/eidb/gb115.dEID                AACCTTGG GGTTCCAA >regress51.new
differ regress51.new /Local/eidb/regress51.old
if errorlevel 1 goto exscan51

exscan -v- -wild -first /Local/eidb/gb115.dEID         AACCTTGG GGTTCCAA >regress52.new
differ regress52.new /Local/eidb/regress52.old
if errorlevel 1 goto exscan52

exscan -v- -wild -maxsize:400 /Local/eidb/gb115.dEID   AACCTTGG GGTTCCAA >regress53.new
differ regress53.new /Local/eidb/regress53.old
if errorlevel 1 goto exscan53

exscan -v- -wild -minsize:32 /Local/eidb/gb115.dEID    AACCTTGG GGTTCCAA >regress54.new
differ regress54.new /Local/eidb/regress54.old
if errorlevel 1 goto exscan54

exscan -v- -wild -rev /Local/eidb/gb115.dEID           AACCTTGG GGTTCCAA >regress55.new
differ regress55.new /Local/eidb/regress55.old
if errorlevel 1 goto exscan55

REM Big file inscan
inscan -v- -wild /Local/eidb/gb115.dEID                aaccttgg ggttccaa >regress61.new
differ regress61.new /Local/eidb/regress61.old
if errorlevel 1 goto inscan61

inscan -v- -wild -maxsize:400 /Local/eidb/gb115.dEID   aaccttgg ggttccaa >regress63.new
differ regress63.new /Local/eidb/regress63.old
if errorlevel 1 goto inscan63

inscan -v- -wild -minsize:32 /Local/eidb/gb115.dEID    aaccttgg ggttccaa >regress64.new
differ regress64.new /Local/eidb/regress64.old
if errorlevel 1 goto inscan64

inscan -v- -wild -rev /Local/eidb/gb115.dEID           aaccttgg ggttccaa >regress65.new
differ regress65.new /Local/eidb/regress65.old
if errorlevel 1 goto inscan65

REM --------------------------------------------------------------------------
REM Other bigfile tests
exscan -v- -wild -sum /Local/eidb/gb115.dEID           GAC ATG >exscan70.new
differ exscan70.new /Local/eidb/exscan70.old
if errorlevel 1 goto exscan70

exscan -v- -atg  -sum /Local/eidb/gb115.dEID           GAC ATG >exscan71.new
differ exscan71.new /Local/eidb/exscan71.old
if errorlevel 1 goto exscan71

exscan -v- -atg -wild -sum /Local/eidb/gb115.dEID      GAC ATG >exscan72.new
differ exscan72.new /Local/eidb/exscan72.old
if errorlevel 1 goto exscan72

border -v- -wild -all16 d:/Local/eidb/gb115.dEID AACC.TTGG GGT.TCCAA >border71.new
differ border71.new /Local/eidb/border71.old
if errorlevel 1 goto border71

border3 -v- -wild -all16 d:/Local/eidb/gb115.dEID AACC.TTGG GGT.TCCAA >border73.new
differ border73.new /Local/eidb/border73.old
if errorlevel 1 goto border73

generate -gene:100000 -peakmin:100 -peakmax:600 -peakscale:100 -seed:32 -symmetric d:/local/eidb/generate.dat AC.CT:.9 AG.GT:.1 >generate70.new
differ generate70.new /Local/eidb/generate70.old
if errorlevel 1 goto generate70

generate -gene:100000 -stop:200 -seed:32 -symmetric d:/local/eidb/generate.dat AC.CT:.9 AG.GT:.1 >generate71.new
differ generate71.new /Local/eidb/generate71.old
if errorlevel 1 goto generate71

REM --- Regression test successful -------------------------------------------
echo OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!OK!!
echo Regression test successful!!
goto exit

:exit
REM End of file
