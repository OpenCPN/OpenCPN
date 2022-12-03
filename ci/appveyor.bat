:: Build and upload OpenCPN artifacts
@echo on

setlocal enabledelayedexpansion

if "%CONFIGURATION%" == "" (set CONFIGURATION=Release)
if "%APPVEYOR_BUILD_FOLDER%" == "" (set "APPVEYOR_BUILD_FOLDER=%~dp0..")

where dumpbin.exe >nul 2>&1
if errorlevel 1 (
  set "VS_BASE=C:\Program Files\Microsoft Visual Studio\2022"
  call "%VS_BASE%\Community\VC\Auxiliary\Build\vcvars32.bat"
)

call %APPVEYOR_BUILD_FOLDER%\buildwin\win_deps.bat
call %APPVEYOR_BUILD_FOLDER%\cache\wx-config.bat

where wxmsw32u_qa_vc14x.dll >nul 2>&1
if errorlevel 1 (
  set "PATH=%PATH%;%wxWidgets_LIB_DIR%"
  echo Appending %wxWidgets_LIB_DIR% to PATH
)

cd %APPVEYOR_BUILD_FOLDER%

if exist build (rmdir /q /s build)
mkdir build && cd build

cmake -T v143 ^
    -DCMAKE_GENERATOR_PLATFORM=Win32 ^
    -A Win32 -G "Visual Studio 17 2022" ^
    -DwxWidgets_ROOT_DIR=%wxWidgets_ROOT_DIR% ^
    -DwxWidgets_LIB_DIR=%wxWidgets_LIB_DIR% ^
    -DwxWidgets_CONFIGURATION=mswu ^
    -DCMAKE_BUILD_TYPE=%CONFIGURATION% ^
    -DCMAKE_LIBRARY_PATH=%~dp0..\cache\buildwin ^
    -DCMAKE_INCLUDE_PATH=%~dp0..\cache\buildwin\include ^
    -DOCPN_CI_BUILD=ON ^
    -DOCPN_BUNDLE_WXDLLS=ON ^
    -DOCPN_BUILD_TEST=OFF ^
    ..

cmake --build . --target package --config %CONFIGURATION%
