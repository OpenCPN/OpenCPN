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
if CMDEXTVERSION 1 goto :start
@echo Error: Command extensions must be enabled.
exit /b 1

:start
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
call "%vsenv%"
set vsenv=
:go
setlocal enabledelayedexpansion
if [%VisualStudioVersion%]==[] (
  @echo Please install Visual Studio 2022 or 2026
  exit /b 1
)
goto :main
:usage
@echo ****************************************************************************
@echo *  This script can be used to create a local win32 build environment.      *
@echo *                                                                          *
@echo *  There are some prequisites before you can effectively use this script.  *
@echo *                                                                          *
@echo *  1. Install Visual Studio 202x Community Edition.                        *
@echo *              https://visualstudio.microsoft.com/downloads/               *
@echo *     Be sure to select the latest dev tools and                           *
@echo *     cmake tools check boxes.                                             *
@echo *                                                                          *
@echo *  2. Install git for Windows (or select it from the VS 202x installer)    *
@echo *              https://git-scm.com/download/win                            *
@echo *                                                                          *
@echo *  3. Open 'x86 Native Tools Command Prompt for Visual Studio 202x'        *
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
@echo Entering :main
::-------------------------------------------------------------
:: Initialize local environment
::-------------------------------------------------------------
pushd %~dp0..
set "OCPN_DIR=%CD%"
popd
SET "CACHE_DIR=%OCPN_DIR%\cache"
SET "DATA_DIR=%OCPN_DIR%\data"
set "wxWidgetsURL=https://github.com/wxWidgets/wxWidgets"
set "wxMajor=v3.2"
set "wxMinor=8"
set "wxVER=%wxMajor%.%wxMinor%"
if [%VisualStudioVersion%]==[16.0] (
  set VCver=16
  set "VCstr=Visual Studio 16 2019"
  set VCtool=141
)
if [%VisualStudioVersion%]==[17.0] (
  set VCver=17
  set "VCstr=Visual Studio 17 2022"
  set VCtool=143
)
if [%VisualStudioVersion%]==[18.0] (
  set VCver=18
  set "VCstr=Visual Studio 18 2026"
  set VCtool=145
)
::-------------------------------------------------------------
:: Initialize local variables
::-------------------------------------------------------------
@echo Checking if tools are available
SET "buildWINtmp=%CACHE_DIR%\buildwintemp"
set PSH=powershell
where pwsh > NUL 2> NUL && set PSH=pwsh
where %PSH% > NUL 2> NUL || echo PowerShell is not installed && goto :fail
where msbuild.exe > NUL 2> NUL && goto :vsok
@echo Please run this from Visual Studio "x86 Native Tools Command Prompt ..."
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
:: By default build all 4 possible configurations the first time
:: Edit and set to 1 at least one configuration
set ocpn_all=1
set ocpn_minsizerel=0
set ocpn_release=0
set ocpn_relwithdebinfo=0
set ocpn_debug=0
set quiet=N
set ocpn_package=0
set ocpn_clean=0
set ocpn_rebuild=0

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
if /I [%1]==[--all] (shift /1 && set ocpn_all=1&& goto :parse)
if /I [%1]==[--clean] (shift /1 && set ocpn_clean=1&& set ocpn_rebuild=0&& goto :parse)
:: Experimental - use cmake to build wxWidgets so we can have RelWithDebInfo
if /I [%1]==[--cmake] (shift /1 && set ocpn_cmake=1&& goto :parse)
if /I [%1]==[--debug] (shift /1 && set ocpn_all=0&& set ocpn_debug=1&& goto :parse)
if /I [%1]==[--help] (shift /1 && goto :usage)
if /I [%1]==[--minsizerel] (shift /1 && set ocpn_all=0&& set ocpn_minsizerel=1&& goto :parse)
if /I [%1]==[--package] (shift /1 && set ocpn_package=1&& goto :parse)
if /I [%1]==[--rebuild] (shift /1 && set ocpn_rebuild=1&& set ocpn_clean=0&& goto :parse)
if /I [%1]==[--release] (shift /1 && set ocpn_all=0&& set ocpn_release=1&& goto :parse)
if /I [%1]==[--relwithdebinfo] (shift /1 && set ocpn_all=0&& set ocpn_relwithdebinfo=1&& goto :parse)
if /I [%1]==[--wxver] (shift /1 && set wxVER=%2&& shift /1 && goto :parse)
if /I [%1]==[--Y] (shift /1 && set "quiet=Y" && goto :parse)
if [%1]==[] (goto :begin) else (
  echo Unknown option: %1
  shift /1
  goto :usage
)
:begin
::-------------------------------------------------------------
:: Check if network is available
::-------------------------------------------------------------
set netok=0
if not ["%gitcmd%"]==[] (
  @echo Checking network connection...
  @echo %cd%
  "%gitcmd%" fetch --dry-run >nul 2>&1 && set netok=1
)
@echo netok=%netok%
if [%ocpn_all%]==[1] (
  set ocpn_minsizerel=1
  set ocpn_release=1
  set ocpn_relwithdebinfo=1
  set ocpn_debug=1
)
::-------------------------------------------------------------
:: Initialize local helper script that can reinitialize environment
::-------------------------------------------------------------
set "wxDIR=%OCPN_DIR%\..\ocpn_wxWidgets"
set "wxSourceDir=%OCPN_DIR%\..\ocpn_wxWidgets"
set "wxWidgets_ROOT_DIR=%wxDIR%"
if "[%ocpn_cmake%]"=="[1]" (
  echo set "wxBuildDir=%wxDIR%\build_cmake"
  set "wxBuildDir=%wxDIR%\build_cmake"
) else (
  echo set "wxBuildDir=%wxDIR%"
  set "wxBuildDir=%wxDIR%"
)
set "wxWidgets_LIB_DIR=%wxBuildDir%\lib\vc_dll"

@echo set "wxDIR=%wxDIR%" > "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "wxWIN=%wxDIR%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "wxSourceDir=%wxSourceDir%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "wxBuildDir=%wxBuildDir%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "wxWidgets_ROOT_DIR=%wxWidgets_ROOT_DIR%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "wxWidgets_LIB_DIR=%wxWidgets_LIB_DIR%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "VCver=%VCver%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "VCstr=%VCstr%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "gitcmd=%gitcmd%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "bashcmd=%bashcmd%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "patchcmd=%patchcmd%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "VCtool=%VCtool%" >> "%OCPN_DIR%\buildwin\configdev.bat"
::-------------------------------------------------------------
:: If this is the first build then initialize all build types
::-------------------------------------------------------------
if not exist "%OCPN_DIR%\build" (
  set ocpn_clean=1
)
@echo ocpn_clean=%ocpn_clean%
@echo ocpn_relwithdebinfo=%ocpn_relwithdebinfo%
@echo ocpn_release=%ocpn_release%
@echo ocpn_debug=%ocpn_debug%
@echo ocpn_minsizerel=%ocpn_minsizerel%
@echo ocpn_all=%ocpn_all%
::-------------------------------------------------------------
:: Save user configuration data and clean build folders but
:: do not delete downloaded tools.  Use this option if no internet
:: connectivity available.
::-------------------------------------------------------------
if [%ocpn_rebuild%]==[1] (
  echo Beginning rebuild cleanout
  if not [%quiet%]==[Y] (timeout /T 15)
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
  set "target=%OCPN_DIR%\build\CMakeCache.txt" & call :removeTarget
  if [%netok%]==[1] (set "target=%OCPN_DIR%\.git\hooks\pre-commit" & call :removeTarget)
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
  set "target=%WXDIR%\build\msw\*.obj" & call :removeTarget
  where /Q /R "%OCPN_DIR%\build" *.cmake && del /Q /S "%OCPN_DIR%\build\*.cmake"
  where /Q /R "%OCPN_DIR%\build" *.txt && del /Q /S "%OCPN_DIR%\build\*.txt"
  where /Q /R "%OCPN_DIR%\build" *.in && del /Q /S "%OCPN_DIR%\build\*.in"
  where /Q /R "%OCPN_DIR%\build" *.xml && del /Q /S "%OCPN_DIR%\build\*.xml"
  where /Q /R "%OCPN_DIR%\build" *.rc && del /Q /S "%OCPN_DIR%\build\*.rc"
  where /Q /R "%OCPN_DIR%\build" *.user && del /Q /S "%OCPN_DIR%\build\*.user"
  where /Q /R "%OCPN_DIR%\build" *.sln && del /Q /S "%OCPN_DIR%\build\*.sln"
  where /Q /R "%OCPN_DIR%\build" *.slnx && del /Q /S "%OCPN_DIR%\build\*.slnx"
  where /Q /R "%OCPN_DIR%\build" *.mo && del /Q /S "%OCPN_DIR%\build\*.mo"
  where /Q /R "%OCPN_DIR%\build" *.vcxproj && del /Q /S "%OCPN_DIR%\build\*.vcxproj"
  where /Q /R "%OCPN_DIR%\build" *.filters && del /Q /S "%OCPN_DIR%\build\*.filters"
  where /Q /R "%OCPN_DIR%\build" *.log && del /Q /S "%OCPN_DIR%\build\*.log"
  @echo Finished rebuild cleanout
)
::-------------------------------------------------------------
:: Save user configuration data and wipe the build folder
::-------------------------------------------------------------
if [%ocpn_clean%]==[1] (
  if [%netok%]==[1] echo Network ok
  if [%netok%]==[0] echo Network not working

  set ocpn_clean=0
  set ocpn_rebuild=0
  if [%netok%]==[0] (
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

  set "target=%CACHE_DIR%" & call :removeTarget
  set "target=%wxDIR%\build\msw\*.obj" & call :removeTarget
  set "target=%buildWINtmp%" & call :removeTarget
  set "target=%OCPN_DIR%\.git\hooks\pre-commit" & call :removeTarget
  if not [%quiet%]==[Y] (timeout /T 5)
)
::-------------------------------------------------------------
:: Create needed folders
::-------------------------------------------------------------
::-------------------------------------------------------------
:: Initialize folders needed to run OpenCPN
::-------------------------------------------------------------
@echo ocpn_relwithdebinfo=%ocpn_relwithdebinfo%
@echo ocpn_release=%ocpn_release%
@echo ocpn_debug=%ocpn_debug%
@echo ocpn_minsizerel=%ocpn_minsizerel%
@echo ocpn_all=%ocpn_all%
@echo ocpn_clean=%ocpn_clean%
@echo setting up folders
if [%ocpn_debug%]==[1] (
  if not exist "%OCPN_DIR%\build\.Debug" (
    mkdir "%OCPN_DIR%\build\.Debug"
    if [%ocpn_package%]==[1] (mkdir "%OCPN_DIR%\build\.Debug\.Pack")
    if [%ocpn_cmake%]==[1] (mkdir "%OCPN_DIR%\build\.Debug\.CMake")
  )
)
if [%ocpn_release%]==[1] (
  if not exist "%OCPN_DIR%\build\.Release" (
    mkdir "%OCPN_DIR%\build\.Release"
  )
  if exist "%OCPN_DIR%\build\.Release" (
    if [%ocpn_package%]==[1] (mkdir "%OCPN_DIR%\build\.Release\.Pack")
    if [%ocpn_cmake%]==[1] (mkdir "%OCPN_DIR%\build\.Release\.CMake")
  )
)
if [%ocpn_relwithdebinfo%]==[1] (
  if not exist "%OCPN_DIR%\build\.RelWithDebInfo" (
    mkdir "%OCPN_DIR%\build\.RelWithDebInfo"
  )
  if exist "%OCPN_DIR%\build\.RelWithDebInfo" (
    if [%ocpn_package%]==[1] (mkdir "%OCPN_DIR%\build\.RelWithDebInfo\.Pack")
    if [%ocpn_cmake%]==[1] (mkdir "%OCPN_DIR%\build\.RelWithDebInfo\.CMake")
  )
)
if [%ocpn_minsizerel%]==[1] (
  if not exist "%OCPN_DIR%\build\.MinSizeRel" (
    mkdir "%OCPN_DIR%\build\.MinSizeRel"
  )
  if exist "%OCPN_DIR%\build\.MinSizeRel" (
    if [%ocpn_package%]==[1] (mkdir "%OCPN_DIR%\build\.MinSizeRel\.Pack")
    if [%ocpn_cmake%]==[1] (mkdir "%OCPN_DIR%\build\.MinSizeRel\.CMake")
  )
)
if exist "%OCPN_DIR%\build\.Release\.CMake" (set ocpn_cmake=1)
if exist "%OCPN_DIR%\build\.RelWithDebInfo\.CMake" (set ocpn_cmake=1)
if exist "%OCPN_DIR%\build\.MinSizeRel\.CMake" (set ocpn_cmake=1)
if exist "%OCPN_DIR%\build\.Debug\.CMake" (set ocpn_cmake=1)
@echo Finished setting up folders
if not exist "%CACHE_DIR%\" (mkdir "%CACHE_DIR%")
if not exist "%CACHE_DIR%\buildwin\" (mkdir "%CACHE_DIR%\buildwin")
if not exist "%buildWINtmp%\" (mkdir "%buildWINtmp%")
::-------------------------------------------------------------
:: Install nuget
::-------------------------------------------------------------
@echo Looking for nuget
where /Q /R %CACHE_DIR% nuget.exe >nul && "%CACHE_DIR%\nuget.exe" >nul && goto :skipnuget
@echo Downloading nuget
set "URL=https://dist.nuget.org/win-x86-commandline/latest/nuget.exe"
set "DEST=%CACHE_DIR%\nuget.exe"
call :download
if errorlevel 1 (if not [%quiet%]==[Y] pause)
where /Q /R %CACHE_DIR% nuget.exe && goto :skipnuget
@echo Error: Could not download nuget.exe
goto :usage
:skipnuget
@echo nuget is present

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
set PYAPP=
if not exist ".\.git\hooks\pre-commit" (
  @echo Installing python into %CACHE_DIR%
  pushd %CACHE_DIR%
  .\nuget install python
  for /f "delims=" %%G in ('where /r . /f python.exe') do (
    set PYAPP=%%G
    set PYFLDR=%%~pG
  )
  popd
)
if not exist ".\.git\hooks\pre-commit" (
  if not [%PYAPP%]==[] (
    pushd %CACHE_DIR%
    @echo Installing pre-commit hooks using "%PYAPP%"
    %PYAPP% -m pip install --no-warn-script-location --upgrade pip
    %PYAPP% -m pip install --no-warn-script-location -q pre-commit
    for /f "delims=" %%P in ('where /R . /F pre-commit.exe') do (%%P install -f)
  ) else (
    @echo Warning: Could not find Python application
    if not [%quiet%]==[Y] pause
  )
  popd
)
if not exist ".\.git\hooks\pre-commit" (
  @echo Error: Could not find pre-commit tool.
  if not [%quiet%]==[Y] pause
)

::-------------------------------------------------------------
:: Download OpenCPN Core dependencies
::-------------------------------------------------------------
if exist "%buildWINtmp%\OCPNWindowsCoreBuildSupport.zip" (goto :skipbuildwin)
@echo Downloading Windows dependencies from OpenCPN repository
set "URL=https://github.com/OpenCPN/OCPNWindowsCoreBuildSupport/archive/refs/tags/v0.5.zip"
set "DEST=%buildWINtmp%\OCPNWindowsCoreBuildSupport.zip"
call :download
if errorlevel 1 (if not [%quiet%]==[Y] pause)

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
set "URL=https://dl.cloudsmith.io/public/david-register/opencpn-docs/raw/files/QuickStartGuide-v0.4.zip"
set "DEST=%CACHE_DIR%\QuickStartManual.zip"
if not exist "%DEST%" (
  @echo Downloading quickstart manual
  call :download
  if errorlevel 1 (if not [%quiet%]==[Y] pause)
  if exist "%CACHE_DIR%\..\data\doc\local" rmdir /s /q "%CACHE_DIR%\..\data\doc\local"
  mkdir "%CACHE_DIR%\..\data\doc\local"
  set "SOURCE=%DEST%"
  set "DEST=%CACHE_DIR%\..\data\doc\local"
  @echo Exploding quickstart manual
  call :explode
)
if exist "%VCToolsRedistDir%\x86\Microsoft.VC%VCtool%.CRT\msvcp140.dll" (
  @echo Updating VC runtime ...
  if not exist "%CACHE_DIR%\buildwin" mkdir "%CACHE_DIR%\buildwin"
  if not exist "%CACHE_DIR%\buildwin\vc" mkdir "%CACHE_DIR%\buildwin\vc"
  cmake -E copy_directory_if_different "%VCToolsRedistDir%\x86\Microsoft.VC%VCtool%.CRT" "%CACHE_DIR%\buildwin\vc"
  if errorlevel 1 (
    @echo [101;93mNOT OK[0m
    if not [%quiet%]==[Y] pause
  ) else (
    echo OK
  )
)
::-------------------------------------------------------------
:: Download wxWidgets sources
::-------------------------------------------------------------
if exist "%wxSourceDir%\.git" goto :skipwxDL
@echo Downloading wxWidgets sources
if "[%gitcmd%]"=="[]" (
  if not exist "%wxSourceDir%" (mkdir "%wxSourceDir%")
  set "URL=https://github.com/wxWidgets/wxWidgets/releases/download/%wxVER%/wxWidgets-%wxVER%.zip"
  set "DEST=%wxSourceDir%\wxWidgets-%wxVER%.zip"
  call :download
  if errorlevel 1 (echo Download %DEST& [101;93mNOT OK[0m ) else (echo Download %DEST% OK )

  @echo exploding wxWidgets
  set "SOURCE=%wxSourceDir%\wxWidgets-%wxVER%.zip"
  set "DEST=%wxSourceDir%"
  call :explode
  if errorlevel 1 (echo [101;93mNOT OK[0m ) else (echo Explode wxWidgets OK )
) else (
  @echo %gitcmd% clone --jobs 2 --depth 1 --recurse-submodules --shallow-submodules ^
         --branch %wxVER% "%wxWidgetsURL%" "%wxSourceDir%"
  "%gitcmd%" clone --jobs 2 --depth 1 --recurse-submodules --shallow-submodules ^
   --branch %wxVER% "%wxWidgetsURL%" "%wxSourceDir%"
  if errorlevel 1 (echo Git clone [101;93mNOT OK[0m&&goto :fail )
)
:skipwxDL
if exist "%wxSourceDir%\build\3rdparty\webview2" goto :wxBuild
@echo Downloading Windows WebView2 kit
set "URL=https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2"
set "DEST=%wxSourceDir%\webview2.zip"
call :download
if errorlevel 1 (echo [101;93mNOT OK[0m ) else (echo Download %DEST% OK )

@echo Exploding WebView2
set "SOURCE=%wxSourceDir%\webview2.zip"
set "DEST=%wxSourceDir%\build\3rdparty\webview2"
call :explode
if errorlevel 1 (echo [101;93mNOT OK[0m ) else (echo Explode WebView2 OK )
:wxBuild
::-------------------------------------------------------------
:: Download and initialize build dependencies
::-------------------------------------------------------------
set "IPHDEST=%CACHE_DIR%\buildwin"
set "DEST=%CACHE_DIR%\buildwin\iphlpapi.lib"
if not exist "%DEST%" (
  if exist "%WindowsSdkDir%\lib\%WindowsSdkVersion%\um\x86\iphlpapi.lib" (
    if not exist "%IPHDEST%\include" mkdir "%IPHDEST%\include"
    if not exist "%DEST%" (
    @echo cmake -E copy_if_different "%WindowsSdkDir%\lib\%WindowsSdkVersion%\um\x86\iphlpapi.lib" "%IPHDEST%"
    cmake -E copy_if_different "%WindowsSdkDir%\lib\%WindowsSdkVersion%\um\x86\iphlpapi.lib" "%IPHDEST%"
    @echo cmake -E copy_if_different "%WindowsSdkDir%\include\%WindowsSdkVersion%\um\iphlpapi.h" "%IPHDEST%\include"
    cmake -E copy_if_different "%WindowsSdkDir%\include\%WindowsSdkVersion%\um\iphlpapi.h" "%IPHDEST%\include"
    )
  ) else (
    @echo [101;93mCould not find local copy of iphlpapi library so will try to download one.[0m
  )
  :: If we could not find local copy of iphlpapi library attempt download
  if not exist "%DEST%" (
    set "opencpn_support_base=https://dl.cloudsmith.io/public/alec-leamas"
    set "URL=%opencpn_support_base%/opencpn-support/raw/files/iphlpapi.lib"
    call :download
    if errorlevel 1 (if not [%quiet%]==[Y] pause)
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
:: Restore build foldder configuration
::-------------------------------------------------------------

::-------------------------------------------------------------
:: Build wxWidgets from sources
::-------------------------------------------------------------
if exist "%wxSourceDir%\.git" (
  if not "[%gitcmd%]"=="[]" (
    pushd "%wxSourceDir%"
    "%gitcmd%" submodule update
    "%gitcmd%" fetch --recurse-submodules
    "%gitcmd%" checkout "%wxVER%" --recurse-submodules
    popd
  )
)
if not exist "%wxDir%" (
  @echo Error: %wxDir% not found...
  goto :buildErr
)
@echo if not exist "%wxBuildDir%"
if not exist "%wxBuildDir%" (
  @echo robocopy /E %wxSourceDir%\build %wxBuildDir%
  robocopy /E %wxSourceDir%\build %wxBuildDir%
  @echo Robocopy finished
)
@echo Checking wxWidgets libraries...this may take a few minutes...
set wxVerb=minimal
@echo ocpn_cmake=%ocpn_cmake%
if "[%ocpn_cmake%]"=="[1]" (
  @echo cmake -S "%wxSourceDir%" -B "%wxBuildDir%" -G "%VCstr%" -A Win32 -DwxBUILD_SHARED=ON -DwxBUILD_SAMPLES=OFF -DwxBUILD_TESTS=OFF -DwxBUILD_VENDOR=14x -T v%VCtool% -DCMAKE_CONFIGURATION_TYPES="Debug;RelWithDebInfo" -DCMAKE_CXX_FLAGS=/EHa
  cmake -S "%wxSourceDir%" -B "%wxBuildDir%" -G "%VCstr%" -A Win32 -DwxBUILD_SHARED=ON -DwxBUILD_SAMPLES=OFF -DwxBUILD_TESTS=OFF -DwxBUILD_VENDOR=14x -T v%VCtool% -DCMAKE_CONFIGURATION_TYPES="Debug;RelWithDebInfo" -DCMAKE_CXX_FLAGS=/EHa
  if ERRORLEVEL 1 goto :buildErr
  rem If debug is required build it now
  if exist "%OCPN_DIR%\build\.Debug" (
    cmake --build %wxBuildDir% --config=Debug -- /v:%wxVerb% /m:%NUMBER_OF_PROCESSORS% /p:UseMultiToolTask=true;BuildPassReferences=true;EnableClServerMode=true;wxVendor=14x;wxVersionString=32;wxToolKitDllNameSuffix=_vc_14x ^
        /l:FileLogger,Microsoft.Build.Engine;logfile="%wxSourceDir%\MSBuild_Debug_WIN32.log"
    if ERRORLEVEL 1 (
      if not [%quiet%]==[Y] pause
      exit /b 1
    )
  )
  rem Always build config RelWithDebInfo
  cmake --build %wxBuildDir% --config=RelWithDebInfo -- /v:%wxVerb% /m:%NUMBER_OF_PROCESSORS% /p:UseMultiToolTask=true;BuildPassReferences=true;EnableClServerMode=true;wxVendor=14x;wxVersionString=32;wxToolKitDllNameSuffix=_vc_14x ^
      /l:FileLogger,Microsoft.Build.Engine;logfile="%wxSourceDir%\MSBuild_RelWithDebInfo_WIN32.log"
  if ERRORLEVEL 1 (
    if not [%quiet%]==[Y] pause
    exit /b 1
  )
) else (
  if exist "%wxSourceDir%\build\msw\wx_vc%VCver%.sln" (set "wxSLN=%wxSourceDir%\build\msw\wx_vc%VCver%.sln" && goto :msbuildWX)
  if exist "%wxSourceDir%\build\msw\wx_vc%VCver%.slnx" (set "wxSLN=%wxSourceDir%\build\msw\wx_vc%VCver%.slnx" && goto :msbuildWX)
  rem Until wxWidgets catches up with VS2026 we can use the VS2022 solution
  if exist "%wxSourceDir%\build\msw\wx_vc17.sln" (set "wxSLN=%wxSourceDir%\build\msw\wx_vc17.sln" && goto :msbuildWX)
  @echo INFO: Could not find "%wxSourceDir%\build\msw\wx_vc%VCver%.sln?"
  goto :buildErr
:msbuildWX
  if exist "%OCPN_DIR%\build\.Debug" (
    msbuild "%wxSLN%" ^
      -noLogo -verbosity:%wxVerb% -maxCpuCount ^
      -property:PlatformToolset=v%VCtool% ^
      -property:AdditionalOptions="/EHa" ^
      -property:UseMultiToolTask=true ^
      -property:EnableClServerMode=true ^
      -property:BuildPassReferences=true ^
      -property:"Configuration=DLL Debug";Platform=Win32 ^
      -property:wxVendor=14x;wxVersionString=32;wxToolkitDllNameSuffix=_vc_14x ^
      -logger:FileLogger,Microsoft.Build.Engine;logfile="%wxSourceDir%\MSBuild_DEBUG_WIN32.log"
    if errorlevel 1 (
      echo wxWidgets Debug build [101;93mNOT OK[0m
      goto :buildErr
    )
    @echo wxWidgets Debug build OK
  )
  msbuild "%wxSLN%" ^
    -noLogo -verbosity:%wxVerb% -maxCpuCount ^
    -property:PlatformToolset=v%VCtool% ^
    -property:AdditionalOptions="/EHa" ^
    -property:Optimize=true ^
    -property:LinkIncremental=false ^
    -property:UseMultiToolTask=true ^
    -property:EnableClServerMode=true ^
    -property:BuildPassReferences=true ^
    -property:"Configuration=DLL Release";Platform=Win32 ^
    -property:wxVendor=14x;wxVersionString=32;wxToolkitDllNameSuffix=_vc_14x ^
    -logger:FileLogger,Microsoft.Build.Engine;logfile="%wxSourceDir%\MSBuild_RELEASE_WIN32.log"
  if errorlevel 1 (
    echo wxWidgets Release build [101;93mNOT OK[0m
    goto :buildErr
  )
  echo wxWidgets Release build OK
)
robocopy "%wxBuildDir%\lib\vc_dll" "%CACHE_DIR%\buildwin\wxWidgets" wxmsw32*.dll /S /E /NFL /NDL /NJH /NJS /NP >NUL
if %errorlevel% GEQ 8 (
  @echo Error: robocopy "%wxBuildDir%\lib\vc_dll" "%CACHE_DIR%\buildwin\wxWidgets" wxmsw32*.dll /S /E /NFL /NDL /NJH /NJS /NP
  @echo wxWidgets is broken and [101;93mNOT OK[0m
  if not [%quiet%]==[Y] pause
  goto :buildErr
)
robocopy "%wxBuildDir%\lib\vc_dll" "%CACHE_DIR%\buildwin\wxWidgets" wxbase32*.dll /S /E /NFL /NDL /NJH /NJS /NP >NUL
if %errorlevel% GEQ 8 (
  @echo Error: robocopy "%wxBuildDir%\lib\vc_dll" "%CACHE_DIR%\buildwin\wxWidgets" wxbase32*.dll /S /E /NFL /NDL /NJH /NJS /NP
  @echo wxWidgets is broken and [101;93mNOT OK[0m
  if not [%quiet%]==[Y] pause
  goto :buildErr
)
robocopy "%wxBuildDir%\lib\vc_dll" "%CACHE_DIR%\buildwin\wxWidgets" wxmsw32*.pdb /S /E /NFL /NDL /NJH /NJS /NP >NUL
if %errorlevel% GEQ 8 (
  @echo Error: robocopy "%wxBuildDir%\lib\vc_dll" "%CACHE_DIR%\buildwin\wxWidgets" wxmsw32*.pdb /S /E /NFL /NDL /NJH /NJS /NP
  @echo wxWidgets is broken and [101;93mNOT OK[0m
  if not [%quiet%]==[Y] pause
  goto :buildErr
)
robocopy "%wxBuildDir%\lib\vc_dll" "%CACHE_DIR%\buildwin\wxWidgets" wxbase32*.pdb /S /E /NFL /NDL /NJH /NJS /NP >NUL
if %errorlevel% GEQ 8 (
  @echo Error: robocopy "%wxBuildDir%\lib\vc_dll" "%CACHE_DIR%\buildwin\wxWidgets" wxbase32*.pdb /S /E /NFL /NDL /NJH /NJS /NP
  @echo wxWidgets is broken and [101;93mNOT OK[0m
  if not [%quiet%]==[Y] pause
  goto :buildErr
)
robocopy /S /E "%wxBuildDir%\locale" "%CACHE_DIR%\buildwin\wxWidgets\locale" *.* /S /E /NFL /NDL /NJH /NJS /NP >NUL
if %errorlevel% GEQ 8 (
  @echo Error: robocopy /S /E "%wxBuildDir%\build\locale" "%CACHE_DIR%\buildwin\wxWidgets\locale" /S /E /NFL /NDL /NJH /NJS /NP
  @echo wxWidgets is broken and [101;93mNOT OK[0m
  if not [%quiet%]==[Y] pause
  goto :buildErr
)
@echo wxWidgets builds seem ok
::-------------------------------------------------------------
:: Finalize local environment helper script
::-------------------------------------------------------------
@echo Finishing %OCPN_DIR%\buildwin\configdev.bat
if "[%nsispath%]"=="[]" (goto :addGettext)
set "_addpath=%nsispath%\NSIS\;%nsispath%\NSIS\bin\"
:addGettext
if "[%gettextpath%]"=="[]" (goto :addPath)
set "_addpath=%_addpath%;%gettextpath%\tools\bin\"
:addPath
if "[%_addpath%]"=="[]" (goto :skipAddPath)
@echo path^|find /i "%_addpath%"    ^>nul ^|^| set "path=%path%;%_addpath%" >> "%OCPN_DIR%\buildwin\configdev.bat"
:skipAddPath
@echo goto :EOF>> "%OCPN_DIR%\buildwin\configdev.bat"
@echo type "%OCPN_DIR%\buildwin\configdev.bat"
type "%OCPN_DIR%\buildwin\configdev.bat"
::-------------------------------------------------------------
:: Setup environment
::-------------------------------------------------------------
endlocal
if exist "%~dp0..\buildwin\configdev.bat" (call "%~dp0..\buildwin\configdev.bat") else (echo Cannot find configdev.bat&& goto :fail)
::-------------------------------------------------------------
:: Build Release and Debug executables
::-------------------------------------------------------------
if exist "%~dp0..\build\.RelWithDebInfo" (
  @echo Building RelWithDebInfo
  set build_type=RelWithDebInfo
  call :ocpnConfig
  if errorlevel 1 (
    @echo :ocpnConfig->RelWithDebInfo failed
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
    @echo :ocpnBuild->RelWithDebInfo failed
    goto :fail
  )
  call :restore
)

if exist "%~dp0..\build\.Release" (
  @echo Building Release
  set build_type=Release
  call :ocpnConfig
  if errorlevel 1 (
    @echo :ocpnConfig->Release failed
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
    @echo :ocpnBuild->Release failed
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
    rmdir "%~dp0..\build\.Debug\.Pack"
    call :ocpnPack
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
::-------------------------------------------------------------
:: Offer some helpful hints
::-------------------------------------------------------------
:hint
@echo.
set build_type=
set buildTarget=
set wxVerb=
set EXE_NAME=devenv.exe
tasklist /FI "IMAGENAME eq %EXE_NAME%" | find /I "%EXE_NAME%" >nul
if %ERRORLEVEL%==0 (
    @echo %EXE_NAME% is running
    @echo.
    goto :success
) else (
  if exist "%~dp0..\build\OpenCPN.sln" (devenv "%~dp0..\build\OpenCPN.sln" && goto :success)
  if exist "%~dp0..\build\OpenCPN.slnx" (devenv "%~dp0..\build\OpenCPN.slnx" && goto :success)
)
@echo To debug OpenCPN in Visual Studio, do this:
@echo.
if exist "%~dp0..\build\OpenCPN.slnx" (
  @echo %CD%^> devenv .\build\opencpn.slnx
) else (
  @echo %CD%^> devenv .\build\opencpn.sln
)
@echo.
@echo or you can start Visual Studio like this:
@echo.
@echo %CD%^> devenv
@echo.
@echo Now you are ready to start debugging
@echo.
rem @echo [101;93mIf you close this CMD prompt and open another be sure to run:[0m
rem @echo  %CD%\buildwin\configdev.bat
rem @echo [101;93mfirst, before starting Visual Studio[0m.
goto :success
::-------------------------------------------------------------
:: Local subroutines
::-------------------------------------------------------------
:removeTarget
::-------------------------------------------------------------
:: Removes a folder or file recursively defined by %target%
:: If locked (usually by VS/OpenCPN), in interactive mode it will
:: repeatedly pause and let the user close VS before retrying.
:: In --Y (non-interactive) mode it tries once and continues with a warning.
::-------------------------------------------------------------
@echo Removing %target%
if not exist "%target%" goto :EOF

:: Initial attempt
if exist "%target%\" (
    rd /s /q "%target%" >nul 2>&1
) else (
    del /f /q "%target%" >nul 2>&1
)
if not exist "%target%" ( goto :EOF )

:: If we get here the item is locked
if [%quiet%]==[Y] (
    @echo "*** WARNING: Skipped locked item (non-interactive mode): "%target%" ***"
    goto :EOF
)
:: Interactive mode - keep asking user to close VS until it succeeds or they abort with Ctrl+C
@echo Entering interactive mode

set Retrying=0
:interactiveRetry
echo.
echo ╔═══════════════════════════════════════════════════════════════════╗
echo ║                                                                   ║
echo ║   Cannot delete: "%target%"                              ║
echo ║                                                                   ║
echo ║   VISUAL STUDIO (or a running OpenCPN instance) is locking it.    ║
echo ║                                                                   ║
echo ║   →→→ CLOSE VISUAL STUDIO AND MAKE SURE OPENCPN is shut down ←←←  ║
echo ║                                                                   ║
echo ║   The clean/rebuild CANNOT continue properly until this is gone.  ║
echo ║   Leaving garbage behind may corrupt your next build.             ║
echo ║                                                                   ║
echo ╚═══════════════════════════════════════════════════════════════════╝
echo.
echo     Press any key to retry deletion (after you have closed VS)...
echo     Or press Ctrl+C to abort the entire script.
echo.
pause >nul

if exist "%target%\" (
    rd /s /q "%target%" >nul 2>&1
) else (
    del /f /q "%target%" >nul 2>&1
)

set /a Retrying+=1
if %Retrying% gtr 4 (
  @echo ╔════════════════════════════════════════╗
  @echo ║  Warning: Could not delete %target%.   ║
  @echo ║  Build process may be unstable.        ║
  @echo ╚════════════════════════════════════════╝
  goto :EOF
)
if exist "%target%" goto :interactiveRetry
echo     Success - %target% deleted.
goto :EOF

::-------------------------------------------------------------
:: Config and build
::-------------------------------------------------------------
:ocpnConfig
cmake -A Win32 -T v%VCtool% -G "%VCstr%" -S "%~dp0.." -B "%~dp0..\build" ^
  -DCMAKE_GENERATOR_PLATFORM=Win32 ^
  -DCMAKE_BUILD_TYPE=%build_type% ^
  -DwxWidgets_LIB_DIR="%wxWidgets_LIB_DIR%" ^
  -DwxWidgets_ROOT_DIR="%wxWidgets_ROOT_DIR%" ^
  -DCMAKE_CXX_FLAGS="/MP /EHa /DWIN32" ^
  -DCMAKE_C_FLAGS="/MP" ^
  -DCMAKE_EXE_LINKER_FLAGS="/MAP" ^
  -DOCPN_CI_BUILD:BOOL=OFF ^
  -DOCPN_TARGET_TUPLE=msvc-wx32;10;x86_64 ^
  -DOCPN_BUNDLE_WXDLLS:BOOL=ON ^
  -DOCPN_BUILD_TEST:BOOL=OFF ^
  -DOCPN_BUNDLE_GSHHS:BOOL=ON ^
  -DOCPN_BUNDLE_TCDATA:BOOL=ON ^
  -DOCPN_BUNDLE_DOCS:BOOL=ON ^
  -DOCPN_ENABLE_SYSTEM_CMD_SOUND:BOOL=OFF ^
  -DOCPN_ENABLE_PORTAUDIO:BOOL=OFF ^
  -DCMAKE_INSTALL_PREFIX="%~dp0..\build\%build_type%" || (
  cmake -A Win32 -T v%VCtool% -G "%VCstr%" -S "%~dp0.." -B "%~dp0..\build" --debug-find ^
    -DCMAKE_GENERATOR_PLATFORM=Win32 ^
    -DCMAKE_BUILD_TYPE=%build_type% ^
    -DwxWidgets_LIB_DIR="%wxWidgets_LIB_DIR%" ^
    -DwxWidgets_ROOT_DIR="%wxWidgets_ROOT_DIR%" ^
    -DCMAKE_CXX_FLAGS="/MP /EHa /DWIN32" ^
    -DCMAKE_C_FLAGS="/MP" ^
    -DCMAKE_EXE_LINKER_FLAGS="/MAP" ^
    -DOCPN_CI_BUILD:BOOL=OFF ^
    -DOCPN_TARGET_TUPLE=msvc-wx32;10;x86_64 ^
    -DOCPN_BUNDLE_WXDLLS:BOOL=ON ^
    -DOCPN_BUILD_TEST:BOOL=OFF ^
    -DOCPN_BUNDLE_GSHHS:BOOL=ON ^
    -DOCPN_BUNDLE_TCDATA:BOOL=ON ^
    -DOCPN_BUNDLE_DOCS:BOOL=ON ^
    -DOCPN_ENABLE_SYSTEM_CMD_SOUND:BOOL=OFF ^
    -DOCPN_ENABLE_PORTAUDIO:BOOL=OFF ^
    -DCMAKE_INSTALL_PREFIX="%~dp0..\build\%build_type%" || goto :cmakeErr
)
goto :EOF

:ocpnBuild
@echo build_type=%build_type%
if exist "%~dp0..\build\ALL_BUILD.vcxproj" (set oBLD="%~dp0..\build\ALL_BUILD.vcxproj" && goto :slnGo)
@echo [101;93mNO PROJECT FOUND[0m
goto :buildErr
:slnGo
msbuild ^
  -property:Configuration=%build_type%;Platform=Win32;GenerateMapFile=true ^
  -target:%buildTarget% ^
  -noLogo ^
  -verbosity:minimal ^
  -maxCpuCount ^
  -property:UseMultiToolTask=true ^
  -property:EnableClServerMode=true ^
  -property:BuildPassReferences=true ^
  -property:CL="/arch:SSE" ^
  /l:FileLogger,Microsoft.Build.Engine;logfile=%CD%\MSBuild_%build_type%_WIN32_Debug.log ^
  "%oBLD%"
if errorlevel 1 (@echo msbuild %oBLD% problem & goto :buildErr)
@echo Install local/portable environment
msbuild ^
  -property:Configuration=%build_type%;Platform=Win32;GenerateMapFile=true ^
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
if errorlevel 1 (@echo msbuild "%~dp0..\build\INSTALL.vcxproj" problem & goto :buildErr)
::
:: Delete the .mo files because they are only valid for this build Configuration.
:: They will not be rebuilt for a different configuration unless we delete them.
del /F "%~dp0..\build\*.mo"
set buildTarget=
@echo OpenCPN %build_type% build successful!
@echo.
goto :EOF

:ocpnPack
@echo build_type=%build_type%
if exist "%~dp0..\build\OpenCPN.vcxproj" (set oBLD="%~dp0..\build\ALL_BUILD.vcxproj" && goto :pckGo)
@echo [101;93mNO ALL_BUILD PROJECT FOUND[0m
goto :buildErr
:pckGo
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
  "%oBLD%" || goto :buildErr
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
  "%~dp0..\build\PACKAGE.vcxproj" || goto :buildErr
:: Move the build package file to config build folder
move "%~dp0..\build\*.exe" "%~dp0..\build\%build_type%"
::
:: Delete the .mo files because they are only valid for this build Configuration.
:: They will not be rebuilt for a different configuration unless we delete them.
del /F "%~dp0..\build\*.mo"
set buildTarget=
@echo OpenCPN %build_type% build successful!
@echo.
goto :EOF

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
if not exist "%~dp0..\build\%folder%\" goto :bexit
@echo Backing up %~dp0..\build\%folder%
if not exist "%~dp0..\tmp" (mkdir "%~dp0..\tmp")
if not exist "%~dp0..\tmp\%folder%" (mkdir "%~dp0..\tmp\%folder%")
@echo backing up %folder%
if exist "%~dp0..\build\.%folder%\" mkdir "%~dp0..\tmp\.%folder%"
if exist "%~dp0..\build\.%folder%\.Cmake\" mkdir "%~dp0..\tmp\.%folder%\.Cmake"
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
REM Don't archive the chart database as it can (should?) be recreated after full rebuild
REM xcopy /Q /Y "%~dp0..\build\%folder%\*" "%~dp0..\tmp\%folder%"
REM if errorlevel 1 (
  REM @echo xcopy /Q /Y "%~dp0..\build\%folder%\*.dat" "%~dp0..\tmp\%folder%" [101;93mNOT OK[0m
  REM if not [%quiet%]==[Y] pause
REM ) else (
  REM echo xcopy /Q /Y "%~dp0..\build\%folder%\*.dat" "%~dp0..\tmp\%folder%" OK
REM )
xcopy /Q /Y "%~dp0..\build\%folder%\*.csv" "%~dp0..\tmp\%folder%"
if errorlevel 1 (
  @echo xcopy /Q /Y "%~dp0..\build\%folder%\*.csv" "%~dp0..\tmp\%folder%" [101;93mNOT OK[0m
  if not [%quiet%]==[Y] pause
) else (
  echo xcopy /Q /Y "%~dp0..\build\%folder%\*.csv" "%~dp0..\tmp\%folder%" OK
)
@echo xcopy /Q /Y "%~dp0..\build\%folder%\navobj.*" "%~dp0..\tmp\%folder%" [101;93mNOT OK[0m
xcopy /Q /Y "%~dp0..\build\%folder%\navobj.*" "%~dp0..\tmp\%folder%"
if errorlevel 1 (
  @echo [101;93mNOT OK[0m
  if not [%quiet%]==[Y] pause
) else (
  echo OK
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
goto :EOF
::-------------------------------------------------------------
:: Restore user configuration to build folder
::-------------------------------------------------------------
:restore
:: Called from withing build folder
if not exist "%~dp0..\tmp\%build_type%" (
  @echo INFO: Did not find "%~dp0..\tmp\%build_type%"
  goto :EOF
)
@echo Restoring %build_type% settings from "%~dp0..\tmp\%build_type%"
cmake -E copy_directory "%~dp0..\tmp\%build_type%" "%~dp0..\build\%build_type%"
if errorlevel 1 (
  @echo Restore %build_type% failed
  if not [%quiet%]==[Y] pause
  goto :EOF
) else (
  @echo Restore successful
  rmdir /s /q "%~dp0..\tmp\%build_type%"
)
:rreturn
@echo restore returning
goto :EOF
::-------------------------------------------------------------
:: Download URL to a DEST folder
::-------------------------------------------------------------
:download
@echo URL=%URL%
@echo DEST=%DEST%
if exist ""%DEST%"" (
  echo Download %DEST% already exists.
  exit /b 1
)
"%PSH%" -Command [System.Net.ServicePointManager]::MaxServicePointIdleTime = 5000000; ^
  if ($PSVersionTable.PSVersion.Major -lt 6) { $ProgressPreference = 'SilentlyContinue' }; ^
  Invoke-WebRequest '%URL%' -OutFile '%DEST%'; ^
  exit $LASTEXITCODE
if errorlevel 1 (echo Download failed && exit /b 1) else (echo Download OK)
goto :EOF
::-------------------------------------------------------------
:: Explode SOURCE zip file to DEST folder
::-------------------------------------------------------------
:explode
@echo SOURCE=%SOURCE%
@echo DEST=%DEST%
:: @echo "%PSH% -Command if ($PSVersionTable.PSVersion.Major -lt 6) { $ProgressPreference = 'SilentlyContinue' }; Expand-Archive -Force -Path '%SOURCE%' -DestinationPath '%DEST%';
%PSH% -Command ^
  if ($PSVersionTable.PSVersion.Major -lt 6) { $ProgressPreference = 'SilentlyContinue' }; ^
  Expand-Archive -Force -Path '%SOURCE%' -DestinationPath '%DEST%'; ^
  exit $LASTEXITCODE
if errorlevel 1 (echo Explode failed && exit /b 1) else (echo Unzip OK)
goto :EOF
::-------------------------------------------------------------
:: Execution time measuring functions
::-------------------------------------------------------------
:StartTimer
:: Store start time
set winConfigStartTime=%TIME%
for /f "usebackq tokens=1-4 delims=:., " %%f in (`echo %winConfigStartTime: =0%`) do set /a Start100S=1%%f*360000+1%%g*6000+1%%h*100+1%%i-36610100
goto :EOF

:StopTimer
:: Get the end time
set winConfigStopTIME=%TIME%
for /f "usebackq tokens=1-4 delims=:., " %%f in (`echo %winConfigStopTIME: =0%`) do set /a Stop100S=1%%f*360000+1%%g*6000+1%%h*100+1%%i-36610100
:: Test midnight rollover. If so, add 1 day=8640000 1/100ths secs
if %Stop100S% LSS %Start100S% set /a Stop100S+=8640000
set /a winConfigTookTime=%Stop100S%-%Start100S%
set winConfigTookTimePadded=0%winConfigTookTime%
goto :EOF

:DisplayTimerResult
:: Show timer start/stop/delta
if not "[%winConfigStartTime%]"=="[]" echo Started: %winConfigStartTime%
if not "[%winConfigStopTime%]"=="[]" echo Stopped: %winConfigStopTime%
if not "[%winConfigTookTime%]"=="[]" echo Elapsed: %winConfigTookTime:~0,-2%.%winConfigTookTimePadded:~-2% seconds
set winConfigStartTime=
set winConfigStopTime=
set winConfigTookTime=
set winConfigTookTimePadded=
goto :EOF

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
goto :EOF
:: ***********
:: * THE END *
:: ***********
