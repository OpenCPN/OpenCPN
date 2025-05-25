@echo off
:: ***************************************************************************
:: *
:: * Project:  OpenCPN
:: * Purpose:  Windows local build script
:: * Author:   Dan Dickey
:: *
:: ***************************************************************************
:: *   Copyright (C) 2010-2023 by David S. Register                          *
:: *                                                                         *
:: *   This program is free software; you can redistribute it and/or modify  *
:: *   it under the terms of the GNU General Public License as published by  *
:: *   the Free Software Foundation; either version 2 of the License, or     *
:: *   (at your option) any later version.                                   *
:: *                                                                         *
:: *   This program is distributed in the hope that it will be useful,       *
:: *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
:: *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
:: *   GNU General Public License for more details.                          *
:: *                                                                         *
:: *   You should have received a copy of the GNU General Public License     *
:: *   along with this program; if not, write to the                         *
:: *   Free Software Foundation, Inc.,                                       *
:: *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
:: ***************************************************************************
:: *
::
:: Revision 2> 2023/08/19 Dan Dickey
::  - Add support for auto rebuilding. To rebuild just run winConfig.bat with
::    no arguments
::
:: Revision 3> 2024/04/05 Dan Dickey
::  - If possible, download wxWidgets from github repository so we always have
::    the latest source.  wxWidgets do not always create source .zip bundles
::    for minor patches but the github repository is always up to date.
::  - Various improvements to wxWidgets download and build speed.
::  - Make improvements to error handling and persistent configuration settings
call :StartTimer
:: If not running in VS Command Prompt try to find VS and set up the environment
@echo VisualStudioVersion=%VisualStudioVersion%
if [%VisualStudioVersion%]==[] (
  @echo Searching for Visual Studio
  for /f "delims=" %%G in ('where /r "%ProgramFiles%" /f VsDevCmd.bat') do (
    set "vsenv=%%G"
    goto :setenv
  )

  for /f "delims=" %%G in ('where /r "\Program Files (x86)" /f VsDevCmd.bat') do (
    set "vsenv=%%G"
    goto :setenv
  )
)
goto go
:setenv
@echo call %vsenv%
call %vsenv%
set vsenv=
:go
setlocal enabledelayedexpansion
if [%VisualStudioVersion%]==[] (
  @echo Please install Visual Studio 2022 or 2019
  exit /b 1
)
goto :main
:usage
@echo ****************************************************************************
@echo *  This script can be used to create a local win32 build environment.      *
@echo *                                                                          *
@echo *  There are some prequisites before you can effectively use this script.  *
@echo *                                                                          *
@echo *  1. Install Visual Studio 2022 Community Edition.                        *
@echo *              https://visualstudio.microsoft.com/downloads/               *
@echo *     Be sure to select the V143 latest dev tools and                      *
@echo *     cmake tools check boxes.                                             *
@echo *                                                                          *
@echo *  2. Install git for Windows (or select it from the VS 2022 installer)    *
@echo *              https://git-scm.com/download/win                            *
@echo *                                                                          *
@echo *  3. Open 'x86 Native Tools Command Prompt for Visual Studio 2022'        *
@echo *                                                                          *
@echo *  4. Create/move to folder where you want to work with OpenCPN sources    *
@echo *        Example: mkdir \Users\myname\source\repos                         *
@echo *                 cd \Users\myname\source\repos                            *
@echo *                                                                          *
@echo *  5. Clone Opencpn                                                        *
@echo *        Example: git clone https://github.com/OpenCPN/OpenCPN             *
@echo *                 cd \Users\myname\source\repos\opencpn                    *
@echo *                                                                          *
@echo *  6. Setup local build environment by executing this script               *
@echo *        Example: .\buildwin\winConfig.bat --debug --relwithdebinfo        *
@echo *                                                                          *
@echo *  7. Open solution file                                                   *
@echo *       (type the solution file name at VS command prompt)                 *
@echo *        Example: .\build\opencpn.sln                                      *
@echo *                                                                          *
@echo *  8. Start building and debugging in Visual Studio.                       *
@echo *                                                                          *
@echo *  Command line options (the first time):                                  *
@echo *      --release          Build Release configuration                      *
@echo *      --relwithdebinfo   Build RelWithDebInfo configuration               *
@echo *      --minsizerel       Build MinSizeRel configuration                   *
@echo *      --debug            Build Debug configuration                        *
@echo *                                                                          *
@echo *      --all              Build all 4 configurations  (default)            *
@echo *                                                                          *
@echo *      --package          Create installer package after building          *
@echo *                                                                          *
@echo *     ***************************************************************      *
@echo *     * By default, the first time you run this script all 4        *      *
@echo *     * configuration types of builds are created. If you don't     *      *
@echo *     * want that, select the build types you need using above      *      *
@echo *     * options.                                                    *      *
@echo *     ***************************************************************      *
@echo *      --Y                Non-interactive mode (for calling from a script) *
@echo *      --wxver vn.n[.n]   Download specific version of wxWidgets sources.  *
@echo *      --help             Print this message                               *
@echo *                                                                          *
@echo ****************************************************************************
@echo *  Later, you can use these options to rebuild or clean up the build       *
@echo *  folder:                                                                 *
@echo *                                                                          *
@echo *      --clean            Remove build folder entirely before building     *
@echo *                         MUST HAVE INTERNET CONNECTION FOR clean OPTION   *
@echo *                         By default --clean builds all 4 configurations.  *
@echo *                         You can include the build config options same as *
@echo *                         a first-time build.                              *
@echo *      --rebuild          Rebuild all sources                              *
@echo *     ***************************************************************      *
@echo *     * By default, after the first time, when you run this script  *      *
@echo *     * with no arguments, it will msbuild the originally selected  *      *
@echo *     * configurations.                                             *      *
@echo *     ***************************************************************      *
@echo *                                                                          *
@echo ****************************************************************************
@echo *  Typical workflow:                                                       *
@echo *  1) C:\repos> git clone https://github.com/OpenCPN/OpenCPN               *
@echo *     C:\repos> cd opencpn                                                 *
@echo *  2) Run this script for the first time to build the desired configs      *
@echo *     C:\repos\OpenCPN> .\buildwin\winConfig --relwithdebinfo --debug      *
@echo *  3) Launch Visual Studio:                                                *
@echo *     C:\repos\OpenCPN> .\build\opencpn.sln                                *
@echo *                                                                          *
@echo *  Later you may wish to catch up with the github repository like this:    *
@echo *  C:\repos> cd OpenCPN                                                    *
@echo *  C:\repos\OpenCPN> git pull upstream master                              *
@echo *  C:\repos\OpenCPN> .\buildwin\winConfig                                  *
@echo *  C:\repos\OpenCPN> .\build\opencpn.sln                                   *
@echo *                                                                          *
@echo *  Or, you can do the same steps within Visual Studio using its built-in   *
@echo *  git integration. Sync with upstream then build from the IDE. Either way *
@echo *  will produce the same result.                                           *
@echo *                                                                          *
@echo ****************************************************************************
goto :fail
:main
::-------------------------------------------------------------
:: Initialize local environment
::-------------------------------------------------------------
pushd %~dp0..
set "OCPN_Dir=%CD%"
popd
SET "CACHE_DIR=%OCPN_DIR%\cache"
SET "DATA_DIR=%OCPN_DIR%\data"
set "wxWidgetsURL=https://github.com/wxWidgets/wxWidgets"
set "wxDIR=%OCPN_DIR%\..\ocpn_wxWidgets"
set "wxWidgets_ROOT_DIR=%wxDIR%"
set "wxWidgets_LIB_DIR=%wxDIR%\lib\vc_dll"
set "wxMajor=v3.2"
set "wxMinor=8"
set "wxVER=%wxMajor%.%wxMinor%"
if [%VisualStudioVersion%]==[16.0] (
  set VCver=16
  set "VCstr=Visual Studio 16"
)
if [%VisualStudioVersion%]==[17.0] (
  set VCver=17
  set "VCstr=Visual Studio 17"
)
if [%VisualStudioVersion%]==[18.0] (
  set VCver=18
  set "VCstr=Visual Studio 18"
)
::-------------------------------------------------------------
:: Initialize local variables
::-------------------------------------------------------------
SET "buildWINtmp=%CACHE_DIR%\buildwintemp"
set PSH=powershell
where pwsh > NUL 2> NUL && set PSH=pwsh
where %PSH% > NUL 2> NUL || echo PowerShell is not installed && goto :fail
where msbuild.exe > NUL 2> NUL && goto :vsok
@echo Please run this from "x86 Native Tools Command Prompt for VS2022 or VS2019
goto :usage
:vsok
@echo Searching for Git
for /f "delims=" %%G in ('where /f git') do (
  set "gitdrv=%%~dG"
  set "gitfldr=%%~pG"
  set "gitcmd=%%~G"
)
@echo Searching for git utilities in %gitdrv%%gitfldr%..
@echo Searching for patch in "%gitdrv%%gitfldr%.."
for /f "delims=" %%P in ('where /f /r "%gitdrv%%gitfldr%.." patch.exe') do (set patchcmd=%%~P)
@echo Searching for bash
for /f "delims=" %%B in ('where /f /r "%gitdrv%%gitfldr%.." bash.exe') do (set bashcmd=%%~B)
@echo Finished searching
@echo gitcmd=%gitcmd%
@echo patchcmd=%patchcmd%
@echo bashcmd=%bashcmd%
if not exist "%gitcmd%" (set gitcmd=& echo [101;93mWarning[0m: git not found)
if not exist "%patchcmd%" (set patchcmd=& echo [101;93mWarning[0m: patch not found)
if not exist "%bashcmd%" (set bashcmd=& echo [101;93mWarning[0m: bash not found)
::-------------------------------------------------------------
:: Initialize local helper script that can reinitialize environment
::-------------------------------------------------------------
@echo set "wxDIR=%wxDIR%" > "%OCPN_Dir%\buildwin\configdev.bat"
@echo set "wxWIN=%wxDIR%" >> "%OCPN_Dir%\buildwin\configdev.bat"
@echo set "wxWidgets_ROOT_DIR=%wxWidgets_ROOT_DIR%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "wxWidgets_LIB_DIR=%wxWidgets_LIB_DIR%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "VCver=%VCver%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "VCstr=%VCstr%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "gitcmd=%gitcmd%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "bashcmd=%bashcmd%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "patchcmd=%patchcmd%" >> "%OCPN_DIR%\buildwin\configdev.bat"
:: By default build all 4 possible configurations the first time
:: Edit and set to 1 at least one configuration
set ocpn_all=1
set ocpn_minsizerel=0
set ocpn_release=0
set ocpn_relwithdebinfo=0
set ocpn_debug=0
set quiet=N
set ocpn_package=0
:: Clean up if previous run "failed"
if exist "%~dp0..\build\.MinSizeRel\.Pack" (
  rmdir "%~dp0..\build\.MinSizeRel\.Pack"
)
if exist "%~dp0..\build\.Release\.Pack" (
  rmdir "%~dp0..\build\.Release\.Pack"
)
if exist "%~dp0..\build\.Debug\.Pack" (
  rmdir "%~dp0..\build\.Debug\.Pack"
)
if exist "%~dp0..\build\.RelWithDebInfo\.Pack" (
  rmdir "%~dp0..\build\.RelWithDebInfo\.Pack"
)
:: If this is a rerun then build existing configurations
if exist "%OCPN_DIR%\build\.Debug" (
  set ocpn_all=0
  set ocpn_debug=1
)
if exist "%OCPN_DIR%\build\.Release" (
  set ocpn_all=0
  set ocpn_release=1
)
if exist "%OCPN_DIR%\build\.RelWithDebInfo" (
  set ocpn_all=0
  set ocpn_relwithdebinfo=1
)
if exist "%OCPN_DIR%\build\.MinSizeRel" (
  set ocpn_all=0
  set ocpn_minsizerel=1
)
:parse
if [%1]==[--clean] (shift /1 && set ocpn_clean=1&& set ocpn_rebuild=0&& goto :parse)
if [%1]==[--rebuild] (shift /1 && set ocpn_rebuild=1&& set ocpn_clean=0&& goto :parse)
if [%1]==[--help] (shift /1 && goto :usage)
if [%1]==[--all] (shift /1 && set ocpn_all=1&& goto :parse)
if [%1]==[--minsizerel] (shift /1 && set ocpn_all=0&& set ocpn_minsizerel=1&& goto :parse)
if [%1]==[--release] (shift /1 && set ocpn_all=0&& set ocpn_release=1&& goto :parse)
if [%1]==[--relwithdebinfo] (shift /1 && set ocpn_all=0&& set ocpn_relwithdebinfo=1&& goto :parse)
if [%1]==[--debug] (shift /1 && set ocpn_all=0&& set ocpn_debug=1&& goto :parse)
if [%1]==[--wxver] (shift /1 && set wxVER=%2&& shift /1 && goto :parse)
if [%1]==[--Y] (shift /1 && set "quiet=Y" && goto :parse)
if [%1]==[--package] (shift /1 && set ocpn_package=1&& goto :parse)
if [%1]==[] (goto :begin) else (
  echo Unknown option: %1
  shift /1
  goto :usage
  )
:begin
if [%ocpn_all%]==[1] (
  set ocpn_minsizerel=1
  set ocpn_release=1
  set ocpn_relwithdebinfo=1
  set ocpn_debug=1
)
::-------------------------------------------------------------
:: If this is the first build then initialize all build types
::-------------------------------------------------------------
if not exist "%OCPN_DIR%\build" (
  set ocpn_clean=1
)
::-------------------------------------------------------------
:: Save user configuration data and clean build folders but
:: do not delete downloaded tools.  Use this option if no internet
:: connectivity available.
::-------------------------------------------------------------
if [%ocpn_rebuild%]==[1] (
  echo Beginning rebuild cleanout
  if exist "%OCPN_DIR%\build" (
    set folder=Release
    call :backup
    set folder=RelWithDebInfo
    call :backup
    set folder=Debug
    call :backup
    set folder=MinSizeRel
    call :backup
    set folder=
    echo Backup complete
  )
  set "target=%OCPN_DIR%\build\.vs" & call :removeTarget
  if exist "%OCPN_DIR%\build\CMakeCache.txt" del "%OCPN_DIR%\build\CMakeCache.txt"
  if exist "%OCPN_DIR%\.git\hooks\pre-commit" del "%OCPN_DIR%\.git\hooks\pre-commit"
  set "target=%OCPN_DIR%\build\Release" & call :removeTarget
  set "target=%OCPN_DIR%\build\RelWithDebInfo" & call :removeTarget
  set "target=%OCPN_DIR%\build\Debug" & call :removeTarget
  set "target=%OCPN_DIR%\build\MinSizeRel" & call :removeTarget
  set "target=%OCPN_DIR%\build\CMakeFiles" & call :removeTarget
  set "target=%OCPN_DIR%\build\ocpn.dir" & call :removeTarget
  set "target=%OCPN_DIR%\build\plugins" & call :removeTarget
  set "target=%OCPN_DIR%\build\cli" & call :removeTarget
  set "target=%OCPN_DIR%\build\lib" & call :removeTarget
  set "target=%OCPN_DIR%\build\libs" & call :removeTarget
  set "target=%OCPN_DIR%\build\glutil" & call :removeTarget
  set "target=%OCPN_DIR%\build\model" & call :removeTarget
  set "target=%OCPN_DIR%\build\include" & call :removeTarget
  set "target=%OCPN_DIR%\build\opencpn.dir" & call :removeTarget
  set "target=%OCPN_DIR%\build\Resources" & call :removeTarget
  set "target=%OCPN_DIR%\build\Win32" & call :removeTarget
  set "target=%OCPN_DIR%\build\_deps" & call :removeTarget
  where /Q /R "%OCPN_DIR%\build" *.cmake && del /Q "%OCPN_DIR%\build\*.cmake"
  where /Q /R "%OCPN_DIR%\build" *.txt && del /Q "%OCPN_DIR%\build\*.txt"
  where /Q /R "%OCPN_DIR%\build" *.in && del /Q "%OCPN_DIR%\build\*.in"
  where /Q /R "%OCPN_DIR%\build" *.xml && del /Q "%OCPN_DIR%\build\*.xml"
  where /Q /R "%OCPN_DIR%\build" *.rc && del /Q "%OCPN_DIR%\build\*.rc"
  where /Q /R "%OCPN_DIR%\build" *.user && del /Q "%OCPN_DIR%\build\*.user"
  where /Q /R "%OCPN_DIR%\build" *.sln && del /Q "%OCPN_DIR%\build\*.sln"
  where /Q /R "%OCPN_DIR%\build" *.mo && del /Q "%OCPN_DIR%\build\*.mo"
  where /Q /R "%OCPN_DIR%\build" *.vcxproj && del /Q "%OCPN_DIR%\build\*.vcxproj"
  where /Q /R "%OCPN_DIR%\build" *.filters && del /Q "%OCPN_DIR%\build\*.filters"
  where /Q /R "%OCPN_DIR%\build" *.log && del /Q "%OCPN_DIR%\build\*.log"
  @echo Finished rebuild cleanout
)
::-------------------------------------------------------------
:: Save user configuration data and wipe the build folder
::-------------------------------------------------------------
:: Check if network is available
set netok=0
if not ["%gitcmd%"]==[] (
  @echo Checking network connection...
  @echo %cd%
  "%gitcmd%" fetch --dry-run >nul 2>&1 && set netok=1
)
@echo netok=%netok%
if [%ocpn_clean%]==[1] (
  if [%netok%]==[1] echo Network ok
  if [%netok%]==[0] echo Network not working

  set ocpn_clean=0
  set ocpn_rebuild=0
  if %netok%==0 (
    if not [%quiet%]==[Y] (
      @echo [101;93mThe --clean option requires an internet connection.[0m
      @echo The internet connection appears to be down. Proceed with caution.
      choice /C YN /T 10 /M "Remove entire build folder including downloaded tools? [yN]" /D N
      if ERRORLEVEL==2  goto :usage
    )
  )
  if exist "%OCPN_DIR%\build" (
    set folder=Release
    call :backup
    set folder=RelWithDebInfo
    call :backup
    set folder=Debug
    call :backup
    set folder=MinSizeRel
    call :backup
    set folder=
    @echo Backup complete
  )
  if exist "%OCPN_DIR%\build" rmdir /s /q "%OCPN_DIR%\build"
  if exist "%OCPN_DIR%\build" (
    @echo Could not remove "%OCPN_DIR%\build" folder
    @echo Is Visual Studio IDE open? If so, please close it so we can try again.
    if not [%quiet%]==[Y] pause
    @echo Retrying...
    rmdir /s /q "%OCPN_DIR%\build"
  )
  if exist "%OCPN_DIR%\build" (
    @echo Could not remove "%OCPN_DIR%\build". Continuing...
  ) else (
    @echo Cleared %OCPN_DIR%\build OK.
  )

  if exist "%CACHE_DIR%" (rmdir /s /q "%CACHE_DIR%" && echo Cleared %CACHE_DIR%)
  if exist "%wxDIR%" (rmdir /s /q "%wxDIR%" && echo Cleared %wxDIR%)
  if exist "%buildWINtmp%" (rmdir /s /q "%buildWINtmp%" && echo Cleared %buildWINtmp%)
  if not [%quiet%]==[Y] (
    timeout /T 5
  )
)
::-------------------------------------------------------------
:: Create needed folders
::-------------------------------------------------------------
if not exist "%OCPN_DIR%\build" (mkdir "%OCPN_DIR%\build")
if not exist "%CACHE_DIR%" (mkdir "%CACHE_DIR%")
if not exist "%CACHE_DIR%\buildwin" (mkdir "%CACHE_DIR%\buildwin")
if not exist "%buildWINtmp%" (mkdir "%buildWINtmp%")
::-------------------------------------------------------------
:: Install nuget
::-------------------------------------------------------------
where /Q /R %CACHE_DIR% nuget.exe && "%CACHE_DIR%\nuget.exe" >nul && goto :skipnuget
@echo Downloading nuget
set "URL=https://dist.nuget.org/win-x86-commandline/latest/nuget.exe"
set "DEST=%CACHE_DIR%\nuget.exe"
call :download
where /Q /R %CACHE_DIR% nuget.exe && goto :skipnuget
@echo Error: Could not download nuget.exe
goto :usage
:skipnuget

:: Is CMake present and the correct version?
where /Q cmake.exe && goto :skipcmake
@echo Error: Could not find required CMake tools.
goto :usage
:skipcmake
:: Run cmake --version and capture the output
for /f "tokens=3" %%i in ('cmake --version') do (
    set CMAKE_VERSION=%%i
    goto :cmdone
)
:cmdone
:: echo CMake version: %CMAKE_VERSION%
:: Split the version number into major, minor, and patch components
for /f "tokens=1,2,3 delims=." %%a in ("%CMAKE_VERSION%") do (
    set CMAKE_MAJOR=%%a
    set CMAKE_MINOR=%%b
    set CMAKE_PATCH=%%c
)
:: @echo CMake version: %CMAKE_MAJOR%.%CMAKE_MINOR%.%CMAKE_PATCH%

:: Check if the version is greater than 3.26
if %CMAKE_MAJOR% gtr 3 goto :cmake_ok
:: @echo CMMAKE_MAJOR is gtr 2.
:: @echo CMMAKE_MINOR=%CMAKE_MINOR%
if %CMAKE_MAJOR% gtr 2 (
  if %CMAKE_MINOR% gtr 25 (
    goto :cmake_ok
  )
)
@echo Error: CMake version is less than 3.26
if not [%quiet%]==[Y] pause
goto :usage

:cmake_ok
@echo CMake version is not less than 3.26

::-------------------------------------------------------------
:: Install pre-commit hooks if not already installed
:: This will only run once
::-------------------------------------------------------------
if exist ".\.git\hooks\pre-commit" (set PRECOMMIT_OK=1)
if DEFINED PRECOMMIT_OK (goto :skipPreCommit)
@echo Installing python
pushd cache
%CACHE_DIR%\nuget install python
for /f "delims=" %%G in ('where /r . /f python.exe') do (
  set "PYAPP=%%G"
  set "PYFLDR=%%~pG"
)
if DEFINED PYAPP (
  @echo Installing pre-commit hooks
  %PYAPP% -m pip install --no-warn-script-location --upgrade pip
  %PYAPP% -m pip install --no-warn-script-location -q pre-commit
  for /f "delims=" %%P in ('where /R . /F pre-commit.exe') do (%%P install -f && set PRECOMMIT_OK=1)
)
popd
:skipPreCommit
if not DEFINED PRECOMMIT_OK (.
  @echo Error: Could not find Python and/or pre-commit tool.
  if not [%quiet%]==[Y] pause
)

::-------------------------------------------------------------
:: Download OpenCPN Core dependencies
::-------------------------------------------------------------
if exist "%buildWINtmp%\OCPNWindowsCoreBuildSupport.zip" (goto :skipbuildwin)
@echo Downloading Windows depencencies from OpenCPN repository
set "URL=https://github.com/OpenCPN/OCPNWindowsCoreBuildSupport/archive/refs/tags/v0.5.zip"
set "DEST=%buildWINtmp%\OCPNWindowsCoreBuildSupport.zip"
call :download

@echo Exploding Windows dependencies
set "SOURCE=%DEST%"
set "DEST=%buildWINtmp%"
call :explode
if errorlevel 1 (echo [101;93mNOT OK[0m ) else (
  cmake -E copy_directory_if_different "%buildWINtmp%\OCPNWindowsCoreBuildSupport-0.5\buildwin" "%CACHE_DIR%\buildwin"
  if errorlevel 1 (
    @echo [101;93mNOT OK[0m
    if not [%quiet%]==[Y] pause
  ) else (
    @echo Windows dependencies OK
  )
)
:skipbuildwin
set URL="https://dl.cloudsmith.io/public/david-register/opencpn-docs/raw/files/QuickStartGuide-v0.4.zip"
set "DEST=%CACHE_DIR%\QuickStartManual.zip"
if not exist "%DEST%" (
  @echo Downloading quickstart manual
  call :download
  if exist "%CACHE_DIR%\..\data\doc\local" rmdir /s /q "%CACHE_DIR%\..\data\doc\local"
  mkdir "%CACHE_DIR%\..\data\doc\local"
  set "SOURCE=%DEST%"
  set "DEST=%CACHE_DIR%\..\data\doc\local"
  @echo Exploding quickstart manual
  call :explode
)
if exist "%VCToolsRedistDir%\x86\Microsoft.VC143.CRT\msvcp140.dll" (
  @echo Updating VC runtime ...
  if not exist "%CACHE_DIR%\buildwin" mkdir "%CACHE_DIR%\buildwin"
  if not exist "%CACHE_DIR%\buildwin\vc" mkdir "%CACHE_DIR%\buildwin\vc"
  cmake -E copy_directory_if_different "%VCToolsRedistDir%\x86\Microsoft.VC143.CRT" "%CACHE_DIR%\buildwin\vc"
  if errorlevel 1 (
    @echo [101;93mNOT OK[0m
    if not [%quiet%]==[Y] pause
  ) else (
    echo OK
  )
)
:: Remove old wxWidgets build folder if present
if exist "%CACHE_DIR%\buildwxWidgets" (rmdir /s /q "%CACHE_DIR%\buildwxWidgets" && echo buildwxWidgets cleared)
::-------------------------------------------------------------
:: Download wxWidgets sources
::-------------------------------------------------------------
if exist "%wxDIR%\build\msw\wx_vc%VCver%.sln" (goto :skipwxDL)
@echo INFO: Could not find "%wxDIR%\build\msw\wx_vc%VCver%.sln"
@echo Downloading wxWidgets sources
if "[%gitcmd%]"=="[]" (
  if not exist "%wxDIR%" (mkdir "%wxDIR%")
  :: set "URL=https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.4/wxWidgets-3.2.4.zip"
  set "URL=https://github.com/wxWidgets/wxWidgets/releases/download/%wxVER%/wxWidgets-%wxVER%.zip"
  set "DEST=%wxDIR%\wxWidgets-%wxVER%.zip"
  call :download
  if errorlevel 1 (echo [101;93mNOT OK[0m ) else (echo Download %DEST% OK )

  @echo exploding wxWidgets
  set "SOURCE=%wxDIR%\wxWidgets-%wxVER%.zip"
  set "DEST=%wxDIR%"
  call :explode
  if errorlevel 1 (echo [101;93mNOT OK[0m ) else (echo Explode wxWidgets OK )
) else (
  @echo %gitcmd% clone --jobs 2 --depth 1 --recurse-submodules --shallow-submodules ^
         --branch %wxVer% "%wxWidgetsURL%" "%wxDIR%"
  "%gitcmd%" clone --jobs 2 --depth 1 --recurse-submodules --shallow-submodules ^
   --branch %wxVer% "%wxWidgetsURL%" "%wxDIR%"
  if errorlevel 1 (echo Git clone [101;93mNOT OK[0m&&goto :fail )
)
:skipwxDL
if exist "%wxDIR%\build\3rdparty\webview2" goto :wxBuild
@echo Downloading Windows WebView2 kit
set "URL=https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2"
set "DEST=%wxDIR%\webview2.zip"
call :download
if errorlevel 1 (echo [101;93mNOT OK[0m ) else (echo Download %DEST% OK )

@echo Exploding WebView2
set "SOURCE=%wxDIR%\webview2.zip"
set "DEST=%wxDIR%\build\3rdparty\webview2"
call :explode
if errorlevel 1 (echo [101;93mNOT OK[0m ) else (echo Explode WebView2 OK )

:wxBuild
::-------------------------------------------------------------
:: Build wxWidgets from sources
::-------------------------------------------------------------
if exist "%wxDIR%\.git" (
  if not "[%gitcmd%]"=="[]" (
    pushd "%wxDIR%"
    "%gitcmd%" submodule update
    "%gitcmd%" fetch --recurse-submodules
    "%gitcmd%" checkout "%wxVer%" --recurse-submodules --force
    popd
  )
)
if not exist "%wxDIR%\lib\vc_dll" (
  mkdir "%wxDIR%\lib\vc_dll"
  @echo Building wxWidgets libraries...
  set wxVerb=minimal
) else (
  @echo Checking wxWidgets libraries...this may take a few minutes...
  set wxVerb=quiet
)
msbuild "%wxDIR%\build\msw\wx_vc%VCver%.sln" ^
  -noLogo -verbosity:%wxVerb% -maxCpuCount ^
  -property:UseMultiToolTask=true ^
  -property:EnableClServerMode=true ^
  -property:BuildPassReferences=true ^
  -property:"Configuration=DLL Debug";Platform=Win32 ^
  -property:wxVendor=14x;wxVersionString=32;wxToolkitDllNameSuffix=_vc14x ^
  -logger:FileLogger,Microsoft.Build.Engine;logfile="%wxDIR%\lib\vc_dll\MSBuild_DEBUG_WIN32.log"
if errorlevel 1 (
  echo wxWidgets Debug build [101;93mNOT OK[0m
  goto :buildErr
)
echo wxWidgets Debug build OK
msbuild "%wxDIR%\build\msw\wx_vc%VCver%.sln" ^
  -noLogo -verbosity:%wxVerb% -maxCpuCount ^
  -property:DebugSymbols=true^
  -property:DebugType=pdbonly^
  -property:UseMultiToolTask=true^
  -property:EnableClServerMode=true ^
  -property:BuildPassReferences=true ^
  -property:"Configuration=DLL Release";Platform=Win32 ^
  -property:wxVendor=14x;wxVersionString=32;wxToolkitDllNameSuffix=_vc14x ^
  -logger:FileLogger,Microsoft.Build.Engine;logfile="%wxDIR%\lib\vc_dll\MSBuild_RELEASE_WIN32.log"
if errorlevel 1 (
  echo wxWidgets Release build [101;93mNOT OK[0m
  goto :buildErr
)
echo wxWidgets Release build OK

for /f "tokens=*" %%p in ('dir "%WXDIR%\lib\vc_dll\wxmsw32*.dll" /b') do (
  ::@echo  copy_if_different "%WXDIR%\lib\vc_dll\%%p" "%CACHE_DIR%\buildwin\wxWidgets\%%~np%%~xp"
  cmake -E copy_if_different "%WXDIR%\lib\vc_dll\%%p" "%CACHE_DIR%\buildwin\wxWidgets\%%~np%%~xp"
  if errorlevel 1 (
    echo wxWidgets is broken and [101;93mNOT OK[0m
    goto :buildErr
  )
)
for /f "tokens=*" %%p in ('dir "%WXDIR%\lib\vc_dll\wxmsw32*.pdb" /b') do (
  ::@echo  copy_if_different "%WXDIR%\lib\vc_dll\%%p" "%CACHE_DIR%\buildwin\wxWidgets\%%~np%%~xp"
  cmake -E copy_if_different "%WXDIR%\lib\vc_dll\%%p" "%CACHE_DIR%\buildwin\wxWidgets\%%~np%%~xp"
  if errorlevel 1 (
    echo wxWidgets is broken and [101;93mNOT OK[0m
    goto :buildErr
  )
)
@echo Copying wxWidgets libraries
cmake -E copy_directory_if_different "%WXDIR%\lib\vc_dll" "%CACHE_DIR%\buildwin\wxWidgets"
if errorlevel 1 (
  echo wxWidgets library copy [101;93mNOT OK[0m
  goto :buildErr
)
@echo Copying wxWidgets locale
cmake -E copy_directory_if_different "%WXDIR%\locale" "%CACHE_DIR%\buildwin\wxWidgets\locale"
if errorlevel 1 (
  echo locale copy [101;93mNOT OK[0m
  goto :buildErr
)
::-------------------------------------------------------------
:: Initialize folders needed to run OpenCPN
::-------------------------------------------------------------
@echo ocpn_relwithdebinfo=%ocpn_relwithdebinfo%
@echo ocpn_release=%ocpn_release%
@echo ocpn_debug=%ocpn_debug%
@echo ocpn_minsizerel=%ocpn_minsizerel%

if [%ocpn_debug%]==[1] (
  if not exist "%OCPN_DIR%\build\.Debug" (mkdir "%OCPN_DIR%\build\.Debug")
  if [%ocpn_package%]==[1] (mkdir "%OCPN_DIR%\build\.Debug\.Pack")
  )
if [%ocpn_release%]==[1] (
  if not exist "%OCPN_DIR%\build\.Release" (mkdir "%OCPN_DIR%\build\.Release")
  if [%ocpn_package%]==[1] (mkdir "%OCPN_DIR%\build\.Release\.Pack")
  )
if [%ocpn_relwithdebinfo%]==[1] (
  if not exist "%OCPN_DIR%\build\.RelWithDebInfo" (mkdir "%OCPN_DIR%\build\.RelWithDebInfo")
  if [%ocpn_package%]==[1] (mkdir "%OCPN_DIR%\build\.RelWithDebInfo\.Pack")
  )
if [%ocpn_minsizerel%]==[1] (
  if not exist "%OCPN_DIR%\build\.MinSizeRel" (mkdir "%OCPN_DIR%\build\.MinSizeRel")
  if [%ocpn_package%]==[1] (mkdir "%OCPN_DIR%\build\.MinSizeRel\.Pack")
)
::-------------------------------------------------------------
:: Download and initialize build dependencies
::-------------------------------------------------------------
set "IPHDEST=%CACHE_DIR%\buildwin"
set "DEST=%CACHE_DIR%\buildwin\iphlpapi.lib
if not exist "%DEST%" (
  if exist "%WindowsSdkDir%\lib\%WindowsSdkLibVersion%\um\x86\iphlpapi.lib" (
    if not exist "%IPHDEST%\include" mkdir "%IPHDEST%\include"
    if not exist "%DEST%" (
    @echo cmake -E copy_if_different "%WindowsSdkDir%\lib\%WindowsSdkLibVersion%\um\x86\iphlpapi.lib" "%IPHDEST%"
    cmake -E copy_if_different "%WindowsSdkDir%\lib\%WindowsSdkLibVersion%\um\x86\iphlpapi.lib" "%IPHDEST%"
    @echo cmake -E copy_if_different "%WindowsSdkDir%\include\%WindowsSdkLibVersion%\um\iphlpapi.h" "%IPHDEST%\include"
    cmake -E copy_if_different "%WindowsSdkDir%\include\%WindowsSdkLibVersion%\um\iphlpapi.h" "%IPHDEST%\include"
    )
  ) else (
    @echo [101;93mCould not find local copy of iphlpapi library so will try to download one.[0m
  )
  :: If we could not find local copy of iphlpapi library attempt download
  if not exist "%DEST%" (
    set "opencpn_support_base=https://dl.cloudsmith.io/public/alec-leamas"
    set "URL=%opencpn_support_base%/opencpn-support/raw/files/iphlpapi.lib"
    call :download
  )
  if not exist "%DEST%" (
    echo [101;93mDownload %DEST% failed.[0m
  )
)
if exist "%DEST%" (echo %DEST% found.)
set DEST=
set IPHDEST=

pushd %OCPN_DIR%\build
where /Q xgettext.exe && goto :skipgettext
%CACHE_DIR%\nuget install Gettext.Tools
where /Q /R . xgettext.exe && echo Found Gettext.Tools && goto :skipgettext
echo Error: Could not install GetText tools.
if not [%quiet%]==[Y] pause
popd
goto :usage
:skipgettext
where /Q makensisw.exe && goto :skipnsis
%CACHE_DIR%\nuget install NSIS-Package
where /Q /R . makensisw.exe && echo Found NSIS-Package && goto :skipnsis
@echo Error: Could not install NSIS installer.
popd
goto :usage
:skipnsis
for /D %%D in (Gettext*) do (set "gettextpath=%%~fD")
for /D %%D in (NSIS-Package*) do (set "nsispath=%%~fD")
@echo gettextpath=%gettextpath%
@echo nsispath=%nsispath%
popd
::-------------------------------------------------------------
:: Finalize local environment helper script
::-------------------------------------------------------------
@echo Finishing %OCPN_DIR%\buildwin\configdev.bat
if "[%nsispath%]"==[] (goto :addGettext)
set "_addpath=%nsispath%\NSIS\;%nsispath%\NSIS\bin\"
:addGettext
if "[%gettextpath%]"==[] (goto :addPath)
set "_addpath=%_addpath%;%gettextpath%\tools\bin\"
:addPath
if "[%_addpath%]"=="[]" (goto :skipAddPath)
@echo path^|find /i "%_addpath%"    ^>nul ^|^| set "path=%path%;%_addpath%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo goto :EOF>> "%OCPN_DIR%\buildwin\configdev.bat"
:skipAddPath
endlocal
::-------------------------------------------------------------
:: Setup environment
::-------------------------------------------------------------
if exist "%~dp0..\buildwin\configdev.bat" (call "%~dp0..\buildwin\configdev.bat") else (echo Cannot find configdev.bat&& goto :fail)
::-------------------------------------------------------------
:: Build Release and Debug executables
::-------------------------------------------------------------
if exist "%~dp0..\build\.RelWithDebInfo" (
  @echo Building RelWithDebInfo
  set build_type=RelWithDebInfo
  call :ocpnConfig
  if errorlevel 1 (
    goto :fail
  )
  set buildTarget=Build
  if exist "%~dp0..\build\.RelWithDebInfo\.Pack" (
    rmdir "%~dp0..\build\.RelWithDebInfo\.Pack"
    call :ocpnPack
  ) else (
    call :ocpnBuild
  )
  if errorlevel 1 (
    goto :fail
  )
  call :restore
)

if exist "%~dp0..\build\.Release" (
  @echo Building Release
  set build_type=Release
  call :ocpnConfig
  if errorlevel 1 (
    goto :fail
  )
  set buildTarget=Build
  if exist "%~dp0..\build\.Release\.Pack" (
    rmdir "%~dp0..\build\.Release\.Pack"
    call :ocpnPack
  ) else (
    call :ocpnBuild
  )
  if errorlevel 1 (
    goto :fail
  )
  call :restore
  :: Compress pdb and mark with git hash
  @echo on
  "%bashcmd%" -c ^
    "tar czf opencpn+%GITHUB_SHA:~0,8%.pdb.tar.gz %CONFIGURATION%/opencpn.pdb"
  @echo off

)

if exist "%~dp0..\build\.Debug" (
  @echo Building Debug
  set build_type=Debug
  call :ocpnConfig
  if errorlevel 1 (
    goto :fail
  )
  set buildTarget=Build
  if exist "%~dp0..\build\.Debug\.Pack" (
    call :ocpnPack
    rmdir "%~dp0..\build\.Debug\.Pack"
  ) else (
    call :ocpnBuild
  )
  if errorlevel 1 (
    goto :fail
  )
  call :restore
)

if exist "%~dp0..\build\.MinSizeRel" (
  @echo Building MinSizeRel
  set build_type=MinSizeRel
  call :ocpnConfig
  if errorlevel 1 (
    goto :fail
  )
  set buildTarget=Build
  if exist "%~dp0..\build\.MinSizeRel\.Pack" (
    rmdir "%~dp0..\build\.MinSizeRel\.Pack"
    call :ocpnPack
  ) else (
    call :ocpnBuild
  )
  if errorlevel 1 (
    goto :fail
  )
  call :restore
)
popd
set build_type=
goto :hint
::-------------------------------------------------------------
:: Offer some helpful hints
::-------------------------------------------------------------
:hint
set build_type=
set buildTarget=
set wxVerb=
@echo To build OpenCPN for debugging at command line do this in the folder
@echo OpenCPN
@echo.
@echo  .\buildwin\configdev.bat
::-------------------------------------------------------------
@echo  msbuild /noLogo /m -p:Configuration=Debug;Platform=Win32 .\build\opencpn.sln
@echo  .\build\opencpn.sln
@echo.
@echo Now you are ready to start debugging
@echo.
@echo [101;93mIf you close this CMD prompt and open another be sure to run:[0m
@echo  %CD%\buildwin\configdev.bat
@echo [101;93mfirst, before starting Visual Studio[0m.
goto :success
::-------------------------------------------------------------
:: Local subroutines
::-------------------------------------------------------------
:removeTarget
if exist "%target%" echo Removing "%target%"
if exist "%target%" rmdir /s /q "%target%"
if exist "%target%" (
  @echo Could not remove "%target%" folder
  if [%quiet%]==[Y] (goto :fail)
  @echo Is Visual Studio IDE or OpenCPN running? If so, please close so we can try again.
  if not [%quiet%]==[Y] pause
  @echo Retrying...
  rmdir /s /q "%target%"
  if exist "%target%" (
    @echo Could not remove "%target%". Continuing...
    exit /b 0
  ) else (
    @echo Ok, removed "%target%"
    exit /b 0
  )
)
exit /b 0
::-------------------------------------------------------------
:: Config and build
::-------------------------------------------------------------
:ocpnConfig
cmake -A Win32 -G "%VCstr%" -S "%~dp0.." -B "%~dp0..\build" ^
  -DCMAKE_GENERATOR_PLATFORM=Win32 ^
  -DCMAKE_BUILD_TYPE=%build_type% ^
  -DwxWidgets_LIB_DIR="%wxWidgets_LIB_DIR%" ^
  -DwxWidgets_ROOT_DIR="%wxWidgets_ROOT_DIR%" ^
  -DCMAKE_CXX_FLAGS="/MP /EHsc /DWIN32" ^
  -DCMAKE_C_FLAGS="/MP" ^
  -DOCPN_CI_BUILD:BOOL=OFF ^
  -DOCPN_TARGET_TUPLE=msvc-wx32;10;x86_64 ^
  -DOCPN_BUNDLE_WXDLLS:BOOL=ON ^
  -DOCPN_BUILD_TEST:BOOL=OFF ^
  -DOCPN_BUNDLE_GSHHS:BOOL=ON ^
  -DOCPN_BUNDLE_TCDATA:BOOL=ON ^
  -DOCPN_BUNDLE_DOCS:BOOL=ON ^
  -DOCPN_ENABLE_SYSTEM_CMD_SOUND:BOOL=OFF ^
  -DOCPN_ENABLE_PORTAUDIO:BOOL=OFF ^
  -DOCPN_BUILD_TEST:BOOL=OFF ^
  -DCMAKE_INSTALL_PREFIX="%~dp0..\build\%build_type%"
if errorlevel 1 (
  cmake -A Win32 -G "%VCstr%" -S "%~dp0.." -B "%~dp0..\build" --debug-find ^
    -DCMAKE_GENERATOR_PLATFORM=Win32 ^
    -DCMAKE_BUILD_TYPE=%build_type% ^
    -DwxWidgets_LIB_DIR="%wxWidgets_LIB_DIR%" ^
    -DwxWidgets_ROOT_DIR="%wxWidgets_ROOT_DIR%" ^
    -DCMAKE_CXX_FLAGS="/MP /EHsc /DWIN32" ^
    -DCMAKE_C_FLAGS="/MP" ^
    -DOCPN_CI_BUILD:BOOL=OFF ^
    -DOCPN_TARGET_TUPLE=msvc-wx32;10;x86_64 ^
    -DOCPN_BUNDLE_WXDLLS:BOOL=ON ^
    -DOCPN_BUILD_TEST:BOOL=OFF ^
    -DOCPN_BUNDLE_GSHHS:BOOL=ON ^
    -DOCPN_BUNDLE_TCDATA:BOOL=ON ^
    -DOCPN_BUNDLE_DOCS:BOOL=ON ^
    -DOCPN_ENABLE_SYSTEM_CMD_SOUND:BOOL=OFF ^
    -DOCPN_ENABLE_PORTAUDIO:BOOL=OFF ^
    -DOCPN_BUILD_TEST:BOOL=OFF ^
    -DCMAKE_INSTALL_PREFIX="%~dp0..\build\%build_type%"
  if errorlevel 1 goto :cmakeErr
)
exit /b 0
:ocpnBuild
@echo build_type=%build_type%
msbuild ^
  -property:Configuration=%build_type%;Platform=Win32 ^
  -target:%buildTarget% ^
  -noLogo ^
  -verbosity:minimal ^
  -maxCpuCount ^
  -property:UseMultiToolTask=true ^
  -property:EnableClServerMode=true ^
  -property:BuildPassReferences=true ^
  -property:CL="/arch:SSE" ^
  /l:FileLogger,Microsoft.Build.Engine;logfile=%CD%\MSBuild_%build_type%_WIN32_Debug.log ^
  "%~dp0..\build\INSTALL.vcxproj"
if errorlevel 1 goto :buildErr
::
:: Delete the .mo files because they are only valid for this build Configuration.
:: They will not be rebuilt for a different configuration unless we delete them.
del /F "%~dp0..\build\*.mo"
set buildTarget=
@echo OpenCPN %build_type% build successful!
@echo.
exit /b 0

:ocpnPack
@echo build_type=%build_type%
msbuild ^
  -property:Configuration=%build_type%;Platform=Win32 ^
  -target:%buildTarget% ^
  -noLogo ^
  -verbosity:minimal ^
  -maxCpuCount ^
  -property:UseMultiToolTask=true ^
  -property:EnableClServerMode=true ^
  -property:BuildPassReferences=true ^
  -property:CL="/arch:SSE" ^
  /l:FileLogger,Microsoft.Build.Engine;logfile=%CD%\MSBuild_%build_type%_WIN32_Debug.log ^
  "%~dp0..\build\PACKAGE.vcxproj"
if errorlevel 1 goto :buildErr
:: Move the build package file to config build folder
move "%~dp0..\build\*.exe" "%~dp0..\build\%build_type%"
::
:: Delete the .mo files because they are only valid for this build Configuration.
:: They will not be rebuilt for a different configuration unless we delete them.
del /F "%~dp0..\build\*.mo"
set buildTarget=
@echo OpenCPN %build_type% build successful!
@echo.
exit /b 0

::-------------------------------------------------------------
:: CMake failed
::-------------------------------------------------------------
:cmakeErr
set build_type=
set buildTarget=
set wxVerb=
@echo CMake failed to configure OpenCPN build folder.
@echo Review the error messages and read the OpenCPN
@echo Developer Manual for help.
if not [%quiet%]==[Y] pause
exit /b 1
::-------------------------------------------------------------
:: Build failed
::-------------------------------------------------------------
:buildErr
@echo Build type '%build_type%' using msbuild failed.
@echo Review the error messages and read the OpenCPN
@echo Developer Manual for help.
set build_type=
set buildTarget=
set wxVerb=
exit /b 1
::-------------------------------------------------------------
:: Backup user configuration
::-------------------------------------------------------------
:backup
if not exist "%~dp0..\build\%folder%" goto :bexit
@echo Backing up %~dp0..\build\%folder%
if not exist "%~dp0..\tmp" (mkdir "%~dp0..\tmp")
if not exist "%~dp0..\tmp\%folder%" (mkdir "%~dp0..\tmp\%folder%")
@echo backing up %folder%
if exist "%~dp0..\build\%folder%\opencpn.ini" (xcopy /Q /Y "%~dp0..\build\%folder%\opencpn.ini" "%~dp0..\tmp\%folder%")
if errorlevel 1 (
  @echo xcopy /Q /Y "%~dp0..\build\%folder%\opencpn.ini" "%~dp0..\tmp\%folder%" [101;93mNOT OK[0m
  if not [%quiet%]==[Y] pause
) else (
  echo xcopy /Q /Y "%~dp0..\build\%folder%\opencpn.ini" "%~dp0..\tmp\%folder%" OK
)
xcopy /Q /Y "%~dp0..\build\%folder%\*.log.log" "%~dp0..\tmp\%folder%"
if errorlevel 1 (
  @echo xcopy /Q /Y "%~dp0..\build\%folder%\*.log.log" [101;93mNOT OK[0m
  if not [%quiet%]==[Y] pause
) else (
  echo xcopy /Q /Y "%~dp0..\build\%folder%\*.log.log" OK
)
xcopy /Q /Y "%~dp0..\build\%folder%\*.log" "%~dp0..\tmp\%folder%"
if errorlevel 1 (
  @echo xcopy /Q /Y "%~dp0..\build\%folder%\*.log" "%~dp0..\tmp\%folder%" [101;93mNOT OK[0m
  if not [%quiet%]==[Y] pause
) else (
  echo xcopy /Q /Y "%~dp0..\build\%folder%\*.log" "%~dp0..\tmp\%folder%" OK
)
xcopy /Q /Y "%~dp0..\build\%folder%\*.dat" "%~dp0..\tmp\%folder%"
if errorlevel 1 (
  @echo xcopy /Q /Y "%~dp0..\build\%folder%\*.dat" "%~dp0..\tmp\%folder%" [101;93mNOT OK[0m
  if not [%quiet%]==[Y] pause
) else (
  echo xcopy /Q /Y "%~dp0..\build\%folder%\*.dat" "%~dp0..\tmp\%folder%" OK
)
xcopy /Q /Y "%~dp0..\build\%folder%\*.csv" "%~dp0..\tmp\%folder%"
if errorlevel 1 (
  @echo xcopy /Q /Y "%~dp0..\build\%folder%\*.csv" "%~dp0..\tmp\%folder%" [101;93mNOT OK[0m
  if not [%quiet%]==[Y] pause
) else (
  echo xcopy /Q /Y "%~dp0..\build\%folder%\*.csv" "%~dp0..\tmp\%folder%" OK
)
xcopy /Q /Y "%~dp0..\build\%folder%\navobj.*" "%~dp0..\tmp\%folder%"
if errorlevel 1 (
  @echo xcopy /Q /Y "%~dp0..\build\%folder%\navobj.*" "%~dp0..\tmp\%folder%" [101;93mNOT OK[0m
  if not [%quiet%]==[Y] pause
) else (
  echo xcopy /Q /Y "%~dp0..\build\%folder%\navobj.*" "%~dp0..\tmp\%folder%" OK
)
xcopy /Q /Y "%~dp0..\build\%folder%\navobj.xml.?" "%~dp0..\tmp\%folder%"
if errorlevel 1 (
  @echo xcopy /Q /Y "%~dp0..\build\%folder%\navobj.xml.?" "%~dp0..\tmp\%folder%" [101;93mNOT OK[0m
  if not [%quiet%]==[Y] pause
) else (
  echo xcopy /Q /Y "%~dp0..\build\%folder%\navobj.xml.?" "%~dp0..\tmp\%folder%" OK
)
if exist "%~dp0..\build\%folder%\plugins" (
  :: Convert old winConfig build structure to latest version
  if not exist "%~dp0..\build\.%folder%" (
    mkdir "%~dp0..\build\.%folder%"
  )
  @echo cmake -E copy_directory "%~dp0..\build\%folder%\plugins" "%~dp0..\tmp\%folder%"
  cmake -E copy_directory "%~dp0..\build\%folder%\plugins" "%~dp0..\tmp\%folder%"
  if errorlevel 1 (
    @echo cmake -E copy_directory "%~dp0..\build\%folder%\plugins" "%~dp0..\tmp\%folder%" [101;93mNOT OK[0m
    if not [%quiet%]==[Y] pause
  ) else (
    echo cmake -E copy_directory "%~dp0..\build\%folder%\plugins" "%~dp0..\tmp\%folder%" OK
  )
)
if not exist "%~dp0..\build\%folder%\Charts" goto :breturn
@echo cmake -E copy_directory "%~dp0..\build\%folder%\Charts" "%~dp0..\tmp\%folder%"
cmake -E copy_directory "%~dp0..\build\%folder%\Charts" "%~dp0..\tmp\%folder%"
if errorlevel 1 (
  @echo cmake -E copy_directory "%~dp0..\build\%folder%\Charts" "%~dp0..\tmp\%folder%" [101;93mNOT OK[0m
  if not [%quiet%]==[Y] pause
) else (
  echo cmake -E copy_directory "%~dp0..\build\%folder%\Charts" "%~dp0..\tmp\%folder%" OK
)
:breturn
@echo Backup returning
:bexit
exit /b 0
::-------------------------------------------------------------
:: Restore user configuration to build folder
::-------------------------------------------------------------
:restore
:: Called from withing build folder
if not exist "%~dp0..\tmp\%build_type%" (
  @echo INFO: Did not find "%~dp0..\tmp\%build_type%"
  exit /b 0
)
@echo Restoring %build_type% settings from "%~dp0..\tmp\%build_type%"
cmake -E copy_directory "%~dp0..\tmp\%build_type%" "%~dp0..\build\%build_type%"
if errorlevel 1 (
  @echo Restore %build_type% failed
  if not [%quiet%]==[Y] pause
  exit /b 0
) else (
  @echo Restore successful
  rmdir /s /q "%~dp0..\tmp\%build_type%"
)
:rreturn
@echo restore returning
exit /b 0
::-------------------------------------------------------------
:: Download URL to a DEST folder
::-------------------------------------------------------------
:download
@echo URL=%URL%
@echo DEST=%DEST%
if exist %DEST% (
  echo Download %DEST% already exists.
  exit /b 0
)
%PSH% -Command [System.Net.ServicePointManager]::MaxServicePointIdleTime = 5000000; ^
  if ($PSVersionTable.PSVersion.Major -lt 6) { $ProgressPreference = 'SilentlyContinue' }; ^
  Invoke-WebRequest '%URL%' -OutFile '%DEST%'; ^
  exit $LASTEXITCODE
if errorlevel 1 (echo Download failed && exit /b 1) else (echo Download OK)
exit /b 0
::-------------------------------------------------------------
:: Explode SOURCE zip file to DEST folder
::-------------------------------------------------------------
:explode
@echo SOURCE=%SOURCE%
@echo DEST=%DEST%
:: @echo "%PSH% -Command if ($PSVersionTable.PSVersion.Major -lt 6) { $ProgressPreference = 'SilentlyContinue' }; Expand-Archive -Force -Path '%SOURCE%' -DestinationPath '%DEST%';


%PSH% -Command if ($PSVersionTable.PSVersion.Major -lt 6) { $ProgressPreference = 'SilentlyContinue' }; Expand-Archive -Force -Path '%SOURCE%' -DestinationPath '%DEST%'; exit $LASTEXITCODE
if errorlevel 1 (echo Explode failed && exit /b 1) else (echo Unzip OK)
exit /b 0
::-------------------------------------------------------------
:: Execution time measuring functions
::-------------------------------------------------------------
:StartTimer
:: Store start time
set winConfigStartTIME=%TIME%
for /f "usebackq tokens=1-4 delims=:., " %%f in (`echo %winConfigStartTIME: =0%`) do set /a Start100S=1%%f*360000+1%%g*6000+1%%h*100+1%%i-36610100
goto :EOF

:StopTimer
:: Get the end time
set winConfigStopTIME=%TIME%
for /f "usebackq tokens=1-4 delims=:., " %%f in (`echo %winConfigStopTIME: =0%`) do set /a Stop100S=1%%f*360000+1%%g*6000+1%%h*100+1%%i-36610100
:: Test midnight rollover. If so, add 1 day=8640000 1/100ths secs
if %Stop100S% LSS %Start100S% set /a Stop100S+=8640000
set /a winConfigTookTime=%Stop100S%-%Start100S%
set winConfigTookTimePadded=0%winConfigTookTime%
exit /b 0

:DisplayTimerResult
:: Show timer start/stop/delta
if not "[%winConfigStartTime%]"=="[]" echo Started: %winConfigStartTime%
if not "[%winConfigStopTime%]"=="[]" echo Stopped: %winConfigStopTime%
if not "[%winConfigTookTime%]"=="[]" echo Elapsed: %winConfigTookTime:~0,-2%.%winConfigTookTimePadded:~-2% seconds
set winConfigStartTime=
set winConfigStopTime=
set winConfigTookTime=
set winConfigTookTimePadded=
exit /b 0

:: Failure exit
:fail
endlocal
call :StopTimer
call :DisplayTimerResult
exit /b 1
:: Succeed exit
:success
call :StopTimer
call :DisplayTimerResult
exit /b 0
:: ***********
:: * THE END *
:: ***********
