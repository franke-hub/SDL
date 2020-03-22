@ECHO OFF
REM ##########################################################################
REM
REM      Copyright (C) Frank Eskesen 2018.
REM
REM      This file is free content, distributed under the MIT license.
REM      (See accompanying file LICENSE.MIT or the original contained
REM      within https://opensource.org/licenses/MIT)
REM
REM ##########################################################################
REM
REM Title-
REM      INSTALL.BAT
REM
REM Purpose-
REM      Install WINDOWS version of JPEG library.
REM
REM Last change date-
REM      2018/01/01
REM
REM ##########################################################################

REM ##########################################################################
REM Check prerequisites
REM   1) profile.bat and setup.bat cc must have been run
REM   2) Required environment targets
REM   3) Need unzip source, NOT EXTRACTED
REM   4) Distribution and backup targets must not simultaneously exist
REM ##########################################################################
goto prereqs
:ERR_01a
echo ERROR: HDRV or HOME envionment variables not set.
echo (You didn't run profile.bat)
goto exit

:ERR_01b
echo ERROR: CC environment variable missing.
echo (You didn't run setup.bat cc)
goto exit

:ERR_02
echo ERROR: This script can only be run within the distribution environment
echo Please re-read .\README.BUILD
goto exit

:ERR_03a
echo ERROR: Both "%_ROOT%\src\c\lib\jpeg.win" and "%_ROOT%\src\c\lib\jpeg.win.old" exist
goto exit

:ERR_03b
echo ERROR: Both "%_ROOT%\src\c\inc\jpeg\win" and "%_ROOT%\src\c\inc\jpeg\win.old" exist
goto exit

:ERR_03c
echo ERROR: Both "%_ROOT%\src\cpp\inc\jpeg\win" and "%_ROOT%\src\cpp\inc\jpeg\win.old" exist
goto exit

:ERR_04a
echo ERROR: Missing jpeg zip file.
goto exit

:ERR_04b
echo ERROR: Jpeg zip file name "%_ZIP%" not in expected jpeg??.zip format.
echo SEE INSTALL.BAT SCRIPT.
goto exit

REM ##########################################################################
REM Verify environment
REM ##########################################################################
:prereqs
set "_INIT=%cd%"
cd ..\..\..\..\..
set "_ROOT=%cd%"
cd  "%_INIT%"

if .%HDRV% == . goto ERR_01a
if .%HOME% == . goto ERR_01a
if .%CC%   == . goto ERR_01b

if not exist "%_ROOT%\obj\c\lib\Makefile" goto ERR_02
if not exist "%_ROOT%\obj\c\lib\jpeg\Makefile" goto ERR_02
if not exist "%_ROOT%\obj\cpp\lib\jpeg\Makefile" goto ERR_02

if not exist "%_ROOT%\src\c\lib\jpeg.win" goto AOK_03a
if exist "%_ROOT%\src\c\lib\jpeg.win.old" goto ERR_03a
:AOK_03a

if not exist "%_ROOT%\src\c\inc\jpeg\win" goto AOK_03b
if exist "%_ROOT%\src\c\inc\jpeg\win.old" goto ERR_03b
:AOK_03b

if not exist "%_ROOT%\src\cpp\inc\jpeg\win" goto AOK_03c
if exist "%_ROOT%\src\cpp\inc\jpeg\win.old" goto ERR_03c
:AOK_03c

if not exist jpeg*.zip goto ERR_04a
for %%F in ("jpeg*.zip") do set "_ZIP=%%F"

REM ##########################################################################
REM ZIP FILE NAME FORMAT CHECK. Need JPEGSR??.ZIP format to continue.
REM ##########################################################################
if NOT "%_ZIP:~0,6%" == "jpegsr" goto ERR_04b
if NOT "%_ZIP:~8,4%" == ".zip" goto ERR_04b
set "_ZIPDIR=jpeg-%_ZIP:~6,2%"

REM ##########################################################################
REM Create the library source
REM ##########################################################################
:AOK_100
@ECHO ON
if exist "%_ZIPDIR%" rmdir "%_ZIPDIR%" /s /q
unzip "%_ZIP%"
@if %ERRORLEVEL% == 0 goto AOK_101
echo ERROR: unzip "%_ZIP%" failed
goto exit

:AOK_101
copy README.*    "%_ZIPDIR%\."
copy Makefile.*  "%_ZIPDIR%\."

if exist "%_ROOT%\src\c\lib\jpeg.win"   rename "%_ROOT%\src\c\lib\jpeg.win"   jpeg.win.old
if exist "%_ROOT%\src\c\inc\jpeg\win"   rename "%_ROOT%\src\c\inc\jpeg\win"   win.old
if exist "%_ROOT%\src\cpp\inc\jpeg\win" rename "%_ROOT%\src\cpp\inc\jpeg\win" win.old
mkdir "%_ROOT%\src\c\inc\jpeg\win"
mkdir "%_ROOT%\src\cpp\inc\jpeg\win"
copy "%_ZIPDIR%\jconfig.vc" "%_ZIPDIR%\jconfig.h"
copy "%_ZIPDIR%\j*.h" "%_ROOT%\src\c\inc\jpeg\win\."
copy "%_ZIPDIR%\j*.h" "%_ROOT%\src\cpp\inc\jpeg\win\."
move "%_ZIPDIR%"      "%_ROOT%\src\c\lib\jpeg.win"
mklink /J "%_ROOT%"\src\c\inc\jpeg\win\README           "%_ROOT%"\src\c\lib\jpeg.win\README
mklink /J "%_ROOT%"\src\c\inc\jpeg\win\README.BUILD     "%_ROOT%"\src\c\lib\jpeg.win\README.BUILD
mklink /J "%_ROOT%"\src\c\inc\jpeg\win\README.INCLUDE   "%_ROOT%"\src\c\lib\jpeg.win\README.INCLUDE
mklink /J "%_ROOT%"\src\cpp\inc\jpeg\win\README         "%_ROOT%"\src\c\lib\jpeg.win\README
mklink /J "%_ROOT%"\src\cpp\inc\jpeg\win\README.BUILD   "%_ROOT%"\src\c\lib\jpeg.win\README.BUILD
mklink /J "%_ROOT%"\src\cpp\inc\jpeg\win\README.INCLUDE "%_ROOT%"\src\c\lib\jpeg.win\README.INCLUDE
goto exit

:AOK_200
cd "%_ROOT%\obj\c\lib\jpeg
make pristine
cd ..
make libjpeg.lib
if %ERRORLEVEL% == 0 goto AOK_210
echo ERROR: ~\obj\c\lib\make failed
goto exit

:AOK_210
cd "%_ROOT%\obj\cpp\lib\jpeg
make pristine
cd ..
make libjpeg.lib
@if %ERRORLEVEL% == 0 goto AOK_999
echo ERROR: ~\obj\cpp\lib\make failed
goto exit

:AOK_999
@ECHO OFF
echo "!!SUCCESS!!"

:exit
cd  "%_INIT%"
set "_INIT="
set "_ROOT="
set "_ZIP="
set "_ZIPDIR="

