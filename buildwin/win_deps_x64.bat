:: Download OpenCPN prebuilt dependencies
::
@echo on
setlocal enabledelayedexpansion

set "CACHE_DIR=%~dp0..\cache"
if not defined OCPN_VCPKG_ROOT set "OCPN_VCPKG_ROOT=%CACHE_DIR%\vcpkg"
if not exist "%CACHE_DIR%" mkdir "%CACHE_DIR%"

:: Install Poedit if required
msgmerge --version >nul 2>&1
if errorlevel 1 (
  choco install poedit -y --no-progress
  if errorlevel 1 exit /b 1
  set "PATH=%PATH%;C:\Program Files (x86)\Poedit\Gettexttools\bin"
)

:: Install git if required.
git --version >nul 2>&1
if errorlevel 1 (
  choco install git -y --no-progress
  if errorlevel 1 exit /b 1
)

makensis /VERSION >nul 2>&1
if errorlevel 1 (
  echo Installing nsis tools using choco
  choco install nsis -y --no-progress
  if errorlevel 1 exit /b 1
  set "PATH=%PATH%;C:\Program Files (x86)\NSIS;C:\Program Files\NSIS"
)
makensis /VERSION >nul 2>&1
if errorlevel 1 exit /b 1

:: curl.exe is supplied by supported Windows build hosts.
curl.exe --version >nul 2>&1
if errorlevel 1 exit /b 1

if not exist "%OCPN_VCPKG_ROOT%\bootstrap-vcpkg.bat" (
  git clone https://github.com/microsoft/vcpkg "%OCPN_VCPKG_ROOT%"
  if errorlevel 1 exit /b 1
)
set "VCPKG_DISABLE_METRICS=1"
call "%OCPN_VCPKG_ROOT%\bootstrap-vcpkg.bat" -disableMetrics
if errorlevel 1 exit /b 1
echo After Bootstrap
call "%OCPN_VCPKG_ROOT%\vcpkg.exe" install openssl:x64-windows curl:x64-windows libarchive:x64-windows glew:x64-windows liblzma:x64-windows --debug
if errorlevel 1 exit /b 1
echo After vcpkg intall...

echo The current working directory is: %CD%
echo The batch file is located in: %~dp0

:: If needed, download wxWidgets binary build.

echo The cache directory is: %CACHE_DIR%

set "GITHUB_DL=https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.9"
set "WXWIN=%CACHE_DIR%\wxWidgets-3.2.9"
set "WX_DEPS_MISSING="
for %%F in (include\wx\wx.h lib\vc14x_x64_dll\mswu\wx\setup.h lib\vc14x_x64_dll\wxmsw32u_core.lib lib\vc14x_x64_dll\wxmsw32u_core_vc14x_x64.dll) do (
  if not exist "%WXWIN%\%%F" set "WX_DEPS_MISSING=1"
)
if defined WX_DEPS_MISSING (
  for %%A in (wxMSW-3.2.9_vc14x_x64_Dev.7z wxWidgets-3.2.9-headers.7z wxMSW-3.2.9_vc14x_x64_ReleaseDLL.7z) do (
    curl.exe -fL --retry 3 --retry-delay 2 "%GITHUB_DL%/%%A" -o "%CACHE_DIR%\%%A"
    if errorlevel 1 exit /b 1
    7z x -y "-o%WXWIN%" "%CACHE_DIR%\%%A"
    if errorlevel 1 exit /b 1
  )
)
for %%F in (include\wx\wx.h lib\vc14x_x64_dll\mswu\wx\setup.h lib\vc14x_x64_dll\wxmsw32u_core.lib lib\vc14x_x64_dll\wxmsw32u_core_vc14x_x64.dll) do (
  if not exist "%WXWIN%\%%F" (
    echo Missing wxWidgets dependency: %WXWIN%\%%F
    exit /b 1
  )
)

:: Create cache\wx-config.bat, paths to downloaded wxWidgets.
echo set "wxWidgets_ROOT_DIR=%WXWIN%" > "%CACHE_DIR%\wx-config.bat"
echo set "wxWidgets_LIB_DIR=%WXWIN%\lib\vc14x_x64_dll" >> "%CACHE_DIR%\wx-config.bat"

:: Verify
@echo on
type "%CACHE_DIR%\wx-config.bat"

:: Make sure the pre-compiled vcpkg libraries are in place

set "vcpkg=%OCPN_VCPKG_ROOT%\installed\x64-windows"
set "dest=%CACHE_DIR%\buildwin"
if not exist "%dest%" mkdir "%dest%"
if not exist "%dest%\include" mkdir "%dest%\include"

echo vcpkg=[%vcpkg%]
echo dest=[%dest%]
dir "%vcpkg%\lib\archive.lib"
dir "%dest%"

:: libarchive
copy "%vcpkg%\lib\archive.lib" "%dest%\"
copy "%vcpkg%\bin\archive.dll" "%dest%\"
copy "%vcpkg%\include\archive.h" "%dest%\include\"
copy "%vcpkg%\include\archive_entry.h" "%dest%\include\"
copy "%vcpkg%\include\lzma.h" "%dest%\include\"

:: libarchive's own runtime dependencies (it was built with lzma/bz2/zlib/xml2/lz4/zstd support)

copy "%vcpkg%\bin\bz2.dll" "%dest%\"
copy "%vcpkg%\bin\libxml2.dll" "%dest%\"
copy "%vcpkg%\bin\lz4.dll" "%dest%\"
copy "%vcpkg%\bin\zstd.dll" "%dest%\"

::LZMA
copy "%vcpkg%\lib\lzma.lib" "%dest%\"
copy "%vcpkg%\bin\liblzma.dll" "%dest%\"
copy "%vcpkg%\include\lzma.h" "%dest%\include\"
if not exist "%dest%\include\lzma" mkdir "%dest%\include\lzma"
xcopy "%vcpkg%\include\lzma" "%dest%\include\lzma"  /I /s /y /q

: zlib real names, per the Curl.cmake edit above
copy "%vcpkg%\lib\z.lib" "%dest%\"
copy "%vcpkg%\bin\z.dll" "%dest%\"
copy "%vcpkg%\include\zlib.h" "%dest%\include"
copy "%vcpkg%\include\zconf.h" "%dest%\include"

:: curl
copy "%vcpkg%\lib\libcurl.lib" "%dest%\"
copy "%vcpkg%\bin\libcurl.dll" "%dest%\"

:: curl headers (model/CMakeLists.txt sets CURL_INCLUDE_DIRS to cache/buildwin/include)
mkdir "%dest%\include\curl"
xcopy "%vcpkg%\include\curl" "%dest%\include\curl"  /I /s /y /q

: openssl
copy "%vcpkg%\lib\libssl.lib" "%dest%\"
copy "%vcpkg%\lib\libcrypto.lib" "%dest%\"
copy "%vcpkg%\bin\libssl-3-x64.dll" "%dest%\"
copy "%vcpkg%\bin\libcrypto-3-x64.dll" "%dest%\"
mkdir "%dest%\include\openssl"
xcopy "%vcpkg%\include\openssl" "%dest%\include\openssl"  /I /s /y /q

:: GLEW
mkdir "%dest%\include\glew"
copy "%vcpkg%\include\GL\glew.h" "%dest%\include\glew\"
copy "%vcpkg%\include\GL\wglew.h" "%dest%\include\glew\"
copy "%vcpkg%\include\GL\eglew.h" "%dest%\include\glew\"
copy "%vcpkg%\lib\glew32.lib" "%dest%\"
copy "%vcpkg%\bin\glew32.dll" "%dest%\"


:: Current Mozilla CA bundle for curl
curl.exe -fL https://curl.se/ca/cacert.pem -o "%dest%\curl-ca-bundle.crt"
if errorlevel 1 exit /b 1

echo Leaving win_deps_x64.bat
