:: Download OpenCPN prebuilt dependencies
::
@echo on
setlocal enabledelayedexpansion

:: Install Poedit if required
msgmerge --version >nul 2>&1
if errorlevel 1 (
  choco install poedit -y --no-progress
  set "PATH=%PATH%;C:\Program Files (x86)\Poedit\Gettexttools\bin"
)

:: Install git if required.
git --version >nul 2>&1
if errorlevel 1 (
  choco install git -y --no-progress
  set "PATH=%PATH%;C:\Program Files\Git\bin"
)

if not exist C:\ProgramData\chocolatey\lib\nsis (
  echo Installing nsis tools using choco
  choco install nsis -y --no-progress
)

:: install wget as required
wget --version >nul 2>&1 || choco install wget -y --no-progress

git clone https://github.com/microsoft/vcpkg D:\code\vcpkg
set "VCPKG_DISABLE_METRICS=1"
call D:\code\vcpkg\bootstrap-vcpkg.bat -disableMetrics
echo After Bootstrap
call D:\code\vcpkg\vcpkg install openssl:x64-windows curl:x64-windows libarchive:x64-windows glew:x64-windows --debug
echo After vcpkg intall...

echo The current working directory is: %CD%
echo The batch file is located in: %~dp0

:: If needed, download wxWidgets binary build.
set "CACHE_DIR=%~dp0..\cache"
if not exist !CACHE_DIR! (mkdir !CACHE_DIR!)

echo The cache directory is: %CACHE_DIR%

set "GITHUB_DL=https://github.com/wxWidgets/wxWidgets/releases/download"
if not exist cache\wxWidgets-3.2.9 (
::  wget -nv %GITHUB_DL%/v3.2.1/wxMSW-3.2.1_vc14x_Dev.7z
::  7z x -y -o%CACHE_DIR%\wxWidgets-3.2.1 wxMSW-3.2.1_vc14x_Dev.7z
::  wget -nv %GITHUB_DL%/v3.2.1/wxWidgets-3.2.1-headers.7z
::  7z x -y -o%CACHE_DIR%\wxWidgets-3.2.1 wxWidgets-3.2.1-headers.7z
::  wget -nv %GITHUB_DL%/v3.2.1/wxMSW-3.2.1_vc14x_ReleaseDLL.7z
::  7z x -y -o%CACHE_DIR%\wxWidgets-3.2.1 wxMSW-3.2.1_vc14x_ReleaseDLL.7z
  wget -nv %GITHUB_DL%/v3.2.9/wxMSW-3.2.9_vc14x_x64_Dev.7z
  7z x -y -o%CACHE_DIR%\wxWidgets-3.2.9 wxMSW-3.2.9_vc14x_x64_Dev.7z
  wget -nv %GITHUB_DL%/v3.2.9/wxWidgets-3.2.9-headers.7z
  7z x -y -o%CACHE_DIR%\wxWidgets-3.2.9 wxWidgets-3.2.9-headers.7z
  wget -nv %GITHUB_DL%/v3.2.9/wxMSW-3.2.9_vc14x_x64_ReleaseDLL.7z
  7z x -y -o%CACHE_DIR%\wxWidgets-3.2.9 wxMSW-3.2.9_vc14x_x64_ReleaseDLL.7z
)


:: Create cache\wx-config.bat, paths to downloaded wxWidgets.
set "WXWIN=!CACHE_DIR!\wxWidgets-3.2.9"
echo set "wxWidgets_ROOT_DIR=%WXWIN%" > %CACHE_DIR%\wx-config.bat
echo set "wxWidgets_LIB_DIR=%WXWIN%\lib\vc14x_dll" >> %CACHE_DIR%\wx-config.bat

@echo on
type %CACHE_DIR%\wx-config.bat

:: Make sure the pre-compiled libraries are in place

::set "GH_DL_BASE=https://github.com/OpenCPN/OCPNWindowsCoreBuildSupport"
::set "opencpn_support_base=https://dl.cloudsmith.io/public/alec-leamas"
::if not exist %CACHE_DIR%\buildwin\libcurl.dll (
::  wget -nv -O !CACHE_DIR!\OCPNWindowsCoreBuildSupport.zip ^
::      %GH_DL_BASE%/archive/refs/tags/v0.5.zip
::  7z x -y !CACHE_DIR!\OCPNWindowsCoreBuildSupport.zip ^
::      -o%CACHE_DIR%\buildwintemp
::  if not exist !CACHE_DIR!\buildwin (mkdir !CACHE_DIR!\buildwin)
::  xcopy ^
::    !CACHE_DIR!\buildwintemp\OCPNWindowsCoreBuildSupport-0.5\buildwin ^
::    !CACHE_DIR!\buildwin /s /y /q
::  if exist !CACHE_DIR!\buildwin\wxWidgets (
::    del !CACHE_DIR!\buildwin\wxWidgets\*.dll /q
::  )
::  wget !opencpn_support_base!/opencpn-support/raw/files/iphlpapi.lib ^
::   -O %CACHE_DIR%\buildwin\iphlpapi.lib
::)

::wget -nv -O !CACHE_DIR!\QuickStartGuide.zip ^
::       https://dl.cloudsmith.io/public/david-register/opencpn-docs/raw/files/QuickStartGuide-v0.4.zip
::if not exist %CACHE_DIR%\..\data\doc\local (mkdir %CACHE_DIR%\..\data\doc\local)
::7z x -y !CACHE_DIR!\QuickStartGuide.zip  -o%CACHE_DIR%\..\data\doc\local

echo Leaving win_deps_x86.bat
