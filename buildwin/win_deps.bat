:: Download OpenCPN prebuilt dependencies
::
@echo on
setlocal enabledelayedexpansion

:: Install Poedit if required
msgmerge --version >nul 2>&1
if errorlevel 1 (
  choco install -y poedit
  set "PATH=%PATH%;C:\Program Files (x86)\Poedit\Gettexttools\bin"
)

:: Install git if required.
git --version >nul 2>&1
if errorlevel 1 (
  choco install -y git
  set "PATH=%PATH%;C:\Program Files\Git\bin"
)

:: install wget as required
wget --version >nul 2>&1 || choco install -y wget

:: If needed, download wxWidgets binary build.
set "CACHE_DIR=%~dp0..\cache"
if not exist !CACHE_DIR! (mkdir !CACHE_DIR!)
set "GITHUB_DL=https://github.com/wxWidgets/wxWidgets/releases/download"
if not exist cache\wxWidgets-3.2.4 (
::  wget -nv %GITHUB_DL%/v3.2.1/wxMSW-3.2.1_vc14x_Dev.7z
::  7z x -y -o%CACHE_DIR%\wxWidgets-3.2.1 wxMSW-3.2.1_vc14x_Dev.7z
::  wget -nv %GITHUB_DL%/v3.2.1/wxWidgets-3.2.1-headers.7z
::  7z x -y -o%CACHE_DIR%\wxWidgets-3.2.1 wxWidgets-3.2.1-headers.7z
::  wget -nv %GITHUB_DL%/v3.2.1/wxMSW-3.2.1_vc14x_ReleaseDLL.7z
::  7z x -y -o%CACHE_DIR%\wxWidgets-3.2.1 wxMSW-3.2.1_vc14x_ReleaseDLL.7z
  wget -nv %GITHUB_DL%/v3.2.4/wxMSW-3.2.4_vc14x_Dev.7z
  7z x -y -o%CACHE_DIR%\wxWidgets-3.2.4 wxMSW-3.2.4_vc14x_Dev.7z
  wget -nv %GITHUB_DL%/v3.2.4/wxWidgets-3.2.4-headers.7z
  7z x -y -o%CACHE_DIR%\wxWidgets-3.2.4 wxWidgets-3.2.4-headers.7z
  wget -nv %GITHUB_DL%/v3.2.4/wxMSW-3.2.4_vc14x_ReleaseDLL.7z
  7z x -y -o%CACHE_DIR%\wxWidgets-3.2.4 wxMSW-3.2.4_vc14x_ReleaseDLL.7z
)
:: Create cache\wx-config.bat, paths to downloaded wxWidgets.
set "WXWIN=!CACHE_DIR!\wxWidgets-3.2.4"
echo set "wxWidgets_ROOT_DIR=%WXWIN%" > %CACHE_DIR%\wx-config.bat
echo set "wxWidgets_LIB_DIR=%WXWIN%\lib\vc14x_dll" >> %CACHE_DIR%\wx-config.bat


if not exist C:\ProgramData\chocolatey\lib\nsis (
  echo Installing nsis tools using choco
  choco install -y nsis
)

:: Make sure the pre-compiled libraries are in place
set "GH_DL_BASE=https://github.com/OpenCPN/OCPNWindowsCoreBuildSupport"
if not exist %CACHE_DIR%\buildwin\libcurl.dll (
  wget -nv -O !CACHE_DIR!\OCPNWindowsCoreBuildSupport.zip ^
      %GH_DL_BASE%/archive/refs/tags/v0.3.zip
  7z x -y !CACHE_DIR!\OCPNWindowsCoreBuildSupport.zip ^
      -o%CACHE_DIR%\buildwintemp
  if not exist !CACHE_DIR!\buildwin (mkdir !CACHE_DIR!\buildwin)
  xcopy ^
    !CACHE_DIR!\buildwintemp\OCPNWindowsCoreBuildSupport-0.3\buildwin ^
    !CACHE_DIR!\buildwin /s /y /q
  if exist !CACHE_DIR!\buildwin\wxWidgets (
    rmdir !CACHE_DIR!\buildwin\wxWidgets /s /q
  )
)
