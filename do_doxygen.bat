@echo off
setlocal
set WORKSPACE=d:\box\
set BUILD_LOG=build.log
set SOURCES_LIST=sources.txt
set BIOS_PLATFORM=MyPlatform
set BIOS_REVISION=00_D01
set DOXYGEN_CFG=%WORKSPACE%doxygen.cfg
set DOXYGEN_OUTPUT=d:\doxygen
call CreateSourceList.bat %WORKSPACE% %BUILD_LOG% %SOURCES_LIST%
call %WORKSPACE%dox.bat %BIOS_PLATFORM% %BIOS_REVISION% %WORKSPACE% %DOXYGEN_CFG% %DOXYGEN_OUTPUT% %WORKSPACE%%SOURCES_LIST%
endlocal
