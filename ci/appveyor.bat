:: Build script for appveyor, https://www.appveyor.com
:: Builds one version linked against wxWidgets 3.2

@echo off
setlocal enabledelayedexpansion

set "SCRIPTDIR=%~dp0"

:: %CONFIGURATION% comes from environment, set a default if invoked elsewise.
if "%CONFIGURATION%" == "" set "CONFIGURATION=Release"

:: On GA, set up N:, see below
if exist D:\a\OpenCPN\OpenCPN (subst N: D:\a\OpenCPN\OpenCPN)

:: If N: exists it is used as base dir to sanitize source file paths
:: in pdb.
if exist N:\ (
  n:
  cd \
  echo "Using virtual drive N: as base path"
)

call %SCRIPTDIR%..\buildwin\win_deps.bat wx32
call %SCRIPTDIR%..\cache\wx-config.bat
echo USING wxWidgets_LIB_DIR: !wxWidgets_LIB_DIR!
echo USING wxWidgets_ROOT_DIR: !wxWidgets_ROOT_DIR!

if not defined VCINSTALLDIR (
  for /f "tokens=* USEBACKQ" %%p in (
    `"%programfiles(x86)%\Microsoft Visual Studio\Installer\vswhere" ^
    -latest -property installationPath`
  ) do call "%%p\Common7\Tools\vsDevCmd.bat"
)

where wxmsw32u_qa_vc14x.dll >nul 2>&1
if errorlevel 1 (
  set "PATH=%PATH%;%wxWidgets_LIB_DIR%"
  echo Appending %wxWidgets_LIB_DIR% to PATH
)


if exist build (rmdir /s /q build)
mkdir build && cd build

cmake -A Win32 -G "Visual Studio 17 2022" ^
    -DCMAKE_GENERATOR_PLATFORM=Win32 ^
    -DCMAKE_BUILD_TYPE=%CONFIGURATION% ^
    -DwxWidgets_LIB_DIR=!wxWidgets_LIB_DIR! ^
    -DwxWidgets_ROOT_DIR=!wxWidgets_ROOT_DIR! ^
    -DwxWidgets_CONFIGURATION=mswu ^
    -DOCPN_TARGET_TUPLE=msvc-wx32;10;x86_64 ^
    -DOCPN_CI_BUILD=ON ^
    -DOCPN_BUNDLE_WXDLLS=ON ^
    -DOCPN_BUILD_TEST=OFF ^
    ..

cmake --build . --target package --config %CONFIGURATION%

:: Compress pdb and mark with git hash
@echo on
"C:\Program Files\Git\bin\bash" -c ^
  "tar czf opencpn+%GITHUB_SHA:~0,8%.pdb.tar.gz %CONFIGURATION%/opencpn.pdb"
@echo off

:: Display dependencies debug info
echo import glob; import subprocess > ldd.py
echo lib = glob.glob("%CONFIGURATION%/opencpn.exe")[0] >> ldd.py
echo subprocess.call(['dumpbin', '/dependents', lib], shell=True) >> ldd.py
python ldd.py
