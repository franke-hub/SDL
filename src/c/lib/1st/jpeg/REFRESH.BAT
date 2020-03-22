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
REM      REFRESH.BAT
REM
REM Purpose-
REM      Remove WINDOWS version of JPEG library and include files.
REM
REM Last change date-
REM      2018/01/01
REM
REM ##########################################################################

REM ##########################################################################
REM Check whether any remove directories exist
REM ##########################################################################
set "_INIT=%cd%"
cd ..\..\..\..\..
set "_ROOT=%cd%"
cd  "%_INIT%"

if exist "%_ROOT%\src\c\inc\jpeg\win"   goto found
if exist "%_ROOT%\src\cpp\inc\jpeg\win" goto found
if exist "%_ROOT%\src\c\lib\jpeg.win"   goto found
echo Nothing to do.
goto exit

REM ##########################################################################
REM WARNING MESSAGE
REM ##########################################################################
:found
echo ** WARNING ** You are about to remove:
if exist "%_ROOT%\src\c\inc\jpeg\win"   echo "%_ROOT%\src\c\inc\jpeg\win"
if exist "%_ROOT%\src\cpp\inc\jpeg\win" echo "%_ROOT%\src\cpp\inc\jpeg\win"
if exist "%_ROOT%\src\c\lib\jpeg.win"   echo "%_ROOT%\src\c\lib\jpeg.win"
echo
set /p _SURE=** ARE YOU SURE ** [y/N]?:
if "%_SURE%" == "Y" goto sure
if "%_SURE%" == "YES" goto sure
if "%_SURE%" == "y" goto sure
if "%_SURE%" == "yes" goto sure
echo OPERATION CANCELLED
goto exit

:sure
if exist "%_ROOT%\src\c\inc\jpeg\win"   rmdir "%_ROOT%\src\c\inc\jpeg\win"   /s /q
if exist "%_ROOT%\src\cpp\inc\jpeg\win" rmdir "%_ROOT%\src\cpp\inc\jpeg\win" /s /q
if exist "%_ROOT%\src\c\lib\jpeg.win"   rmdir "%_ROOT%\src\c\lib\jpeg.win"   /s /q

:exit
cd  "%_INIT%"
set "_INIT="
set "_ROOT="
set "_SURE="

