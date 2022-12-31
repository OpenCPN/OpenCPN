:: circle ci windows build script.
::
:: There is quoting problems in .circleci/config.yml making
:: it hard to use paths when calling scripts. This script works around
:: this.
::
@echo off

:: PATH C:\Program Files\Cmake\bin;%PATH%
cd %~dp0..
call ci\appveyor.bat
