::
:: Install build dependencies. Requires a working choco installation,
:: see https://docs.chocolatey.org/en-us/choco/setup.
::
:: Initial run will do choco installs requiring administrative
:: privileges.
::

:: Install the pathman tool: https://github.com/therootcompany/pathman
:: Fix PATH so it can be used in this script
::
if not exist "%HomeDrive%%HomePath%\.local\bin\pathman.exe" (
    pushd "%HomeDrive%%HomePath%"
    curl.exe -sA "MS" https://webinstall.dev/pathman | powershell
    popd
)
pathman list > nul 2>&1
if errorlevel 1 set PATH=%PATH%;%HomeDrive%\%HomePath%\.local\bin
pathman add %HomeDrive%%HomePath%\.local\bin >nul

:: Install choco cmake and add it's persistent user path element
::
set CMAKE_HOME=C:\Program Files\CMake
if not exist "%CMAKE_HOME%\bin\cmake.exe" choco install --no-progress -y cmake
pathman add "%CMAKE_HOME%\bin" > nul

:: Install choco poedit and add it's persistent user path element
::
set POEDIT_HOME=C:\Program Files (x86)\Poedit\Gettexttools
if not exist "%POEDIT_HOME%" choco install --no-progress -y poedit
pathman add "%POEDIT_HOME%\bin" > nul

:: Update required python stuff
::
python --version > nul 2>&1 && python -m ensurepip > nul 2>&1
if errorlevel 1 choco install --no-progress -y python
python --version
python -m ensurepip
python -m pip install --upgrade pip
python -m pip install -q setuptools wheel
python -m pip install -q cloudsmith-cli
python -m pip install -q cryptography

:: Install pre-compiled wxWidgets and other DLL; add required paths.
::
set SCRIPTDIR=%~dp0
set WXWIN=%SCRIPTDIR%..\cache\wxWidgets-3.1.2
set wxWidgets_ROOT_DIR=%WXWIN%
set wxWidgets_LIB_DIR=%WXWIN%\lib\vc_dll
if not exist "%WXWIN%" (
  wget --version > nul 2>&1 || choco install --no-progress -y wget
  wget -q https://download.opencpn.org/s/E2p4nLDzeqx4SdX/download ^
      -O wxWidgets-3.1.2.7z
  7z i > nul 2>&1 || choco install -y 7zip
  7z x wxWidgets-3.1.2.7z -o%WXWIN%
)
pathman add "%WXWIN%" > nul
pathman add "%wxWidgets_LIB_DIR%" > nul

refreshenv
