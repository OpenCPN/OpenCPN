@echo off
setlocal enableextensions
goto :main
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
:usage
@echo ****************************************************************************
@echo *  This script can be used to create a local build environment for OpenCPN.*
@echo *  Here are some basic steps that are known to work.  There are            *
@echo *  probably other ways to do this, but this is the way that works for me.  *
@echo *                                                                          *
@echo *  1. Install Visual Studio 2022 Community Edition.                        *
@echo *              https://visualstudio.microsoft.com/downloads/               *
@echo *  2. Install git for Windows                                              *
@echo *              https://git-scm.com/download/win                            *
@echo *  3. Open 'x86 Native Tools Command Prompt for Visual Studio 2022'        *
@echo *  4. Create folder where you want to work with OpenCPN sources            *
@echo *        Example: mkdir \Users\myname\source\repos                         *
@echo *                 cd \Users\myname\source\repos                            *
@echo *  5. Clone Opencpn                                                        *
@echo *        Example: clone https://github.com/opencpn/opencpn                 *
@echo *                 cd \Users\myname\source\repos\opencpn                    *
@echo *                 git checkout master                                      *
@echo *  6. Set up local build environment by executing this script              *
@echo *        Example: .\buildwin\winConfig.bat                                 *
@echo *  7. Open solution file                                                   *
@echo *       (type the solution file name at VS command prompt)                 *
@echo *        Example: .\build\opencpn.sln                                      *
@echo *  8. Start building and debugging in Visual Studio.                       *
@echo *                                                                          *
@echo *  Command line options:                                                   *
@echo *      --clean            Remove build folder entirely before building     *
@echo *                         MUST HAVE INTERNET CONNECTION FOR clean OPTION!  *
@echo *      --rebuild          Rebuild all sources                              *
@echo *                                                                          *
@echo *      --release          Build Release configuration                      *
@echo *      --relwithdebinfo   Build RelWithDebInfo configuration               *
@echo *      --minsizerel       Build MinSizeRel configuration                   *
@echo *      --debug            Build Debug configuration                        *
@echo *                                                                          *
@echo *      --all              Build all 4 configurations  (default)            *
@echo *                                                                          *
@echo *      --help             Print thie message                               *
@echo *                                                                          *
@echo ****************************************************************************
exit /b 1
:main
::-------------------------------------------------------------
:: Initialize local environment
::-------------------------------------------------------------
set "OD=%~dp0.."
@echo OD=%OD%
cd %OD%
set "OCPN_Dir=%CD%"
set "wxDIR=%OCPN_DIR%\cache\buildwxWidgets"
set "wxWidgets_ROOT_DIR=%wxDIR%"
set "wxWidgets_LIB_DIR=%wxDIR%\lib\vc_dll"
if [%VisualStudioVersion%]==[16.0] (^
  set VCver=16
  set "VCstr=Visual Studio 16"
)
if [%VisualStudioVersion%]==[17.0] (^
  set VCver=17
  set "VCstr=Visual Studio 17"
)
::-------------------------------------------------------------
:: Initialize local helper script that can reinitialize environment
::-------------------------------------------------------------
@echo set "wxDIR=%wxDIR%" > "%OCPN_Dir%\buildwin\configdev.bat"
@echo set "wxWidgets_ROOT_DIR=%wxWidgets_ROOT_DIR%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "wxWidgets_LIB_DIR=%wxWidgets_LIB_DIR%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "VCver=%VCver%" >> "%OCPN_DIR%\buildwin\configdev.bat"
@echo set "VCstr=%VCstr%" >> "%OCPN_DIR%\buildwin\configdev.bat"
::-------------------------------------------------------------
:: Initialize local variables
::-------------------------------------------------------------
SET "CACHE_DIR=%OCPN_DIR%\cache"
SET "buildWINtmp=%CACHE_DIR%\buildwintemp"
set PSH=powershell
where pwsh > NUL 2> NUL && set PSH=pwsh
where %PSH% > NUL 2> NUL || @echo PowerShell is not installed && exit /b 1
where msbuild && goto :vsok
@echo Please run this from "x86 Native Tools Command Prompt for VS2022
goto :usage
:vsok
set ocpn_clean=0
set ocpn_rebuild=0
:: By default build all 4 possible configurations
:: Edit and set to 0 any configurations you do not wish to build
set ocpn_all=1
set ocpn_minsizerel=0
set ocpn_release=0
set ocpn_relwithdebinfo=0
set ocpn_debug=0

:parse
if [%1]==[--clean] (shift /1 && set ocpn_clean=1&& set ocpn_rebuild=0&& goto :parse)
if [%1]==[--rebuild] (shift /1 && set ocpn_rebuild=1&& set ocpn_clean=0&& goto :parse)
if [%1]==[--help] (shift /1 && goto :usage)
if [%1]==[--all] (shift /1 && set ocpn_all=1&& goto :parse)
if [%1]==[--minsizerel] (shift /1 && set ocpn_all=0&& set ocpn_minsizerel=1)
if [%1]==[--release] (shift /1 && set ocpn_all=0&& set ocpn_release=1)
if [%1]==[--relwithdebinfo] (shift /1 && set ocpn_all=0&& set ocpn_relwithdebinfo=1)
if [%1]==[--debug] (shift /1 && set ocpn_all=0&& set ocpn_debug=1)
if [%1]==[] (goto :begin) else (^
  @echo Unknown option: %1
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
if not exist "%OCPN_DIR%\build" (^
  set ocpn_clean=1
)
::-------------------------------------------------------------
:: Save user configuration data and clean build folders but
:: do not delete downloaded tools.  Use this option if no internet
:: connectivity available.
::-------------------------------------------------------------
if [%ocpn_rebuild%]==[1] (
  @echo Beginning rebuild cleanout
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
  if exist "%OCPN_DIR%\build\Release" rmdir /s /q "%OCPN_DIR%\build\Release"
  if exist "%OCPN_DIR%\build\Release" @echo Could not remove "%OCPN_DIR%\build\Release" folder
  if exist "%OCPN_DIR%\build\RelWithDebInfo" rmdir /s /q "%OCPN_DIR%\build\RelWithDebInfo"
  if exist "%OCPN_DIR%\build\RelWithDebInfo" @echo Could not remove "%OCPN_DIR%\build\RelWithDebInfo" folder
  if exist "%OCPN_DIR%\build\Debug" rmdir /s /q "%OCPN_DIR%\build\Debug"
  if exist "%OCPN_DIR%\build\Debug" @echo Could not remove "%OCPN_DIR%\build\Debug" folder
  if exist "%OCPN_DIR%\build\MinSizeRel" rmdir /s /q "%OCPN_DIR%\build\MinSizeRel"
  if exist "%OCPN_DIR%\build\MinSizeRel" @echo Could not remove "%OCPN_DIR%\build\MinSizeRel" folder
  if exist "%OCPN_DIR%\build\CMakeFiles" rmdir /s /q "%OCPN_DIR%\build\CMakeFiles"
  if exist "%OCPN_DIR%\build\CMakeFiles" @echo Could not remove "%OCPN_DIR%\build\CMakeFiles" folder
  if exist "%OCPN_DIR%\build\.vs" rmdir /s /q "%OCPN_DIR%\build\.vs"
  if exist "%OCPN_DIR%\build\.vs" (
	@echo Could not remove "%OCPN_DIR%\build\.vs" folder
	@echo Is Visual Studio IDE open? If so, please close it so we can try again.
	pause
	@echo Retrying...
	rmdir /s /q "%OCPN_DIR%\build\.vs"
    if exist "%OCPN_DIR%\build\.vs" (
	  @echo Could not remove "%OCPN_DIR%\.vs". Continuing...
	) else (
	  @echo Ok, removed "%OCPN_DIR%\build\.vs"
	)
  )
  @echo Finished rebuild cleanout
)
::-------------------------------------------------------------
:: Save user configuration data and wipe the build folder
::-------------------------------------------------------------
if [%ocpn_clean%]==[1] (
  @echo [101;93mThe --clean option requires an internet connection.[0m
  choice /C YN /T 10 /M "Remove entire build folder including downloaded tools? [yN]" /D N
  if ERRORLEVEL==2  goto :usage
:clean
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
  if exist "%OCPN_DIR%\build" rmdir /s /q "%OCPN_DIR%\build"
  if exist "%OCPN_DIR%\build" (
    @echo Could not remove "%OCPN_DIR%\build" folder
    @echo Is Visual Studio IDE open? If so, please close it so we can try again.
    pause
    @echo Retrying...
    rmdir /s /q "%OCPN_DIR%\build"
  )
  if exist "%OCPN_DIR%\build" (
    @echo Could not remove "%OCPN_DIR%\build". Continuing...
  ) else (
    @echo Cleared %OCPN_DIR%\build OK.
  )
  if exist "%CACHE_DIR%" (rmdir /s /q "%CACHE_DIR%" && @echo Cleared %CACHE_DIR%)
  if exist "%buildWINtmp%" (rmdir /s /q "%buildWINtmp%" && @echo Cleared %buildWINtmp%)
  timeout /T 5
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
@echo Downloading nuget
set "URL=https://dist.nuget.org/win-x86-commandline/latest/nuget.exe"
set "DEST=%buildWINtmp%\nuget.exe"
call :download
::-------------------------------------------------------------
:: Download OpenCPN Core dependencies
::-------------------------------------------------------------
@echo Downloading Windows depencencies from OpenCPN repository
set "URL=https://github.com/OpenCPN/OCPNWindowsCoreBuildSupport/archive/refs/tags/v0.3.zip"
set "DEST=%buildWINtmp%\OCPNWindowsCoreBuildSupport.zip"
call :download

@echo Exploding Windows dependencies
set "SOURCE=%DEST%"
set "DEST=%buildWINtmp%"
call :explode
if errorlevel 1 (@echo [101;93mNOT OK[0m) else (
  xcopy /e /q /y "%buildWINtmp%\OCPNWindowsCoreBuildSupport-0.3\buildwin" "%CACHE_DIR%\buildwin"
  if errorlevel 1 (@echo [101;93mNOT OK[0m) else (echo OK))
::-------------------------------------------------------------
:: Download wxWidgets 3.2.2 sources
::-------------------------------------------------------------
if exist "%wxDIR%\build\msw\wx_vc%VCver%.sln" (goto :skipwxDL)
@echo Downloading wxWidgets sources
mkdir "%wxDIR%"
set "URL=https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.2.1/wxWidgets-3.2.2.1.zip"
set "DEST=%wxDIR%\wxWidgets-3.2.2.1.zip"
call :download
if errorlevel 1 (@echo [101;93mNOT OK[0m) else (echo OK)

@echo exploding wxWidgets
set "SOURCE=%wxDIR%\wxWidgets-3.2.2.1.zip"
set "DEST=%wxDIR%"
call :explode
if errorlevel 1 (@echo [101;93mNOT OK[0m) else (echo OK)

@echo Downloading Windows WebView2 kit
set "URL=https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2"
set "DEST=%wxDIR%\webview2.zip"
call :download
if errorlevel 1 (@echo [101;93mNOT OK[0m) else (echo OK)

@echo Exploding WebView2
set "SOURCE=%wxDIR%\webview2.zip"
set "DEST=%wxDIR%\build\3rdparty\webview2"
call :explode
if errorlevel 1 (@echo [101;93mNOT OK[0m) else (echo OK)

:skipwxDL
::-------------------------------------------------------------
:: Build wxWidgets from sources
::-------------------------------------------------------------
@echo Building wxWidgets

msbuild /noLogo /v:m /m "-p:Configuration=DLL Debug";Platform=Win32 ^
  -p:wxVendor=14x;wxVersionString=32;wxToolkitDllNameSuffix="_vc14x" ^
  /l:FileLogger,Microsoft.Build.Engine;logfile=%CACHE_DIR%\buildwin\wxWidgets\MSBuild_DEBUG_WIN32_Debug.log ^
  "%wxDIR%\build\msw\wx_vc%VCver%.sln"

msbuild /noLogo /v:m /m "-p:Configuration=DLL Release";Platform=Win32 ^
  -p:wxVendor=14x;wxVersionString=32;wxToolkitDllNameSuffix="_vc14x" ^
  /l:FileLogger,Microsoft.Build.Engine;logfile=%CACHE_DIR%\buildwin\wxWidgets\MSBuild_RELEASE_WIN32_Debug.log ^
  "%wxDIR%\build\msw\wx_vc%VCver%.sln"

if not exist "%CACHE_DIR%\buildwin\wxWidgets" (
    mkdir "%CACHE_DIR%\buildwin\wxWidgets"
)
xcopy /e /q /y "%WXDIR%\lib\vc_dll\" "%CACHE_DIR%\buildwin\wxWidgets"
if not exist "%CACHE_DIR%\buildwin\wxWidgets\locale" (
  mkdir "%CACHE_DIR%\buildwin\wxWidgets\locale"
)
xcopy /e /q /y "%WXDIR%\locale\" "%CACHE_DIR%\buildwin\wxWidgets\locale"
::-------------------------------------------------------------
:: Initialize folders needed to run OpenCPN
::-------------------------------------------------------------
@echo ocpn_relwithdebinfo=%ocpn_relwithdebinfo%
@echo ocpn_release=%ocpn_release%
@echo ocpn_debug=%ocpn_debug%
@echo ocpn_minsizerel=%ocpn_minsizerel%

if [%ocpn_debug%]==[1] (^
  if not exist "%OCPN_DIR%\build\Debug" (mkdir "%OCPN_DIR%\build\Debug")
  if not exist "%OCPN_DIR%\build\Debug\plugins" (mkdir "%OCPN_DIR%\build\Debug\plugins")
  )
if [%ocpn_release%]==[1] (^
  if not exist "%OCPN_DIR%\build\Release" (mkdir "%OCPN_DIR%\build\Release")
  if not exist "%OCPN_DIR%\build\Release\plugins" (mkdir "%OCPN_DIR%\build\Release\plugins")
  )
if [%ocpn_relwithdebinfo%]==[1] (^
  if not exist "%OCPN_DIR%\build\RelWithDebInfo" (mkdir "%OCPN_DIR%\build\RelWithDebInfo")
  if not exist "%OCPN_DIR%\build\RelWithDebInfo\plugins" (mkdir "%OCPN_DIR%\build\RelWithDebInfo\plugins")
  )
if [%ocpn_minsizerel%]==[1] (^
  if not exist "%OCPN_DIR%\build\MinSizeRel" (mkdir "%OCPN_DIR%\build\MinSizeRel")
  if not exist "%OCPN_DIR%\build\MinSizeRel\plugins" (mkdir "%OCPN_DIR%\build\MinSizeRel\plugins")
  )
::-------------------------------------------------------------
:: Download and initialize build dependencies
::-------------------------------------------------------------
cd %OCPN_DIR%\build
%buildWINtmp%\nuget install Gettext.Tools
%buildWINtmp%\nuget install NSIS-Package
for /D %%D in ("Gettext*") do (set gettext=%%~D)
for /D %%D in ("NSIS-Package*") do (set nsis=%%~D)
@echo gettext=%gettext%
@echo nsis=%nsis%
cd %OCPN_DIR%
::-------------------------------------------------------------
:: Finalize local environment helper script
::-------------------------------------------------------------
@echo Finishing %OCPN_DIR%\buildwin\configdev.bat
set "_addpath=%OCPN_DIR%\build\%nsis%\NSIS\;%OCPN_DIR%\build\%nsis%\NSIS\bin\"
set "_addpath=%_addpath%;%OCPN_DIR%\build\%gettext%\tools\bin\"
@echo path^|find /i "%_addpath%"    ^>nul ^|^| set "path=%path%;%_addpath%" >> "%OCPN_DIR%\buildwin\configdev.bat"
set _addpath=
endlocal
::-------------------------------------------------------------
:: Setup environment
::-------------------------------------------------------------
cd /D "%~dp0.."
@echo In folder %CD%
if exist .\buildwin\configdev.bat (call .\buildwin\configdev.bat) else (goto :hint)
:: @echo path=%path%
::-------------------------------------------------------------
:: Build Release and Debug executables
::-------------------------------------------------------------
pushd build
@echo In folder %CD%
if exist .\Debug (^
  @echo Building Debug
  set build_type=Debug
  call :config_build
  if errorlevel 1 goto :buildErr
  call :restore
  )
if exist .\RelWithDebInfo (^
  @echo Building RelWithDebInfo
  set build_type=RelWithDebInfo
  call :config_build
  if errorlevel 1 goto :buildErr
  call :restore
  )
if exist .\Release (^
  @echo Building Release
  set build_type=Release
  call :config_build
  if errorlevel 1 goto :buildErr
  call :restore
  )
if exist .\MinSizeRel (^
  @echo Building MinSizeRel
  set build_type=MinSizeRel
  call :config_build
  if errorlevel 1 goto :buildErr
  call :restore
  )
popd
set build_type=
goto :hint
::-------------------------------------------------------------
:: Offer some helpful hints
::-------------------------------------------------------------
:hint
@echo To build OpenCPN for debugging at command line do this in the folder
@echo %CD%
@echo.
@echo  .\buildwin\configdev.bat
@echo  cd build
@echo  msbuild /noLogo /m -p:Configuration=Debug;Platform=Win32 opencpn.sln
@echo  .\opencpn.sln
@echo.
@echo Now you are ready to start debugging
@echo.
@echo [101;93mIf you close this CMD prompt and open another be sure to run:[0m
@echo  %CD%\buildwin\configdev.bat
@echo [101;93mfirst, before starting Visual Studio[0m.
goto :EOF
::-------------------------------------------------------------
:: Local subroutines
::-------------------------------------------------------------
::-------------------------------------------------------------
:: Config and build
::-------------------------------------------------------------
:config_build
cmake -A Win32 -G "%VCstr%" ^
  -DCMAKE_GENERATOR_PLATFORM=Win32 ^
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
  -DCMAKE_INSTALL_PREFIX="%CD%\%build_type%" ^
  ..
if errorlevel 1 goto :cmakeErr
msbuild /noLogo /v:m /m -p:Configuration=%build_type%;Platform=Win32 ^
/l:FileLogger,Microsoft.Build.Engine;logfile=%CD%\MSBuild_%build_type%_WIN32_Debug.log ^
INSTALL.vcxproj
if errorlevel 1 goto :buildErr
@echo OpenCPN %build_type% build successful!
@echo.
exit /b 0
::-------------------------------------------------------------
:: CMake failed
::-------------------------------------------------------------
:cmakeErr
@echo CMake failed to configure OpenCPN build folder.
@echo Review the error messages and read the OpenCPN
@echo Developer Manual for help.
pause
exit /b 1
::-------------------------------------------------------------
:: Build failed
::-------------------------------------------------------------
:buildErr
@echo Build using msbuild failed.
@echo Review the error messages and read the OpenCPN
@echo Developer Manual for help.
pause
exit /b 1
::-------------------------------------------------------------
:: Backup user configuration
::-------------------------------------------------------------
:backup
@echo "Backing up %~dp0..\build\%folder%"
if not exist "%~dp0..\build\%folder%" goto :breturn
if not exist "%~dp0..\tmp" (mkdir "%~dp0..\tmp")
if not exist "%~dp0..\tmp\%folder%" (mkdir "%~dp0..\tmp\%folder%")
@echo backing up %folder%
xcopy /Q /Y "%~dp0..\build\%folder%\opencpn.ini" "%~dp0..\tmp\%folder%"
xcopy /Q /Y "%~dp0..\build\%folder%\*.log.log" "%~dp0..\tmp\%folder%"
xcopy /Q /Y "%~dp0..\build\%folder%\*.log" "%~dp0..\tmp\%folder%"
xcopy /Q /Y "%~dp0..\build\%folder%\*.dat" "%~dp0..\tmp\%folder%"
xcopy /Q /Y "%~dp0..\build\%folder%\*.csv" "%~dp0..\tmp\%folder%"
xcopy /Q /Y "%~dp0..\build\%folder%\navobj.*" "%~dp0..\tmp\%folder%"
xcopy /Q /Y "%~dp0..\build\%folder%\navobj.xml.?" "%~dp0..\tmp\%folder%"
@echo cmake -E copy_directory "%~dp0..\build\%folder%\plugins" "%~dp0..\tmp\%folder%"
cmake -E copy_directory "%~dp0..\build\%folder%\plugins" "%~dp0..\tmp\%folder%"
if not exist "%~dp0..\build\%folder%\Charts" goto :breturn
@echo cmake -E copy_directory "%~dp0..\build\%folder%\Charts" "%~dp0..\tmp\%folder%"
cmake -E copy_directory "%~dp0..\build\%folder%\Charts" "%~dp0..\tmp\%folder%"
:breturn
@echo backup returning
exit /b 0
::-------------------------------------------------------------
:: Restore user configuration to build folder
::-------------------------------------------------------------
:restore
:: Called from withing build folder
if not exist "%~dp0..\tmp\%build_type%" (
  @echo INFO: Did not find "%~dp0..\tmp\%build_type%"
  exit /b 1
)
@echo Restoring %build_type% settings from "%~dp0..\tmp\%build_type%"
cmake -E copy_directory "%~dp0..\tmp\%build_type%" "%~dp0..\build\%build_type%"
if errorlevel 1 (
  @echo Restore %build_type% failed
  pause
  goto ::rreturn
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
  @echo Download %DEST% already exists.
  exit /b 0
)
%PSH% -Command [System.Net.ServicePointManager]::MaxServicePointIdleTime = 5000000; ^
  if ($PSVersionTable.PSVersion.Major -lt 6) { $ProgressPreference = 'SilentlyContinue' }; ^
  Invoke-WebRequest "%URL%" -OutFile '%DEST%'; ^
  exit $LASTEXITCODE
if errorlevel 1 (@echo Download failed && pause && exit /b 1) else (@echo Download OK)
exit /b 0
::-------------------------------------------------------------
:: Explode SOURCE zip file to DEST folder
::-------------------------------------------------------------
:explode
@echo SOURCE=%SOURCE%
@echo DEST=%DEST%
%PSH% -Command if ($PSVersionTable.PSVersion.Major -lt 6) { $ProgressPreference = 'SilentlyContinue' }; ^
  Expand-Archive -Force -Path '%SOURCE%' -DestinationPath '%DEST%'
if errorlevel 1 (@echo Explode failed && pause && exit /b 1) else (@echo Unzip OK)
exit /b 0
