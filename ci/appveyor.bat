:: Build script for appveyor, https://www.appveyor.com
:: Builds one version linked against wxWidgets 3.2

@echo off
setlocal enabledelayedexpansion

set "SCRIPTDIR=%~dp0"

:: %CONFIGURATION% comes from environment, set a default if invoked elsewise.
if "%CONFIGURATION%" == "" set "CONFIGURATION=Release"

call %SCRIPTDIR%..\buildwin\win_deps.bat wx32
call %SCRIPTDIR%..\cache\wx-config.bat
echo USING wxWidgets_LIB_DIR: !wxWidgets_LIB_DIR!
echo USING wxWidgets_ROOT_DIR: !wxWidgets_ROOT_DIR!

set "VS_HOME=C:\Program Files\Microsoft Visual Studio\2022"
where dumpbin.exe >nul 2>&1
if errorlevel 1 (
  call "%VS_HOME%\Community\VC\Auxiliary\Build\vcvars32.bat"
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

:: Display dependencies debug info
echo import glob; import subprocess > ldd.py
echo lib = glob.glob("%CONFIGURATION%/opencpn.exe")[0] >> ldd.py
echo subprocess.call(['dumpbin', '/dependents', lib], shell=True) >> ldd.py
python ldd.py
