@echo off

set PATH=%PATH%;C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x64;C:\usr\lib\opensg;C:\usr\vcpkg\installed\x64-windows\lib;C:\usr\lib\cef;
set PYTHONPATH=%PYTHONPATH%;C:\usr\vcpkg\installed\x64-windows\share\python2\Lib
rem TODO: fix gschemas path!
rem  set XDG_DATA_DIR=%XDG_DATA_DIR%;ressources\gui\schemas
build\Release\polyvr.exe %*