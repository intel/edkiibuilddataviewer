@echo off
setlocal enabledelayedexpansion

REM *** CHANGE ME ***
set WORKSPACE=c:\source.dev\ServerGen2\Intel\

set BUILD_LOG=BIOSBuild.log
set SOURCES_LIST=BIOSSources.txt
set SOURCES_LIST_TMP=BIOSSources.tmp
set SOURCES_LIST_FULL=BIOSSources_full.txt
set BUILD_FSP_LOG=BIOSBuild_FSP.log
set SOURCES_FSP_LIST=BIOSSources_FSP.txt
set BIOS_PLATFORM=ServerGen2_EGS
set BIOS_REVISION=00_D01
set DOXYGEN_CFG=%cd%\doxygen.cfg
set DOXYGEN_OUTPUT=c:\doxygen

:checkRequirements
if not exist %WORKSPACE% goto error_workspace
if not exist %WORKSPACE%%BUILD_LOG% goto error_buildlog

set TEMP_PATH="%Path:;=";"%"
:check_doxygen_installed
FOR %%a IN (%TEMP_PATH%) DO if exist %%~a\doxygen.exe goto check_graphviz_installed
goto error_doxygen_install

:check_graphviz_installed
FOR %%a IN (%TEMP_PATH%) DO if exist %%~a\dot.exe if exist %%~a\gvgen.exe (
  set DOT_PATH=%%~a
  goto check_other
)
goto error_graphviz_install

:check_other

echo.
set CHOICE_SECONDS=5
set CHOICE_YN_DEFAULT=Y
echo Default choice of %CHOICE_YN_DEFAULT% in %CHOICE_SECONDS% seconds ...
choice /t %CHOICE_SECONDS% /d %CHOICE_YN_DEFAULT% /m "Generating doxygen data for a BIOS tree can take 1-2 hours.  Are you sure?"
if %ERRORLEVEL% == 2 goto done

echo Generating source files used in build from build logs, please wait...
echo **********************************************************************************
echo                      DO NOT TOUCH THE KEYBOARD OR MOUSE
echo.
echo                App button clicking automation is about commence...
echo **********************************************************************************
timeout /t 3
call CreateSourceList.bat %WORKSPACE% %BUILD_FSP_LOG% %SOURCES_FSP_LIST%
call CreateSourceList.bat %WORKSPACE% %BUILD_LOG% %SOURCES_LIST%

echo Filtering output ...
set LINE_COUNT=0
for /F "delims=" %%i IN (%WORKSPACE%%SOURCES_LIST%) DO (
  set newStr=%%i
  if "!LINE_COUNT!" == "0" (
    set newStr=!newStr:~8!
  )
  echo !newStr! >> %WORKSPACE%%SOURCES_LIST_TMP%
  set /a LINE_COUNT=LINE_COUNT+1
)

echo Creating full source file list ...
copy %WORKSPACE%%SOURCES_FSP_LIST% + %WORKSPACE%%SOURCES_LIST_TMP% %WORKSPACE%%SOURCES_LIST_FULL% 1>nul
del %WORKSPACE%%SOURCES_LIST_TMP%

call %cd%\dox.bat %BIOS_PLATFORM% %BIOS_REVISION% %WORKSPACE% %DOXYGEN_CFG% %DOXYGEN_OUTPUT% %WORKSPACE%%SOURCES_LIST_FULL%
del %WORKSPACE%%SOURCES_LIST_FULL%
goto done

:error_workspace
echo ERROR: WORKSPACE env var doesn't exist, did you change it in this file?
goto done

:error_buildlog
echo ERROR: %WORKSPACE%%BUILD_LOG% build log not found, did you build BIOS?
goto done

:error_doxygen_output
echo ERROR: doxygen output folder %DOXYGEN_OUTPUT% doesn't exist 
goto done

:error_doxygen_install
echo ERROR: doxygen.exe not found in PATH, is doxygen installed?
goto done

:error_graphviz_install
echo ERROR: dot.exe not found in PATH, is graphviz installed?
goto done

:done
endlocal
